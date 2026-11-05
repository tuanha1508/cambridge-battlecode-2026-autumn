#pragma once
#include "z135_switch.h"
#if Z135_ON
#include "z135_layout.h"
namespace z135_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,1,0,8,3,4,5,6,7,9,10},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,3,0,8,1,4,5,6,7,9,10},{3,0,8,1,2,4,5,6,7,9,10,11},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,0,8,1,3,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{2,6,8,0,10,1,3,4,5,7,9,11},{11,2,6,0,8,1,3,4,5,7,9,10},{6,2,0,1,3,4,5,7,8,9,10,11},{11,2,6,0,8,1,3,4,5,7,9,10},{8,6,0,1,2,3,4,5,7,9,10,11}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{10,79},{-1,-1},{5,51},{-1,-1},{14,98},{5,51},{7,68},{7,68},{7,68},{15,114},{15,113},{6,61},{-1,-1},{10,80},{-1,-1},{-1,-1}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];if(marker==98){plan.resize(10,0);plan[9]=70;plan.resize(15,255);}else{plan.resize(pos+1,marker>=113?255:0);}plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
