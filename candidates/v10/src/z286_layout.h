#pragma once
#include <cambc.h>
#include <cstring>
#include <vector>
namespace z286_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m1[]={0ULL,0ULL,0ULL,0ULL,0ULL,17361641477096079360ULL,217020518463700992ULL,0ULL,0ULL,61680ULL,771ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2[]={0ULL,0ULL,0ULL,0ULL,9223372036854775808ULL,248ULL,18085043209519231ULL,0ULL,141289400074368ULL,280375465082880ULL,7696581394432ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m3[]={0ULL,0ULL,0ULL,67108864ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,524288ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m4[]={460551ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,61814108848128ULL};
inline constexpr std::uint32_t c[]={0,110,22,60,3,0,1,3,6,6,3,3,7,29,2,4,6,1,110,22,60,3,1,1,26,23,7,3,26,22,28,2,25,23,2,110,22,60,3,2,1,9,2,8,3,10,2,20,2,9,3,3,110,22,60,3,3,1,20,27,9,3,19,27,21,2,20,26,4,110,22,60,3,4,1,3,12,10,3,3,13,27,2,3,11,5,110,22,60,3,5,1,26,17,11,3,26,16,26,2,26,18,6,110,22,60,3,12,1,19,2,14,3,20,2,22,2,20,3,7,110,22,60,3,13,1,10,27,15,3,9,27,23,2,9,26,8,110,22,60,3,16,1,26,9,18,3,26,10,24,2,26,8,9,110,22,60,3,17,1,3,20,19,3,3,19,25,2,3,21};
inline constexpr Layout layouts[]={{30,30,5,2,1,{m0,m1,m2,m3,m4,m5},16,c,170}};
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
