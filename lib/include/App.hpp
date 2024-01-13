// Description: This file contains the implementation of the App class. It is the main class of the library, responsible for handling requests and responses.

#ifndef LIBHTTPR_APP_H

#define LIBHTTPR_APP_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/regex.hpp>

#include "./Middleware.hpp"
#include "./Request.hpp"
#include "./Response.hpp"
#include "./Router.hpp"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

void handle_request(Request &req, Response &res, Router &router,
                    const std::vector<std::shared_ptr<Middleware>> &global_middlewares)
{
    // Apply global middlewares
    for (const auto &middleware : global_middlewares)
    {
        middleware->process(req, res);
    }

    try
    {
        for (size_t i = 0; i < global_middlewares.size() - 1; ++i)
        {
            if (global_middlewares.at(i))
                global_middlewares[i]->set_next(global_middlewares[i + 1]);
        }
    }
    catch (std::exception &e)
    {
    }

    // Find the appropriate route and handler from the router
    bool route_found = false;
    std::string pathWithoutParam = req.path();
    std::size_t query_start = req.path().find("?");
    if (query_start != std::string::npos)
    {
        pathWithoutParam = req.path().substr(0, query_start);
    }
    for (const auto &route : router.routes())
    {
        if (req.method() == route.method())
        {
            std::string path_regex = route.path();
            size_t wildcard_pos = path_regex.find("*");
            if (wildcard_pos != std::string::npos)
            {
                path_regex.replace(wildcard_pos, 1, "([^/]*)");
            }
            boost::regex regex_path(path_regex);
            boost::smatch match;
            if (boost::regex_match(pathWithoutParam, match, regex_path))
            {
                // Apply route middlewares
                for (const auto &middleware : route.middlewares())
                {
                    middleware->process(req, res);
                }

                // Call the appropriate handler
                route.handler()(req, res);
                route_found = true;
                break;
            }
        }
    }

    // If no route is found, check for sub-routers
    if (!route_found)
    {
        for (const auto &[path, sub_router] : router.sub_routers())
        {
            std::string sub_path_regex = path;
            size_t wildcard_pos = sub_path_regex.find("*");
            if (wildcard_pos != std::string::npos)
            {
                sub_path_regex.replace(wildcard_pos, 1, "([^/]*)");
            }
            boost::regex regex_sub_path(sub_path_regex);
            boost::smatch match;
            if (boost::regex_search(pathWithoutParam, match, regex_sub_path, boost::match_continuous))
            {
                std::string sub_path = pathWithoutParam.substr(match[0].length());
                if (sub_path.empty() || sub_path[0] != '/')
                {
                    sub_path = "/" + sub_path;
                }
                Request sub_req(req.method(), sub_path, req.headers());
                handle_request(sub_req, res, *sub_router, {});
                route_found = true;
                break;
            }
        }
    }

    // If no route found, set the status to 404 Not Found
    if (!route_found)
    {
        res.set_status(http::status::not_found);
        res.set_body("404 Not Found");
    }
}

void session(tcp::socket socket, Router &router,
             const std::vector<std::shared_ptr<Middleware>> &global_middlewares)
{
    boost::beast::error_code ec;

    boost::beast::flat_buffer buffer;
    http::request<http::string_body> req;
    http::read(socket, buffer, req, ec);
    if (ec == http::error::end_of_stream)
    {
        return;
    }
    if (ec)
    {
        // Error handling
        return;
    }

    // Prepare Request and Response objects
    Request request(req);
    Response response(http::status::ok, req.version());

    handle_request(request, response, router, global_middlewares);

    // Send the response
    http::write(socket, response.get_http_response(), ec);
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

class App
{
public:
    Router &get_router()
    {
        return router_;
    }

    void use(std::shared_ptr<Middleware> middleware)
    {
        global_middlewares_.push_back(middleware);
    }
    void use_router(const std::string &path, std::shared_ptr<Router> router)
    {
        router_.use_router(path, router);
    }

    void add_route(const std::string &method, const std::string &path,
                   const std::function<void(const Request &, Response &)> &handler,
                   const std::vector<std::shared_ptr<Middleware>> &middlewares = {})
    {
        router_.add_route(method, path, handler, middlewares);
    }

    void run(unsigned short port)
    {
        boost::asio::io_context ioc{1};
        tcp::acceptor acceptor{ioc, {tcp::v4(), port}};

        while (true)
        {
            tcp::socket socket{ioc};
            acceptor.accept(socket);

            std::thread{[&](tcp::socket s)
                        {
                            session(std::move(s), router_, global_middlewares_);
                        },
                        std::move(socket)}
                .detach();
        }
    }

private:
    std::vector<std::shared_ptr<Middleware>> global_middlewares_;
    Router router_;
};

#endif // LIBHTTPR_APP_H