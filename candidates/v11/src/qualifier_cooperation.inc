// Exact-group, advisory-only rendezvous for one immutable pair.
// All eight team bytes belong to this format. Root must disable legacy writers.
enum class QualifierCooperationPhase : std::uint8_t { None, Approach, Arrived, Withdraw };
struct QualifierCooperationPair {
    bool enabled = false;
    std::array<std::uint32_t, 2> actors{};
    int own_slot = -1;
    std::uint32_t group_count = 0;
};
struct QualifierCooperationReport {
    bool valid = false, fresh = false;
    std::uint32_t writer = 0, full_group = 0;
    int group_index = -1, reported_round = -1, age = -1;
    QualifierCooperationPhase phase = QualifierCooperationPhase::None;
};
struct QualifierCooperationNomination {
    std::uint32_t writer = 0, full_group = 0;
    int group_index = -1, reported_round = -1, age = -1;
    QualifierCooperationPhase phase = QualifierCooperationPhase::None;
    bool writer_is_anchor = false;
    bool lifecycle_known = false, capture_known = false;
    int lifecycle = -1; // Current own view: 0 alive, 1 reward, 2 respawning.
    int capture_timer = -1, capture_status = -1;
};
namespace qualifier_cooperation_detail {
constexpr std::uint32_t commit_bit = 1u << 29;
constexpr int init_reserve = 30000, read_reserve = 10000, publish_reserve = 6000;
// Fuel reserves are unmeasured optional-work guards, not cost guarantees.
struct Entry { std::uint32_t full; int index; };
struct Runtime {
    QualifierCooperationPair pair;
    std::array<std::optional<champion>, 2> writers{};
    std::vector<Entry> dictionary;
    std::array<QualifierCooperationReport, 2> reports{};
    std::optional<QualifierCooperationNomination> nomination;
    bool permanently_disabled = false;
    int frame_round = -1, last_begin_round = -1, last_committed_round = -1;
    std::uint32_t frame_actor = 0;
    std::uint16_t previous_token = 0;
    std::uint8_t generation = 0;
};
Runtime runtime;
std::uint32_t actor_id(const champion& body) {
    return static_cast<std::uint32_t>(static_cast<handle_champion_t>(body));
}
const map::camp_group* public_group(const Group& group) {
#if QUALIFIER_LEGACY_BACKEND
    return &group.h;
#else
    return group.qualifier_public_group ? &*group.qualifier_public_group : nullptr;
#endif
}
bool initialize() {
    auto& rt = runtime;
    if (rt.pair.enabled) return true;
    if (rt.permanently_disabled || !S || fuel_remaining() < init_reserve) return false;
    if (shared_memory.size() < 8 || constants::max_rounds > 2047
        || S->groups.empty() || S->groups.size() > 65535) {
        rt.permanently_disabled = true; return false;
    }
    struct Body { std::uint32_t id; champion_type type; double hp; bool melee; };
    auto allies = champions(team::ally);
    if (allies.size() < 2 || allies.size() > 6) { rt.permanently_disabled = true; return false; }
    std::vector<Body> bodies;
    for (const auto& ally : allies) {
        const auto type = ally.champion_name(); // Public identity, including hidden bodies.
        const auto design = champion(type).base_stats(); // Design-only, never hidden live HP.
        if (!std::isfinite(design.base_health) || design.base_health <= 0 || design.base_attack_range < 0)
            return false;
        bodies.push_back({actor_id(ally), type, design.base_health, design.base_attack_range <= 1});
    }
    std::sort(bodies.begin(), bodies.end(), [](const auto& a, const auto& b) { return a.id < b.id; });
    for (std::size_t i = 1; i < bodies.size(); ++i)
        if (bodies[i - 1].id == bodies[i].id) { rt.permanently_disabled = true; return false; }
    auto unlinked = [](const Body& b) { return b.type != champion_type::cassi && b.type != champion_type::issac; };
    std::vector<Body> candidates;
    for (const auto& body : bodies) if (unlinked(body)) candidates.push_back(body);
    if (candidates.size() < 2) candidates = bodies;
    // Uncalibrated static role preference: a melee, high-base-HP anchor;
    // then range-class diversity, high base HP and stable full-handle ties.
    std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        if (a.melee != b.melee) return a.melee;
        if (a.hp != b.hp) return a.hp > b.hp;
        return a.id < b.id;
    });
    const Body anchor = candidates.front();
    candidates.erase(candidates.begin());
    std::sort(candidates.begin(), candidates.end(), [&](const auto& a, const auto& b) {
        const bool ad = a.melee != anchor.melee, bd = b.melee != anchor.melee;
        if (ad != bd) return ad;
        if (a.hp != b.hp) return a.hp > b.hp;
        return a.id < b.id;
    });
    std::vector<Entry> dictionary;
    dictionary.reserve(S->groups.size());
    for (std::size_t i = 0; i < S->groups.size(); ++i) {
        const auto& group = S->groups[i];
        const auto full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(group.h));
        const auto* object = public_group(group);
        if (!object || static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(*object)) != full)
            return false;
        dictionary.push_back({full, static_cast<int>(i)});
    }
    std::sort(dictionary.begin(), dictionary.end(), [](const auto& a, const auto& b) { return a.full < b.full; });
    for (std::size_t i = 1; i < dictionary.size(); ++i)
        if (dictionary[i - 1].full == dictionary[i].full) { rt.permanently_disabled = true; return false; }
    const auto own = actor_id(self());
    rt.pair.actors = {anchor.id, candidates.front().id};
    for (int slot = 0; slot < 2; ++slot) {
        rt.writers[slot].reset();
        for (const auto& ally : allies)
            if (actor_id(ally) == rt.pair.actors[slot]) rt.writers[slot] = ally;
        if (!rt.writers[slot]) return false;
    }
    rt.pair.own_slot = own == rt.pair.actors[0] ? 0 : own == rt.pair.actors[1] ? 1 : -1;
    rt.pair.group_count = static_cast<std::uint32_t>(dictionary.size());
    rt.dictionary = std::move(dictionary);
    rt.pair.enabled = true;
    return true;
}
std::uint16_t token_for(int group) {
    const auto& rt = runtime;
    if (group < 0 || group >= static_cast<int>(S->groups.size()) || !public_group(S->groups[group])) return 0;
    const auto full = static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[group].h));
    const auto it = std::lower_bound(rt.dictionary.begin(), rt.dictionary.end(), full,
        [](const Entry& entry, std::uint32_t id) { return entry.full < id; });
    if (it == rt.dictionary.end() || it->full != full || it->index != group) return 0;
    return static_cast<std::uint16_t>(1 + (it - rt.dictionary.begin()));
}
QualifierCooperationReport read_report(int slot, int now) {
    auto& rt = runtime;
    QualifierCooperationReport report;
    report.writer = rt.pair.actors[slot];
    const std::size_t offset = static_cast<std::size_t>(slot * 4);
    const auto first = static_cast<std::uint8_t>(shared_memory[offset + 3]);
    if ((first & 0x20u) == 0) return report;
    std::array<std::byte, 3> low{};
    shared_memory.read(offset, std::span<std::byte>{low});
    const auto last = static_cast<std::uint8_t>(shared_memory[offset + 3]);
    if (first != last || (last & 0x20u) == 0) return report;
    std::uint32_t word = static_cast<std::uint32_t>(last) << 24;
    for (int i = 0; i < 3; ++i) word |= std::to_integer<std::uint32_t>(low[i]) << (8 * i);
    const std::uint16_t token = static_cast<std::uint16_t>(word & 65535u);
    report.reported_round = static_cast<int>((word >> 16) & 2047u);
    report.phase = static_cast<QualifierCooperationPhase>((word >> 27) & 3u);
    report.age = now - report.reported_round;
    if (report.age < 0 || token > rt.dictionary.size()) return report;
    if (token == 0) {
        if (report.phase == QualifierCooperationPhase::Approach || report.phase == QualifierCooperationPhase::Arrived) return report;
    } else {
        const auto& entry = rt.dictionary[token - 1];
        if (!S || entry.index < 0 || entry.index >= static_cast<int>(S->groups.size())
            || static_cast<std::uint32_t>(static_cast<handle_camp_group_t>(S->groups[entry.index].h)) != entry.full
            || !public_group(S->groups[entry.index])) return report;
        report.full_group = entry.full; report.group_index = entry.index;
    }
    report.valid = true; report.fresh = report.age <= 1;
    return report;
}
void invalidate_owned() noexcept {
    const auto& rt = runtime;
    if (!rt.pair.enabled || rt.pair.own_slot < 0 || shared_memory.size() < 8) return;
    try { shared_memory[rt.pair.own_slot * 4 + 3] = static_cast<std::uint8_t>(0); } catch (...) {}
}
void finish(int frame, std::uint32_t actor) noexcept {
    auto& rt = runtime;
    if (!rt.pair.enabled || rt.pair.own_slot < 0 || frame < 0) return;
    try {
        const auto now = static_cast<int>(round_number());
        if (!S || actor_id(self()) != actor || now != frame || S->round != frame) {
            invalidate_owned(); return;
        }
        if (rt.last_committed_round == now) return;
        if (fuel_remaining() < publish_reserve) { invalidate_owned(); return; }
        auto& me = self();
        std::uint16_t token = rt.previous_token;
        auto phase = token ? QualifierCooperationPhase::Withdraw : QualifierCooperationPhase::None;
        const bool alive = me.is_alive();
        if (!alive) phase = QualifierCooperationPhase::Withdraw;
        const bool active = alive && S->mode == Mode::farm && S->mission < 0
            && !me.is_recalling() && !me.is_submitting_gems() && !me.is_teleporting()
            && S->target >= 0 && S->target == S->qualifier_committed_target;
        if (active) {
            token = token_for(S->target);
            if (token) {
                phase = QualifierCooperationPhase::Approach;
                const auto& group = S->groups[S->target];
                try {
                    const auto state = public_group(group)->state();
                    if (std::holds_alternative<map::camp_group_respawning>(state)) phase = QualifierCooperationPhase::Withdraw;
                    else if (std::holds_alternative<map::camp_group_reward_available>(state)
                        && qualifier_tiles(group).is_set(me.position())) {
                        const auto capture = public_group(group)->capture_state();
                        if (capture.timer >= 0 && (capture.state == map::camp_group_capture_type::ally_capturing
                            || capture.state == map::camp_group_capture_type::capture_available))
                            phase = QualifierCooperationPhase::Arrived;
                    }
                } catch (const cambc::error&) { /* Intent only; unknown is never an arrival certificate. */ }
            }
        }
        if (!token && phase != QualifierCooperationPhase::Withdraw) phase = QualifierCooperationPhase::None;
        if (static_cast<int>(round_number()) != frame || !qualifier_current()) { invalidate_owned(); return; }
        rt.generation = static_cast<std::uint8_t>((rt.generation + 1u) & 3u);
        const std::uint32_t word = static_cast<std::uint32_t>(token)
            | (static_cast<std::uint32_t>(frame) << 16)
            | (static_cast<std::uint32_t>(phase) << 27) | commit_bit
            | (static_cast<std::uint32_t>(rt.generation) << 30);
        const std::size_t offset = static_cast<std::size_t>(rt.pair.own_slot * 4);
        const auto header = static_cast<std::uint8_t>(word >> 24);
        shared_memory[offset + 3] = static_cast<std::uint8_t>(header & ~0x20u);
        std::array<std::byte, 3> low{};
        for (int i = 0; i < 3; ++i) low[i] = static_cast<std::byte>((word >> (8 * i)) & 255u);
        shared_memory.write(offset, std::span<const std::byte>{low});
        if (static_cast<int>(round_number()) != frame) { invalidate_owned(); return; }
        // Reserve before the final host call. A throw/uncertain return cannot
        // lead to a second committed publication by another guard this round.
        rt.last_committed_round = frame;
        shared_memory[offset + 3] = header;
        rt.previous_token = token;
        if (static_cast<int>(round_number()) != frame) invalidate_owned();
    } catch (...) { invalidate_owned(); } // Query/write errors mean Unknown, not inferred death/withdrawal.
}
} // namespace qualifier_cooperation_detail

