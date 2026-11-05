// Read-only current spell references. These do not predict future activations,
// visibility, landing legality, damage, survival or the next enemy decision.
enum class QualifierMovePurpose { Ordinary, Delivery, Recovery };
struct QualifierEnemyAdvice {
    bool available = false;
    double cast_reference = 0, raw_reference = 0, reset_nominal = 0;
    int potential_casters = 0, controls = 0, unknown = 0;
    int raw_unknown = 0;
    bool necessary_geometry = false;
};
namespace qualifier_enemy_advice_detail {
enum class Kind { Unused, Other, Direct, Area, Dash, Control, Reset };
struct Slot {
    Kind kind = Kind::Unused;
    bool metadata_known = false, target_enemy = false, target_ground = false;
    bool exclusive = false, damage_known = false, raw_known = false;
    bool damage_kind = false;
    bool cooldown_known = false, current_ready_known = false, current_ready = false;
    int cooldown = -1, control = 0;
    double range = 0, radius = 0, reference = 0, raw = 0;
};
struct Enemy {
    std::uint32_t id = 0;
    vec2 position{};
    double strength = 0, move_speed = 0;
    int basic_range = 0;
    bool current_can_move = false, current_can_attack = false, current_can_cast = false;
    std::array<Slot, 2> slots{};
};
struct DefenseSnapshot {
    // The SDK collection owns immutable storage and cannot be assigned. Copy
    // its three released typed payloads into an ordinary owning value instead.
    static_assert(static_cast<std::size_t>(buff_types::count) == 3);
    std::array<std::optional<buff_t>, 3> values{};
    explicit DefenseSnapshot(const buff_collection& buffs) {
        for (const auto kind : {buff_types::shield, buff_types::stun, buff_types::enhanced_attack})
            if (buffs.has(kind)) values[static_cast<std::size_t>(kind)] = buffs.get(kind);
    }
    friend bool operator==(const DefenseSnapshot&, const DefenseSnapshot&) = default;
};
struct Frame {
    bool valid = false, omitted = false;
    std::uint32_t actor = 0;
    int round = -1;
    std::uint64_t revision = 0;
    std::uint64_t serial = 0;
    vec2 origin{};
    double health = 0;
    bool stun_immune = false;
    std::optional<DefenseSnapshot> defenses;
    std::array<Enemy, 16> enemies{};
    std::size_t count = 0;
};
static_assert(std::is_copy_assignable_v<Frame> && std::is_move_assignable_v<Frame>);
struct Runtime {
    Frame frame;
    std::uint32_t budget_actor = 0;
    int budget_round = -1, attempts = 0;
    std::uint64_t serial = 0;
};
Runtime runtime;
bool nonnegative(double x) { return std::isfinite(x) && x >= 0; }
bool immediate_area(champion_type type, int slot) {
    return (type == champion_type::ryth && slot == 0)
        || (type == champion_type::prism && slot == 1)
        || (type == champion_type::stellar && slot == 1);
}
bool same_defenses(const DefenseSnapshot& a, const DefenseSnapshot& b) {
    return a == b;
}
bool matching(const Frame& f) {
    return f.valid && qualifier_current() && f.round == S->round
        && f.revision == qualifier_runtime.observation_revision
        && f.actor == static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()))
        && f.origin == self().position() && f.health == self().health()
        && f.defenses && same_defenses(*f.defenses, DefenseSnapshot(self().buffs()));
}
const Frame* obtain() {
    auto& rt = runtime;
    bool charged = false;
    try {
        if (!S) return nullptr;
        const auto actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(self()));
        if (rt.budget_round != S->round || rt.budget_actor != actor) {
            rt.budget_actor = actor; rt.budget_round = S->round; rt.attempts = 0;
        }
        if (matching(rt.frame)) return &rt.frame;
        // Never relabel an old defense snapshot after a changed action/frame.
        rt.frame = {};
        if (rt.attempts >= 3) return nullptr;
        if (!qualifier_current() || !self().is_alive()) return nullptr;
        // Optional, unmeasured work bounds. Exhaustion means no new advisory.
        if (rt.attempts >= 3 || fuel_remaining() < 24000) return nullptr;
        ++rt.attempts; charged = true;
        Frame fresh;
        auto& me = self();
        fresh.actor = static_cast<std::uint32_t>(static_cast<handle_champion_t>(me));
        fresh.round = S->round; fresh.revision = qualifier_runtime.observation_revision;
        fresh.origin = me.position(); fresh.health = me.health();
        if (!nonnegative(fresh.health) || fresh.health == 0) return nullptr;
        fresh.defenses.emplace(me.buffs());
        fresh.stun_immune = me.champion_name() == champion_type::marcella;
        for (const auto& observed : qualifier_runtime.enemies) {
            if (!qualifier_current() || fuel_remaining() < 12000) { fresh.omitted = true; break; }
            if (fresh.count == fresh.enemies.size()) { fresh.omitted = true; break; }
            const auto& unit = observed.unit;
            if (!unit.is_visible() || !unit.is_alive()) continue;
            auto& enemy = fresh.enemies[fresh.count++];
            enemy.id = static_cast<std::uint32_t>(static_cast<handle_champion_t>(unit));
            enemy.position = unit.position(); enemy.strength = unit.strength();
            enemy.basic_range = unit.attack_range();
            enemy.move_speed = unit.base_stats().base_move_speed;
            if (!nonnegative(enemy.strength) || !nonnegative(enemy.move_speed) || enemy.basic_range < 0)
                return nullptr;
            const auto flags = unit.state();
            enemy.current_can_move = flags.contains(champion_state::can_move);
            enemy.current_can_attack = flags.contains(champion_state::can_attack);
            enemy.current_can_cast = flags.contains(champion_state::can_cast);
            const auto type = unit.champion_name();
            const auto abilities = unit.abilities();
            for (std::size_t index = 0; index < enemy.slots.size(); ++index) {
                auto& slot = enemy.slots[index];
                try {
                    const auto& ability = abilities[index];
                    const auto& info = ability.info(); // Unknown visibility is not an unused slot.
                    if (!info) { slot.metadata_known = true; continue; }
                    slot.target_enemy = ability.can_target_enemies();
                    slot.target_ground = ability.can_target_ground();
                    slot.exclusive = std::visit([](const auto& m) { return m.exclusive_with_movement; }, *info);
                    slot.kind = Kind::Other;
                    bool damage = false, supported = true;
                    if (ability.is_damage()) {
                        slot.kind = Kind::Direct; slot.range = ability.get<ability_damage>().range;
                        damage = slot.target_enemy;
                    } else if (ability.is_cc()) {
                        const auto m = ability.get<ability_cc>();
                        slot.kind = Kind::Control; slot.range = m.range; slot.control = std::max(0, m.duration);
                        damage = slot.target_enemy;
                    } else if (ability.is_aoe_damage()) {
                        const auto m = ability.get<ability_aoe_damage>();
                        slot.kind = Kind::Area; slot.range = m.range; slot.radius = m.radius;
                        damage = slot.target_ground;
                        supported = immediate_area(type, static_cast<int>(index));
                        if (type == champion_type::ryth && index == 0 && m.duration > 0) slot.control = 1;
                    } else if (ability.is_dash()) {
                        const auto m = ability.get<ability_dash>();
                        slot.kind = Kind::Dash; slot.range = m.range; slot.radius = m.radius;
                        slot.control = std::max(0, m.duration);
                        damage = slot.target_ground && m.modifier > 0;
                        // The action mask names a tile destination. Published
                        // HoR landing effects hit enemies/camps without an
                        // enemy-unit action target bit (SDK2818, rules824).
                        if (damage) supported = type == champion_type::heart_of_ruin && index == 1;
                    } else if (ability.is_attack_reset()) slot.kind = Kind::Reset;
                    if (!nonnegative(slot.range) || !nonnegative(slot.radius)
                        || slot.range > 16 || slot.radius > 16) continue;
                    slot.metadata_known = true;
                    slot.damage_kind = damage;
                    try { slot.cooldown = ability.cooldown(); slot.cooldown_known = slot.cooldown >= 0; }
                    catch (...) { /* Current cooldown remains unknown, never future zero. */ }
                    try { slot.current_ready = ability.is_ready(); slot.current_ready_known = true; }
                    catch (...) { /* Readiness is an annotation, never an admission gate. */ }
                    if (!damage) { slot.damage_known = true; continue; }
                    if (!supported) continue; // No generic duration-to-tick damage inference.
                    try { slot.raw = ability.damage_amount(); slot.raw_known = nonnegative(slot.raw); }
                    catch (...) { /* Raw outgoing and capped target loss are separate. */ }
                    try { slot.reference = ability.damage_amount(me); slot.damage_known = nonnegative(slot.reference); }
                    catch (...) { /* A failed prospective query is not zero damage. */ }
                } catch (...) { slot.metadata_known = false; }
            }
        }
        if (!qualifier_current() || fresh.revision != qualifier_runtime.observation_revision
            || fresh.origin != me.position() || fresh.health != me.health()
            || !fresh.defenses || !same_defenses(*fresh.defenses, DefenseSnapshot(me.buffs()))) return nullptr;
        fresh.valid = true; fresh.serial = ++rt.serial; rt.frame = std::move(fresh);
        return &rt.frame;
    } catch (...) {
        rt.frame = {};
        if (!charged && rt.attempts < 3) ++rt.attempts;
        return nullptr;
    }
}
QualifierEnemyAdvice project(const Frame& frame, vec2 point) {
    QualifierEnemyAdvice result;
    result.available = frame.valid;
    if (frame.omitted) ++result.unknown;
    for (std::size_t i = 0; i < frame.count; ++i) {
        const auto& enemy = frame.enemies[i];
        double one_cast = 0, one_raw = 0;
        bool possible = false;
        for (const auto& slot : enemy.slots) {
            if (!slot.metadata_known) { ++result.unknown; continue; }
            if (slot.kind == Kind::Unused || slot.kind == Kind::Other) continue;
            const double distance = cheb(point, enemy.position);
            if (slot.kind == Kind::Reset) {
                if (distance <= enemy.basic_range + enemy.move_speed)
                    result.reset_nominal = std::max(result.reset_nominal, 2.0 * enemy.strength);
                continue;
            }
            const bool area = slot.kind == Kind::Area || slot.kind == Kind::Dash;
            if (area ? !slot.target_ground : !slot.target_enemy) continue;
            // A necessary geometric envelope, not an enemy path/vision oracle.
            // Nonexclusive movement is conditional; current spent flags do not
            // establish what will be available on a later enemy input.
            const double walk = slot.exclusive ? 0 : enemy.move_speed;
            if (distance > slot.range + slot.radius + walk) continue;
            result.necessary_geometry = true;
            possible = true;
            if (slot.control > 0 && !frame.stun_immune) ++result.controls;
            if (slot.raw_known) one_raw = std::max(one_raw, slot.raw);
            else if (slot.damage_kind) ++result.raw_unknown;
            if (!slot.damage_known) { ++result.unknown; continue; }
            one_cast = std::max(one_cast, slot.reference);
        }
        if (possible) ++result.potential_casters;
        // Distinct actors and slots are not a shared-window lethal burst.
        result.cast_reference = std::max(result.cast_reference, one_cast);
        result.raw_reference = std::max(result.raw_reference, one_raw);
    }
    return result;
}
bool numeric_complete(const Frame& frame) {
    if (!frame.valid || frame.omitted || !frame.defenses) return false;
    for (std::size_t i = 0; i < frame.count; ++i)
        for (const auto& slot : frame.enemies[i].slots)
            if (!slot.metadata_known || (slot.damage_kind && !slot.damage_known)) return false;
    return true;
}
bitboard positive_envelopes(const Frame& frame) {
    bitboard result;
    for (std::size_t i = 0; i < frame.count; ++i) {
        const auto& enemy = frame.enemies[i];
        for (const auto& slot : enemy.slots) {
            if (!slot.damage_kind || !slot.damage_known || slot.reference <= 0) continue;
            const bool area = slot.kind == Kind::Area || slot.kind == Kind::Dash;
            if (area ? !slot.target_ground : !slot.target_enemy) continue;
            const double walk = slot.exclusive ? 0 : enemy.move_speed;
            result.draw_chebyshev(enemy.position, static_cast<float>(slot.range + slot.radius + walk));
        }
    }
    return result;
}
} // namespace qualifier_enemy_advice_detail
