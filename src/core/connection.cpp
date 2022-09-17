#include <optional>

#define SPDLOG_FMT_EXTERNAL
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "core/http/connection.hpp"
#include <memory>

namespace http {

Connection::Connection(boost::asio::ip::tcp::socket &&socket,
                       unsigned int timeout)
    : stream_(std::move(socket)), is_alive_(true), timeout_(timeout) {
  stream_.expires_after(std::chrono::seconds(timeout_));
  spdlog::info("Connection opened");
}

Connection::~Connection() { spdlog::info("Connection destructed"); }

bool Connection::IsAlive() const { return is_alive_; }

void Connection::Disconnect() { is_alive_ = false; }

boost::asio::awaitable<std::optional<ConnectionPtr>>
ConnectTo(const std::string &url) {
  auto executor = co_await boost::asio::this_coro::executor;
  boost::asio::ip::tcp::resolver resolver(executor);
  try {
    auto enabled_ips =
        co_await resolver.async_resolve(url, boost::asio::use_awaitable);

    boost::asio::ip::tcp::socket socket(executor);

    co_await boost::asio::async_connect(socket, enabled_ips,
                                        boost::asio::use_awaitable);

    co_return std::make_optional<ConnectionPtr>(
        std::make_unique<Connection>(std::move(socket)));
  } catch (const std::exception &exc) {
    spdlog::error(
        "Exception when trying to connect to {}\n Exception message: {}", url,
        exc.what());
    co_return std::nullopt;
  }
}

boost::asio::awaitable<std::optional<ConnectionPtr>>
ConnectTo(std::string &&url) {
  auto res = co_await ConnectTo(url);
  co_return res;
}

boost::asio::awaitable<std::optional<ConnectionPtr>>
ConnectTo(std::string_view url) {
  auto executor = co_await boost::asio::this_coro::executor;
}

} // namespace http
