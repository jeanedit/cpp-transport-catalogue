#pragma once

#include <deque>
#include <ostream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "domain.h"
#include "transport_router.h"


namespace tr_catalogue{

	void SerializeTransportCatalogue(std::ostream& out, const TransportCatalogue& ts);
	std::optional<TransportCatalogue>  DeserializeTransportCatalogue(const domain::SerializationSettings& ss);


	void SerializeMapRenderer(std::ostream& out, const renderer::RenderSettings& ts, const std::deque<domain::Bus>& buses);
	std::optional<renderer::MapRenderer> DeserializeMapRenderer(const domain::SerializationSettings& ss);

	void SerializeTransportRouter(std::ostream& out, const router::TransportRouter& tr);
	std::optional<router::TransportRouter> DeserializeTransportRouter(const domain::SerializationSettings& ss, const TransportCatalogue& ts);
}

