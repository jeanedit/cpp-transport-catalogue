#pragma once
#include "graph.h"
#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <iterator>
#include <optional>

namespace tr_catalogue::router {

	using UnorderMapStopPairsTimeSpan = std::unordered_map<domain::StopPair, domain::TimeBusSpan, domain::StopPairHasher>;

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
		explicit TransportGraph(const TransportCatalogue& ts, domain::RoutingSettings route_settings);
		explicit TransportGraph(const TransportCatalogue& ts, domain::RoutingSettings&& route_settings, graph::DirectedWeightedGraph<double>&& graph)
			:transport_catalogue_(ts),
			route_settings_(std::move(route_settings)), 
			routes_graph_(std::move(graph))
		{
		}

		void FillGraphWithStops();

		void AddBusRoute(const domain::Bus* bus);

		void FillGraphWithBusRoutes();

		UnorderMapStopPairsTimeSpan GetStopPairsTimeSpanOnRoute(const domain::Bus* bus) const;

		template<typename InputIt>
		void FillUnorderMapWithTimeSpan(InputIt begin, InputIt last, UnorderMapStopPairsTimeSpan& unmap) const;

		static inline double ComputeTime(double distance, double velocity);


		inline const TransportCatalogue& GetTransportCatalogue() const {
			return transport_catalogue_;
		}

		inline const domain::RoutingSettings& GetRoutingSettings() const {
			return route_settings_;
		}

		inline const graph::DirectedWeightedGraph<double>& GetRoutesGraph() const {
			return routes_graph_;
		}

		inline const std::unordered_map<const domain::Stop*, graph::VertexId>& GetStopsToVertices() const {
			return stops_to_vertices_;
		}

		inline const std::unordered_map<graph::EdgeId, CombinedEdge>& GetEdgeIdToEdge() const {
			return edgeid_to_combined_edge_;
		}

		inline graph::DirectedWeightedGraph<double>& GetRoutesGraph() {
			return routes_graph_;
		}

		inline std::unordered_map<const domain::Stop*, graph::VertexId>& GetStopsToVertices(){
			return stops_to_vertices_;
		}

		inline std::unordered_map<graph::EdgeId, CombinedEdge>& GetEdgeIdToEdge(){
			return edgeid_to_combined_edge_;
		}

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

		explicit TransportRouter(TransportGraph&& transport_graph,graph::Router<double>::RoutesInternalData&& routes_internal_data)
			:graph_(std::move(transport_graph)),
			router_(graph_.GetRoutesGraph(),
			std::move(routes_internal_data))
		{
		}

		std::optional<OptimalRoute> BuildOptimalRoute(const domain::Stop* from, const domain::Stop* to) const;

		inline const TransportGraph& GetTransportGraph() const {
			return graph_;
		}

		inline const graph::Router<double>& GetRouter() const {
			return router_;
		}

		inline graph::Router<double>& GetRouter() {
			return router_;
		}

		TransportRouter(TransportRouter&& transport_router) noexcept
			:graph_(std::move(transport_router.graph_)),
			router_(graph_.GetRoutesGraph(), std::move(transport_router.GetRouter().GetRoutesInternalData()))
		{
		}

		TransportRouter(const TransportRouter& transport_router)
			:graph_(transport_router.graph_),
			router_(graph_.GetRoutesGraph(),transport_router.GetRouter().GetRoutesInternalData())
		{
		}

		TransportRouter operator=(const TransportRouter& transport_router) = delete;
		TransportRouter operator=(TransportRouter&& transport_router) = delete;

	private:
		TransportGraph graph_;
		graph::Router<double> router_;

	};




	template<typename InputIt>
	void TransportGraph::FillUnorderMapWithTimeSpan(InputIt begin, InputIt last, UnorderMapStopPairsTimeSpan& unmap) const {
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