#pragma once
#include "z180_switch.h"
#if Z180_ON
#include "z180_layout.h"
namespace z180_ {
// active[0]: 00eight.
// active[1]: 00rows.
// active[2]: 00gem_fields.
inline bool enabled(int id){return id==1;}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{3,4,1,11,7,0,2,5,6,8,9,10},{1,2,6,7,0,3,4,5,8,9,10,11},{11,2,6,0,7,1,3,4,5,8,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{15,115}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
