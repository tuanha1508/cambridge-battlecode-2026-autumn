#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

namespace qualifier {
struct PublicWorker { std::uint32_t id; };
struct PublicJob { std::uint32_t id; double work; };

// A static workload preference, not a promise that another body will help.
// Results index the supplied workers; -1 means no preference is available.
inline std::vector<int> preferred_workers(const std::vector<PublicWorker>& workers,
                                          const std::vector<PublicJob>& jobs) {
    if (workers.size() > 16 || jobs.size() > 512) return {};
    std::vector<int> result(jobs.size(), -1);
    if (workers.empty()) return result;
    std::vector<std::uint32_t> identities;
    for (const auto worker : workers) identities.push_back(worker.id);
    std::sort(identities.begin(), identities.end());
    if (std::adjacent_find(identities.begin(), identities.end()) != identities.end()) return result;
    identities.clear();
    for (const auto job : jobs) identities.push_back(job.id);
    std::sort(identities.begin(), identities.end());
    if (std::adjacent_find(identities.begin(), identities.end()) != identities.end()) return result;

    std::vector<std::size_t> order;
    for (std::size_t i = 0; i < jobs.size(); ++i)
        if (std::isfinite(jobs[i].work) && jobs[i].work >= 0) order.push_back(i);
    std::sort(order.begin(), order.end(), [&](auto a, auto b) {
        return jobs[a].work != jobs[b].work ? jobs[a].work > jobs[b].work : jobs[a].id < jobs[b].id;
    });
    auto tie_key = [](std::uint32_t group, std::uint32_t worker) {
        // Defined unsigned arithmetic, identical across actor instances.
        std::uint64_t key = (std::uint64_t(group) << 32) | worker;
        key += 0x9e3779b97f4a7c15ULL;
        key = (key ^ (key >> 30)) * 0xbf58476d1ce4e5b9ULL;
        key = (key ^ (key >> 27)) * 0x94d049bb133111ebULL;
        return key ^ (key >> 31);
    };
    std::vector<double> load(workers.size(), 0);
    for (const auto j : order) {
        std::size_t best = 0;
        for (std::size_t w = 1; w < workers.size(); ++w) {
            const auto key = tie_key(jobs[j].id, workers[w].id);
            const auto best_key = tie_key(jobs[j].id, workers[best].id);
            if (load[w] < load[best] || (load[w] == load[best] &&
                (key < best_key || (key == best_key && workers[w].id < workers[best].id)))) best = w;
        }
        const double updated = load[best] + jobs[j].work;
        if (!std::isfinite(updated)) return std::vector<int>(jobs.size(), -1);
        result[j] = static_cast<int>(best);
        load[best] = updated;
    }
    return result;
}
} // namespace qualifier
