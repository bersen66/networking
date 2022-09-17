#include <boost/asio.hpp>
#include <boost/beast.hpp>
#define BOOST_STACKTRACE_USE_BACKTRACE
#include <fmt/core.h>
#include <fmt/ostream.h>

#include <boost/stacktrace.hpp>
#define SPDLOG_FMT_EXTERNAL
#include <spdlog/spdlog.h>

#include "core/http/server/config/config.hpp"
#include "core/http/server/listener.hpp"
#include "core/http/server/session/session.hpp"

using boost::asio::awaitable;
using boost::asio::make_strand;

namespace http {
namespace server {

Listener::Listener(boost::asio::io_context &ioc,
                   boost::asio::ip::tcp::endpoint ep, const ConfigPtr &config,
                   const RouterPtr &router)
    : ioc_(ioc), acceptor_(boost::asio::make_strand(ioc), ep), config(config),
      routes(router) {
  spdlog::info("Construct http::Listener");
}

Listener::~Listener() { spdlog::info("http::Listener destructed!"); }

boost::asio::awaitable<void> Listener::Listen() {
  spdlog::info("RunServer http listening");
  for (;;) {
    try {
      auto client_socket =
          co_await acceptor_.async_accept(boost::asio::use_awaitable);
      boost::asio::co_spawn(
          boost::asio::make_strand(ioc_),
          RunSession(std::move(client_socket), config, routes),
          boost::asio::detached);
    } catch (const std::exception &exc) {
      spdlog::error("\nException while http listening:\nException message: "
                    "{}\nStacktrace: {}\n",
                    exc.what(), boost::stacktrace::stacktrace());
    }
  }
}

} // namespace server
} // namespace http
