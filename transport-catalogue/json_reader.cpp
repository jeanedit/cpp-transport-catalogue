#include "json_reader.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"
#include "json_builder.h"
#include "transport_router.h"


#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <string_view>
#include <deque>
#include<sstream>


namespace tr_catalogue {
	namespace json_reader {
		using namespace std::string_view_literals;
        using namespace std::string_literals;
        using namespace domain;
        using namespace json;
        using namespace router;
        
        Dict JsonReader::StopResponse(int request_id,const std::set<std::string_view>* buses) const{
            json::Builder stop_response;
            stop_response.StartDict();
            if(buses){
               stop_response.Key("buses"s).StartArray();
               for(const std::string_view bus: *buses){
                    stop_response.Value(std::string(bus));
                }
               stop_response.EndArray();
               stop_response.Key("request_id"s).Value(request_id);
            }
            else{
                stop_response.Key("request_id"s).Value(request_id)
                             .Key("error_message"s).Value("not found"s);
            }
            stop_response.EndDict();
            return stop_response.Build().AsMap();
        }
        
        Dict JsonReader::BusResponse(int request_id,const std::optional<BusStats>& bs) const{
            json::Builder bus_response;
            bus_response.StartDict();
            if(bs.has_value()){
                bus_response.Key("curvature"s).Value(bs->curvature)
                            .Key("request_id"s).Value(request_id)
                            .Key("route_length"s).Value(static_cast<int>(bs->route_length))
                            .Key("stop_count"s).Value(static_cast<int>(bs->stops))
                            .Key("unique_stop_count"s).Value(static_cast<int>(bs->unique_stops));
            }
            else{
                bus_response.Key("request_id"s).Value(request_id)
                            .Key("error_message"s).Value("not found"s);
            }
            bus_response.EndDict();
            return bus_response.Build().AsMap();
        }
        
        Dict JsonReader::MapResponse(int request_id,const svg::Document& render_doc) const{
            json::Builder map_response;
            std::ostringstream ostr;
            render_doc.Render(ostr);
        
            std::string svg = ostr.str();
            
            map_response.StartDict()
                        .Key("map"s).Value(std::move(svg))
                        .Key("request_id"s).Value(request_id)
                        .EndDict();
            return map_response.Build().AsMap();
        }


        Dict JsonReader::EdgeResponse(const WaitEdge& wait_edge) const {
            json::Builder wait_edge_response;
            wait_edge_response.StartDict()
                .Key("type"s).Value("Wait"s)
                .Key("time"s).Value(wait_edge.wait_time)
                .Key("stop_name"s).Value(wait_edge.stop->name)
            .EndDict();
            return wait_edge_response.Build().AsMap();
        }

        Dict JsonReader::EdgeResponse(const BusEdge& bus_edge) const {
            json::Builder bus_edge_response;
            bus_edge_response.StartDict()
                .Key("type"s).Value("Bus"s)
                .Key("time"s).Value(bus_edge.total_time)
                .Key("span_count"s).Value(static_cast<int>(bus_edge.span_count))
                .Key("bus"s).Value(bus_edge.bus->name)
            .EndDict();

            return bus_edge_response.Build().AsMap();
        }

         Dict JsonReader::RouteResponse(int request_id, const std::optional<OptimalRoute>& total_route) const {
             json::Builder route_response;
             route_response.StartDict()
                 .Key("request_id"s).Value(request_id);

             if (total_route) {
                 route_response.Key("total_time"s).Value(total_route->total_time);
                 route_response.Key("items"s).StartArray();
                 if (!total_route->edges.empty()) {
                     for (size_t i = 0; i < total_route->edges.size(); ++i) {
                         route_response.Value(EdgeResponse(total_route->edges[i].wait_edge));
                         route_response.Value(EdgeResponse(total_route->edges[i].bus_edge));
                     }
                 }

                 route_response.EndArray();
             }
             else {
                 route_response.Key("error_message"s).Value("not found"s);
             }

             route_response.EndDict();
             return route_response.Build().AsMap();
         }


        void JsonReader::ResponseRequests(std::ostream& os,const RequestHandler& rq) const{
            json::Builder responses;
            responses.StartArray();
            for(const auto& request:json_document_.GetRoot().AsMap().at("stat_requests"s).AsArray()){
                        if(request.AsMap().at("type"s).AsString() == "Stop"sv){
responses.Value(StopResponse(request.AsMap().at("id"s).AsInt(),rq.GetBusesByStop(request.AsMap().at("name"s).AsString())));     
                        }
                
                        if(request.AsMap().at("type"s).AsString() == "Bus"sv){
responses.Value(BusResponse(request.AsMap().at("id"s).AsInt(),rq.GetBusStat(request.AsMap().at("name"s).AsString())));           
                        }
            
                        if(request.AsMap().at("type"s).AsString() == "Map"sv){
            responses.Value(MapResponse(request.AsMap().at("id"s).AsInt(),rq.RenderMap()));            
                         }

                   if (request.AsMap().at("type"s).AsString() == "Route"sv) {
                        responses.Value(RouteResponse(request.AsMap().at("id"s).AsInt(),rq.GetOptimalRoute(request.AsMap().at("from"s).AsString(), request.AsMap().at("to"s).AsString())));
                    }
            }
            responses.EndArray();
            Print(Document(responses.Build()),os);
        }
        
