#include "request_handler.h"
#include "domain.h"


namespace tr_catalogue {
     using namespace domain;
     using namespace router;

    std::optional<BusStats> RequestHandler::GetBusStat(std::string_view bus_name) const{
         BusStats bus_stats;
		const Bus* bus = tc_.FindBus(bus_name);
		if (bus == nullptr) {
			return std::nullopt;
		}
		bus_stats.bus_name = bus->name;
		bus_stats.stops = bus->is_loop ? bus->route.size() : bus->route.size() * 2 - 1;
		bus_stats.unique_stops = detail::GetUniqueStops(bus->route);

		bus_stats.route_length = tc_.ComputeActualRouteLength(bus);
		bus_stats.curvature = static_cast<double>(tc_.ComputeActualRouteLength(bus)) / tc_.ComputeGeoRouteLength(bus);
		return std::optional<BusStats>(bus_stats);
    }

    const std::set<std::string_view>* RequestHandler::GetBusesByStop(std::string_view stop_name) const{
        if (tc_.StopToBuses().count(stop_name)) {
			return &tc_.StopToBuses().at(stop_name);
		}
		return nullptr;
    }

	std::optional<OptimalRoute> RequestHandler::GetOptimalRoute(std::string_view from, std::string_view to) const {
		return tr_.BuildOptimalRoute(tc_.FindStop(from), tc_.FindStop(to));
	}

    svg::Document RequestHandler::RenderMap() const{
        return renderer_.RenderRoutes(tc_.GetAllBuses(),tc_.StopsWithBus());
    }

} // end of namespace tr_catalogue