//
// Created by gaojian on 2021/12/19.
//

#ifndef NETPP_ROUTE_H
#define NETPP_ROUTE_H

#include <functional>
#include <string>
#include <map>
#include <mutex>

namespace netpp::http {
class Httprequest;
class Route {
	using RouteHandler = std::function<void(Httprequest)>;
public:
	Route();
	void addRoute(const char *route, RouteHandler handler);
	void addRoute(const std::string &route, RouteHandler handler);
	[[nodiscard]] RouteHandler findRequest(const char *route) const;
	[[nodiscard]] RouteHandler findRequest(const std::string &route) const;

private:
	mutable std::mutex m_mutex;
	std::map<std::string, RouteHandler> m_routes;
};
}

#endif //NETPP_ROUTE_H
