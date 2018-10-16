#pragma once

#include <atomic>
#include <experimental/optional>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include <fmt/format.h>
#include <glog/logging.h>

#include "communication/init.hpp"
#include "communication/listener.hpp"
#include "io/network/socket.hpp"
#include "utils/thread.hpp"

namespace communication {

/**
 * Communication server.
 *
 * Listens for incoming connections on the server port and assigns them to the
 * connection listener. The listener processes the events with a thread pool
 * that has `num_workers` threads. It is started automatically on constructor,
 * and stopped at destructor.
 *
 * Current Server achitecture:
 * incoming connection -> server -> listener -> session
 *
 * NOTE: If you use this server you **must** call `communication::Init()` from
 * the `main` function before using the server!
 *
 * @tparam TSession the server can handle different Sessions, each session
 *         represents a different protocol so the same network infrastructure
 *         can be used for handling different protocols
 * @tparam TSessionData the class with objects that will be forwarded to the
 *         session
 */
template <typename TSession, typename TSessionData>
class Server final {
 public:
  using Socket = io::network::Socket;

  /**
   * Constructs and binds server to endpoint, operates on session data and
   * invokes workers_count workers
   */
  Server(const io::network::Endpoint &endpoint, TSessionData *session_data,
         ServerContext *context, int inactivity_timeout_sec,
         const std::string &service_name,
         size_t workers_count = std::thread::hardware_concurrency())
      : alive_(false),
        endpoint_(endpoint),
        listener_(session_data, context, inactivity_timeout_sec, service_name,
                  workers_count),
        service_name_(service_name) {}

  ~Server() {
    CHECK(!alive_ && !thread_.joinable()) << "You should call Shutdown and "
                                             "AwaitShutdown on "
                                             "communication::Server!";
  }

  Server(const Server &) = delete;
  Server(Server &&) = delete;
  Server &operator=(const Server &) = delete;
  Server &operator=(Server &&) = delete;

  const auto &endpoint() const {
    CHECK(alive_) << "You can't get the server endpoint when it's not running!";
    return socket_.endpoint();
  }

  /// Starts the server
  bool Start() {
    CHECK(!alive_) << "The server was already started!";
    alive_.store(true);

    if (!socket_.Bind(endpoint_)) {
      LOG(ERROR) << "Cannot bind to socket on " << endpoint_;
      alive_.store(false);
      return false;
    }
    socket_.SetTimeout(1, 0);
    if (!socket_.Listen(1024)) {
      LOG(ERROR) << "Cannot listen on socket!";
      alive_.store(false);
      return false;
    }

    listener_.Start();

    std::string service_name(service_name_);
    thread_ = std::thread([this, service_name]() {
      utils::ThreadSetName(fmt::format("{} server", service_name));

      std::cout << service_name_ << " server is fully armed and operational"
                << std::endl;
      std::cout << service_name_ << " listening on " << socket_.endpoint()
                << std::endl;

      while (alive_) {
        AcceptConnection();
      }

      std::cout << service_name << " shutting down..." << std::endl;
    });

    return true;
  }

  /// Signals the server to start shutting down
  void Shutdown() {
    // This should be as simple as possible, so that it can be called inside a
    // signal handler.
    alive_.store(false);
    // Shutdown the socket to return from any waiting `Accept` calls.
    socket_.Shutdown();
    // Shutdown the listener.
    listener_.Shutdown();
  }

  /// Waits for the server to be signaled to shutdown
  void AwaitShutdown() {
    if (thread_.joinable()) thread_.join();
    listener_.AwaitShutdown();
  }

 private:
  void AcceptConnection() {
    // Accept a connection from a socket.
    auto s = socket_.Accept();
    if (!s) {
      // Connection is not available anymore or configuration failed.
      return;
    }
    LOG(INFO) << "Accepted a " << service_name_ << " connection from "
              << s->endpoint();
    listener_.AddConnection(std::move(*s));
  }

  std::atomic<bool> alive_;
  std::thread thread_;

  Socket socket_;
  io::network::Endpoint endpoint_;
  Listener<TSession, TSessionData> listener_;

  const std::string service_name_;
};

}  // namespace communication
