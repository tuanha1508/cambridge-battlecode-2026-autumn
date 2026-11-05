#pragma once
// Z38: independent decisions only. No engine/version/roster dispatcher.
#ifndef CXV_RECALL_ELK
#define CXV_RECALL_ELK 1
#endif
#ifndef CXV_BANK11
#define CXV_BANK11 0
#endif
#ifndef CXV_TAVI14
#define CXV_TAVI14 0
#endif
#if CXV_BANK11
#define CXV_NEAR_BANK 11
#else
#define CXV_NEAR_BANK 20
#endif
#if CXV_TAVI14
#define CXV_TAVI_BURST 1.4f
#else
#define CXV_TAVI_BURST 2.2f
#endif
