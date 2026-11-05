#pragma once
#include "z251_switch.h"
#if Z251_ON
#include <cambc.h>
#include <algorithm>
#include <vector>
namespace z251_ {
// Exact Shortcut layout was already checked by z158_::detect().
inline void pick(){
 const auto chosen=cambc::draft::current_draft(cambc::team::ally);
 constexpr int order[]={11,2,6,0,1,8,3,4,5,7,9,10};
 for(int id:order){
  const auto candidate=static_cast<cambc::champion_draft>(id);
  if(std::find(chosen.begin(),chosen.end(),candidate)!=chosen.end())continue;
  std::vector<std::uint8_t> plan;
  for(auto c:chosen)plan.push_back(static_cast<std::uint8_t>(c));
  plan.push_back(static_cast<std::uint8_t>(id));
  plan.resize(14,0);plan[13]=85;
  cambc::draft_memory=plan;cambc::draft::pick(candidate);return;
 }
}
}
#endif
