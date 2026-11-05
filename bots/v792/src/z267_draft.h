#pragma once
#include "z267_switch.h"
#if Z267_DRAFT
#include "z267_layout.h"
#include <array>
#include <algorithm>
namespace z267_ {
inline void pick(){
 const auto chosen=cambc::draft::current_draft(cambc::team::ally);
 static constexpr std::array<int,12> order={6,10,0,2,1,3,4,5,7,8,9,11};
 for(const int id:order){const auto c=static_cast<cambc::champion_draft>(id);
  if(std::find(chosen.begin(),chosen.end(),c)!=chosen.end())continue;
  std::vector<std::uint8_t> plan;for(const auto p:chosen)plan.push_back(static_cast<std::uint8_t>(p));plan.push_back(static_cast<std::uint8_t>(id));cambc::draft_memory=plan;cambc::draft::pick(c);return;
 }
}
}
#endif
