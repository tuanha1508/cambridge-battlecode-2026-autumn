#pragma once
#include <cambc.h>
#include <array>
#include <cstring>
#include <vector>
namespace z135_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m2_0[]={71776119061217280ULL,281474976710656ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,141289400074494ULL,0ULL,0ULL,0ULL,211930866253824ULL,6944690808842878976ULL,0ULL,0ULL,1729382256910270464ULL,0ULL,0ULL,0ULL,0ULL,6316128ULL,0ULL,0ULL,273744439810072ULL,16557098926080ULL,0ULL,0ULL,0ULL,289360691284934656ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,64516ULL,256ULL,0ULL,0ULL,4261412864ULL,50331648ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2_1[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2_2[]={0ULL,72057594037927936ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,72340172838076673ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,16843009ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,32896ULL,65535ULL,65535ULL,1799ULL,144678138029277184ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,144680345676153346ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,131586ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2_3[]={4096ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,1048576ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,134217728ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,18014398509481984ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2305843009213693952ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,32ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2_4[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2025524718735458304ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m2_5[]={0ULL,0ULL,0ULL,0ULL,0ULL,965845450752ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint32_t c2[]={0,300,256,150,9,0,4,1,1,20,2,0,2,22,2,0,1,24,2,0,0,26,2,1,0,28,2,2,0,30,2,2,1,32,2,1,2,34,2,2,2,1,300,256,150,9,1,4,48,48,21,2,49,47,23,2,49,48,25,2,49,49,27,2,48,49,29,2,47,49,31,2,47,48,33,2,48,47,35,2,47,47,2,100,15,50,4,2,0,20,3,4,0,20,2,56,3,19,3,58,2,19,2,3,100,15,50,4,3,0,29,46,5,0,29,47,57,3,30,46,59,2,30,47,4,40,11,50,2,6,0,34,3,66,2,34,4,5,40,11,50,2,7,0,15,46,67,2,15,45,6,40,4,40,1,8,0,46,10,7,40,4,40,1,9,0,3,39,8,60,4,50,2,10,0,46,18,54,3,45,18,9,60,4,50,2,11,0,3,31,55,3,4,31,10,130,22,60,4,12,1,36,11,60,3,35,11,62,3,36,12,64,2,35,12,11,130,22,60,4,13,1,13,38,61,3,14,38,63,3,13,37,65,2,14,37,12,40,0,50,2,14,3,23,18,48,3,24,18,13,40,0,50,2,15,3,26,31,49,3,25,31,14,40,0,50,2,16,3,18,13,46,3,19,13,15,40,0,50,2,17,3,31,36,47,3,30,36,16,120,8,50,4,18,3,4,10,40,0,3,9,42,0,4,9,44,3,3,10,17,120,8,50,4,19,3,45,39,41,0,46,40,43,0,45,40,45,3,46,39,18,100,15,50,4,36,0,40,25,38,0,41,25,50,3,39,25,52,2,39,24,19,100,15,50,4,37,0,9,24,39,0,8,24,51,3,10,24,53,2,10,25};
inline constexpr Layout layouts[]={
{50,50,5,2,3,{m2_0,m2_1,m2_2,m2_3,m2_4,m2_5},56,c2,372}
};
inline int detect() {
 const int w=map::width(),h=map::height(),n=static_cast<int>(draft::team_size());
 for (const auto& r:layouts) {
  if (w!=r.w || h!=r.h || n!=r.n || static_cast<int>(map::symmetry())!=r.sym) continue;
  const std::size_t bytes=bitboard::raw_buffer_size();
  if(bytes!=r.words*sizeof(std::uint64_t))continue;
  const bitboard* bb[]={&map::walls(),&map::holes(),&map::bushes(),&map::altars()};
  bool same=true;
  for(int i=0;i<4;++i)if(std::memcmp(bb[i]->ptr(),r.masks[i],bytes)) {same=false;break;}
  if(!same)continue;
  // The two absolute spawn masks are unchanged when playing side B.
  const auto* own=map::spawn_region(team::ally).ptr();
  const auto* foe=map::spawn_region(team::enemy).ptr();
  if(!((!std::memcmp(own,r.masks[4],bytes)&&!std::memcmp(foe,r.masks[5],bytes)) ||
       (!std::memcmp(own,r.masks[5],bytes)&&!std::memcmp(foe,r.masks[4],bytes))))continue;
  std::vector<std::uint32_t> camps(r.count);std::uint32_t count=0;
  detail::check(detail::sdk_map_camp_layout(camps.data(),static_cast<std::uint32_t>(camps.size()),&count));
  if(count!=r.count || std::memcmp(camps.data(),r.camps,r.count*sizeof(std::uint32_t)))continue;
  return r.id;
 }
 return 0;
}

}