void qualifier_cooperation_begin() noexcept {
    namespace d = qualifier_cooperation_detail;
    auto& rt = d::runtime;
    rt.nomination.reset(); rt.reports = {}; rt.frame_round = -1;
    try {
        if (!S || !d::initialize() || rt.pair.own_slot < 0) return;
        const int now = static_cast<int>(round_number());
        if (now < 0 || now > 2047 || now > static_cast<int>(constants::max_rounds)
            || S->round != now || fuel_remaining() < d::read_reserve) return;
        rt.frame_actor = d::actor_id(self()); rt.frame_round = now;
        if (rt.last_begin_round == now) return;
        rt.last_begin_round = now;
        rt.reports = {};
        for (int slot = 0; slot < 2; ++slot) rt.reports[slot] = d::read_report(slot, now);
        if (static_cast<int>(round_number()) != now) { rt.reports = {}; rt.frame_round = -1; return; }
        const auto& own = rt.reports[rt.pair.own_slot];
        if (own.valid && own.reported_round == now) rt.last_committed_round = now;
        const auto& other = rt.reports[1 - rt.pair.own_slot];
        if (!other.valid || !other.fresh || other.group_index < 0
            || (other.phase != QualifierCooperationPhase::Approach && other.phase != QualifierCooperationPhase::Arrived)) return;
        // A dated intention cannot survive a currently known dead writer.
        // Public life is separate from hidden position/HP. Failed queries
        // leave no nomination; they never establish death or a safe vacancy.
        try {
            const auto& writer = rt.writers[1 - rt.pair.own_slot];
            if (!writer || !writer->is_alive()) return;
        } catch (...) { return; }
        // The secondary may inspect the anchor's offer. The anchor only
        // consumes a same-job arrival, avoiding two competing chase proposals.
        if (rt.pair.own_slot == 0 && (other.phase != QualifierCooperationPhase::Arrived
            || S->qualifier_committed_target != other.group_index)) return;
        QualifierCooperationNomination nomination;
        nomination.writer = other.writer; nomination.full_group = other.full_group;
        nomination.group_index = other.group_index; nomination.reported_round = other.reported_round;
        nomination.age = other.age; nomination.phase = other.phase;
        nomination.writer_is_anchor = rt.pair.own_slot == 1;
        const auto& group = S->groups[other.group_index];
        try {
            const auto state = d::public_group(group)->state();
            nomination.lifecycle_known = true;
            if (std::holds_alternative<map::camp_group_alive>(state)) nomination.lifecycle = 0;
            else if (std::holds_alternative<map::camp_group_reward_available>(state)) {
                nomination.lifecycle = 1;
                const auto capture = d::public_group(group)->capture_state();
                if (capture.timer >= 0) {
                    nomination.capture_known = true; nomination.capture_timer = capture.timer;
                    nomination.capture_status = static_cast<int>(capture.state);
                }
            } else nomination.lifecycle = 2;
        } catch (const cambc::error&) { /* Reported intent remains dated; current facts remain unknown. */ }
        if (nomination.lifecycle == 2 || (other.phase == QualifierCooperationPhase::Arrived
            && nomination.lifecycle_known && nomination.lifecycle != 1)) return;
        if (static_cast<int>(round_number()) == now) rt.nomination = nomination;
    } catch (...) { rt.reports = {}; rt.nomination.reset(); rt.frame_round = -1; }
}

