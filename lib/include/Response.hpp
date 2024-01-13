#ifndef LIBHTTPR_RESPONSE_H

#define LIBHTTPR_RESPONSE_H

#include <regex>

#include <boost/beast/http.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace http = boost::beast::http;

class Response
{
public:
    Response(http::status status, int version)
        : m_http_response(status, version) {}

    void set_header(const std::string &key, const std::string &value)
    {
        m_http_response.set(key, value);
    }

    void set_body(const std::string &body)
    {
        m_http_response.body() = body;
        m_http_response.prepare_payload();
    }

    std::string get_header(const std::string &key) const
    {
        auto it = m_http_response.find(key);
        if (it != m_http_response.end())
        {
            return it->value().to_string();
        }
        return "";
    }

    void set_cookie(const std::string &key, const std::string &value,
                    const std::string &path = "/", int max_age = -1)
    {
        std::string cookie = key + "=" + value + "; Path=" + path;
        if (max_age >= 0)
        {
            cookie += "; Max-Age=" + std::to_string(max_age);
        }
        m_http_response.set(http::field::set_cookie, cookie);
    }

    std::string get_cookie(const std::string &key) const
    {
        std::string cookies_str = get_header("Cookie");
        if (!cookies_str.empty())
        {
            std::regex cookie_pattern("(^|; )" + key + "=([^;]*)");
            std::smatch match;
            if (std::regex_search(cookies_str, match, cookie_pattern))
            {
                return match[2].str();
            }
        }
        return "";
    }

    const http::response<http::string_body> &get_http_response() const
    {
        return m_http_response;
    }
    void set_status(http::status status)
    {
        m_http_response.result(status);
    }

private:
    http::response<http::string_body> m_http_response;
};

#endif // LIBHTTPR_RESPONSE_H