#pragma once
#include "z226_switch.h"
#if Z226_ON
#include "z226_layout.h"
namespace z226_ {
// active[0]: 01the-farm.
// active[1]: 01boss-battle.
// active[2]: 01starfish.
inline bool enabled(int id){static constexpr bool active[]={false,true,true};return id>0 && id<=3 && active[id-1];}
inline std::array<int,12> order(int id){static constexpr std::array<int,12> orders[]={{11,0,8,1,2,3,4,5,6,7,9,10},{9,2,1,0,8,3,4,5,6,7,10,11},{11,6,8,0,1,2,3,4,5,7,9,10}};return orders[id-1];}
inline void pick(int layout){const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){const auto candidate=static_cast<champion_draft>(id);if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
 int selected_id=id;
 if(layout==2 && id==8 && std::find(chosen.begin(),chosen.end(),static_cast<champion_draft>(6))==chosen.end()
    && chosen.size()+1==draft::team_size())selected_id=6;
 const auto selected_candidate=static_cast<champion_draft>(selected_id);
 std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(selected_id));
 if(layout==1){plan.resize(10,0);plan[9]=70;}
 static constexpr int markers[][2]={{14,98},{5,51},{9,70}};const int pos=markers[layout-1][0];if(pos>=0){const int marker=markers[layout-1][1];plan.resize(pos+1,(layout==1 || marker>=113)?255:0);plan[pos]=marker;}
 draft_memory=plan;draft::pick(selected_candidate);return;}
}
}
#endif
