#pragma once
// cc_hor5 (29 Oct): also on 01starfish, 00swirl, 02geodes, 00interlock via hor5::on() (hor5_gate.h),
// evaluated after the old terms so every other map takes the unchanged path.
// cc_builds3g (29 Oct): HoR Focus build only on 00contest (exact z195_ id 1), 00elkring (exact z160_ id 5)
// and 00corners (71x71 with 5 picks: unique among all 69 ladder maps seen; the exact z300_ check cost a
// fuelExhausted turn on its first call). Platform trial of all-map cc_builds3: contest +2, elkring +1, corners +1.
#include "z195_layout.h"
#include "z160_layout.h"
#include "hor5_gate.h"
inline bool cb3g_on() {
    static const bool on = (cambc::map::width() == 71 && cambc::map::height() == 71 && cambc::draft::team_size() == 5)
        || z195_::detect() == 1 || z160_::detect() == 5 || hor5::on();
    return on;
}
