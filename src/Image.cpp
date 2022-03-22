#include <stdafx.h>

#include "Image.h"

struct PdbInfo
{
    DWORD     Signature;
    BYTE      Guid[16];
    DWORD     Age;
    char      PdbFileName[1];
};

struct ImageVersion
{
    uint8_t Guid[16];
    uint64_t Version;
};

void Image::Initialize()
{
    const ImageVersion cVersions[] = {
        {{0x2B, 0x4E, 0x65, 0x3D, 0xD4, 0x68, 0xC7, 0x42, 0xBF, 0xC9, 0x58, 0xDC, 0x38, 0xD4, 0x2A, 0x36}, MakeVersion(1, 4)},
        {{0x93, 0x5B, 0x36, 0x35, 0xDF, 0xA8, 0xE7, 0x41, 0x91, 0x8A, 0x64, 0x64, 0xF7, 0xA4, 0xF0, 0x8E}, MakeVersion(1, 5)},
        {{0x67, 0xFB, 0x96, 0x6B, 0xAA, 0x3D, 0x57, 0x4E, 0x93, 0x8F, 0x1C, 0xC5, 0x85, 0xc6, 0xF5, 0x29}, MakeVersion(1, 6)},
        {{0xE0, 0xC2, 0x94, 0x64, 0x5C, 0xB4, 0x32, 0x45, 0x95, 0x10, 0x09, 0xA1, 0x0F, 0xB2, 0x53, 0xF8}, MakeVersion(1, 10)},
        {{0x8C, 0x13, 0x59, 0xA9, 0x7E, 0x6E, 0x49, 0x4F, 0x82, 0xF9, 0xCF, 0x58, 0x71, 0x6B, 0x7D, 0x3A}, MakeVersion(1, 11)},
        {{0x7B, 0x51, 0xF5, 0x2C, 0x87, 0xD0, 0xFF, 0x40, 0x83, 0xE5, 0xAA, 0x6C, 0x07, 0xE9, 0x95, 0x20}, MakeVersion(1, 12)},
        {{0x17, 0x1B, 0x2C, 0x4A, 0xC7, 0xA5, 0x38, 0x49, 0x8C, 0x43, 0x20, 0xFC, 0x47, 0x14, 0xF2, 0x33}, MakeVersion(1, 20)},
        {{0xEA, 0xD7, 0xE8, 0xCB, 0x7F, 0x9D, 0x6C, 0x4F, 0x9E, 0x3E, 0x99, 0x13, 0x92, 0x8F, 0x46, 0x62}, MakeVersion(1, 21)},
        {{0xD0, 0x43, 0x98, 0xA6, 0x91, 0xBD, 0xC3, 0x40, 0x85, 0x97, 0xC7, 0xC1, 0x1A, 0xC9, 0x50, 0x91}, MakeVersion(1, 22)},
        {{0x2E, 0x36, 0x66, 0x8E, 0x25, 0x0E, 0xE6, 0x43, 0xBD, 0xEF, 0x47, 0xA1, 0x3E, 0xF7, 0x7F, 0xA6}, MakeVersion(1, 23)},
        {{0xE7, 0x28, 0xD7, 0x5D, 0xB8, 0xF9, 0x79, 0x4D, 0x89, 0x76, 0x16, 0x64, 0xAB, 0x1E, 0xA0, 0x47}, MakeVersion(1, 30)},
        {{0x8E, 0x08, 0xA2, 0x5B, 0xC7, 0xDD, 0xD5, 0x48, 0xB4, 0xE9, 0x90, 0x38, 0x15, 0xB8, 0xBB, 0xEB}, MakeVersion(1, 31)},
        {{0x5B, 0x6C, 0xAD, 0x70, 0x67, 0x2b, 0x54, 0x40, 0xBC, 0x45, 0x06, 0x60, 0x44, 0xA9, 0x12, 0x61}, MakeVersion(1, 50)},
        {{0x73, 0x05, 0x22, 0x1B, 0x23, 0x65, 0x14, 0x4E, 0xB0, 0xE0, 0xAE, 0xC3, 0x4E, 0xD6, 0x4F, 0xBF}, MakeVersion(1, 5001)},
        {{0xDB, 0xE2, 0xBB, 0x94, 0x3B, 0x18, 0xB0, 0x4C, 0xAB, 0x56, 0xB6, 0xB8, 0x9A, 0x7E, 0xD2, 0xA5}, MakeVersion(1, 5002)},
        {{0xB0, 0x99, 0x3F, 0x8E, 0xB2, 0x57, 0x46, 0x41, 0xA3, 0x77, 0x3B, 0x98, 0x97, 0x15, 0xC4, 0xBB}, MakeVersion(1, 52)}
    };

    mem::module mainModule = mem::module::main();

    auto* pImage = GetModuleHandle(nullptr);

    auto sectionHeaders = mainModule.section_headers();

    base_address = reinterpret_cast<uintptr_t>(pImage);

    for (const auto& cHeader : sectionHeaders)
    {
        if (memcmp(cHeader.Name, ".text", 5) == 0)
        {
            TextRegion = mem::region(reinterpret_cast<uint8_t*>(base_address + cHeader.VirtualAddress),
                                  cHeader.Misc.VirtualSize);
            break;
        }
    }

    auto& dosHeader = mainModule.dos_header();
    auto* pFileHeader = reinterpret_cast<IMAGE_FILE_HEADER*>(base_address + dosHeader.e_lfanew + 4);
    auto* pOptionalHeader = reinterpret_cast<IMAGE_OPTIONAL_HEADER*>(((char*)pFileHeader) + sizeof(IMAGE_FILE_HEADER));
    auto* pDataDirectory = &pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    auto* pDebugDirectory = reinterpret_cast<IMAGE_DEBUG_DIRECTORY*>(base_address + pDataDirectory->VirtualAddress);

    // Check to see that the data has the right type
    if (IMAGE_DEBUG_TYPE_CODEVIEW == pDebugDirectory->Type)
    {
        PdbInfo* pdb_info = (PdbInfo*)(base_address + pDebugDirectory->AddressOfRawData);
        if (0 == memcmp(&pdb_info->Signature, "RSDS", 4))
        {
            for (const auto& v : cVersions)
            {
                if (memcmp(&pdb_info->Guid, v.Guid, 16) == 0)
                {
                    version = v.Version;
                    break;
                }
            }

            if (version == 0)
            {
                for (auto c : pdb_info->Guid)
                    Log::Error("{:X}", (uint32_t)c);
            }
        }
    }
}
