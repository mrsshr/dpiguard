#pragma once

class WinDivertPacket
{
public:
    WinDivertPacket(size_t size = 4096);
    WinDivertPacket(const WinDivertPacket& rhs);
    WinDivertPacket& operator=(const WinDivertPacket& rhs);

    const std::vector<uint8_t>& Buffer() const;
    std::vector<uint8_t>& Buffer();

    const WINDIVERT_ADDRESS& Address() const;
    WINDIVERT_ADDRESS& Address();

    bool Dissect();
    bool RecalcChecksum();

    PWINDIVERT_IPHDR IPv4();
    PWINDIVERT_IPV6HDR IPv6();
    PWINDIVERT_TCPHDR Tcp();

    const uint8_t* Data() const;
    uint8_t* Data();

    uint32_t DataLength() const;
private:
    std::vector<uint8_t> m_buffer;
    WINDIVERT_ADDRESS m_address;

    PWINDIVERT_IPHDR m_ipv4;
    PWINDIVERT_IPV6HDR m_ipv6;
    PWINDIVERT_TCPHDR m_tcp;

    uint8_t* m_data;
    uint32_t m_dataLength;
};
