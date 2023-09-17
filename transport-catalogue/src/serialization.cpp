#pragma once

#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>
#include <map_renderer.pb.h>
#include <fstream>
#include <iostream>
#include <vector>

#include "geo.h"
#include "serialization.h"


namespace tr_catalogue {

	namespace util {
		void SetProtoColor(proto_svg::Color* proto_color, const svg::Color& color) {
			if (std::holds_alternative<std::monostate>(color)) {
				proto_color->set_is_monostate(true);
			}
			else if (std::holds_alternative<std::string>(color)) {
				proto_color->set_color(std::get<std::string>(color));
			}
			else if (std::holds_alternative<svg::Rgb>(color)) {
				svg::Rgb rgb = std::get<svg::Rgb>(color);
				proto_svg::Rgb* proto_rgb = proto_color->mutable_rgb();
				proto_rgb->set_red(rgb.red);
				proto_rgb->set_green(rgb.green);
				proto_rgb->set_blue(rgb.blue);
			}
			else {
				svg::Rgba rgba = std::get<svg::Rgba>(color);
				proto_svg::Rgba* proto_rgba = proto_color->mutable_rgba();
				proto_rgba->mutable_rgb()->set_red(rgba.red);
				proto_rgba->mutable_rgb()->set_green(rgba.green);
				proto_rgba->mutable_rgb()->set_blue(rgba.blue);
				proto_rgba->set_opacity(rgba.opacity);
			}
		}


		svg::Color ColorFromProto(const proto_svg::Color& proto_color) {
			svg::Color color;

			if (proto_color.is_monostate()) {
				color = svg::NoneColor;
			}
			else if (proto_color.format_case() == proto_svg::Color::FormatCase::kColor) {
				color = proto_color.color();
			}
			else if (proto_color.format_case() == proto_svg::Color::FormatCase::kRgb) {
				color = svg::Rgb{
					static_cast<uint8_t>(proto_color.rgb().red()),
					static_cast<uint8_t>(proto_color.rgb().green()),
					static_cast<uint8_t>(proto_color.rgb().blue())
				};
			}
			else {
				color = svg::Rgba{
					static_cast<uint8_t>(proto_color.rgba().rgb().red()),
					static_cast<uint8_t>(proto_color.rgba().rgb().green()),
					static_cast<uint8_t>(proto_color.rgba().rgb().blue()),
					proto_color.rgba().opacity()
				};
			}
			return color;
		}
	} // end of namespace util


	void SerializeTransportCatalogue(std::ostream& out, const TransportCatalogue& ts) {
		proto_transport_catalogue::TransportCatalogue  ts_out;

		for (const auto& stop : ts.GetAllStops()) {
			proto_transport_catalogue::Stop* added_stop = ts_out.add_stops();
			added_stop->set_name(stop.name);
			added_stop->set_lat(stop.geo_coords.lat);
			added_stop->set_lng(stop.geo_coords.lng);
		}

		for (const auto& bus : ts.GetAllBuses()) {
			proto_transport_catalogue::Bus* added_bus = ts_out.add_buses();
			added_bus->set_name(bus.name);
			added_bus->set_is_loop(bus.is_loop);
			for (const domain::Stop* stop : bus.route) {
				added_bus->add_stop_ids(ts.GetIndexByStop(stop));
			}

		}

		for (const auto& [stop_pair,distance] : ts.GetStopPairsDistances()) {
			proto_transport_catalogue::StopPairDistance* added_stop_pair = ts_out.add_distances();
			added_stop_pair->set_stop_from(ts.GetIndexByStop(stop_pair.first));
			added_stop_pair->set_stop_to(ts.GetIndexByStop(stop_pair.second));
			added_stop_pair->set_distance(distance);
		}

		ts_out.SerializeToOstream(&out);
	}

