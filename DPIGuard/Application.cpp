#include "StdAfx.h"
#include "Application.h"
#include "BufferReader.h"
#include "Utils.h"

Application theApp;

static wchar_t SERVICE_NAME[] = L"DPIGuard";

static const char* WINDIVERT_HTTPS_FILTER = \
"!loopback && outbound && (ip || ipv6) && length <= 4096 && " \
"(tcp.DstPort == 443) && " \
"(tcp.PayloadLength > 0 && tcp.PayloadLength <= 1024)";

Application::Application()
    : m_appConfigModifiedTime(), m_serviceMode(false), m_serviceStatusHandle(nullptr)
{
}

int Application::Run(int argc, wchar_t* argv[])
{
    if (!StartServiceMode())
    {
        StartMainThread();
        WaitMainThread();
    }

    return 0;
}

void Application::Main()
{
    printf("[+] Loading configuration\n");

    m_appConfigPath = Utils::GetApplicationConfigPath();

    if (!m_appConfig.LoadFile(m_appConfigPath))
    {
        printf("[-] The configuration file is invalid or corrupted. Aborting\n");

        ReportStopped();
        return;
    }

    m_appConfig.SaveFile(m_appConfigPath);

    Utils::CheckFileModified(m_appConfigPath.c_str(), m_appConfigModifiedTime);

    if (!m_serviceMode)
        SetConsoleCtrlHandler(&Application::ConsoleCtrlHandlerProc, TRUE);

    try
    {
        WinDivertPacket packet;

        printf("[+] Initializing packet filter module\n");

        if (!m_divert.Open(WINDIVERT_HTTPS_FILTER))
            throw std::system_error(GetLastError(), std::system_category());

        printf("[+] Initialization complete\n");

        ReportRunning();

        while (m_divert.Recv(packet))
        {
            if (packet.Dissect())
            {
                if (HandlePacket(packet))
                    continue;
            }

            m_divert.Send(packet);
        }

        m_divert.Close();
    }
    catch (const std::exception& e)
    {
        printf("[-] Unexpected error. Aborting (%s)\n", e.what());
    }

    StopWinDivert();

    ReportStopped();

    printf("[+] Stopped\n");
}

bool Application::HandlePacket(WinDivertPacket& packet)
{
    if (!packet.IPv4() && !packet.IPv6())
        return false;

    if (!packet.Tcp())
        return false;

    if (Utils::ntohs(packet.Tcp()->DstPort) == 443)
        return HandleHttps(packet);

    return false;
}

bool Application::HandleHttps(WinDivertPacket& packet)
{
    if (!packet.Data())
        return false;

    try
    {
        BufferReader reader(packet.Data(), packet.DataLength());

        // Content Type: Handshake (22)
        // Version: TLS 1.0 (0x0301)
        // Length: 512

        try
        {
            uint8_t contentType = reader.UInt8();
            uint16_t version = Utils::ntohs(reader.UInt16());
            uint16_t length = Utils::ntohs(reader.UInt16());

            if (contentType != 22 || version != 0x0301)
                return false;
        }
        catch (const std::out_of_range&)
        {
            return false;
        }

        // Handshake Type: Client Hello (1)
        // Length: 508
        // Version: TLS 1.2 (0x0303)
        // Random: 19ecfc70399dbe45a24a73d099ba4a3b8ad91042e90105c8e6271fb93cd6b78f
        // Session ID Length: 32
        // Session ID: 51ca63b4499b93beba73f581ef06c088abc7cd0e7a21166bd60dce014a00b321
        // Cipher Suites Length: 62
        // Cipher Suites (31 suites)
        // Compression Methods Length: 1
        // Compression Methods (1 method)

        uint8_t handshakeType = reader.UInt8();
        if (handshakeType != 1)
            return false;

        uint8_t handshakeLengthBytes[4];
        handshakeLengthBytes[0] = 0;
        handshakeLengthBytes[1] = reader.UInt8();
        handshakeLengthBytes[2] = reader.UInt8();
        handshakeLengthBytes[3] = reader.UInt8();
        uint8_t handshakeLength = Utils::ntohl(*reinterpret_cast<uint32_t*>(handshakeLengthBytes));

        uint16_t handshakeVersion = Utils::ntohs(reader.UInt16());
        // TLS 1.0, TLS 1.1, TLS 1.2
        if (handshakeVersion != 0x0301 && handshakeVersion != 0x0302 && handshakeVersion != 0x0303)
            return false;

        reader.Forward(32);

        uint8_t sessionIdLength = reader.UInt8();
        reader.Forward(sessionIdLength);

        uint16_t cipherSuitesLength = Utils::ntohs(reader.UInt16());
        reader.Forward(cipherSuitesLength);

        uint8_t compressionMethodsLength = reader.UInt8();
        reader.Forward(compressionMethodsLength);

        // Extensions Length: 373
        uint16_t extensionsLength = Utils::ntohs(reader.UInt16());
        while (extensionsLength)
        {
            // Extension: server_name (len=14)
            // Type: server_name (0)
            // Length: 14

            uint16_t extensionType = Utils::ntohs(reader.UInt16());
            uint16_t extensionLength = Utils::ntohs(reader.UInt16());

            size_t nextOffset = reader.Offset() + extensionLength;

            if (extensionType == 0)
            {
                // Server Name list length: 12

                // Extension: server_name (len=14)
                // Type: server_name (0)
                // Length: 14

                uint16_t serverNameListLength = Utils::ntohs(reader.UInt16());

                uint16_t serverNameType = reader.UInt8();
                uint16_t serverNameLength = Utils::ntohs(reader.UInt16());

                size_t serverNameOffset = reader.Offset();

                const char* serverNameBuffer = reinterpret_cast<const char*>(reader.Consume(serverNameLength));
                std::string serverName = std::string(serverNameBuffer, serverNameBuffer + serverNameLength);

                return HandleTlsFragmentation(packet, serverName, serverNameOffset);
            }

            reader.Offset(nextOffset);

            uint32_t totalExtensionLength = sizeof(uint16_t) * 2 + extensionLength;
            if (extensionsLength < totalExtensionLength)
                break;

            extensionsLength -= totalExtensionLength;
        }
    }
    catch (const std::exception& e)
    {
        printf("[-] Unexpected error while parsing TLS packet (%s)\n", e.what());

        // TODO Dump raw packet bytes
    }

    return false;
}

