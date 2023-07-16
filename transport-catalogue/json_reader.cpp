#include "json_reader.h"
#include "transport_catalogue.h"
#include "geo.h"
#include "json.h"

#include<iostream>
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
        
        Dict JsonReader::StopResponse(int request_id,const std::set<std::string_view>* buses) const{
            Dict stop_response;
            Array json_buses;
            if(buses){
                for(const std::string_view bus: *buses){
                    json_buses.emplace_back(std::string(bus));
                }
               stop_response.emplace("buses"s,json_buses);
                stop_response.emplace("request_id"s,request_id);
            }
            else{
                stop_response.emplace("request_id"s,request_id);       
                stop_response.emplace("error_message"s,"not found"s);
            }
            return stop_response;
        }
        
        Dict JsonReader::BusResponse(int request_id,const std::optional<BusStats>& bs) const{
            Dict bus_response;
            if(bs.has_value()){
                bus_response.emplace("curvature"s,bs->curvature);
                bus_response.emplace("request_id"s,request_id);
                bus_response.emplace("route_length"s,static_cast<int>(bs->route_length));
                bus_response.emplace("stop_count"s,static_cast<int>(bs->stops));
                bus_response.emplace("unique_stop_count"s,static_cast<int>(bs->unique_stops));
            }
            else{
                bus_response.emplace("request_id"s,request_id);       
                bus_response.emplace("error_message"s,"not found"s);
            }
            return bus_response;
        }
        
        Dict JsonReader::MapResponse(int request_id,const svg::Document& render_doc) const{
            Dict map_response;
            std::ostringstream ostr;
            render_doc.Render(ostr);

            std::string svg = ostr.str();
            map_response.emplace("map"s,std::move(svg));
            map_response.emplace("request_id"s,request_id);

            return map_response;
        }
        
        void JsonReader::ResponseRequests(std::ostream& os,const RequestHandler& rq) const{
            Array responses;
            for(const auto& request:json_document_.GetRoot().AsMap().at("stat_requests"s).AsArray()){
                        if(request.AsMap().at("type"s).AsString() == "Stop"sv){
responses.emplace_back(StopResponse(request.AsMap().at("id"s).AsInt(),rq.GetBusesByStop(request.AsMap().at("name"s).AsString())));           
                        }
                
            if(request.AsMap().at("type"s).AsString() == "Bus"sv){
responses.emplace_back(BusResponse(request.AsMap().at("id"s).AsInt(),rq.GetBusStat(request.AsMap().at("name"s).AsString())));           
            }
            
            if(request.AsMap().at("type"s).AsString() == "Map"sv){
            responses.emplace_back(MapResponse(request.AsMap().at("id"s).AsInt(),rq.RenderMap()));            
            }
            }
                
            Print(Document(Node(responses)),os);
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
        
	}
}
