#ifndef LIBHTTPR_REQUEST_H

#define LIBHTTPR_REQUEST_H

#include <regex>

#include <boost/beast/http.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace http = boost::beast::http;

class Request
{
public:
    Request(http::request<http::string_body> &req)
        : method_(req.method_string().to_string()),
          path_(req.target().to_string())
    {
        for (const auto &header : req)
        {
            headers_[header.name_string().to_string()] = header.value().to_string();
        }
        parse_query_params();
    }

    Request(const std::string &method, const std::string &path, const std::map<std::string, std::string> &headers)
        : method_(method), path_(path), headers_(headers)
    {
        parse_query_params();
    }

    boost::optional<std::string> get_query_param(const std::string &name) const
    {
        auto it = query_params_.find(name);
        if (it != query_params_.end())
        {
            return it->second;
        }
        else
        {
            return boost::none;
        }
    }
    void set_header(const std::string &key, const std::string &value)
    {
        headers_[key] = value;
    }

    std::string get_header(const std::string &key) const
    {
        auto it = headers_.find(key);
        if (it != headers_.end())
        {
            return it->second;
        }
        return "";
    }

    void set_cookie(const std::string &key, const std::string &value)
    {
        std::string cookie = key + "=" + value;
        set_header("Cookie", cookie);
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

    const std::string &method() const { return method_; }
    const std::string &path() const { return path_; }
    const std::map<std::string, std::string> &headers() const { return headers_; }

private:
    std::string method_;
    std::string path_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> query_params_;

    void parse_query_params()
    {
        std::size_t query_start = path_.find("?");
        if (query_start != std::string::npos)
        {
            std::string query_string = path_.substr(query_start + 1);
            std::vector<std::string> query_parts;
            boost::split(query_parts, query_string, boost::is_any_of("&"));

            for (const auto &part : query_parts)
            {
                std::vector<std::string> key_value;
                boost::split(key_value, part, boost::is_any_of("="));
                if (key_value.size() == 2)
                {
                    query_params_[key_value[0]] = key_value[1];
                }
            }
        }
    }
};

#endif // LIBHTTPR_REQUEST_H