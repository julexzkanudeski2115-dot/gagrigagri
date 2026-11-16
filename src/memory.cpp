#include "memory.hpp"
#include "pattern.hpp"

#include <tlhelp32.h>
#include <algorithm>
#include <cstring>

namespace adrenaline {

bool Memory::attach(const std::wstring& processName) {
    detach();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    DWORD foundPid = 0;

    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, processName.c_str()) == 0) {
                foundPid = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (!foundPid) {
        return false;
    }

    process_ = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, foundPid);
    if (!process_) {
        return false;
    }
    pid_ = foundPid;

    HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, foundPid);
    if (modules == INVALID_HANDLE_VALUE) {
        detach();
        return false;
    }

    MODULEENTRY32W mod{};
    mod.dwSize = sizeof(mod);
    if (Module32FirstW(modules, &mod)) {
        base_ = reinterpret_cast<uintptr_t>(mod.modBaseAddr);
    }
    CloseHandle(modules);

    if (!base_) {
        detach();
        return false;
    }

    if (!cacheTextSection()) {
        detach();
        return false;
    }

    return true;
}

void Memory::detach() {
    if (process_) {
        CloseHandle(process_);
        process_ = nullptr;
    }
    pid_ = 0;
    base_ = 0;
    scanBase_ = 0;
    image_.clear();
}

bool Memory::readRaw(uintptr_t address, void* buffer, size_t size) const {
    if (!process_ || !address || !size) {
        return false;
    }
    SIZE_T bytesRead = 0;
    const BOOL ok = ReadProcessMemory(process_, reinterpret_cast<LPCVOID>(address), buffer, size, &bytesRead);
    return ok && bytesRead == size;
}

std::string Memory::readString(uintptr_t address, size_t maxLength) const {
    if (!address || !maxLength) {
        return {};
    }
    std::vector<char> buffer(maxLength + 1, '\0');
    if (!readRaw(address, buffer.data(), maxLength)) {
        return {};
    }
    buffer[maxLength] = '\0';
    return std::string(buffer.data());
}

bool Memory::cacheTextSection() {
    IMAGE_DOS_HEADER dos{};
    if (!readRaw(base_, &dos, sizeof(dos)) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    IMAGE_NT_HEADERS nt{};
    if (!readRaw(base_ + dos.e_lfanew, &nt, sizeof(nt)) || nt.Signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    const uintptr_t sectionTable = base_ + dos.e_lfanew + sizeof(IMAGE_NT_HEADERS);
    const size_t sectionCount = nt.FileHeader.NumberOfSections;

    uintptr_t textAddress = 0;
    size_t textSize = 0;

    for (size_t i = 0; i < sectionCount; ++i) {
        IMAGE_SECTION_HEADER header{};
        if (!readRaw(sectionTable + i * sizeof(IMAGE_SECTION_HEADER), &header, sizeof(header))) {
            break;
        }
        char name[9]{};
        std::memcpy(name, header.Name, 8);
        if (std::strcmp(name, ".text") == 0) {
            textAddress = base_ + header.VirtualAddress;
            textSize = header.Misc.VirtualSize;
            break;
        }
    }

    if (!textAddress || !textSize) {
        return false;
    }

    scanBase_ = textAddress;
    image_.assign(textSize, 0);

    size_t offset = 0;
    while (offset < textSize) {
        const size_t toRead = std::min<size_t>(0x100000, textSize - offset);
        if (!readRaw(textAddress + offset, image_.data() + offset, toRead)) {
            // Fall back to page granularity so a single bad page doesn't kill the scan.
            for (size_t page = 0; page < toRead; page += 0x1000) {
                const size_t chunk = std::min<size_t>(0x1000, toRead - page);
                if (!readRaw(textAddress + offset + page, image_.data() + offset + page, chunk)) {
                    std::fill(image_.begin() + offset + page, image_.begin() + offset + page + chunk, 0);
                }
            }
        }
        offset += toRead;
    }

    return true;
}

std::optional<uintptr_t> Memory::scan(const std::string& pattern) const {
    if (image_.empty()) {
        return std::nullopt;
    }
    const std::vector<int> parsed = parsePattern(pattern);
    const std::optional<size_t> index = findPattern(image_.data(), image_.size(), parsed);
    if (!index) {
        return std::nullopt;
    }
    return scanBase_ + *index;
}

} // namespace adrenaline
