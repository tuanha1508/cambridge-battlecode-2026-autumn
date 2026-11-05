#pragma once
#include "z134_switch.h"
#if Z134_ON
#include "z134_layout.h"
namespace z134_ {
inline bool enabled(int id){static constexpr bool active[]={true,true,false,true,true,true,false,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,1,6,2,3,4,5,0,7,8,9,10},{11,2,1,0,6,3,4,5,8,7,9,10},{11,2,0,8,1,3,4,5,6,7,9,10},{8,6,1,0,11,3,2,5,4,7,9,10},{11,2,6,8,0,1,4,5,3,7,9,10},{5,4,6,0,10,1,2,11,3,7,8,9},{11,2,0,1,3,4,5,6,7,8,9,10},{11,4,8,0,10,1,2,3,5,6,7,9}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{8,69},{-1,-1},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
