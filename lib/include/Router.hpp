#ifndef LIBHTTPR_ROUTER_H

#define LIBHTTPR_ROUTER_H

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>

#include "./Request.hpp"
#include "./Response.hpp"
#include "./Middleware.hpp"

class Route
{
public:
    Route(const std::string &method, const std::string &path,
          const std::function<void(const Request &, Response &)> &handler,
          const std::vector<std::shared_ptr<Middleware>> &middlewares = {})
        : method_(method), path_(path), handler_(handler), middlewares_(middlewares) {}

    const std::string &method() const { return method_; }
    const std::string &path() const { return path_; }
    const std::function<void(const Request &, Response &)> &handler() const { return handler_; }
    const std::vector<std::shared_ptr<Middleware>> &middlewares() const { return middlewares_; }

private:
    std::string method_;
    std::string path_;
    std::function<void(const Request &, Response &)> handler_;
    std::vector<std::shared_ptr<Middleware>> middlewares_;
};

class Router
{
public:
    void add_route(const std::string &method, const std::string &path,
                   const std::function<void(const Request &, Response &)> &handler,
                   const std::vector<std::shared_ptr<Middleware>> &middlewares = {})
    {
        routes_.emplace_back(method, path, handler, middlewares);
    }

    void use_router(const std::string &path, std::shared_ptr<Router> router)
    {
        sub_routers_.emplace(path, router);
    }

    const std::vector<Route> &routes() const { return routes_; }
    const std::unordered_map<std::string, std::shared_ptr<Router>> &sub_routers() const
    {
        return sub_routers_;
    }

private:
    std::vector<Route> routes_;
    std::unordered_map<std::string, std::shared_ptr<Router>> sub_routers_;
};

class StaticFileHandler
{
public:
    static void serve_static_files(const std::string &path, Response &res)
    {
        boost::filesystem::path target_path(path);
        boost::system::error_code ec;

        if (boost::filesystem::exists(target_path, ec) && !ec)
        {
            if (boost::filesystem::is_directory(target_path, ec) && !ec)
            {
                std::stringstream file_list;
                file_list << "<html><body><ul>";
                for (const auto &entry : boost::filesystem::directory_iterator(target_path))
                {
                    file_list << "<li><a href=\"" << entry.path().string() << "\">"
                              << entry.path().filename().string() << "</a></li>";
                }
                file_list << "</ul></body></html>";

                res.set_status(http::status::ok);
                res.set_header("Content-Type", "text/html");
                res.set_body(file_list.str());
            }
            else if (boost::filesystem::is_regular_file(target_path, ec) && !ec)
            {
                boost::filesystem::ifstream file_stream(target_path);
                if (file_stream)
                {
                    std::stringstream file_content;
                    file_content << file_stream.rdbuf();

                    res.set_status(http::status::ok);
                    res.set_header("Content-Type", "text/plain"); // Change this to the appropriate MIME type based on the file extension
                    res.set_body(file_content.str());
                }
                else
                {
                    res.set_status(http::status::internal_server_error);
                    res.set_body("500 Internal Server Error");
                }
            }
            else
            {
                res.set_status(http::status::forbidden);
                res.set_body("403 Forbidden");
            }
        }
        else
        {
            res.set_status(http::status::not_found);
            res.set_body("404 Not Found");
        }
    }
};

#endif // LIBHTTPR_ROUTER_H