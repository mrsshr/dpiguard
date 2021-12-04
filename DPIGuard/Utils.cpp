#include "StdAfx.h"
#include "Utils.h"

std::wstring Utils::GetApplicationPath()
{
    wchar_t buffer[MAX_PATH + 1];
    buffer[GetModuleFileNameW(nullptr, buffer, MAX_PATH)] = L'\0';

    return buffer;
}

std::wstring Utils::GetApplicationConfigPath()
{
    std::wstring result;

    std::wstring fullPath = GetApplicationPath();

    result.reserve(fullPath.size() + 16);

    size_t filenameOffset = fullPath.rfind(L'\\');

    std::wstring directory = fullPath.substr(0, filenameOffset);
    std::wstring fullFileName = fullPath.substr(filenameOffset + 1);

    size_t extensionOffset = fullFileName.rfind(L'.');

    if (extensionOffset == std::wstring::npos ||
        extensionOffset == 0)
    {
        result.append(directory);
        result.push_back(L'\\');
        result.append(fullFileName);
        result.append(L".config.yml");

        return result;
    }

    std::wstring fileName = fullFileName.substr(0, extensionOffset);

    result.append(directory);
    result.push_back(L'\\');
    result.append(fileName);
    result.append(L".config.yml");

    return result;
}

std::string Utils::ReadTextFile(const wchar_t* filePath)
{
    std::string content;
    uint8_t buffer[4096];

    HANDLE handle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
        return content;

    DWORD size = GetFileSize(handle, nullptr);
    if (size == INVALID_FILE_SIZE)
    {
        CloseHandle(handle);
        return content;
    }

    content.reserve(size);

    BOOL result = FALSE;
    DWORD read = 0;

    while (result = ReadFile(handle, buffer, sizeof(buffer), &read, nullptr))
    {
        if (read == 0)
            break;

        content.insert(content.cend(), buffer, buffer + read);
    }

    CloseHandle(handle);

    if (result == FALSE)
        content.clear();

    return content;
}

bool Utils::WriteTextFile(const std::string& buffer, const wchar_t* filePath)
{
    HANDLE handle = CreateFileW(filePath, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    DWORD written = 0;
    BOOL result = ::WriteFile(handle, buffer.c_str(), static_cast<DWORD>(buffer.size()), &written, nullptr);

    CloseHandle(handle);

    if (result == FALSE || buffer.size() != written)
        return false;

    return true;
}

bool Utils::CheckFileModified(const wchar_t* filePath, FILETIME& lastModifiedTime)
{
    HANDLE handle = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    FILETIME currentModifiedTime = { 0 };
    BOOL result = GetFileTime(handle, nullptr, nullptr, &currentModifiedTime);

    CloseHandle(handle);

    if (result == FALSE)
        return false;

    if ((currentModifiedTime.dwHighDateTime == lastModifiedTime.dwHighDateTime) &&
        (currentModifiedTime.dwLowDateTime == lastModifiedTime.dwLowDateTime))
        return false;

    lastModifiedTime = currentModifiedTime;
    return true;
}

bool Utils::MatchString(const char* s, const char* pattern)
{
    while (*s && *pattern)
    {
        if (*pattern == '*')
        {
            do
            {
                if (MatchString(s, pattern + 1))
                    return true;
            } while (*s++);

            return false;
        }

        if ((toupper(*s) != toupper(*pattern)) && *pattern != '?')
            return false;

        s++;
        pattern++;
    }

    if (*s == '\0')
    {
        while (*pattern == '*')
            pattern++;

        if (*pattern == '\0')
            return true;
    }

    return false;
}

std::string Utils::FormatIPAddress(uint32_t addr)
{
    char buffer[32];

    if (WinDivertHelperFormatIPv4Address(addr, buffer, sizeof(buffer)) == FALSE)
        return std::string();

    return buffer;
}

std::string Utils::FormatIPAddress(const uint32_t* addr)
{
    char buffer[64];

    if (WinDivertHelperFormatIPv6Address(addr, buffer, sizeof(buffer)) == FALSE)
        return std::string();

    return buffer;
}

uint16_t Utils::ntohs(uint16_t value)
{
    return WinDivertHelperNtohs(value);
}

uint16_t Utils::htons(uint16_t value)
{
    return WinDivertHelperHtons(value);
}

uint32_t Utils::ntohl(uint32_t value)
{
    return WinDivertHelperNtohl(value);
}

uint32_t Utils::htonl(uint32_t value)
{
    return WinDivertHelperHtonl(value);
}

uint64_t Utils::ntohll(uint64_t value)
{
    return WinDivertHelperNtohll(value);
}

uint64_t Utils::htonll(uint64_t value)
{
    return WinDivertHelperHtonll(value);
}
