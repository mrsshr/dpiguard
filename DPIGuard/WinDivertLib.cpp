#include "StdAfx.h"
#include "WinDivertLib.h"

WinDivertLib::WinDivertLib()
    : m_handle(INVALID_HANDLE_VALUE)
{
}

WinDivertLib::WinDivertLib(const char* filter, WINDIVERT_LAYER layer /*= WINDIVERT_LAYER_NETWORK */, int16_t priority /*= 0 */, uint64_t flags /*= 0 */)
    : WinDivertLib()
{
    if (!Open(filter, layer, priority, flags))
        throw std::system_error(GetLastError(), std::system_category());
}

WinDivertLib::~WinDivertLib()
{
    Close();
}

bool WinDivertLib::Open(const char* filter, WINDIVERT_LAYER layer /*= WINDIVERT_LAYER_NETWORK*/, int16_t priority /*= 0*/, uint64_t flags /*= 0*/)
{
    Close();

    HANDLE handle = WinDivertOpen(filter, layer, priority, flags);
    if (handle == INVALID_HANDLE_VALUE)
        return false;

    m_handle = handle;
    return true;
}

void WinDivertLib::Close()
{
    if (m_handle != INVALID_HANDLE_VALUE)
    {
        WinDivertClose(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}

bool WinDivertLib::Shutdown(WINDIVERT_SHUTDOWN how /*= WINDIVERT_SHUTDOWN_RECV */)
{
    if (WinDivertShutdown(m_handle, how) == FALSE)
        return false;

    return true;
}

bool WinDivertLib::Recv(WinDivertPacket& packet)
{
    uint32_t recvLength = 0;

    packet.Buffer().resize(packet.Buffer().capacity());

    if (WinDivertRecv(m_handle, packet.Buffer().data(), (uint32_t)packet.Buffer().size(), &recvLength, &packet.Address()) == FALSE)
    {
        DWORD error = GetLastError();

        switch (error)
        {
        case ERROR_INSUFFICIENT_BUFFER:
            break;
        default:
            return false;
        }
    }

    packet.Buffer().resize(recvLength);

    return true;
}

bool WinDivertLib::Send(const WinDivertPacket& packet)
{
    if (WinDivertSend(m_handle, packet.Buffer().data(), (uint32_t)packet.Buffer().size(), nullptr, &packet.Address()) == FALSE)
        return false;

    return true;
}
