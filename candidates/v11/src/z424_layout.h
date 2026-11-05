#pragma once
#include <cambc.h>
#include <vector>
namespace z424_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0_0[]={0ULL,506096457711878144ULL,0ULL,2020994756598824960ULL,0ULL,0ULL,4042309760ULL,9259542537192932103ULL,4557431716649631744ULL,3773855804ULL,16842752ULL,0ULL,0ULL,16709760ULL,16777023ULL,983808ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_1[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,289360691352043520ULL,0ULL,0ULL,0ULL,0ULL,0ULL,1130315200594948ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_2[]={0ULL,4051049437470916608ULL,9259541571362095104ULL,217020505578799104ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,12811792ULL,6340608ULL,1576705ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_3[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,4ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_4[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2025493932409880576ULL,0ULL,0ULL,0ULL,0ULL,0ULL,28ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,506373483102470144ULL,0ULL,0ULL,0ULL,0ULL,0ULL,7ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint32_t c0[]={0,60,4,50,2,0,0,3,26,2,3,4,26,1,60,4,50,2,1,0,33,26,3,3,32,26,2,60,4,50,2,4,0,7,2,6,3,8,2,3,60,4,50,2,5,0,29,2,7,3,28,2,4,40,0,50,2,8,3,8,21,10,3,9,21,5,40,0,50,2,9,3,28,21,11,3,27,21,6,20,0,50,1,12,3,10,13,7,20,0,50,1,13,3,26,13,8,0,21,50,3,14,2,16,8,16,2,16,9,19,2,16,7,9,0,21,50,3,15,2,20,8,17,2,20,9,20,2,20,7,10,300,200,150,1,18,4,18,5,11,40,0,50,2,21,3,14,23,23,3,15,23,12,40,0,50,2,22,3,22,23,24,3,21,23};
inline constexpr Layout layouts[]={{37,31,3,0,1,{m0_0,m0_1,m0_2,m0_3,m0_4,m0_5},24,c0,165}};
inline bool eq64(const std::uint64_t*a,const std::uint64_t*b,std::size_t n){std::uint64_t d=0;for(std::size_t i=0;i<n;++i)d|=a[i]^b[i];return d==0;}
inline bool eq32(const std::uint32_t*a,const std::uint32_t*b,std::size_t n){std::uint32_t d=0;for(std::size_t i=0;i<n;++i)d|=a[i]^b[i];return d==0;}
inline int detect() {
 const int w=map::width(),h=map::height(),n=static_cast<int>(draft::team_size());
 for (const auto& r:layouts) {
  if (w!=r.w || h!=r.h || n!=r.n || static_cast<int>(map::symmetry())!=r.sym) continue;
  const std::size_t bytes=bitboard::raw_buffer_size();
  if(bytes!=r.words*sizeof(std::uint64_t))continue;
  const bitboard* bb[]={&map::walls(),&map::holes(),&map::bushes(),&map::altars()};
  bool same=true;
  for(int i=0;i<4;++i)if(!eq64(bb[i]->ptr(),r.masks[i],r.words)) {same=false;break;}
  if(!same)continue;
  // The two absolute spawn masks are unchanged when playing side B.
  const auto* own=map::spawn_region(team::ally).ptr();
  const auto* foe=map::spawn_region(team::enemy).ptr();
  if(!((eq64(own,r.masks[4],r.words)&&eq64(foe,r.masks[5],r.words)) ||
       (eq64(own,r.masks[5],r.words)&&eq64(foe,r.masks[4],r.words))))continue;
  std::vector<std::uint32_t> camps(r.count);std::uint32_t count=0;
  detail::check(detail::sdk_map_camp_layout(camps.data(),static_cast<std::uint32_t>(camps.size()),&count));
  if(count!=r.count || !eq32(camps.data(),r.camps,r.count))continue;
  return r.id;
 }
 return 0;
}

}
