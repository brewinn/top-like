#include <boost/asio.hpp>
#include <functional>
#include <unordered_map>

namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

#include "cpu.hpp"
#include "web.hpp"
#include <fstream>

#define CSS_PATH "src/styles.css"
#define JS_PATH "src/index.js"
#define FAVICON_PATH "src/favicon.ico"

#define HOME "/"
#define CSS "/styles.css"
#define JS "/index.js"
#define FAVICON "/favicon.ico"
#define STATS "/stats"

static std::unordered_map<std::string_view,
                          std::function<void(std::ostringstream &)>>
    route_map{{HOME, home_route},
              {CSS, css_route},
              {JS, js_route},
              {FAVICON, favicon_route},
              {STATS, stats_route}};

std::string_view get_path(std::string_view message) {
  auto first_line = message.substr(0, message.find("\n"));
  auto start = first_line.find("/");
  auto end = first_line.rfind(" HTTP");
  if (start == std::string_view::npos || end == std::string_view::npos) {
    return HOME;
  }
  return message.substr(start, end - start);
}

void css_route(std::ostringstream &response) {
  std::ifstream css_file(CSS_PATH);
  if (css_file.is_open()) {
    response << "Content-Type: text/css\r\n\r\n";
    response << css_file.rdbuf();
  } else {
    response << "Content-Type: text/plain\r\n\r\n";
    response << "Failed to open CSS file.";
  }
}

void js_route(std::ostringstream &response) {
  std::ifstream js_file(JS_PATH);
  if (js_file.is_open()) {
    response << "Content-Type: application/javascript\r\n\r\n";
    response << js_file.rdbuf();
  } else {
    response << "Content-Type: text/plain\r\n\r\n";
    response << "Failed to open JS file.";
  }
}

void favicon_route(std::ostringstream &response) {
  std::ifstream favicon_file(FAVICON_PATH);
  if (favicon_file.is_open()) {
    response << "Content-Type: image/x-icon\r\n\r\n";
    response << favicon_file.rdbuf();
  } else {
    response << "Content-Type: text/plain\r\n\r\n";
    response << "Failed to open favicon.";
  }
}

void stats_route(std::ostringstream &response) {
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
}

void home_route(std::ostringstream &response) {
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
  // Initial stats will be populated by JS
  response << "</ul>\n";
  response << "</body>\n";
  response << "</html>\n";
}

void invalid_route(std::ostringstream &response) {
  response << "Content-Type: text/plain\r\n\r\n";
  response << "Invalid request.";
}

void handle_request(tcp::socket &socket) {
  net::streambuf request;
  net::read_until(socket, request, "\r\n");

  std::string_view message = net::buffer_cast<const char *>(request.data());
  std::string_view path = get_path(message);

  std::ostringstream response;
  response << "HTTP/1.1 200 OK\r\n";

  try {
    route_map.at(path)(response);
  } catch (std::out_of_range) {
    invalid_route(response);
  }

  net::write(socket, boost::asio::buffer(response.str()));
}
