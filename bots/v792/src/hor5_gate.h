#pragma once
// cc_hor5 (29 Oct): Heart of Ruin Focus build (cb3_hor.h, no wards) also on four g117 maps.
// One switch per map (1 = on). Map identity costs no fuel worth noting: exact width/height, team size
// and our own draft-memory picks (written by the exact-layout draft code in main.cpp). No layout
// detector runs here (z69::detect after play_turn exhausted fuel on Stampede in cc_pin20).
// Checked against all 70 kit-4.0.0 map dumps (research/claude-hw/dump): 89x71 and 45x30 are unique
// sizes; on 50x50 and 40x40 the picks differ from every other map. Platform picks in g117 (v682):
// starfish [11,6,2,0,1], swirl [11,1,6,0], geodes [11,2,3,0,6], interlock [11,2,6,0].
// If a draft ever goes wrong the picks differ and the map stays off (v692 behaviour).
#include <cambc.h>
#include <cstddef>
#include <initializer_list>

#ifndef HOR5_STARFISH
#define HOR5_STARFISH 0   // off: G137 -1
#endif
#ifndef HOR5_SWIRL
#define HOR5_SWIRL 0      // off: G137 +1/-1
#endif
#ifndef HOR5_GEODES
#define HOR5_GEODES 1     // 02geodes
#endif
#ifndef HOR5_INTERLOCK
#define HOR5_INTERLOCK 0  // off: G137 -1
#endif

namespace hor5 {
inline bool mem_is(std::initializer_list<int> picks) {
    const auto& m = cambc::draft_memory;
    std::size_t i = 0;
    for (int p : picks) {
        if (m.size() <= i || std::to_integer<int>(m[i]) != p) return false;
        ++i;
    }
    return true;
}

inline bool on() {
    static const bool v = [] {
        if (cambc::is_draft()) return false;
        const int w = cambc::map::width(), h = cambc::map::height();
        const int n = static_cast<int>(cambc::draft::team_size());
        if (HOR5_STARFISH && w == 50 && h == 50 && n == 5 && mem_is({11, 6, 2, 0, 1})) return true;
        if (HOR5_SWIRL && w == 45 && h == 30 && n == 4 && mem_is({11, 1, 6, 0})) return true;
        if (HOR5_GEODES && w == 89 && h == 71 && n == 5) return true;
        if (HOR5_INTERLOCK && w == 40 && h == 40 && n == 4 && mem_is({11, 2, 6, 0})) return true;
        return false;
    }();
    return v;
}
}  // namespace hor5
