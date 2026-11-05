#pragma once
#include "z300_draft.h"
#include "z294_draft.h"
namespace z326_ {
template<class T> inline bool z326_equal(const T* a,const T* b,std::size_t n){T d=0;for(std::size_t i=0;i<n;++i)d|=a[i]^b[i];return d==0;}
template<class Layout> inline bool z326_match(const Layout& r){
 using namespace cambc;
 if(map::width()!=r.w || map::height()!=r.h || static_cast<int>(draft::team_size())!=r.n || static_cast<int>(map::symmetry())!=r.sym || bitboard::raw_buffer_size()!=r.words*sizeof(std::uint64_t))return false;
 const bitboard* bb[]={&map::walls(),&map::holes(),&map::bushes(),&map::altars()};
 for(int i=0;i<4;++i)if(!z326_equal(bb[i]->ptr(),r.masks[i],r.words))return false;
 const auto* own=map::spawn_region(team::ally).ptr();const auto* foe=map::spawn_region(team::enemy).ptr();
 if(!((z326_equal(own,r.masks[4],r.words)&&z326_equal(foe,r.masks[5],r.words)) || (z326_equal(own,r.masks[5],r.words)&&z326_equal(foe,r.masks[4],r.words))))return false;
 std::vector<std::uint32_t> camps(r.count);std::uint32_t n=0;
 detail::check(detail::sdk_map_camp_layout(camps.data(),static_cast<std::uint32_t>(camps.size()),&n));
 return n==r.count && z326_equal(camps.data(),r.camps,r.count);
}
inline int detect(){
#if Z326_CORRIDORS
 if(cambc::map::width()==51 && cambc::map::height()==40 && z326_match(z300_::layouts[1]))return 1;
#endif
#if Z326_WEALTH
 if(cambc::map::width()==61 && cambc::map::height()==61 && z326_match(z294_::layouts[0]))return 2;
#endif
 return 0;
}
inline void pick(){
 const auto chosen=cambc::draft::current_draft(cambc::team::ally);
 for(int id : {11,5,2,1,6,0,3,4,7,8,9,10}){
  const auto c=static_cast<cambc::champion_draft>(id);
  if(std::find(chosen.begin(),chosen.end(),c)!=chosen.end())continue;
  std::vector<std::uint8_t> p;for(auto v:chosen)p.push_back(static_cast<std::uint8_t>(v));p.push_back(id);
  p.resize(16,255);p[15]=229;cambc::draft_memory=p;cambc::draft::pick(c);return;
 }
}
}
