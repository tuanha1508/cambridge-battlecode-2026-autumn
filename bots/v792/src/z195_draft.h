#pragma once
#include "z195_switch.h"
#if Z195_ON
#include "z195_layout.h"
namespace z195_ {
// active[0]: 00contest.
// active[1]: 00pilgrimage.
// active[2]: 00road_of_riches.
inline bool enabled(int id){static constexpr bool active[]={true,false,false};return id>0 && id<=3 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,1,6,0,10,2,3,4,5,7,8,9},{11,2,6,1,8,0,3,4,5,7,9,10},{11,2,1,0,8,3,4,5,6,7,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{15,113}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
