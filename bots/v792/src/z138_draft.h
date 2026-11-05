#pragma once
#include "z138_switch.h"
#if Z138_ON
#include "z138_layout.h"
namespace z138_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,true,false,false,false,true,false,true,false,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,1,0,6,2,3,4,5,8,7,9,10},{6,2,0,1,3,4,5,11,7,8,9,10},{11,4,6,0,10,1,2,3,5,8,7,9},{9,2,5,0,8,1,4,3,6,7,10,11},{11,2,1,0,8,3,4,5,6,7,9,10},{11,2,1,0,8,3,4,5,6,7,9,10},{11,2,3,0,6,1,4,5,8,7,9,10},{11,2,6,0,8,1,4,5,3,7,9,10},{11,2,6,0,1,8,4,5,3,7,9,10},{3,2,8,11,0,1,4,5,6,7,9,10},{6,2,0,8,1,3,4,5,11,7,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 auto ids=order(layout);if(layout==10){const auto enemy=draft::current_draft(team::enemy);if(enemy.size()>=3 && static_cast<int>(enemy[0])==4 && static_cast<int>(enemy[1])==10 && static_cast<int>(enemy[2])==3)ids={{3,2,10,7,5,8,11,0,4,6,1,9}};} for(int id:ids){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{-1,-1},{5,51},{-1,-1},{10,79},{7,68},{7,68},{-1,-1},{-1,-1},{7,68}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];if(marker==98){plan.resize(10,0);plan[9]=70;plan.resize(15,255);}else{plan.resize(pos+1,marker>=113?255:0);}plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
