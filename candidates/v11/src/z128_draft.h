#pragma once
#include "z128_switch.h"
#if Z128_ON
#include "z128_layout.h"
namespace z128_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,true,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{8,1,6,0,11,4,2,5,3,7,9,10},{6,1,8,11,0,4,2,5,3,7,9,10},{1,2,3,0,6,9,4,5,8,7,10,11},{11,1,6,8,0,4,2,5,3,7,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{5,51},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
