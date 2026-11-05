#pragma once
// cc_surv2 (29 Oct 2026): exact-layout SURVIVAL rules, docs/claude-surv2-2026-10-29.md. One switch per map.
// A champion outside the map's death zone does not step (or dash) into it while visible enemies in or next to the
// zone (cheb <= 2) are at least as many as our champions in the zone or within S2_AR of it (itself included). It
// never holds with an enemy within 2 tiles (normal fight logic), and holds at most S2_WAIT consecutive turns, then
// S2_WAIT free turns follow. Zones come from the ranked/platform death tiles (research/claude-surv2/report.txt).
// S2_SWIRL (00swirl, altars (23,10),(21,19)): centre box x 19..25, y 11..18 (our deaths 40/loss vs winners 8.4,
//   top tiles (21,13),(23,15),(22,14),(23,16)).
// S2_STARV (01starvation, altars (35,15),(44,15)): north approach x 30..49, y 0..12 (deaths 21.5/loss vs 5.6,
//   top tiles (37,4),(37,6),(35,8),(36,7)).
#include <cambc.h>
#ifndef S2_SWIRL
#define S2_SWIRL 0   // off: lost H2H vs cc_pin11 on both sides (554:1012 as A, 727:1018 as B); S2_STRICT=1 never fired vs pin11
#endif
#ifndef S2_STARV
#define S2_STARV 0   // off: G85 +1/-1
#endif
#ifndef S2_AR
#define S2_AR 3
#endif
#ifndef S2_STRICT
#define S2_STRICT 0   // 1 = hold only when strictly outnumbered (ne > na)
#endif
#ifndef S2_WAIT
#define S2_WAIT 20
#endif
// 1 = swirl, 2 = starvation, 0 = other. Called once on turn 8 (size + team size + exact altar set; no digest).
inline int surv2_map() {
    const int w = cambc::map::width(), h = cambc::map::height(), n = static_cast<int>(cambc::draft::team_size());
    const auto& al = cambc::map::altars();
    if (S2_SWIRL && w == 45 && h == 30 && n == 4 && al.count() == 2 && al.is_set(cambc::vec2{ 23, 10 }) && al.is_set(cambc::vec2{ 21, 19 })) return 1;
    if (S2_STARV && w == 80 && h == 80 && n == 5 && al.count() == 2 && al.is_set(cambc::vec2{ 35, 15 }) && al.is_set(cambc::vec2{ 44, 15 })) return 2;
    return 0;
}
inline bool surv2_in_zone(int m, cambc::vec2 p) {
    if (m == 1) return p.x >= 19 && p.x <= 25 && p.y >= 11 && p.y <= 18;
    if (m == 2) return p.x >= 30 && p.x <= 49 && p.y >= 0 && p.y <= 12;
    return false;
}
inline int surv2_zone_dist(int m, cambc::vec2 p) {   // Chebyshev distance to the zone box
    int x0, x1, y0, y1;
    if (m == 1) { x0 = 19; x1 = 25; y0 = 11; y1 = 18; } else { x0 = 30; x1 = 49; y0 = 0; y1 = 12; }
    const int dx = p.x < x0 ? x0 - p.x : (p.x > x1 ? p.x - x1 : 0);
    const int dy = p.y < y0 ? y0 - p.y : (p.y > y1 ? p.y - y1 : 0);
    return dx > dy ? dx : dy;
}
