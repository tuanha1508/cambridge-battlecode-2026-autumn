#pragma once
// cc_fuel2 (30 Oct): equality-only memcmp for the exact-layout detectors. wasi-libc memcmp walks bytes; on a
// matching 50x65 layout one z195_::detect() cost ~108k fuel (fuelExhausted on HoR's first shop, Pilgrimage).
// Returns 0 iff equal, 1 otherwise; every call site only tests the result against zero.
#include <cstddef>
#include <cstdint>
#include <cstring>
inline int fu2_memcmp(const void* a, const void* b, std::size_t n) {
    const unsigned char* p = static_cast<const unsigned char*>(a);
    const unsigned char* q = static_cast<const unsigned char*>(b);
    std::size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        std::uint64_t x, y;
        __builtin_memcpy(&x, p + i, 8);
        __builtin_memcpy(&y, q + i, 8);
        if (x != y) return 1;
    }
    for (; i < n; ++i) if (p[i] != q[i]) return 1;
    return 0;
}
