#pragma once
#include <cambc.h>
#include <vector>
namespace z289_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m1[]={9259400833873739776ULL,284837801885696ULL,4278190080ULL,579842880152010752ULL,9277485877083259072ULL,61440ULL,13853178145287308032ULL,0ULL,865817028361977984ULL,1031262372097ULL,6935755097179717760ULL,0ULL,14694408ULL,16711680ULL,465968ULL,0ULL};
inline constexpr std::uint64_t m2[]={0ULL,0ULL,0ULL,0ULL,0ULL,569599502319616ULL,1152921504606846976ULL,0ULL,0ULL,0ULL,461848ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m3[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,8589934592ULL,0ULL,0ULL,4096ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m4[]={235802112ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,120730681344ULL};
inline constexpr std::uint32_t c[]={0,80,78,50,12,0,2,14,14,1,2,15,15,2,2,13,15,3,2,16,14,4,2,14,16,5,2,15,13,6,0,15,14,7,0,14,15,8,2,14,13,9,2,15,16,10,2,13,14,11,2,16,15,1,20,0,50,1,12,3,2,15,2,20,0,50,1,13,3,27,14,3,0,7,50,1,14,2,13,6,4,0,7,50,1,15,2,16,23,5,20,0,50,1,16,3,20,2,6,20,0,50,1,17,3,9,27,7,0,7,50,1,18,2,8,1,8,0,7,50,1,19,2,21,28,9,0,7,50,1,20,2,26,9,10,0,7,50,1,21,2,3,20,11,60,4,50,2,22,0,7,5,26,3,6,5,12,60,4,50,2,23,0,22,24,27,3,23,24,13,110,22,60,3,24,1,21,6,30,3,20,6,32,2,22,6,14,110,22,60,3,25,1,8,23,31,3,9,23,33,2,7,23,15,0,7,50,1,28,2,8,10,16,0,7,50,1,29,2,21,19,17,0,7,50,1,34,2,2,9,18,0,7,50,1,35,2,27,20};
inline constexpr Layout layouts[]={{30,30,4,2,1,{m0,m1,m2,m3,m4,m5},16,c,239}};
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
