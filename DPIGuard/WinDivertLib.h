#pragma once

#include "WinDivertPacket.h"

class WinDivertLib
{
public:
    WinDivertLib();
    WinDivertLib(const char* filter, WINDIVERT_LAYER layer = WINDIVERT_LAYER_NETWORK, int16_t priority = 0, uint64_t flags = 0);
    ~WinDivertLib();

    bool Open(const char* filter, WINDIVERT_LAYER layer = WINDIVERT_LAYER_NETWORK, int16_t priority = 0, uint64_t flags = 0);
    void Close();

    bool Shutdown(WINDIVERT_SHUTDOWN how = WINDIVERT_SHUTDOWN_RECV);

    bool Recv(WinDivertPacket& packet);
    bool Send(const WinDivertPacket& packet);
private:
    HANDLE m_handle;
};
