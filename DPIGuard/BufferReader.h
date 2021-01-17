#pragma once

class BufferReader
{
public:
    BufferReader(const void* buffer, size_t length);

    template<typename T>
    T Read()
    {
        return *reinterpret_cast<const T*>(Consume(sizeof(T)));
    }

    int8_t Int8();
    int16_t Int16();
    int32_t Int32();
    int64_t Int64();

    uint8_t UInt8();
    uint16_t UInt16();
    uint32_t UInt32();
    uint64_t UInt64();

    const void* Consume(size_t length);

    void Forward(size_t length);
    void Backward(size_t length);

    void Offset(size_t value);
    size_t Offset() const;

    size_t Length() const;
private:
    const uint8_t* m_buffer;
    size_t m_offset;
    size_t m_length;
};