        const json::Document& JsonReader::GetJsonDocument() const{
            return json_document_;
        }
        

		Stop JsonReader::ParseStopQuery(const Node& type_stop) const{
            const auto& type_stop_map = type_stop.AsMap();
            return Stop{type_stop_map.at("name"s).AsString(),geo::Coordinates{type_stop_map.at("latitude"s).AsDouble(),type_stop_map.at("longitude"s).AsDouble()}};
		}

		void JsonReader::ParseStopQueryDistance(const Node& type_stop) {
            const auto& type_stop_map = type_stop.AsMap();
            for(const auto& [to_stop,dist_to_stop]:type_stop_map.at("road_distances"s).AsMap()){
transport_catalogue_.AddStopPairsDistances(transport_catalogue_.FindStop(type_stop_map.at("name"s).AsString()), transport_catalogue_.FindStop(to_stop), dist_to_stop.AsInt());
            }

		}

		Bus JsonReader::ParseBusQuery(const Node& node_bus) const {
            const auto& node_bus_map = node_bus.AsMap();
			Bus bus;
			bus.is_loop = node_bus_map.at("is_roundtrip"s).AsBool();
			bus.name = node_bus_map.at("name"s).AsString();
            for(const auto& stop:node_bus_map.at("stops"s).AsArray()){
                bus.route.push_back(transport_catalogue_.FindStop(stop.AsString()));
            }
			return bus;
		}
        
        
        
		void JsonReader::ReadJson() {
            
            for(const auto& request:json_document_.GetRoot().AsMap().at("base_requests"s).AsArray()){
                        if(request.AsMap().at("type"s).AsString() == "Stop"sv){
                            transport_catalogue_.AddStop(ParseStopQuery(request.AsMap()));                     
                        }
            }
            
            for(const auto& request:json_document_.GetRoot().AsMap().at("base_requests"s).AsArray()){
                        if(request.AsMap().at("type"s).AsString() == "Stop"sv){
                            ParseStopQueryDistance(request);                      
                        }
            }

            for(const auto& request:json_document_.GetRoot().AsMap().at("base_requests"s).AsArray()){
                        if(request.AsMap().at("type"s).AsString() == "Bus"sv){
                             transport_catalogue_.AddBus(ParseBusQuery(request));                      
                        }
            }
		}

        svg::Color JsonReader::ParseColor(const json::Node& node) const{
            if (node.IsString()){
                return node.AsString();
            }

            const auto& array = node.AsArray();
            uint8_t red = array.at(0).AsInt();
            uint8_t green = array.at(1).AsInt();
            uint8_t blue = array.at(2).AsInt();
        
            if (array.size() == 3){
                return svg::Rgb(red, green, blue);
            }
        
            double alpha = array.at(3).AsDouble();
            return svg::Rgba(red, green, blue, alpha);
        }

        
        renderer::RenderSettings JsonReader::ParseRenderSettings() const{
            renderer::RenderSettings rs;
            const auto rs_map = json_document_.GetRoot().AsMap().at("render_settings"s).AsMap();
            rs.width = rs_map.at("width"s).AsDouble();
            rs.height = rs_map.at("height"s).AsDouble();

            rs.padding = rs_map.at("padding"s).AsDouble();
            rs.line_width = rs_map.at("line_width"s).AsDouble();;
   
            rs.stop_radius = rs_map.at("stop_radius"s).AsDouble();
            rs.bus_label_font_size = rs_map.at("bus_label_font_size"s).AsInt();
            rs.bus_label_offset = {rs_map.at("bus_label_offset"s).AsArray().at(0).AsDouble(),rs_map.at("bus_label_offset"s).AsArray().at(1).AsDouble()};
            rs.stop_label_font_size = rs_map.at("stop_label_font_size"s).AsInt();
            rs.stop_label_offset = {rs_map.at("stop_label_offset"s).AsArray().at(0).AsDouble(),rs_map.at("stop_label_offset"s).AsArray().at(1).AsDouble()};

            rs.underlayer_color = ParseColor(rs_map.at("underlayer_color"s));
            rs.underlayer_width = rs_map.at("underlayer_width"s).AsDouble();
            const auto& color_pl = rs_map.at("color_palette"s).AsArray();
            rs.color_palette.reserve(color_pl.size());
            for(const auto& color:color_pl){
                rs.color_palette.emplace_back(ParseColor(color));
            }
            return rs;
        }
        
        renderer::MapRenderer JsonReader::MapRenderFromJson() const{
            std::vector<geo::Coordinates> stops_coords;
            stops_coords.reserve(transport_catalogue_.GetAllStops().size());        
            for(const Bus& bus:transport_catalogue_.GetAllBuses()){
                for(const Stop* stop:bus.route){
                    stops_coords.emplace_back(stop->geo_coords); 
                }

            }
           
            return  renderer::MapRenderer(ParseRenderSettings(),stops_coords);
        }
        
        domain::RoutingSettings JsonReader::ParseRoutingSettings() const {
            const auto routings_map = json_document_.GetRoot().AsMap().at("routing_settings"s).AsMap();
            return { routings_map.at("bus_velocity"s).AsDouble(), routings_map.at("bus_wait_time"s).AsDouble() };
        }

        router::TransportRouter JsonReader::TransportRouterFromJson() const {
            return TransportRouter(transport_catalogue_, ParseRoutingSettings());
        }

	}
}