	std::optional<TransportCatalogue> DeserializeTransportCatalogue(const domain::SerializationSettings& ss) {
		proto_transport_catalogue::TransportCatalogue ts_in;
		TransportCatalogue ts;
		std::ifstream in(ss.file, std::ios::binary);
		if (!ts_in.ParseFromIstream(&in)) {
			return std::nullopt;
		}
		for (auto&& stop_in : ts_in.stops()) {
			domain::Stop stop;
			stop.name = stop_in.name();
			stop.geo_coords = { stop_in.lat(),stop_in.lng() };
			ts.AddStop(std::move(stop));
		}

		for (auto&& bus_in : ts_in.buses()) {
			domain::Bus bus;
			bus.name = bus_in.name();
			bus.is_loop = bus_in.is_loop();
			for (const auto& stop : bus_in.stop_ids()) {
				bus.route.push_back(&ts.GetAllStops()[stop]);
			}
			ts.AddBus(std::move(bus));
		}


		for (const auto& stop_pair : ts_in.distances()) {
			ts.AddStopPairsDistances(&ts.GetAllStops()[stop_pair.stop_from()], &ts.GetAllStops()[stop_pair.stop_to()], stop_pair.distance());
		}

		return { std::move(ts) };
	}


	void SerializeMapRenderer(std::ostream& out, const renderer::RenderSettings& rs, const std::deque<domain::Bus>& buses) {
		proto_renderer::MapRenderer map_renderer;
		map_renderer.mutable_render_settings()->set_width(rs.width);
		map_renderer.mutable_render_settings()->set_height(rs.height);
		map_renderer.mutable_render_settings()->set_padding(rs.padding);
		map_renderer.mutable_render_settings()->set_line_width(rs.line_width);
		map_renderer.mutable_render_settings()->set_stop_radius(rs.stop_radius);
		map_renderer.mutable_render_settings()->set_bus_label_font_size(rs.bus_label_font_size);
		map_renderer.mutable_render_settings()->set_stop_label_font_size(rs.stop_label_font_size);
		map_renderer.mutable_render_settings()->set_underlayer_width(rs.underlayer_width);

		proto_svg::Point* proto_point = map_renderer.mutable_render_settings()->mutable_bus_label_offset();
		proto_point->set_x(rs.bus_label_offset.x);
		proto_point->set_y(rs.bus_label_offset.y);

		proto_point = map_renderer.mutable_render_settings()->mutable_stop_label_offset();
		proto_point->set_x(rs.stop_label_offset.x);
		proto_point->set_y(rs.stop_label_offset.y);

		proto_svg::Color* proto_color = map_renderer.mutable_render_settings()->mutable_underlayer_color();
		util::SetProtoColor(proto_color, rs.underlayer_color);

		for (const auto& color : rs.color_palette) {
			proto_color = map_renderer.mutable_render_settings()->add_color_palette();
			util::SetProtoColor(proto_color, color);
		}

		for (const domain::Bus& bus : buses) {
			for (const domain::Stop* stop : bus.route) {
				proto_renderer::Coordinates* coords = map_renderer.add_stops_coords();
				coords->set_lat(stop->geo_coords.lat);
				coords->set_lng(stop->geo_coords.lng);
			}
		}

		map_renderer.SerializeToOstream(&out);
	}


	std::optional<renderer::MapRenderer> DeserializeMapRenderer(const domain::SerializationSettings& ss) {
		proto_renderer::MapRenderer map_renderer;


		std::ifstream in(ss.file,std::ios::binary);
		if (!map_renderer.ParseFromIstream(&in)) {
			return std::nullopt;
		}

		renderer::RenderSettings rs;
		std::vector<geo::Coordinates> coords;

		rs.width = map_renderer.render_settings().width();
		rs.height = map_renderer.render_settings().height();
		rs.padding = map_renderer.render_settings().padding();
		rs.line_width = map_renderer.render_settings().line_width();
		rs.stop_radius = map_renderer.render_settings().stop_radius();
		rs.bus_label_font_size = map_renderer.render_settings().bus_label_font_size();
		rs.stop_label_font_size = map_renderer.render_settings().stop_label_font_size();
		rs.underlayer_width = map_renderer.render_settings().underlayer_width();

		rs.bus_label_offset = svg::Point{ map_renderer.render_settings().bus_label_offset().x(),
										  map_renderer.render_settings().bus_label_offset().y() };

		rs.stop_label_offset = svg::Point{ map_renderer.render_settings().stop_label_offset().x(),
										   map_renderer.render_settings().stop_label_offset().y() };


		rs.underlayer_color = util::ColorFromProto(map_renderer.render_settings().underlayer_color());

		for (const auto& proto_color : map_renderer.render_settings().color_palette()) {
			rs.color_palette.push_back(util::ColorFromProto(proto_color));
		}

		coords.reserve(map_renderer.stops_coords().size());
		for (const auto& proto_coords : map_renderer.stops_coords()) {
			coords.push_back({ proto_coords.lat(), proto_coords.lng() });
		}


		return {std::move(renderer::MapRenderer (std::move(rs), coords))};
	}


