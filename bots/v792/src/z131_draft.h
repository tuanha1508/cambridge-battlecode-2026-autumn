#pragma once
#include "z131_switch.h"
#if Z131_ON
#include "z131_layout.h"
namespace z131_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,false,false,false,false,false,true,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,4,8,0,10,1,2,3,5,6,7,9},{11,4,6,0,1,2,3,5,7,8,9,10},{3,2,8,11,0,1,4,5,6,7,9,10},{8,4,3,0,11,1,2,5,6,7,9,10},{11,4,3,8,0,1,2,5,6,7,9,10},{2,4,6,0,8,1,3,5,7,9,10,11},{11,4,6,1,8,0,2,3,5,7,9,10},{11,2,3,10,0,1,4,5,6,7,8,9},{11,4,3,0,10,1,2,5,6,7,8,9}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 auto ids=order(layout);if(layout==3){const auto enemy=draft::current_draft(team::enemy);if(enemy.size()>=3 && static_cast<int>(enemy[0])==4 && static_cast<int>(enemy[1])==10 && static_cast<int>(enemy[2])==3)ids={{3,2,10,7,5,8,11,0,4,6,1,9}};} for(int id:ids){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{8,69}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
