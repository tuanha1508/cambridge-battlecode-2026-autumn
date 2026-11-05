#pragma once
#include <cambc.h>
#include <cstring>
#include <vector>
namespace z275_ {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0_0[]={1157443727212412928ULL,1095216660480ULL,1095216660480ULL,1095216660480ULL,64424509440ULL,0ULL,269488144ULL,33686270ULL,255ULL,272678883688703ULL,578729044791525631ULL,0ULL,1157442765409226992ULL,279275953455359ULL,18303195134134599807ULL,18374686479679946752ULL,1083124540970108936ULL,0ULL,18374686479687356432ULL,18374686479673655296ULL,18374686479671623680ULL,9169399484948414464ULL,578721382569869312ULL,0ULL,4026531840ULL,4278190080ULL,4278190080ULL,4278190080ULL,252184584ULL,0ULL};
inline constexpr std::uint64_t m0_1[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,1157442765139738624ULL,0ULL,4629771061636907008ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,565157600297474ULL,0ULL,134744072ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_2[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2893606912849346560ULL,0ULL,11574427654092267520ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,1412894000743685ULL,0ULL,336860180ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_3[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,4503599627372544ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_4[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,8477364002488320ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,517417236480ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint32_t c0[]={0,0,28,50,4,0,2,20,19,1,2,19,20,2,2,19,19,3,2,20,20,1,160,8,50,6,4,0,15,18,6,0,15,19,8,3,16,18,10,3,16,19,12,3,14,18,14,3,14,19,2,160,8,50,6,5,0,24,21,7,0,24,20,9,3,23,21,11,3,23,20,13,3,25,21,15,3,25,20,3,90,36,60,4,16,1,11,19,18,2,11,18,20,2,10,19,22,2,10,18,4,90,36,60,4,17,1,28,20,19,2,28,21,21,2,29,20,23,2,29,21,5,40,0,50,2,24,3,7,6,32,3,6,6,6,40,0,50,2,25,3,32,33,33,3,33,33,7,110,22,60,3,26,2,6,13,30,1,6,14,114,3,7,14,8,110,22,60,3,27,2,33,26,31,1,33,25,115,3,32,25,9,80,22,50,4,28,2,2,15,112,2,1,15,128,0,1,16,130,0,2,16,10,80,22,50,4,29,2,37,24,113,2,38,24,129,0,38,23,131,0,37,23,11,0,14,50,2,34,2,1,29,126,2,2,29,12,0,14,50,2,35,2,38,10,127,2,37,10,13,20,0,50,1,36,3,13,29,14,20,0,50,1,37,3,26,10,15,40,0,50,2,38,3,28,2,102,3,27,2,16,40,0,50,2,39,3,11,37,103,3,12,37,17,0,7,50,1,40,2,6,10,18,0,7,50,1,41,2,33,29,19,20,0,50,1,42,3,17,6,20,20,0,50,1,43,3,22,33,21,0,14,50,2,44,2,25,6,46,2,26,6,22,0,14,50,2,45,2,14,33,47,2,13,33,23,0,7,50,1,48,2,33,6,24,0,7,50,1,49,2,6,33,25,300,256,150,9,50,2,1,2,52,2,2,0,54,2,1,0,56,2,0,0,58,2,0,1,60,2,0,2,62,2,2,2,64,2,2,1,86,4,1,1,26,300,256,150,9,51,2,38,37,53,2,37,39,55,2,38,39,57,2,39,39,59,2,39,38,61,2,39,37,63,2,37,37,65,2,37,38,87,4,38,38,27,0,21,50,3,66,2,14,23,70,2,15,23,72,2,15,24,28,0,21,50,3,67,2,25,16,71,2,24,16,73,2,24,15,29,0,7,50,1,68,2,6,24,30,0,7,50,1,69,2,33,15,31,40,0,50,2,74,3,6,20,76,3,6,19,32,40,0,50,2,75,3,33,19,77,3,33,20,33,130,33,60,4,78,1,18,2,80,2,18,1,82,2,19,2,84,0,19,1,34,130,33,60,4,79,1,21,37,81,2,21,38,83,2,20,37,85,0,20,38,35,160,71,80,6,88,5,38,1,90,2,37,0,92,2,38,0,94,2,39,0,96,2,39,1,98,2,39,2,36,160,71,80,6,89,5,1,38,91,2,2,39,93,2,1,39,95,2,0,39,97,2,0,38,99,2,0,37,37,20,0,50,1,100,3,7,2,38,20,0,50,1,101,3,32,37,39,0,7,50,1,104,2,23,1,40,0,7,50,1,105,2,16,38,41,0,7,50,1,106,2,33,2,42,0,7,50,1,107,2,6,37,43,20,0,50,1,108,3,1,6,44,20,0,50,1,109,3,38,33,45,0,7,50,1,110,2,12,1,46,0,7,50,1,111,2,27,38,47,0,7,50,1,116,2,2,11,48,0,7,50,1,117,2,37,28,49,170,19,60,4,118,0,1,22,120,1,1,23,122,3,2,23,124,3,2,22,50,170,19,60,4,119,0,38,17,121,1,38,16,123,3,37,16,125,3,37,17};
inline constexpr Layout layouts[]={
{40,40,5,2,1,{m0_0,m0_1,m0_2,m0_3,m0_4,m0_5},30,c0,783}
};
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