	void SerializeTransportRouter(std::ostream& out, const router::TransportRouter& tr) {
		proto_router::TransportRouter tr_out;
		tr_out.mutable_graph()->mutable_routing_settings()->set_bus_velocity(tr.GetTransportGraph().GetRoutingSettings().bus_velocity);
		tr_out.mutable_graph()->mutable_routing_settings()->set_bus_wait_time(tr.GetTransportGraph().GetRoutingSettings().bus_wait_time);
	
		const auto& graph = tr.GetTransportGraph().GetRoutesGraph();

		for (size_t id = 0; id < graph.GetEdgeCount(); ++id) {
			proto_graph::Edge* edge = tr_out.mutable_graph()->mutable_routes_graph()->add_edges();
			edge->set_from(graph.GetEdge(id).from);
			edge->set_to(graph.GetEdge(id).to);
			edge->set_weight(graph.GetEdge(id).weight);

		}

		const auto& stops_to_vertices = tr.GetTransportGraph().GetStopsToVertices();
		for (const auto& [stop_ptr, vertex_id] : stops_to_vertices) {
			size_t stop_id = tr.GetTransportGraph().GetTransportCatalogue().GetIndexByStop(stop_ptr);
			auto* proto_map = tr_out.mutable_graph()->mutable_stops_to_vertices();
			(*proto_map)[stop_id] = vertex_id;
		}
	
		const auto& edgeid_to_edge = tr.GetTransportGraph().GetEdgeIdToEdge();

		for (const auto& [edge_id, edge] : edgeid_to_edge) {
			size_t bus_id = tr.GetTransportGraph().GetTransportCatalogue().GetIndexByBus(edge.bus_edge.bus);
			size_t stop_id = tr.GetTransportGraph().GetTransportCatalogue().GetIndexByStop(edge.wait_edge.stop);
	
			proto_router::CombinedEdge proto_edge;
			proto_edge.mutable_bus_edge()->set_bus(bus_id);
			proto_edge.mutable_bus_edge()->set_span_count(edge.bus_edge.span_count);
			proto_edge.mutable_bus_edge()->set_total_time(edge.bus_edge.total_time);
			proto_edge.mutable_wait_edge()->set_stop(stop_id);
			proto_edge.mutable_wait_edge()->set_wait_time(edge.wait_edge.wait_time);
			auto* proto_map = tr_out.mutable_graph()->mutable_edgeid_to_combined_edge();
			(*proto_map)[edge_id] = proto_edge;
		}


		const auto& routes_internal_data = tr.GetRouter().GetRoutesInternalData();
		for (size_t i = 0; i < routes_internal_data.size(); ++i) {
			proto_graph::ArrayRouteInternalData* proto_routes_internal_data = tr_out.mutable_router()->add_matrix_data();
			for (size_t j = 0; j < routes_internal_data[i].size(); ++j) {
				proto_graph::OptionalRouteInternalData* opt_data = proto_routes_internal_data->add_array_data();
				if (routes_internal_data[i][j]) {
					opt_data->set_is_route_internal_data(true);
					opt_data->mutable_route_internal_data()->set_weight(routes_internal_data[i][j]->weight);
					if (routes_internal_data[i][j]->prev_edge) {
						opt_data->mutable_route_internal_data()->set_prev_edge(*routes_internal_data[i][j]->prev_edge);
						opt_data->mutable_route_internal_data()->set_is_prev_edge(true);
					}
				}
			}
		}
		tr_out.SerializeToOstream(&out);
	}

