#pragma once
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <iterator>
#include <optional>

namespace tr_catalogue::router {

	using UnMapStopPairsTimeSpan = std::unordered_map<domain::StopPair, domain::TimeBusSpan, domain::StopPairHasher>;

	struct WaitEdge {
		const domain::Stop* stop;
		double wait_time;
	};

	struct BusEdge {
		const domain::Bus* bus;
		size_t span_count;
		double total_time;
	};

	struct CombinedEdge {
		WaitEdge wait_edge;
		BusEdge bus_edge;
	};

	struct OptimalRoute {
		double total_time;
		std::vector<CombinedEdge> edges;
	};

	class TransportGraph {
	public:
		TransportGraph(const TransportCatalogue& ts, domain::RoutingSettings route_settings);

		void InitializeVerticies();

		void AddBusRoute(const domain::Bus* bus);

		void AddAllBusRoutes();

		UnMapStopPairsTimeSpan GetStopPairsTimeSpanOnRoute(const domain::Bus* bus) const;

		template<typename InputIt>
		void FillUnMapWithTimeSpan(InputIt begin, InputIt last, UnMapStopPairsTimeSpan& unmap) const;

		static inline double ComputeTime(double distance, double velocity);
		friend class TransportRouter;
	private:
		const TransportCatalogue& transport_catalogue_;
		domain::RoutingSettings route_settings_;

		graph::DirectedWeightedGraph<double> routes_graph_;

		std::unordered_map<const domain::Stop*, graph::VertexId> stops_to_vertices_;
		std::unordered_map<graph::EdgeId, CombinedEdge> edgeid_to_combined_edge_;
	};



	class TransportRouter {
	public:
		explicit TransportRouter(const TransportCatalogue& ts, domain::RoutingSettings route_settings);

		std::optional<OptimalRoute> BuildOptimalRoute(const domain::Stop* from, const domain::Stop* to) const;

	private:
		TransportGraph graph_;
		graph::Router<double> router_;

	};




	template<typename InputIt>
	void TransportGraph::FillUnMapWithTimeSpan(InputIt begin, InputIt last, UnMapStopPairsTimeSpan& unmap) const {
		for (auto from = begin; from != last; std::advance(from, 1)) {
			double time = 0.0;
			for (auto to = std::next(from); to != last; std::advance(to, 1)) {
				size_t distance = transport_catalogue_.GetDistanceBetweenStops(*std::prev(to), *to);
				time += ComputeTime(distance, route_settings_.bus_velocity);
				auto stop_pair = std::make_pair(*from, *to);
				domain::TimeBusSpan time_span = domain::TimeBusSpan{ time, static_cast<size_t>(std::distance(from, to)) };

				auto [_, is_insert] = unmap.emplace(stop_pair, time_span);

				if (!is_insert) {
					unmap[stop_pair] = std::min(unmap.at(stop_pair), time_span);
				}
			}
		}
	}


}