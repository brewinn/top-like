#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/format.hpp>

namespace http = boost::beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#include "cpu.hpp"
#include <fstream>

std::string html_wrap(std::string cpu, double usage) {
  auto fmt = boost::format(R"(
<div class="bar">
  <div class="bar-inner" style="width: %2$.2f"></div>
  <label>%1%:%%%2$.2f</label>
</div>)") % cpu %
             usage;
  return fmt.str();
}

std::string format_stats(std::vector<double> &&stats) {
  if (stats.size() < 1) {
    return "";
  }

  std::string result = html_wrap("Overall Usage", stats[0]) + "\n";
  auto individual_cpus = std::vector<int>(stats.begin() + 1, stats.end());
  int i = 0;
  for (auto usage : individual_cpus) {
    result += html_wrap("core " + std::to_string(i), usage) + "\n";
    i++;
  }
  return result;
}

http::request<http::string_body> parse_request(tcp::socket &socket) {
  boost::asio::streambuf buffer;
  boost::asio::read_until(socket, buffer, "\r\n\r\n");

  std::string data = boost::asio::buffer_cast<const char *>(buffer.data());
  http::request<http::string_body> request;
  request.method(http::verb::get);
  request.version(11);
  request.target("/");
  request.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  // Parse the HTTP request
  boost::system::error_code error;
  http::read(socket, buffer, request, error);
  if (error == boost::asio::error::eof) {
    // Connection closed by peer
    boost::system::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);
  } else if (error) {
    // Error while reading request
    throw boost::system::system_error(error);
  }

  return request;
}

void handle_request(tcp::socket &socket) {
  // Respond to GET requests with the generated stats
  auto req = parse_request(socket);
  if (req.method() == http::verb::get && req.target() == "/") {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    auto html_body = format_stats(get_cpu_usage());
    auto body = boost::format(R"(
<!DOCTYPE html>
<html>
<head>
    <title>Generated Stats</title>
    <link rel="stylesheet" type="text/css" href="styles.css">
    <link rel="shortcut icon" href="/favicon.ico" />
</head>
<body>
    %1%
</body>
</html>
)") % html_body;
    res.body() = body.str();
    res.prepare_payload();

    http::write(socket, res);
  } else if (req.method() == http::verb::get && req.target() == "/styles.css") {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.keep_alive(req.keep_alive());
    std::ifstream css_file("styles.css");
    if (css_file.is_open()) {
      std::stringstream ss;
      ss << css_file.rdbuf();
      res.body() = ss.str();
    }
    res.prepare_payload();
    res.set(http::field::content_type, "text/css");

    http::write(socket, res);
  } else if (req.method() == http::verb::get &&
             req.target() == "/favicon.ico") {
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.keep_alive(req.keep_alive());
    std::ifstream favicon_file("favicon.ico");
    if (favicon_file.is_open()) {
      std::stringstream ss;
      ss << favicon_file.rdbuf();
      res.body() = ss.str();
    }
    res.prepare_payload();
    res.set(http::field::content_type, "image/x-icon");

    http::write(socket, res);
  } else {
    // Respond to other requests with an error
    http::response<http::string_body> res{http::status::bad_request,
                                          req.version()};
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/plain");
    res.keep_alive(req.keep_alive());
    res.body() = "Invalid request";
    res.prepare_payload();

    http::write(socket, res);
  }
}

int main() {
  net::io_context io{1};
  tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 8080));
  while (true) {
    tcp::socket socket(io);
    acceptor.accept(socket);

    handle_request(socket);
  }
  return 0;
}
