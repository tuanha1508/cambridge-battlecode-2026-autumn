#pragma once
// Z227 fuel fix: the same exact Fracture check as z169_::detect()==5 (size, team size, symmetry,
// walls/holes/bushes/altars, both spawn masks in either order, full camp layout), but compared
// word-wise. z169_::detect() uses byte-wise memcmp over ~5.4 KB and cost ~117k fuel on the
// 100k-fuel second play turn, exhausting all five champions at round 2.
#include "z169_layout.h"
namespace z227_ {
using namespace cambc;
inline bool eq64(const std::uint64_t* a, const std::uint64_t* b, std::size_t n) {
 std::uint64_t d=0; for(std::size_t i=0;i<n;++i) d|=a[i]^b[i]; return d==0;
}
inline bool eq32(const std::uint32_t* a, const std::uint32_t* b, std::size_t n) {
 std::uint32_t d=0; for(std::size_t i=0;i<n;++i) d|=a[i]^b[i]; return d==0;
}
inline bool is_fracture() {
 const auto& r=z169_::layouts[4];
 if(r.id!=5) return false;
 const int w=map::width(),h=map::height(),n=static_cast<int>(draft::team_size());
 if(w!=r.w || h!=r.h || n!=r.n || static_cast<int>(map::symmetry())!=r.sym) return false;
 const std::size_t bytes=bitboard::raw_buffer_size();
 if(bytes!=r.words*sizeof(std::uint64_t)) return false;
 const bitboard* bb[]={&map::walls(),&map::holes(),&map::bushes(),&map::altars()};
 for(int i=0;i<4;++i) if(!eq64(bb[i]->ptr(),r.masks[i],r.words)) return false;
 const auto* own=map::spawn_region(team::ally).ptr();
 const auto* foe=map::spawn_region(team::enemy).ptr();
 if(!((eq64(own,r.masks[4],r.words)&&eq64(foe,r.masks[5],r.words)) ||
      (eq64(own,r.masks[5],r.words)&&eq64(foe,r.masks[4],r.words)))) return false;
 std::vector<std::uint32_t> camps(r.count); std::uint32_t count=0;
 detail::check(detail::sdk_map_camp_layout(camps.data(),static_cast<std::uint32_t>(camps.size()),&count));
 return count==r.count && eq32(camps.data(),r.camps,r.count);
}
}
