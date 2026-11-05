#pragma once
#include <cambc.h>
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>

// Immutable advisory assignments, never live task/actor/route certificates.
namespace qualifier_opening_plan {
using pick_type = cambc::champion_draft;
using body_type = cambc::champion_type;
struct Entry {
    pick_type pick = pick_type::marcella;
    body_type body = body_type::marcella;
    std::uint32_t group = 0;
    bool assigned = false;
};
struct Plan {
    std::uint8_t slots = 0, accepted = 0;
    std::array<Entry, 5> entries{};
};
inline constexpr std::size_t serialized_size = 49;
inline bool valid_pick(pick_type pick) {
    const auto value = static_cast<std::int32_t>(pick);
    return value >= 0 && value < 12;
}
inline bool body_allowed(pick_type pick, body_type body) {
    using P = pick_type;
    using B = body_type;
    switch (pick) {
    case P::ryth: return body == B::ryth;
    case P::vesper_vale: return body == B::vesper_vale;
    case P::vendetta: return body == B::vendetta;
    case P::cassi_and_issac: return body == B::cassi || body == B::issac;
    case P::stellar: return body == B::stellar;
    case P::prism: return body == B::prism;
    case P::marcella: return body == B::marcella;
    case P::willow: return body == B::willow;
    case P::ren: return body == B::ren;
    case P::varg: return body == B::varg;
    case P::tavi: return body == B::tavi;
    case P::heart_of_ruin: return body == B::heart_of_ruin;
    default: return false;
    }
}
inline body_type default_body(pick_type pick) {
    using P = pick_type;
    using B = body_type;
    switch (pick) {
    case P::ryth: return B::ryth;
    case P::vesper_vale: return B::vesper_vale;
    case P::vendetta: return B::vendetta;
    case P::cassi_and_issac: return B::cassi;
    case P::stellar: return B::stellar;
    case P::prism: return B::prism;
    case P::marcella: return B::marcella;
    case P::willow: return B::willow;
    case P::ren: return B::ren;
    case P::varg: return B::varg;
    case P::tavi: return B::tavi;
    case P::heart_of_ruin: return B::heart_of_ruin;
    default: throw std::invalid_argument("invalid opening pick");
    }
}
inline bool valid(const Plan& plan) {
    if (plan.slots < 3 || plan.slots > 5 || plan.accepted > plan.slots) return false;
    for (std::size_t i = 0; i < plan.accepted; ++i) {
        const auto& entry = plan.entries[i];
        if (!valid_pick(entry.pick) || !body_allowed(entry.pick, entry.body)
            || (!entry.assigned && entry.group != 0)) return false;
        for (std::size_t j = 0; j < i; ++j) {
            if (entry.pick == plan.entries[j].pick) return false;
            if (entry.assigned && plan.entries[j].assigned && entry.group == plan.entries[j].group) return false;
        }
    }
    return true;
}
inline std::vector<std::uint8_t> encode(const Plan& plan) {
    if (!valid(plan)) throw std::invalid_argument("invalid opening record");
    std::vector<std::uint8_t> out(serialized_size, 0);
    for (std::size_t i = 0; i < 5; ++i) out[i] = 255;
    out[5] = 'Q'; out[6] = 'O'; out[7] = 'P'; out[8] = '1';
    out[9] = 1; out[10] = static_cast<std::uint8_t>(serialized_size); out[11] = 0;
    out[12] = plan.slots; out[13] = plan.accepted;
    for (std::size_t i = 0; i < plan.accepted; ++i) {
        const auto& entry = plan.entries[i];
        const auto at = 14 + 7 * i;
        out[i] = out[at] = static_cast<std::uint8_t>(entry.pick);
        out[at + 1] = static_cast<std::uint8_t>(entry.body);
        for (unsigned byte = 0; byte < 4; ++byte)
            out[at + 2 + byte] = static_cast<std::uint8_t>(entry.group >> (8 * byte));
        out[at + 6] = entry.assigned ? 1 : 0;
    }
    return out;
}
// Accept the SDK's capacity-sized startup span. Only the declared first49
// bytes belong to this version; untouched trailing engine bytes are ignored.
inline std::optional<Plan> decode(std::span<const std::uint8_t> bytes) {
    if (bytes.size() < serialized_size || bytes[5] != 'Q' || bytes[6] != 'O'
        || bytes[7] != 'P' || bytes[8] != '1' || bytes[9] != 1
        || (std::uint32_t(bytes[10]) | (std::uint32_t(bytes[11]) << 8)) != serialized_size) return {};
    Plan plan;
    plan.slots = bytes[12]; plan.accepted = bytes[13];
    if (plan.slots < 3 || plan.slots > 5 || plan.accepted > plan.slots) return {};
    for (std::size_t i = 0; i < 5; ++i) {
        const auto at = 14 + 7 * i;
        if (i >= plan.accepted) {
            if (bytes[i] != 255) return {};
            for (std::size_t j = 0; j < 7; ++j) if (bytes[at + j] != 0) return {};
            continue;
        }
        if (bytes[i] != bytes[at] || bytes[at + 6] > 1) return {};
        auto& entry = plan.entries[i];
        entry.pick = static_cast<pick_type>(bytes[at]);
        entry.body = static_cast<body_type>(bytes[at + 1]);
        for (unsigned byte = 0; byte < 4; ++byte)
            entry.group |= std::uint32_t(bytes[at + 2 + byte]) << (8 * byte);
        entry.assigned = bytes[at + 6] != 0;
    }
    return valid(plan) ? std::optional<Plan>{plan} : std::nullopt;
}
} // namespace qualifier_opening_plan
