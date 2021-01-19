#pragma once

// Modified Boost Asio HTTP Request Parser
// https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/example/cpp11/http/server/request_parser.hpp

class HttpRequestParser
{
public:
    HttpRequestParser();

    enum class Result
    {
        OK,
        Indeterminate,
        Bad
    };

    Result Parse(const uint8_t* data, uint32_t dataLength);

    bool HasMethod() const;
    bool HasUri() const;
    bool HasVersion() const;
    bool HasHeaders() const;

    const std::array<int, 4>* GetHeader(const char* name) const;
private:
    bool IsChar(int c);

    bool IsCtrl(int c);

    bool IsTSpecial(int c);

    bool IsDigit(int c);
private:
    enum class State
    {
        MethodStart,
        Method,
        Uri,
        HttpVersionH,
        HttpVersionT1,
        HttpVersionT2,
        HttpVersionP,
        HttpVersionSlash,
        HttpVersionMajorStart,
        HttpVersionMajor,
        HttpVersionMinorStart,
        HttpVersionMinor,
        NewLine1,
        HeaderStart,
        HeaderLws, // Linear White Space
        HeaderName,
        HeaderBeforeValue,
        HeaderValue,
        NewLine2,
        NewLine3
    };

    State m_state;

    const uint8_t* m_data;
    uint32_t m_dataLength;

    int m_methodBegin;
    int m_methodEnd;
    int m_uriBegin;
    int m_uriEnd;
    int m_httpVersionMajor;
    int m_httpVersionMinor;

    // keyBegin, keyEnd, valueBegin, valueEnd
    std::vector<std::array<int, 4>> m_headers;
};
