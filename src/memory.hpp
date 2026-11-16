#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace adrenaline {

// Read-only handle onto RobloxPlayerBeta.exe. No writes, no injection.
class Memory {
public:
    bool attach(const std::wstring& processName);
    void detach();

    bool isAttached() const { return process_ != nullptr; }
    uintptr_t base() const { return base_; }
    DWORD pid() const { return pid_; }

    template <typename T>
    T read(uintptr_t address) const {
        T value{};
        readRaw(address, &value, sizeof(T));
        return value;
    }

    bool readRaw(uintptr_t address, void* buffer, size_t size) const;

    // Reads a null-terminated C string, capped at maxLength bytes.
    std::string readString(uintptr_t address, size_t maxLength = 256) const;

    // Scans the cached .text section. Returns an absolute address.
    std::optional<uintptr_t> scan(const std::string& pattern) const;

private:
    bool cacheTextSection();

    HANDLE process_ = nullptr;
    DWORD pid_ = 0;
    uintptr_t base_ = 0;
    uintptr_t scanBase_ = 0;
    std::vector<uint8_t> image_;
};

} // namespace adrenaline
