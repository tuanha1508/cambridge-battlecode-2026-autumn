#pragma once
// SDK submission is the authority for locks, action restrictions and channel legality.
// Called before optional shopping, recall, combat, objectives and farm selection.
template<class Champion, class Altars>
static bool z362_deposit_here(const Champion& z362_me, const Altars& z362_altars) {
    if (z362_me.gem_count() == 0) return false;
    const auto z362_pos = z362_me.position();
    for (const auto z362_a : z362_altars) {
        if (std::max(std::abs(z362_pos.x-z362_a.x),std::abs(z362_pos.y-z362_a.y)) > cambc::constants::submit_gems_range) continue;
        if (z362_me.is_submitting_gems()) return true;
        if (z362_me.is_recalling() || z362_me.is_teleporting()) return false;
        if (cambc::vision::altar_lock_tiles(cambc::team::enemy).is_set(z362_a)) continue;
        try { z362_me.submit_gems(z362_a); return true; }
        catch (const cambc::error&) { /* Illegal: retain the existing legal fallback. */ }
    }
    return false;
}
