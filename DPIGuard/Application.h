#pragma once

#include "ApplicationConfig.h"
#include "WinDivertLib.h"

class Application
{
public:
    Application();

    int Run(int argc, wchar_t* argv[]);
private:
    void Main();

    bool HandlePacket(WinDivertPacket& packet);
    bool HandleHttps(WinDivertPacket& packet);

    bool HandleTlsFragmentation(WinDivertPacket& packet, const std::string& serverName, size_t serverNameOffset);

    bool DoTcpFragmentation(WinDivertPacket& packet, size_t offset);

    const ApplicationConfig::DomainConfig* GetDomainConfig(const std::string& domain);

    void StartMainThread();
    void WaitMainThread();

    void StopWinDivert();
private:
    BOOL ConsoleCtrlHandler(DWORD ctrlType);

    static BOOL WINAPI ConsoleCtrlHandlerProc(DWORD dwCtrlType);
private:
    bool StartServiceMode();

    void ServiceMain(DWORD args, wchar_t* argv[]);
    DWORD ServiceCtrlHandler(DWORD control, DWORD eventType, LPVOID eventData);

    void ReportStartPending();
    void ReportRunning();
    void ReportStopPending();
    void ReportStopped();

    static void WINAPI ServiceMainProc(DWORD args, wchar_t* argv[]);
    static DWORD WINAPI ServiceCtrlHandlerProc(DWORD dwControl, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);
private:
    ApplicationConfig m_appConfig;
    std::wstring m_appConfigPath;
    FILETIME m_appConfigModifiedTime;

    bool m_serviceMode;
    SERVICE_STATUS_HANDLE m_serviceStatusHandle;

    std::unique_ptr<std::thread> m_mainThread;

    WinDivertLib m_divert;
};

extern Application theApp;
