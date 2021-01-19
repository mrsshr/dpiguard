#include "StdAfx.h"
#include "HttpRequestParser.h"

// Modified Boost Asio HTTP Request Parser
// https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio/example/cpp11/http/server/request_parser.cpp

HttpRequestParser::HttpRequestParser()
    : m_state(State::MethodStart), m_data(nullptr), m_dataLength(0)
    , m_methodBegin(-1), m_methodEnd(-1)
    , m_uriBegin(-1), m_uriEnd(-1)
    , m_httpVersionMajor(-1), m_httpVersionMinor(-1)
{
}

HttpRequestParser::Result HttpRequestParser::Parse(const uint8_t* data, uint32_t dataLength)
{
    int keyBegin = -1;
    int keyEnd = -1;
    int valueBegin = -1;
    int valueEnd = -1;

    m_data = data;
    m_dataLength = dataLength;

    for (uint32_t i = 0; i < dataLength; i++)
    {
        int c = data[i];

        switch (m_state)
        {
        case State::MethodStart:
            if (!IsChar(c) || IsCtrl(c) || IsTSpecial(c))
                return Result::Bad;

            m_methodBegin = i;
            m_state = State::Method;
            break;
        case State::Method:
            if (c == ' ')
            {
                m_methodEnd = i;
                m_state = State::Uri;
                continue;
            }

            if (!IsChar(c) || IsCtrl(c) || IsTSpecial(c))
                return Result::Bad;

            break;
        case State::Uri:
            if (c == ' ')
            {
                m_uriEnd = i;
                m_state = State::HttpVersionH;
                continue;
            }
            
            if (IsCtrl(c))
                return Result::Bad;

            if (m_uriBegin < 0)
                m_uriBegin = i;

            break;
        case State::HttpVersionH:
            if (c != 'H')
                return Result::Bad;

            m_state = State::HttpVersionT1;
            break;
        case State::HttpVersionT1:
            if (c != 'T')
                return Result::Bad;

            m_state = State::HttpVersionT2;
            break;
        case State::HttpVersionT2:
            if (c != 'T')
                return Result::Bad;

            m_state = State::HttpVersionP;
            break;
        case State::HttpVersionP:
            if (c != 'P')
                return Result::Bad;

            m_state = State::HttpVersionSlash;
            break;
        case State::HttpVersionSlash:
            if (c != '/')
                return Result::Bad;

            m_httpVersionMajor = 0;
            m_state = State::HttpVersionMajorStart;
            break;
        case State::HttpVersionMajorStart:
            if (!IsDigit(c))
                return Result::Bad;

            m_httpVersionMajor = m_httpVersionMajor * 10 + c - '0';
            m_state = State::HttpVersionMajor;
            break;
        case State::HttpVersionMajor:
            if (c == '.')
            {
                m_httpVersionMinor = 0;
                m_state = State::HttpVersionMinorStart;
                continue;
            }

            if (!IsDigit(c))
                return Result::Bad;

            m_httpVersionMajor = m_httpVersionMajor * 10 + c - '0';
            break;
        case State::HttpVersionMinorStart:
            if (!IsDigit(c))
                return Result::Bad;

            m_httpVersionMinor = m_httpVersionMinor * 10 + c - '0';
            m_state = State::HttpVersionMinor;
            break;
        case State::HttpVersionMinor:
            if (c == '\r')
            {
                m_state = State::NewLine1;
                continue;
            }

            if (!IsDigit(c))
                return Result::Bad;

            m_httpVersionMinor = m_httpVersionMinor * 10 + c - '0';
            break;
        case State::NewLine1:
            if (c != '\n')
                return Result::Bad;

            m_state = State::HeaderStart;
            break;
        case State::HeaderStart:
            if (c == '\r')
            {
                m_state = State::NewLine3;
                continue;
            }

            if (!m_headers.empty() && (c == ' ' || c == '\t'))
            {
                m_state = State::HeaderLws;
                continue;
            }

            keyBegin = i;
            m_state = State::HeaderName;
            break;
        case State::HeaderLws:
            if (c == '\r')
            {
                m_state = State::NewLine2;
                continue;
            }

            if (c == ' ' || c == '\t')
                continue;

            if (IsCtrl(c))
                return Result::Bad;

            m_state = State::HeaderValue;
            break;
        case State::HeaderName:
            if (c == ':')
            {
                keyEnd = i;
                m_state = State::HeaderBeforeValue;
                continue;
            }

            if (!IsChar(c) || IsCtrl(c) || IsTSpecial(c))
                return Result::Bad;

            break;
        case State::HeaderBeforeValue:
            if (c != ' ')
                return Result::Bad;

            m_state = State::HeaderValue;
            break;
        case State::HeaderValue:
            if (c == '\r')
            {
                if (valueBegin < 0)
                    valueBegin = i;

                valueEnd = i;

                std::array<int, 4> header = { keyBegin, keyEnd, valueBegin, valueEnd };
                m_headers.push_back(header);

                keyBegin = -1;
                keyEnd = -1;
                valueBegin = -1;
                valueEnd = -1;

                m_state = State::NewLine2;
                continue;
            }

            if (IsCtrl(c))
                return Result::Bad;

            if (valueBegin < 0)
                valueBegin = i;

            break;
        case State::NewLine2:
            if (c != '\n')
                return Result::Bad;

            m_state = State::HeaderStart;
            break;
        case State::NewLine3:
            if (c != '\n')
                return Result::Bad;

            return Result::OK;
        default:
            return Result::Bad;
        }
    }

    return Result::Indeterminate;
}

bool HttpRequestParser::HasMethod() const
{
    return m_methodBegin >= 0 && m_methodEnd >= 0;
}

bool HttpRequestParser::HasUri() const
{
    return m_uriBegin >= 0 && m_uriEnd >= 0;
}

bool HttpRequestParser::HasVersion() const
{
    return m_httpVersionMajor >= 0 && m_httpVersionMinor >= 0;
}

bool HttpRequestParser::HasHeaders() const
{
    return !m_headers.empty();
}

const std::array<int, 4>* HttpRequestParser::GetHeader(const char* name) const
{
    const char* data = reinterpret_cast<const char*>(m_data);

    for (const std::array<int, 4>& header : m_headers)
    {
        int keyBegin = header[0];
        int keyEnd = header[1];

        if (strncmp(data + keyBegin, name, keyEnd - keyBegin) == 0)
            return &header;
    }

    return nullptr;
}

bool HttpRequestParser::IsChar(int c)
{
    return 0 <= c && c <= 127;
}

bool HttpRequestParser::IsCtrl(int c)
{
    return (0 <= c && c <= 31) || (c == 127);
}

bool HttpRequestParser::IsTSpecial(int c)
{
    switch (c)
    {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
        return true;
    default:
        return false;
    }
}

bool HttpRequestParser::IsDigit(int c)
{
    return '0' <= c && c <= '9';
}
