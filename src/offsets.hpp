#pragma once
#include <cstdint>
#include <string>

namespace adrenaline {

// Every value here is an offset from the start of the object it belongs to.
// Roblox ships a new client every few weeks and shuffles these. When the ESP
// goes blank or draws garbage, re-dump with ReClass.NET / Cheat Engine and edit
// only this file. Nothing else in the project hard-codes an offset.
struct Offsets {
    // ---- Instance ----
    uintptr_t ChildrenStart          = 0x78;   // Instance** begin
    uintptr_t ChildrenEnd            = 0x80;   // Instance** end
    uintptr_t Name                   = 0x08;   // std::string
    uintptr_t ClassDescriptor        = 0x18;   // ClassDescriptor*
    uintptr_t ClassDescriptorToName  = 0x08;   // std::string inside ClassDescriptor

    // ---- DataModel ----
    uintptr_t FakeDataModelToDataModel = 0x1C0;

    // ---- Camera ----
    uintptr_t CameraRotation         = 0xF0;   // float[9], row-major 3x3
    uintptr_t CameraPosition         = 0x114;  // Vector3
    uintptr_t CameraFov              = 0x130;  // float

    // ---- Players ----
    uintptr_t PlayersToLocalPlayer   = 0x138;

    // ---- Player ----
    uintptr_t PlayerToCharacter      = 0x3A8;
    uintptr_t PlayerToTeam           = 0x2B8;
    uintptr_t PlayerToDisplayName    = 0x130;  // std::string

    // ---- Humanoid ----
    uintptr_t HumanoidToHealth       = 0x194;  // float
    uintptr_t HumanoidToMaxHealth    = 0x1A4;  // float

    // ---- BasePart ----
    uintptr_t BasePartToPrimitive    = 0x148;  // Primitive*
    uintptr_t PrimitiveToPosition    = 0x140;  // Vector3
    uintptr_t PrimitiveToSize        = 0x1A8;  // Vector3

    // ---- FakeDataModel signature ----
    // 48 8B 0D ?? ?? ?? ??  ->  mov rcx, [rip+rel]
    std::string fakeDataModelSignature = "48 8B 0D ? ? ? ? 48 8B 01 FF 50 ? 48 8B D8";
};

} // namespace adrenaline
