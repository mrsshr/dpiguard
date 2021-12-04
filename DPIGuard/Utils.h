#pragma once

class Utils
{
public:
    static std::wstring GetApplicationPath();
    static std::wstring GetApplicationConfigPath();

    static std::string ReadTextFile(const wchar_t* filePath);
    static bool WriteTextFile(const std::string& buffer, const wchar_t* filePath);

    static bool CheckFileModified(const wchar_t* filePath, FILETIME& lastModifiedTime);

    static bool MatchString(const char* s, const char* pattern);

    static std::string FormatIPAddress(uint32_t addr);
    static std::string FormatIPAddress(const uint32_t* addr);

    static uint16_t ntohs(uint16_t value);
    static uint16_t htons(uint16_t value);
    static uint32_t ntohl(uint32_t value);
    static uint32_t htonl(uint32_t value);
    static uint64_t ntohll(uint64_t value);
    static uint64_t htonll(uint64_t value);
};
