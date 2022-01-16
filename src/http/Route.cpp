//
// Created by gaojian on 2021/12/19.
//

#include "http/Route.h"

namespace netpp::http {
Route::Route() = default;

void Route::addRoute(const char *route, RouteHandler request)
{
	std::lock_guard lck(m_mutex);
	m_routes.emplace(std::string(route), std::move(request));
}

void Route::addRoute(const std::string &route, RouteHandler request)
{
	std::lock_guard lck(m_mutex);
	m_routes.emplace(route, std::move(request));
}

Route::RouteHandler Route::findRequest(const char *route) const
{
	std::lock_guard lck(m_mutex);
	auto it = m_routes.find(std::string(route));
	if (it != m_routes.end())
		return it->second;
	return {};
}

Route::RouteHandler Route::findRequest(const std::string &route) const
{
	std::lock_guard lck(m_mutex);
	auto it = m_routes.find(route);
	if (it != m_routes.end())
		return it->second;
	return {};
}
}