	std::optional<router::TransportRouter> DeserializeTransportRouter(const domain::SerializationSettings& ss, const TransportCatalogue& ts) {
		proto_router::TransportRouter tr_in;

		std::ifstream in(ss.file, std::ios::binary);

		if (!tr_in.ParseFromIstream(&in)) {
			return std::nullopt;
		}

		/*1. Parse Routing Settings */
		domain::RoutingSettings routing_settings;
		routing_settings.bus_velocity = tr_in.graph().routing_settings().bus_velocity();
		routing_settings.bus_wait_time = tr_in.graph().routing_settings().bus_wait_time();

		/*2. Parse Routes Graph */
		graph::DirectedWeightedGraph<double> routes_graph(ts.GetAllStops().size());

		const auto& proto_graph = tr_in.graph().routes_graph();

		for (size_t id = 0; id < proto_graph.edges_size(); ++id) {
			graph::Edge<double> edge;
			edge.from = proto_graph.edges()[id].from();
			edge.to = proto_graph.edges()[id].to();
			edge.weight = proto_graph.edges()[id].weight();
			routes_graph.AddEdge(edge);
		}

		/*3. Create Transport Graph using Routes Graph */
		router::TransportGraph tr_graph(ts, std::move(routing_settings), std::move(routes_graph));

		/*4. Parse Stops To Vertices Map */
		auto& stops_to_vertices = tr_graph.GetStopsToVertices();
		for (const auto& [stop_id, vertex_id] : tr_in.graph().stops_to_vertices()) {
			stops_to_vertices[&ts.GetAllStops()[stop_id]] = vertex_id;
		}

		/*5. Parse EdgeId to Edge Map */
		auto& edgeid_to_edge = tr_graph.GetEdgeIdToEdge();
		for (const auto& [edge_id, proto_edge] : tr_in.graph().edgeid_to_combined_edge()) {

			router::CombinedEdge comb_edge;
			comb_edge.bus_edge.bus = &ts.GetAllBuses()[proto_edge.bus_edge().bus()];
			comb_edge.bus_edge.span_count = proto_edge.bus_edge().span_count();
			comb_edge.bus_edge.total_time = proto_edge.bus_edge().total_time();
			comb_edge.wait_edge.stop = &ts.GetAllStops()[proto_edge.wait_edge().stop()];
			comb_edge.wait_edge.wait_time = proto_edge.wait_edge().wait_time();

			edgeid_to_edge[edge_id] = comb_edge;
		}

		/*5. Parse Routes Internal Data (Vector of vectors) */
		graph::Router<double>::RoutesInternalData routes_internal_data(tr_graph.GetRoutesGraph().GetVertexCount(),
			std::vector <std::optional<graph::Router<double>::RouteInternalData>>(tr_graph.GetRoutesGraph().GetVertexCount()));

		const auto& proto_routes_internal_data = tr_in.router().matrix_data();

		for (size_t i = 0; i < proto_routes_internal_data.size(); ++i) {
			for (size_t j = 0; j < proto_routes_internal_data[i].array_data_size(); ++j) {
				if (proto_routes_internal_data[i].array_data()[j].is_route_internal_data()) {
					routes_internal_data[i][j] = graph::Router<double>::RouteInternalData{ 0.0,std::nullopt };
					routes_internal_data[i][j]->weight = proto_routes_internal_data[i].array_data()[j].route_internal_data().weight();
					if (proto_routes_internal_data[i].array_data()[j].route_internal_data().is_prev_edge()) {
						routes_internal_data[i][j]->prev_edge = proto_routes_internal_data[i].array_data()[j].route_internal_data().prev_edge();
					}
				}

			}
		}

		return { std::move(router::TransportRouter(std::move(tr_graph),std::move(routes_internal_data)))};
	}

} // end of namespace tr_catalogue
