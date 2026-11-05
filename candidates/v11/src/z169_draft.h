#pragma once
#include "z169_switch.h"
#if Z169_ON
#include "z169_layout.h"
namespace z169_ {
// active[0]: 00eight.
// active[1]: 00great_wall.
// active[2]: 00interlock.
// active[3]: 00monolith.
// active[4]: 01fracture.
inline bool enabled(int id){static constexpr bool active[]={true,false,true,true,true};return id>0 && id<=5 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{3,4,1,11,0,2,5,6,7,8,9,10},{11,2,6,0,1,3,4,5,7,8,9,10},{11,2,6,0,1,3,4,5,7,8,9,10},{11,2,1,0,6,3,4,5,7,8,9,10},{10,2,6,0,1,3,4,5,7,8,9,11}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{15,114},{-1,-1},{-1,-1},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
