#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <regex>

#include "./lib/include/libhttpr.hpp"

class ExampleMiddleware : public Middleware
{
public:
    ExampleMiddleware() = default;

    void process(Request &req, Response &res) override
    {
        std::cout << "Middleware: Processing request " << req.path() << std::endl;
        res.set_header("X-Custom-Header", "Middleware-Value");
        auto x = req.get_query_param("x");
        if (x)
            std::cout << x.value() << std::endl;
        else
            std::cout << "No x\n";
        auto c = req.get_cookie("check");
        std::cout << "Cookie " << c << std::endl;
        res.set_cookie("check", "value");
        if (next_)
        {
            next_->process(req, res);
        }
    }
};

int main()
{
    auto app = std::make_shared<App>();

    // Global middleware
    app->use(std::make_shared<ExampleMiddleware>());

    // Root route
    app->add_route("GET", "/", [](const Request &req, Response &res)
                   { res.set_body("Welcome to the home page!"); });

    // API routes
    auto api_router = std::make_shared<Router>();

    api_router->add_route("GET", "/users", [](const Request &req, Response &res)
                          { res.set_body("List of users"); });

    api_router->add_route("POST", "/users", [](const Request &req, Response &res)
                          { res.set_body("User created"); });

    // Nested routes for user details
    auto user_router = std::make_shared<Router>();

    user_router->add_route("GET", "/details", [](const Request &req, Response &res)
                           { res.set_body("User details"); });

    user_router->add_route("PUT", "/details", [](const Request &req, Response &res)
                           { res.set_body("User details updated"); });

    user_router->add_route("DELETE", "/details", [](const Request &req, Response &res)
                           { res.set_body("User deleted"); });

    // Register nested routers
    api_router->use_router("/user/*", user_router);
    app->use_router("/api", api_router);

    app->add_route("GET", "/static", [](const Request &req, Response &res)
                   {
    std::string static_files_path = "./static/";  // Replace with the path to your static files
    auto requested_path = req.get_query_param("path");           // Remove "/static" from the requested path
    if (requested_path){
      std::string target_path = static_files_path+"/" + requested_path.value();
    StaticFileHandler::serve_static_files(target_path, res);
    }
    else{
      res.set_body("No path given");
    } });

    app->run(8080);

    return 0;
}
