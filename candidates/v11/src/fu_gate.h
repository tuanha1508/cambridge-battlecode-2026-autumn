#pragma once
// cc_merge_all: cc_fuel5 (docs/claude-fuel5-2026-10-30.md, built on cc_pin38) fuel fixes behind per-map switches
// (mg_switch.h, all default 0):
//   FUEL_AMBUSH   legacy.cpp on exact 00ambush (100x100, altars (33,20)/(62,40)/(37,59)/(66,79)): fuel3 camp loader,
//                 post-init yield, capped am_map field, and the FU5_AMB side-B yield (side B plays as the pin).
//   FUEL_HARVEST  the same legacy.cpp changes on any other 100x100 legacy map (02harvest; Big World runs z78_m1).
//   FUEL_PILGRIM  z195_layout.h: fu2_memcmp only for the 50x65 layout (00pilgrimage, z195 id 2).
//   FUEL_BIGWORLD z78_m1.cpp (runs only on 01big-world): fuel3 loader, post-init yield, capped am_map.
#include <cambc.h>
#include "mg_switch.h"
#define FU_LEG (FUEL_AMBUSH || FUEL_HARVEST)
#if FU_LEG
#include "fu3_cg.h"
namespace fu_ {
inline int legacy_map() {
    if (cambc::map::width() != 100 || cambc::map::height() != 100) return 0;
    const auto& A = cambc::map::altars();
    const bool amb = A.is_set(cambc::vec2{ 33, 20 }) && A.is_set(cambc::vec2{ 62, 40 }) && A.is_set(cambc::vec2{ 37, 59 }) && A.is_set(cambc::vec2{ 66, 79 });
    return amb ? (FUEL_AMBUSH ? 1 : 0) : (FUEL_HARVEST ? 2 : 0);
}
}  // namespace fu_
#endif
