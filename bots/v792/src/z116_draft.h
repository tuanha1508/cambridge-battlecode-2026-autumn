#pragma once
#include "z116_switch.h"
#if Z116_ON
#include "z116_layout.h"
namespace z116_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,false,false,false,true,true,false};return id>0 && active[id-1];}
inline std::array<int,11> order(int id){
 std::array<int,11> original{};
 static constexpr int swaps[][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{3,6},{4,6},{8,6},{-1,-1}};
 const int old=swaps[id-1][0],replacement=swaps[id-1][1];
 if(old<0)return original;
 static constexpr std::array<int,11> orders[]={{9,4,0,8,1,2,3,5,6,7,10},{8,4,3,0,9,1,2,5,6,7,10},{9,4,3,10,8,0,1,2,5,6,7},{9,2,0,1,3,4,5,6,7,8,10},{9,4,3,10,0,1,2,5,6,7,8},{9,4,8,0,10,1,2,3,5,6,7},{9,2,0,8,1,3,4,5,6,7,10},{9,2,3,0,8,1,4,5,6,7,10}};
 original=orders[id-1];
 for(int& c:original){if(c==old)c=replacement;else if(c==replacement)c=old;}
 return original;
}
inline void pick(int layout){
 const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){
  const int actual=id==9?11:id;const auto candidate=static_cast<champion_draft>(actual);
  if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
  std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));
  plan.push_back(static_cast<std::uint8_t>(actual));
  if(layout==6){plan.resize(10,0);plan[9]=70;}
  draft_memory=plan;draft::pick(candidate);return;
 }
}
}
#endif
