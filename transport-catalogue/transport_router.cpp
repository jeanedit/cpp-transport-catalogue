
#include "transport_router.h"
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <iterator>
#include <optional>


namespace tr_catalogue::router {

		TransportGraph::TransportGraph(const TransportCatalogue& ts, domain::RoutingSettings route_settings) : transport_catalogue_(ts), 
			route_settings_(route_settings), 
			routes_graph_(ts.GetAllStops().size()) {
			InitializeVerticies();
			AddAllBusRoutes();
		}

		void TransportGraph::InitializeVerticies() {
			const auto& stops = transport_catalogue_.GetAllStops();
			graph::VertexId id = 0;
			for (const domain::Stop& stop : stops) {
				stops_to_vertices_.emplace(&stop, id);
				++id;
			}
		}

		void TransportGraph::AddBusRoute(const domain::Bus* bus) {
			const auto bus_route = GetStopPairsTimeSpanOnRoute(bus);
			for (auto& [stop_pair, time_span] : bus_route) {
				graph::VertexId from = stops_to_vertices_.at(stop_pair.first);
				graph::VertexId to = stops_to_vertices_.at(stop_pair.second);
				auto edge = graph::Edge<double>{ from, to, time_span.time + route_settings_.bus_wait_time };
				WaitEdge wait_edge{ stop_pair.first,route_settings_.bus_wait_time };
				BusEdge bus_edge{ bus,time_span.span_count,time_span.time };
				edgeid_to_combined_edge_.emplace(routes_graph_.AddEdge(edge), CombinedEdge{ wait_edge,bus_edge });

			}
		}

		void TransportGraph::AddAllBusRoutes() {
			const auto& buses = transport_catalogue_.GetAllBuses();
			for (const domain::Bus& bus : buses) {
				AddBusRoute(&bus);
			}
		}

		UnMapStopPairsTimeSpan TransportGraph::GetStopPairsTimeSpanOnRoute(const domain::Bus* bus) const {
			UnMapStopPairsTimeSpan result;
			FillUnMapWithTimeSpan(bus->route.begin(), bus->route.end(), result);
			if (!bus->is_loop) {
				FillUnMapWithTimeSpan(bus->route.rbegin(), bus->route.rend(), result);
			}

			return result;
		}


		inline double TransportGraph::ComputeTime(double distance, double velocity) {
			return distance / (velocity * 1000.0 / 60.0); // convert velocity from km/h to m/min
		}


		TransportRouter::TransportRouter(const TransportCatalogue& ts, domain::RoutingSettings route_settings) :graph_(ts, route_settings), router_(graph_.routes_graph_) {
		}

		std::optional<OptimalRoute> TransportRouter::BuildOptimalRoute(const domain::Stop* from, const domain::Stop* to) const {
			std::optional<OptimalRoute> optimal_route{ std::nullopt };
			graph::VertexId from_id = graph_.stops_to_vertices_.at(from);
			graph::VertexId to_id = graph_.stops_to_vertices_.at(to);
			if (auto router = router_.BuildRoute(from_id, to_id)) {
				OptimalRoute total_route{};
				total_route.total_time = router->weight;
				for (const auto edge_id : router->edges) {
					total_route.edges.push_back(graph_.edgeid_to_combined_edge_.at(edge_id));

				}
				optimal_route = std::make_optional(total_route);
			}
			return optimal_route;
		}

} // end of namespace tr_catalogue::router