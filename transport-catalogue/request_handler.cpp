#include "request_handler.h"
#include "domain.h"


namespace tr_catalogue {
        using namespace domain;
    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStats> RequestHandler::GetBusStat(const std::string_view& bus_name) const{
         BusStats bus_stats;
		const Bus* bus = db_.FindBus(bus_name);
		if (bus == nullptr) {
			return std::nullopt;
		}
		bus_stats.bus_name = bus->name;
		bus_stats.stops = bus->is_loop ? bus->route.size() : bus->route.size() * 2 - 1;
		bus_stats.unique_stops = detail::GetUniqueStops(bus->route);

		bus_stats.route_length = db_.ComputeActualRouteLength(bus);
		bus_stats.curvature = static_cast<double>(db_.ComputeActualRouteLength(bus)) / db_.ComputeGeoRouteLength(bus);
		return std::optional<BusStats>(bus_stats);
    }

    // Возвращает маршруты, проходящие через
    const std::set<std::string_view>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
        if (db_.StopToBuses().count(stop_name)) {
			return &db_.StopToBuses().at(stop_name);
		}
		return nullptr;
    }
    
    svg::Document RequestHandler::RenderMap() const{
        return renderer_.RenderRoutes(db_.GetAllBuses(),db_.StopsWithBus());
    }

    }