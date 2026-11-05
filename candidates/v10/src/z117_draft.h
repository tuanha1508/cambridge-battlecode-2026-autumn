#pragma once
#include "z117_switch.h"
#if Z117_ON
#include "z117_layout.h"
namespace z117_ {
inline bool enabled(int id){static constexpr bool active[]={false,false,false,true,false,true,false,false,false,false,true,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,false,true,false};return id>0 && active[id-1];}
inline std::array<int,12> order(int id){
 static constexpr std::array<int,12> orders[]={{8,4,0,1,2,3,5,6,7,9,10,11},{11,4,3,8,0,1,2,5,6,7,9,10},{11,2,0,8,1,3,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,4,8,0,10,1,2,3,5,6,7,9},{8,4,0,1,2,3,5,6,7,9,10,11},{11,4,3,0,8,1,2,5,6,7,9,10},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,0,1,3,4,5,6,7,8,9,10},{11,2,6,0,8,1,3,4,5,7,9,10},{11,2,0,1,3,4,5,6,7,8,9,10},{11,4,8,0,10,1,2,3,5,6,7,9},{11,2,3,0,8,1,4,5,6,7,9,10},{11,4,3,0,8,1,2,5,6,7,9,10},{3,4,8,11,0,1,2,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,0,8,1,3,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,4,3,0,10,1,2,5,6,7,8,9},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,4,8,0,1,3,5,6,7,9,10},{9,2,3,0,8,1,4,5,6,7,10,11},{11,2,3,0,1,4,5,6,7,8,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{3,0,8,1,2,4,5,6,7,9,10,11},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10},{11,2,3,0,8,1,4,5,6,7,9,10}};
 static constexpr int swaps[][2]={{-1,-1},{-1,-1},{-1,-1},{3,6},{-1,-1},{4,6},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{11,6},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{3,6},{3,6}};
 auto original=orders[id-1];if(id==14){const auto chosen=draft::current_draft(team::ally);const auto enemy=draft::current_draft(team::enemy);auto has=[](const auto& v,champion_draft c){return std::find(v.begin(),v.end(),c)!=v.end();};if(has(chosen,champion_draft::vendetta)||(!has(chosen,champion_draft::stellar)&&has(enemy,champion_draft::varg))){for(int& c:original){if(c==4)c=2;else if(c==2)c=4;}}}
 const int old=swaps[id-1][0],replacement=swaps[id-1][1];
 if(old>=0)for(int& c:original){if(c==old)c=replacement;else if(c==replacement)c=old;}
 return original;
}
inline void pick(int layout){
 const auto chosen=draft::current_draft(team::ally);
 for(int id:order(layout)){
  const int actual=id;const auto candidate=static_cast<champion_draft>(actual);
  if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
  std::vector<std::uint8_t> plan;for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));plan.push_back(static_cast<std::uint8_t>(actual));
  static constexpr int markers[][2]={{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{-1,-1},{5,51},{-1,-1},{15,115},{10,80},{6,61},{-1,-1},{-1,-1},{-1,-1},{15,114},{7,68},{15,113},{8,69},{-1,-1},{7,68},{13,85},{5,51},{-1,-1},{10,79},{-1,-1},{14,98},{-1,-1},{7,68},{-1,-1},{-1,-1}};const int pos=markers[layout-1][0];
  if(pos>=0){const int marker=markers[layout-1][1];if(marker==98){plan.resize(10,0);plan[9]=70;plan.resize(15,255);}else{plan.resize(pos+1,marker>=113?255:0);}plan[pos]=marker;}
  draft_memory=plan;draft::pick(candidate);return;
 }
}
}
#endif
