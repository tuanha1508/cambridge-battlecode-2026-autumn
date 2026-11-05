#pragma once
#include <cambc.h>
#include <cstring>
#include <vector>
#include <algorithm>
namespace z85 {
using namespace cambc;
struct Layout {int w,h,n,sym,id; const std::uint64_t* masks[6]; std::size_t words; const std::uint32_t* camps; std::size_t count;};
inline constexpr std::uint64_t m0_0[]={0ULL,217020518463700992ULL,0ULL,217021605073649664ULL,1099494850560ULL,0ULL,0ULL,217020518514230019ULL,0ULL,230584300934001411ULL,868082074057117440ULL,0ULL,0ULL,14123287331939024835ULL,0ULL,288229276656860931ULL,868929745781198604ULL,0ULL,0ULL,217020518514230019ULL,0ULL,284838602682348291ULL,4238669066800140ULL,0ULL,0ULL,50529027ULL,0ULL,1099495047939ULL,1099494850560ULL,0ULL};
inline constexpr std::uint64_t m0_1[]={4539629506721218560ULL,4038608446222761984ULL,1157451556937793536ULL,0ULL,0ULL,0ULL,69269249064960ULL,224ULL,1821ULL,0ULL,0ULL,0ULL,18158513697557840124ULL,0ULL,18374686479671623935ULL,0ULL,0ULL,0ULL,277076934328320ULL,16140901064495857664ULL,2091640551936884736ULL,0ULL,0ULL,0ULL,4227858495ULL,4228123704ULL,253235216ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_2[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,3512807709349776384ULL,844424930131968ULL,0ULL,271639904256ULL,0ULL,71776119061282560ULL,3458764513820540976ULL,0ULL,0ULL,0ULL,0ULL,0ULL,3390893860110384ULL,768ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_3[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,4294967296ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,16777216ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_4[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,1010580480ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,16954726998343680ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint32_t c0[]={0,0,28,50,4,0,2,19,19,1,2,19,20,2,2,20,19,3,2,20,20,1,220,30,60,4,4,1,22,20,5,1,22,19,38,3,23,19,39,3,23,20,2,120,8,50,4,6,0,17,20,7,0,17,19,36,3,16,19,37,3,16,20,3,130,22,60,4,8,1,12,14,42,3,12,13,44,3,13,14,46,2,13,13,4,130,22,60,4,9,1,12,25,43,3,12,26,45,3,13,25,47,2,13,26,5,300,235,150,6,10,4,31,12,76,2,30,12,78,2,31,11,80,2,31,13,82,2,30,11,84,2,30,13,6,300,235,150,6,11,4,31,27,77,2,30,27,79,2,31,28,81,2,31,26,83,2,30,28,85,2,30,26,7,120,8,50,4,12,0,16,6,14,0,15,6,48,3,14,6,50,3,17,6,8,120,8,50,4,13,0,16,33,15,0,15,33,49,3,14,33,51,3,17,33,9,0,21,50,3,16,2,39,0,60,2,38,0,62,2,39,1,10,0,21,50,3,17,2,39,39,61,2,38,39,63,2,39,38,11,400,74,60,8,18,1,1,2,20,1,1,1,22,1,2,1,24,1,2,2,52,2,3,1,54,2,3,2,56,3,0,1,58,3,0,2,12,400,74,60,8,19,1,1,37,21,1,1,38,23,1,2,38,25,1,2,37,53,2,3,38,55,2,3,37,57,3,0,38,59,3,0,37,13,180,44,60,4,26,1,33,6,32,1,32,6,66,2,32,7,68,2,33,7,14,180,44,60,4,27,1,33,33,33,1,32,33,67,2,32,32,69,2,33,32,15,90,29,60,3,28,1,27,6,64,2,27,7,74,2,28,6,16,90,29,60,3,29,1,27,33,65,2,27,32,75,2,28,33,17,90,29,60,3,30,1,38,6,70,2,38,7,72,2,37,6,18,90,29,60,3,31,1,38,33,71,2,38,32,73,2,37,33,19,40,0,50,2,34,3,16,11,40,3,15,11,20,40,0,50,2,35,3,16,28,41,3,15,28};
inline constexpr Layout layouts[]={{40,40,4,1,1,{m0_0,m0_1,m0_2,m0_3,m0_4,m0_5},30,c0,449}};
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
