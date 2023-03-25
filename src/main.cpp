#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/format.hpp>
#include <iostream>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#include "cpu.hpp"
#include <fstream>

std::string_view get_path(std::string_view message) {
  auto first_line = message.substr(0, message.find("\n"));
  auto start = first_line.find("/");
  auto end = first_line.rfind(" HTTP");
  if (start == std::string_view::npos || end == std::string_view::npos) {
    return "/";
  }
  return message.substr(start, end - start);
}

void handle_request(tcp::socket &socket) {
  boost::asio::streambuf request;
  boost::asio::read_until(socket, request, "\r\n");

  std::string_view message =
      boost::asio::buffer_cast<const char *>(request.data());
  std::string_view path = get_path(message);

  std::ostringstream response;
  response << "HTTP/1.1 200 OK\r\n";

  if (path == "/styles.css") {
    std::ifstream css_file("src/styles.css");
    if (css_file.is_open()) {
      response << "Content-Type: text/css\r\n\r\n";
      response << css_file.rdbuf();
    } else {
      response << "Content-Type: text/plain\r\n\r\n";
      response << "Failed to open CSS file.";
    }
  } else if (path == "/index.js") {
    std::ifstream js_file("src/index.js");
    if (js_file.is_open()) {
      response << "Content-Type: application/javascript\r\n\r\n";
      response << js_file.rdbuf();
    } else {
      response << "Content-Type: text/plain\r\n\r\n";
      response << "Failed to open JS file.";
    }
  } else if (path == "/favicon.ico") {
    std::ifstream favicon_file("src/favicon.ico");
    if (favicon_file.is_open()) {
      response << "Content-Type: image/x-icon\r\n\r\n";
      response << favicon_file.rdbuf();
    } else {
      response << "Content-Type: text/plain\r\n\r\n";
      response << "Failed to open favicon.";
    }
  } else if (path == "/stats") {
    auto usages = get_cpu_usage();
    response << "Content-Type: application/json\r\n\r\n";
    response << R"({"usages":{)";
    int i = 0;
    for (auto usage : usages) {
      if (i == 0) {
        response << "\"overall"
                 << "\":" << usage << ",";
      } else if (i == usages.size() - 1) {
        response << "\"core " << i << "\":" << usage;
        break;
      } else {
        response << "\"core " << i << "\":" << usage << ",";
      }
      i++;
    }
    response << "}}\r\n\r\n";
  } else if (path == "/") {
    response << "Content-Type: text/html\r\n\r\n";
    response << "<!DOCTYPE html>\n";
    response << "<html>\n";
    response << "<head>\n";
    response
        << "<link rel=\"stylesheet\" type=\"text/css\" href=\"/styles.css\">\n";
    response << "<script src=\"/index.js\"></script>\n";
    response << "</head>\n";
    response << "<body>\n";
    response << "<h1>Hello, world!</h1>\n";
    response << "<p>Stats:</p>\n";
    response << "<ul id=\"usages\">\n";
    // Output the initial stats here
    response << "</ul>\n";
    response << "</body>\n";
    response << "</html>\n";
  } else {
    response << "Content-Type: text/plain\r\n\r\n";
    response << "Invalid request.";
  }

  boost::asio::write(socket, boost::asio::buffer(response.str()));
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
