#pragma once
#include "z160_switch.h"
#if Z160_ON
#include "z160_layout.h"
namespace z160_ {
// active[0]: 00rushbait-v2.
// active[1]: 01boss-battle.
// active[2]: 01corridors.
// active[3]: 00gem-fields.
// active[4]: 00elkring.
inline bool enabled(int id){static constexpr bool active[]={true,false,false,false,true};return id>0 && id<=5 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,2,6,10,0,1,3,4,5,7,8,9},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,6,0,1,3,4,5,7,8,9,10},{11,2,6,0,1,3,4,5,7,8,9,10},{2,6,1,0,11,3,4,5,7,8,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{5,51},{-1,-1},{15,115},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
