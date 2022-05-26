#pragma once

#include "ApplicationConfig.h"
#include "WinDivertLib.h"

class Application
{
public:
    Application();

    int Run(int argc, wchar_t* argv[]);
private:
    int CommandHelp();
    int CommandVersion();
    int CommandInstall();
    int CommandUninstall();

    bool ParseCommandLine(int argc, wchar_t* argv[]);

    void Main();
    void ConfigMonitor();

    bool HandlePacket(WinDivertPacket& packet);
    bool HandleHttp(WinDivertPacket& packet);
    bool HandleHttps(WinDivertPacket& packet);

    bool HandleHttpFragmentation(WinDivertPacket& packet, const std::string& hostName, size_t hostNameOffset);
    bool HandleTlsFragmentation(WinDivertPacket& packet, const std::string& serverName, size_t serverNameOffset);

    bool DoTcpFragmentation(WinDivertPacket& packet, size_t offset, bool outOfOrder);

    void StartMainThread();
    void WaitMainThread();

    void StartConfigMonitor();
    void StopConfigMonitor();

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
    std::mutex m_mainThreadLock;

    std::unique_ptr<std::thread> m_configMonitorThread;
    std::condition_variable m_configMonitorCv;
    std::mutex m_configMonitorLock;
    bool m_configMonitorStop;

    WinDivertLib m_divert;

    enum class CommandType
    {
        None = 0,
        Version,
        Install,
        Uninstall
    };

    CommandType m_commandType;
};

extern Application theApp;
