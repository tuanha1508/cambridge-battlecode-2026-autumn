#pragma once
#include "z254_switch.h"
#if Z254_ON
#include "z254_layout.h"
namespace z254_ {
// active[0]: 01concentrated_wealth.
// active[1]: 00road_of_riches.
// active[2]: 01gold_rush.
// active[3]: 00great_wall.
// active[4]: 00eight.
// active[5]: 01starvation.
inline bool enabled(int id){static constexpr bool active[]={false,false,false,false,true,false};return id>0 && id<=6 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{1,2,9,0,6,3,4,5,7,8,10,11},{11,5,3,0,8,1,2,4,6,7,9,10},{11,1,6,0,2,3,4,5,7,8,9,10},{9,2,3,0,8,1,4,5,6,7,10,11},{3,4,1,11,5,0,2,6,7,8,9,10},{11,2,1,0,6,3,4,5,7,8,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{5,51},{15,113},{-1,-1},{15,114},{-1,-1},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