bool Application::HandleTlsFragmentation(WinDivertPacket& packet, const std::string& serverName, size_t serverNameOffset)
{
    const ApplicationConfig::DomainConfig* domainConfig = GetDomainConfig(serverName);

    if (domainConfig == nullptr)
    {
        printf("[-] Skip: %s\n", serverName.c_str());
        return false;
    }

    if (!domainConfig->tlsFragmentationEnabled)
        return false;

    printf("[+] OK: %s\n", serverName.c_str());
    return DoTcpFragmentation(packet, domainConfig->tlsFragmentationOffset);
}

bool Application::DoTcpFragmentation(WinDivertPacket& packet, size_t offset)
{
    size_t headerLength = packet.Data() - packet.Buffer().data();

    if (packet.DataLength() <= offset)
        return false;

    size_t firstDataOffset = 0;
    size_t firstDataLength = offset;
    size_t secondDataOffset = firstDataOffset + firstDataLength;
    size_t secondDataLength = packet.DataLength() - secondDataOffset;

    WinDivertPacket firstPacket(packet);
    WinDivertPacket secondPacket(packet);

    firstPacket.Buffer().resize(headerLength + firstDataOffset + firstDataLength);
    if (firstPacket.IPv4())
        firstPacket.IPv4()->Length = Utils::htons(static_cast<uint16_t>(firstPacket.Buffer().size()));
    else
        firstPacket.IPv6()->Length = Utils::htons(static_cast<uint16_t>(firstPacket.Buffer().size()));
    firstPacket.RecalcChecksum();

    secondPacket.Buffer().resize(headerLength + secondDataLength);
    memcpy(secondPacket.Data(), packet.Data() + secondDataOffset, secondDataLength);
    if (secondPacket.IPv4())
        secondPacket.IPv4()->Length = Utils::htons(static_cast<uint16_t>(secondPacket.Buffer().size()));
    else
        secondPacket.IPv6()->Length = Utils::htons(static_cast<uint16_t>(secondPacket.Buffer().size()));
    secondPacket.Tcp()->SeqNum = Utils::htonl(Utils::ntohl(firstPacket.Tcp()->SeqNum) + static_cast<uint32_t>(firstDataLength));
    secondPacket.RecalcChecksum();

    m_divert.Send(firstPacket);
    m_divert.Send(secondPacket);

    return true;
}

