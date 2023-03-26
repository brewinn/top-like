#include <boost/asio.hpp>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#include "cpu.hpp"
#include "web.hpp"
#include <fstream>

int main() {
  // Initialize a network context and an endpoint to listen to
  net::io_context io{1};
  tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 8080));

  // For each connection, create a socket and handle the request.
  while (true) {
    tcp::socket socket(io);
    acceptor.accept(socket);

    handle_request(socket);
  }

  return EXIT_SUCCESS;
}
