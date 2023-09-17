#include "map_renderer.h"
#include <variant>

namespace renderer{
    
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }
    
    MapRenderer::MapRenderer(RenderSettings&& render_settings, const std::vector<geo::Coordinates>& stops_coords)
        :render_settings_(std::move(render_settings)),
        sphere_proj_(stops_coords.begin(),stops_coords.end(),render_settings_.width,render_settings_.height,render_settings_.padding)
    {
    }
    

	void MapRenderer::BusRouteRender(svg::Document& render_doc, const domain::Bus& bus, size_t color) const {
		svg::Polyline poly = svg::Polyline()
			.SetFillColor(svg::NoneColor)
			.SetStrokeColor(render_settings_.color_palette.at(color))
			.SetStrokeWidth(render_settings_.line_width)
			.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
			.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		std::for_each(bus.route.begin(),
			bus.route.end(),
			[&poly, this](const domain::Stop* elem) {
				poly.AddPoint(sphere_proj_(elem->geo_coords));
			});
		if (!bus.is_loop) {
			std::for_each(std::next(bus.route.rbegin()),
				bus.route.rend(),
				[&poly, this](const domain::Stop* elem) {
					poly.AddPoint(sphere_proj_(elem->geo_coords));
				});
		}
		render_doc.Add(std::move(poly));
	}

	svg::Text MapRenderer::BusTextRenderSettings(const std::string& bus_name, const domain::Stop* stop, bool is_underlayer, size_t color) const {
		svg::Text bus_text = svg::Text()
			.SetPosition(sphere_proj_(stop->geo_coords))
			.SetOffset(render_settings_.bus_label_offset)
			.SetFontSize(render_settings_.bus_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetFontWeight("bold"s)
			.SetData(bus_name);
		if (is_underlayer) {
			bus_text
				.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		else {
			bus_text.SetFillColor(render_settings_.color_palette.at(color));
		}
		return bus_text;
	}

	void MapRenderer::BusTextRender(svg::Document& render_doc, const domain::Bus& bus, size_t color) const {
		render_doc.Add(std::move(BusTextRenderSettings(bus.name, bus.route.front(), true)));
		render_doc.Add(std::move(BusTextRenderSettings(bus.name, bus.route.front(), false, color)));
		if (!bus.is_loop && bus.route.front() != bus.route.back()) {
			render_doc.Add(std::move(BusTextRenderSettings(bus.name, bus.route.back(), true)));
			render_doc.Add(std::move(BusTextRenderSettings(bus.name, bus.route.back(), false, color)));
		}
	}

	svg::Text MapRenderer::StopTextRenderSettings(const domain::Stop* stop, bool is_underlayer) const {
		svg::Text stop_text = svg::Text()
			.SetPosition(sphere_proj_(stop->geo_coords))
			.SetOffset(render_settings_.stop_label_offset)
			.SetFontSize(render_settings_.stop_label_font_size)
			.SetFontFamily("Verdana"s)
			.SetData(stop->name);
		if (is_underlayer) {
			stop_text.SetFillColor(render_settings_.underlayer_color)
				.SetStrokeColor(render_settings_.underlayer_color)
				.SetStrokeWidth(render_settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		}
		else {
			stop_text.SetFillColor("black"s);
		}
		return stop_text;
	}
         
                           
    void MapRenderer::StopRender(svg::Document& render_doc,const domain::Stop* stop) const{
            render_doc.Add(std::move(svg::Circle()
                .SetCenter(sphere_proj_(stop->geo_coords))
                .SetRadius(render_settings_.stop_radius)
                .SetFillColor("white"s)));
        }
    
        svg::Document MapRenderer::RenderRoutes(const std::deque<domain::Bus>& buses_storage,const std::deque<const domain::Stop*>& stops_with_bus) const{
            svg::Document render_doc;
            std::deque<domain::Bus> buses;
            for(const domain::Bus& bus:buses_storage){
                if(!bus.route.empty()){
                    buses.push_back(bus);
                }

            }
            std::sort(buses.begin(),buses.end(),[](const domain::Bus& lhs, const domain::Bus& rhs){
                return lhs.name < rhs.name;
            });
            size_t bus_counter = 0;
            for(const auto& bus:buses){
                BusRouteRender(render_doc,bus,bus_counter%render_settings_.color_palette.size());
                ++bus_counter;
            }
            bus_counter = 0;
            for(const auto& bus:buses){
                BusTextRender(render_doc,bus,bus_counter%render_settings_.color_palette.size());
                ++bus_counter;
            }
            for(const domain::Stop* stop:stops_with_bus){
                StopRender(render_doc,stop);
            }
            for(const domain::Stop* stop:stops_with_bus){
                StopTextRender(render_doc,stop);
            }
            return render_doc;
        }  
}