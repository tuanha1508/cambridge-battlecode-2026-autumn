#pragma once
#ifndef Z170_ON
#define Z170_ON 1
#endif
// Z170 caps the existing primary bank trigger; no combat gate changes.
#ifndef Z170_THRESHOLD
#define Z170_THRESHOLD 32
#endif
#if Z170_ON
static constexpr int z170_bank_threshold(int original) {
    return original < Z170_THRESHOLD ? original : Z170_THRESHOLD;
}
#endif
