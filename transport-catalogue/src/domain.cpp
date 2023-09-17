#include "domain.h"
#include <algorithm>


namespace domain{
	namespace detail {
		size_t GetUniqueStops(std::vector<const Stop*> stops) {
			std::sort(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name;});
			auto last = std::unique(stops.begin(), stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->name == rhs->name; });
			return std::distance(stops.begin(),last);
		}
	}
}