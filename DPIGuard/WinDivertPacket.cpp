#include "StdAfx.h"
#include "WinDivertPacket.h"

WinDivertPacket::WinDivertPacket(size_t size /*= 4096*/)
    : m_address(), m_ipv4(nullptr), m_ipv6(nullptr), m_tcp(nullptr)
    , m_data(nullptr), m_dataLength(0)
{
    m_buffer.reserve(size);
}

WinDivertPacket::WinDivertPacket(const WinDivertPacket& rhs)
    : WinDivertPacket(rhs.m_buffer.capacity())
{
    *this = rhs;
}

WinDivertPacket& WinDivertPacket::operator=(const WinDivertPacket& rhs)
{
    m_buffer = rhs.m_buffer;
    m_address = rhs.m_address;

    if (rhs.m_ipv4)
        m_ipv4 = reinterpret_cast<PWINDIVERT_IPHDR>(m_buffer.data() + (reinterpret_cast<const uint8_t*>(rhs.m_ipv4) - rhs.m_buffer.data()));

    if (rhs.m_ipv6)
        m_ipv6 = reinterpret_cast<PWINDIVERT_IPV6HDR>(m_buffer.data() + (reinterpret_cast<const uint8_t*>(rhs.m_ipv6) - rhs.m_buffer.data()));

    if (rhs.m_tcp)
        m_tcp = reinterpret_cast<PWINDIVERT_TCPHDR>(m_buffer.data() + (reinterpret_cast<const uint8_t*>(rhs.m_tcp) - rhs.m_buffer.data()));

    if (rhs.m_data)
        m_data = m_buffer.data() + (rhs.m_data - rhs.m_buffer.data());

    m_dataLength = rhs.m_dataLength;

    return *this;
}

const std::vector<uint8_t>& WinDivertPacket::Buffer() const
{
    return m_buffer;
}

std::vector<uint8_t>& WinDivertPacket::Buffer()
{
    return m_buffer;
}

const WINDIVERT_ADDRESS& WinDivertPacket::Address() const
{
    return m_address;
}

WINDIVERT_ADDRESS& WinDivertPacket::Address()
{
    return m_address;
}

bool WinDivertPacket::Dissect()
{
    m_ipv4 = nullptr;
    m_ipv6 = nullptr;
    m_tcp = nullptr;
    m_data = nullptr;
    m_dataLength = 0;

    return WinDivertHelperParsePacket(
        m_buffer.data(), (uint32_t)m_buffer.size(),
        &m_ipv4, &m_ipv6, nullptr, nullptr, nullptr, &m_tcp, nullptr,
        reinterpret_cast<void**>(&m_data), &m_dataLength,
        nullptr, nullptr) != FALSE;
}

bool WinDivertPacket::RecalcChecksum()
{
    if (WinDivertHelperCalcChecksums(m_buffer.data(), (uint32_t)m_buffer.size(), &m_address, 0) == FALSE)
        return false;

    return true;
}

PWINDIVERT_IPHDR WinDivertPacket::IPv4()
{
    return m_ipv4;
}

PWINDIVERT_IPV6HDR WinDivertPacket::IPv6()
{
    return m_ipv6;
}

PWINDIVERT_TCPHDR WinDivertPacket::Tcp()
{
    return m_tcp;
}

const uint8_t* WinDivertPacket::Data() const
{
    return m_data;
}

uint8_t* WinDivertPacket::Data()
{
    return m_data;
}

uint32_t WinDivertPacket::DataLength() const
{
    return m_dataLength;
}
