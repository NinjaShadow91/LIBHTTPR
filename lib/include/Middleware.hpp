#ifndef LIBHTTPR_MIDDLEWARE_H

#define LIBHTTPR_MIDDLEWARE_H

#include <functional>
#include <memory>

#include "./Request.hpp"
#include "./Response.hpp"

class Middleware
{
public:
    using MiddlewareFunction = std::function<void(const Request &, Response &)>;

    Middleware() : next_(nullptr) {}
    virtual ~Middleware() = default;

    virtual void process(Request &req, Response &res) = 0;

    void set_next(std::shared_ptr<Middleware> next)
    {
        if (next)
            next_ = next;
    }

protected:
    std::shared_ptr<Middleware> next_;
};

#endif // LIBHTTPR_MIDDLEWARE_H