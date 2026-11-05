#pragma once
#include "z123_switch.h"
#if Z123_ON
#include "z123_layout.h"
namespace z123_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,false,false,false,false,false,false,false,false,false,true,true,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{8,4,3,0,11,1,2,5,6,7,9,10},{11,4,3,10,8,0,1,2,5,6,7,9},{3,4,8,11,0,1,2,5,6,7,9,10},{11,2,0,8,1,3,4,5,6,7,9,10},{9,2,3,0,8,1,4,5,6,7,10,11},{11,4,3,8,0,1,2,5,6,7,9,10},{11,2,0,1,3,4,5,6,7,8,9,10},{11,4,8,0,10,1,2,3,5,6,7,9},{11,2,0,1,3,4,5,6,7,8,9,10},{11,4,6,0,1,8,2,5,3,7,9,10},{1,2,6,0,11,4,5,3,7,8,9,10},{3,0,8,1,2,4,5,6,7,9,10,11}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 auto ids=order(layout);if(layout==12){const auto enemy=draft::current_draft(team::enemy);const auto has=[&](const auto& v,int id){return std::find(v.begin(),v.end(),static_cast<champion_draft>(id))!=v.end();};if(has(chosen,2)||(!has(chosen,4)&&has(enemy,9)))for(auto& id:ids){if(id==4)id=2;else if(id==2)id=4;}}for(int id:ids){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(id));
 static constexpr int markers[][2]={{7,68},{7,68},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{5,51},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{14,98}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,marker>=113?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(candidate);return;}
}
}
#endif
