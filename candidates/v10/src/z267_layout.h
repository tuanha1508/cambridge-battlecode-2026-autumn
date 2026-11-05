#pragma once
#include <cambc.h>
#include <cstring>
#include <vector>
namespace z267_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m1[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2[]={0ULL,0ULL,0ULL,0ULL,16711680ULL,16711680ULL,16711680ULL,8323072ULL,1095216660480ULL,1095216660480ULL,1095216660480ULL,545460846592ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m3[]={1125899906842624ULL,0ULL,0ULL,4503599627370496ULL,288230376151711744ULL,0ULL,0ULL,1152921504606846976ULL,0ULL,0ULL,0ULL,0ULL,4ULL,0ULL,0ULL,16ULL};
inline constexpr std::uint64_t m4[]={460551ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,31648823730241536ULL};
inline constexpr std::uint32_t c[]={0,120,8,50,4,0,0,8,2,2,0,8,3,27,3,7,2,31,3,7,3,1,120,8,50,4,1,0,22,28,3,0,22,27,26,3,23,28,30,3,23,27,2,100,28,50,9,4,3,12,18,6,3,11,17,12,2,11,18,14,2,12,17,32,2,10,17,34,3,10,18,36,3,10,16,38,3,12,16,40,2,11,16,3,100,28,50,9,5,3,18,12,7,3,19,13,13,2,19,12,15,2,18,13,33,2,20,13,35,3,20,12,37,3,20,14,39,3,18,14,41,2,19,14,4,120,8,50,4,8,0,16,2,10,0,16,3,25,3,15,2,29,3,15,3,5,120,8,50,4,9,0,14,28,11,0,14,27,24,3,15,28,28,3,15,27,6,220,30,60,4,16,1,24,3,18,1,24,2,21,3,23,2,23,3,23,3,7,220,30,60,4,17,1,6,27,19,1,6,28,20,3,7,28,22,3,7,27};
inline constexpr Layout layouts[]={{31,31,3,2,1,{m0,m1,m2,m3,m4,m5},16,c,208}};
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
