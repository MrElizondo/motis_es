#include "motis/nigiri/geo_station_lookup.h"

#include "nigiri/timetable.h"

#include "utl/to_vec.h"

namespace mm = motis::module;
namespace n = ::nigiri;

namespace motis::nigiri {

motis::module::msg_ptr geo_station_lookup(std::vector<std::string> const& tags,
                                          ::nigiri::timetable const& tt,
                                          geo::point_rtree const& index,
                                          motis::module::msg_ptr const& msg) {
  using motis::lookup::CreateLookupGeoStationRequest;
  using motis::lookup::CreateLookupGeoStationResponse;
  using motis::lookup::LookupGeoStationRequest;
  auto const req = motis_content(LookupGeoStationRequest, msg);
  mm::message_creator mc;
  mc.create_and_finish(
      MsgContent_LookupGeoStationResponse,
      CreateLookupGeoStationResponse(
          mc,
          mc.CreateVector(utl::to_vec(
              index.in_radius({req->pos()->lat(), req->pos()->lng()},
                              req->min_radius(), req->max_radius()),
              [&](auto const idx) {
                auto const l = n::location_idx_t{idx};
                auto const& locations = tt.locations_;
                auto const coord = locations.coordinates_.at(l);
                auto const pos = Position(coord.lat_, coord.lng_);
                auto const src = to_idx(locations.src_.at(l));
                return CreateStation(
                    mc,
                    mc.CreateString(tags.at(src) +
                                    std::string{locations.ids_.at(l).view()}),
                    mc.CreateString(locations.names_.at(l).view()), &pos);
              })))
          .Union());
  return mm::make_msg(mc);
}

}  // namespace motis::nigiri