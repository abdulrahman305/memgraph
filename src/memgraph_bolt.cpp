#include <signal.h>
#include <iostream>

#include "communication/bolt/v1/server/server.hpp"
#include "communication/bolt/v1/server/worker.hpp"

#include "io/network/socket.hpp"

#include "logging/default.hpp"
#include "logging/streams/stdout.hpp"

#include "utils/config/config.hpp"
#include "utils/signals/handler.hpp"
#include "utils/stacktrace.hpp"
#include "utils/terminate_handler.hpp"

static bolt::Server<bolt::Worker>* serverptr;

Logger logger;

static constexpr const char* interface = "0.0.0.0";
static constexpr const char* port = "7687";

void throw_and_stacktace(std::string message) {
  Stacktrace stacktrace;
  logger.info(stacktrace.dump());
}

int main(void) {
  // TODO figure out what is the relationship between this and signals
  // that are configured below
  std::set_terminate(&terminate_handler);

// logger init
#ifdef SYNC_LOGGER
  logging::init_sync();
#else
  logging::init_async();
#endif
  logging::log->pipe(std::make_unique<Stdout>());

  // get Main logger
  logger = logging::log->logger("Main");
  logger.info("{}", logging::log->type());

  SignalHandler::register_handler(Signal::SegmentationFault, []() {
    throw_and_stacktace("SegmentationFault signal raised");
    exit(1);
  });

  SignalHandler::register_handler(Signal::Terminate, []() {
    throw_and_stacktace("Terminate signal raised");
    exit(1);
  });

  SignalHandler::register_handler(Signal::Abort, []() {
    throw_and_stacktace("Abort signal raised");
    exit(1);
  });

  // CONFIG call

  io::Socket socket;

  try {
    socket = io::Socket::bind(interface, port);
  } catch (io::NetworkError e) {
    logger.error("Cannot bind to socket on {} at {}", interface, port);
    logger.error("{}", e.what());

    std::exit(EXIT_FAILURE);
  }

  socket.set_non_blocking();
  socket.listen(1024);

  logger.info("Listening on {} at {}", interface, port);

  bolt::Server<bolt::Worker> server(std::move(socket));
  serverptr = &server;

  // TODO: N should be configurable
  auto N = std::thread::hardware_concurrency();
  logger.info("Starting {} workers", N);
  server.start(N);

  logger.info("Shutting down...");

  return EXIT_SUCCESS;
}
