#pragma once
#include "z129_switch.h"
#if Z129_ON
#include "z129_layout.h"
namespace z129_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false,false,true};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,1,0,6,2,3,4,5,8,7,9,10},{11,2,0,1,3,4,5,6,7,8,9,10},{1,2,6,11,3,4,5,0,7,8,9,10},{11,4,8,0,10,1,2,3,5,6,7,9},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,3,0,8,1,4,5,6,7,9,10},{3,0,8,1,2,4,5,6,7,9,10,11},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,0,8,1,3,4,5,6,7,9,10},{11,4,3,0,10,1,2,5,6,7,8,9},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,6,0,1,8,4,5,3,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,1,6,8,0,2,3,5,4,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,6,0,1,8,4,5,3,7,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{5,51},{-1,-1},{14,98},{10,79},{-1,-1},{-1,-1},{7,68},{8,69},{5,51},{-1,-1},{7,68},{7,68},{13,85},{15,114},{15,113},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
