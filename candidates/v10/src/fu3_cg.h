#pragma once
// fuel3 (30 Oct): decision-neutral camp-group loader. Returns exactly what
// cambc::map::camp_groups() returns (same groups, order, camps and values) but
// asks the host once with a large buffer instead of 256 words + a resize and
// a second query (Big World/Harvest need ~490 words), and reserves the result.
#include <cambc.h>
#include <vector>
namespace fu3 {
inline std::vector<cambc::map::camp_group> camp_groups() {
    namespace d = cambc::detail;
    constexpr std::uint32_t N = 4096;
    static std::uint32_t words[N];
    std::uint32_t count{};
    d::check(d::sdk_map_camp_layout(words, N, &count));
    if (count > N) return cambc::map::camp_groups();
    std::vector<cambc::map::camp_group> result;
    result.reserve(count / 5 + 1);
    for (std::size_t i = 0; i + 5 <= count;) {
        const auto group = words[i];
        const auto gold = words[i + 1], gems = words[i + 2];
        const auto respawn = static_cast<std::int32_t>(words[i + 3]);
        const auto size = words[i + 4];
        i += 5;
        std::vector<cambc::camp> camps;
        camps.reserve(size);
        for (std::uint32_t m = 0; m < size && i + 4 <= count; ++m, i += 4)
            camps.push_back(d::handle_access::make_camp(static_cast<cambc::handle_camp_t>(words[i]), static_cast<cambc::handle_camp_group_t>(group),
                static_cast<cambc::camp_type>(words[i + 1]), cambc::vec2{ static_cast<std::int32_t>(words[i + 2]), static_cast<std::int32_t>(words[i + 3]) }));
        result.push_back(d::handle_access::make_camp_group(static_cast<cambc::handle_camp_group_t>(group), gold, gems, respawn, std::move(camps)));
    }
    return result;
}
}
