#include "StdAfx.h"
#include "BufferReader.h"

BufferReader::BufferReader(const void* buffer, size_t length)
    : m_buffer(reinterpret_cast<const uint8_t*>(buffer)), m_offset(0), m_length(length)
{
}

int8_t BufferReader::Int8()
{
    return Read<int8_t>();
}

int16_t BufferReader::Int16()
{
    return Read<int16_t>();
}

int32_t BufferReader::Int32()
{
    return Read<int32_t>();
}

int64_t BufferReader::Int64()
{
    return Read<int64_t>();
}

uint8_t BufferReader::UInt8()
{
    return Read<uint8_t>();
}

uint16_t BufferReader::UInt16()
{
    return Read<uint16_t>();
}

uint32_t BufferReader::UInt32()
{
    return Read<uint32_t>();
}

uint64_t BufferReader::UInt64()
{
    return Read<uint64_t>();
}

const void* BufferReader::Consume(size_t length)
{
    size_t offset = m_offset;

    if (m_length < (offset + length))
        throw std::out_of_range("out of range");

    m_offset += length;
    return &m_buffer[offset];
}

void BufferReader::Forward(size_t length)
{
    if (m_length < (m_offset + length))
        throw std::out_of_range("out of range");

    m_offset += length;
}

void BufferReader::Backward(size_t length)
{
    if (m_offset < length)
        throw std::out_of_range("out of range");

    m_offset -= length;
}

void BufferReader::Offset(size_t value)
{
    if (m_length < value)
        throw std::out_of_range("out of range");

    m_offset = value;
}

size_t BufferReader::Offset() const
{
    return m_offset;
}

size_t BufferReader::Length() const
{
    return m_length;
}