class QualifierCooperationEndInput {
    int frame_ = -1;
    std::uint32_t actor_ = 0;
public:
    QualifierCooperationEndInput() noexcept
        : frame_{qualifier_cooperation_detail::runtime.frame_round}, actor_{qualifier_cooperation_detail::runtime.frame_actor} {}
    QualifierCooperationEndInput(const QualifierCooperationEndInput&) = delete;
    QualifierCooperationEndInput& operator=(const QualifierCooperationEndInput&) = delete;
    ~QualifierCooperationEndInput() noexcept { qualifier_cooperation_detail::finish(frame_, actor_); }
};

const QualifierCooperationPair& qualifier_cooperation_pair() noexcept {
    return qualifier_cooperation_detail::runtime.pair;
}
const std::array<QualifierCooperationReport, 2>& qualifier_cooperation_reports() noexcept {
    return qualifier_cooperation_detail::runtime.reports;
}
std::optional<QualifierCooperationNomination> qualifier_cooperation_nomination() noexcept {
    const auto& rt = qualifier_cooperation_detail::runtime;
    try {
        if (rt.nomination && S && S->round == rt.frame_round
            && static_cast<int>(round_number()) == rt.frame_round
            && qualifier_cooperation_detail::actor_id(self()) == rt.frame_actor) return rt.nomination;
    } catch (...) {}
    return {};
}
