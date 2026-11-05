#pragma once
#include <cambc.h>
#include <cstring>
#include <vector>
namespace z115 {
using namespace cambc;
struct Layout {int w,h,n,sym,id;const std::uint64_t* masks[6];std::size_t words;const std::uint32_t* camps;std::size_t count;};
inline constexpr std::uint64_t m0_0[]={0ULL,0ULL,0ULL,0ULL,3228565504ULL,3465533137482678272ULL,0ULL,0ULL,1166679143849394176ULL,1095216660480ULL,1095216660480ULL,0ULL,0ULL,9259542124343345248ULL,0ULL,0ULL,289360691352832024ULL,0ULL,0ULL,0ULL,0ULL,6935755084294815872ULL,0ULL,0ULL,1731642904423957508ULL,0ULL,0ULL,4227858432ULL,4278190080ULL,119287840ULL,0ULL,0ULL,2160091184ULL,61632831029248ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_1[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,144681440859127808ULL,0ULL,0ULL,0ULL,0ULL,0ULL,246290604621824ULL,16736959070208ULL,72908738188608258ULL,2031616ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,33751297ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_2[]={0ULL,17361482116629528576ULL,6316032ULL,0ULL,873168414644895744ULL,0ULL,0ULL,0ULL,0ULL,96ULL,0ULL,36169534511003704ULL,508641805655343104ULL,0ULL,0ULL,0ULL,0ULL,0ULL,18155135997837312ULL,4328785920ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2160115840ULL,8102098874992625411ULL,0ULL,1729382256910270464ULL,0ULL,0ULL,0ULL,0ULL,3235963072ULL,65792ULL,6781787720122368ULL,1588284ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_3[]={0ULL,0ULL,0ULL,0ULL,0ULL,137438953472ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,65536ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,2199023255552ULL,0ULL,0ULL,0ULL,0ULL,0ULL,268435456ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_4[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,8101975729639522304ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,112ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint64_t m0_5[]={0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,4035225266123964416ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,14392ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL,0ULL};
inline constexpr std::uint32_t c0[]={0,0,7,50,1,0,2,10,19,1,0,7,50,1,1,2,39,20,2,80,36,50,6,2,2,4,4,4,2,5,4,6,2,5,3,16,2,4,3,27,0,6,3,29,0,6,4,3,80,36,50,6,3,2,45,35,5,2,44,35,7,2,44,36,17,2,45,36,26,0,43,36,28,0,43,35,4,90,71,60,9,8,1,21,24,35,2,22,23,37,2,22,24,39,2,22,25,41,2,21,25,43,2,20,25,45,2,20,24,47,2,20,23,49,2,21,23,5,90,71,60,9,9,1,28,15,34,2,27,16,36,2,27,15,38,2,27,14,40,2,28,14,42,2,29,14,44,2,29,15,46,2,29,16,48,2,28,16,6,40,4,40,1,10,0,40,9,7,40,4,40,1,11,0,9,30,8,0,7,50,1,12,2,5,22,9,0,7,50,1,13,2,44,17,10,90,36,60,4,14,2,2,38,18,1,2,37,20,2,1,37,22,2,1,38,11,90,36,60,4,15,2,47,1,19,1,47,2,21,2,48,2,23,2,48,1,12,0,7,50,1,24,2,19,36,13,0,7,50,1,25,2,30,3,14,0,7,50,1,30,2,14,15,15,0,7,50,1,31,2,35,24,16,0,7,50,1,32,2,21,7,17,0,7,50,1,33,2,28,32};
inline constexpr Layout layouts[]={
{50,40,5,2,1,{m0_0,m0_1,m0_2,m0_3,m0_4,m0_5},40,c0,290}
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