const ApplicationConfig::DomainConfig* Application::GetDomainConfig(const std::string& domain)
{
    for (const ApplicationConfig::DomainConfig& domainConfig : m_appConfig.Domains())
    {
        size_t offset = 0;

        if (domain.size() < domainConfig.domain.size())
            continue;

        if (domainConfig.includeSubdomains)
            offset = domain.size() - domainConfig.domain.size();

        bool match = std::equal(
            domain.cbegin() + offset,
            domain.cend(),
            domainConfig.domain.cbegin(),
            domainConfig.domain.cend(),
            [](char a, char b) {
            return std::toupper(a) == std::toupper(b);
        });

        if (!match)
            continue;

        if (offset > 0 && domain[offset - 1] != '.')
            continue; // Not a subdomain

        return &domainConfig;
    }

    return nullptr;
}

void Application::StartMainThread()
{
    m_mainThread = std::make_unique<std::thread>(&Application::Main, this);
}

void Application::WaitMainThread()
{
    m_mainThread->join();
}

void Application::StopWinDivert()
{
    SC_HANDLE scmHandle = nullptr;
    SC_HANDLE serviceHandle = nullptr;

    scmHandle = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (scmHandle == nullptr)
        return;

    serviceHandle = OpenServiceW(scmHandle, L"WinDivert", SERVICE_ALL_ACCESS);
    if (serviceHandle == nullptr)
    {
        CloseServiceHandle(scmHandle);
        return;
    }

    SERVICE_STATUS ss = { 0 };
    ControlService(serviceHandle, SERVICE_CONTROL_STOP, &ss);

    CloseServiceHandle(serviceHandle);
    CloseServiceHandle(scmHandle);
}

BOOL Application::ConsoleCtrlHandler(DWORD ctrlType)
{
    if (ctrlType == CTRL_C_EVENT ||
        ctrlType == CTRL_BREAK_EVENT)
    {
        m_divert.Shutdown();
        return TRUE;
    }

    return FALSE;
}

BOOL WINAPI Application::ConsoleCtrlHandlerProc(DWORD dwCtrlType)
{
    return theApp.ConsoleCtrlHandler(dwCtrlType);
}

bool Application::StartServiceMode()
{
    SERVICE_TABLE_ENTRYW serviceTableEntries[] = {
        { SERVICE_NAME, &Application::ServiceMainProc },
        { nullptr, nullptr }
    };

    if (StartServiceCtrlDispatcherW(serviceTableEntries) == FALSE)
        return false;

    return true;
}

void Application::ServiceMain(DWORD args, wchar_t* argv[])
{
    m_serviceMode = true;

    m_serviceStatusHandle = RegisterServiceCtrlHandlerExW(SERVICE_NAME, &Application::ServiceCtrlHandlerProc, this);
    if (m_serviceStatusHandle == nullptr)
        return;

    ReportStartPending();

    StartMainThread();
}

DWORD Application::ServiceCtrlHandler(DWORD control, DWORD eventType, LPVOID eventData)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
        ReportStopPending();
        m_divert.Shutdown();
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    default:
        return ERROR_CALL_NOT_IMPLEMENTED;
    }

    return NO_ERROR;
}

void Application::ReportStartPending()
{
    if (m_serviceStatusHandle == nullptr)
        return;

    SERVICE_STATUS ss = { 0 };
    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState = SERVICE_START_PENDING;
    ss.dwControlsAccepted = 0;
    SetServiceStatus(m_serviceStatusHandle, &ss);
}

void Application::ReportRunning()
{
    if (m_serviceStatusHandle == nullptr)
        return;

    SERVICE_STATUS ss = { 0 };
    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState = SERVICE_RUNNING;
    ss.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    SetServiceStatus(m_serviceStatusHandle, &ss);
}

void Application::ReportStopPending()
{
    if (m_serviceStatusHandle == nullptr)
        return;

    SERVICE_STATUS ss = { 0 };
    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState = SERVICE_STOP_PENDING;
    ss.dwControlsAccepted = 0;
    SetServiceStatus(m_serviceStatusHandle, &ss);
}

void Application::ReportStopped()
{
    if (m_serviceStatusHandle == nullptr)
        return;

    SERVICE_STATUS ss = { 0 };
    ss.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ss.dwCurrentState = SERVICE_STOPPED;
    ss.dwControlsAccepted = 0;
    SetServiceStatus(m_serviceStatusHandle, &ss);
}

void WINAPI Application::ServiceMainProc(DWORD args, wchar_t* argv[])
{
    theApp.ServiceMain(args, argv);
}

DWORD WINAPI Application::ServiceCtrlHandlerProc(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext)
{
    Application* _this = reinterpret_cast<Application*>(lpContext);
    return _this->ServiceCtrlHandler(dwControl, dwEventType, lpEventData);
}
