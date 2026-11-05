#include <iostream>
#include <string>
#ifndef CC2_DUMP
#define CC2_DUMP 0
#endif
#include "z451_layout.h"
#include "z445_layout.h"
#include "z443_layout.h"
#include "z441_layout.h"
#include "z436_layout.h"
#include "z434_layout.h"
#include "z431_layout.h"
#include "z424_layout.h"
#include "z357_switch.h"
#if Z357_DRAFT
#include "z357_layout.h"
#endif
#line 1 "main.cpp"
#include "z346_switch.h"
#if Z346_DRAFT
#include "z346_layout.h"
#endif
#line 1
#include "z344_switch.h"
#if Z340_DRAFT
#include "z344_layout.h"
#endif
#line 1
#include "z335_switch.h"
#if (Z335_ROAD & 2)
#include "z335_layout.h"
int z335_road_main();
#endif
#include "z335_switch.h"
#if Z335_DRAFT
#include "z335_layout.h"
#endif
#line 1
#line 1
#include "z328_switch.h"
#if Z328_DRAFT
#include "z328_layout.h"
#endif
#if (Z328_GW & 2)
int z328_gw_main();
#endif
#line 1
#include "z326_switch.h"
#if (Z326_CORRIDORS & 1)
#include "z326_layout.h"
#endif
#include "z300_draft.h"
#include "z299_draft.h"
#include "z298_draft.h"
#include "z295_draft.h"
#include "z294_draft.h"
#include "z267_draft.h"
#include "z260_draft.h"
#include "z251_draft.h"
#include "z254_draft.h"
#include "z195_draft.h"
#include "z226_draft.h"
#include "z180_draft.h"
#include "z182_draft.h"
#include "z169_draft.h"
#include "z160_draft.h"
#include "z161_draft.h"
#include "z158_draft.h"
#include "z138_draft.h"
#include "z135_draft.h"
#include "z134_draft.h"
#include "z131_draft.h"
#include "z129_draft.h"
#include "z128_draft.h"
#include "z127_draft.h"
#include "z123_draft.h"
#include "z118_draft.h"
#include "z117_draft.h"
#include "z116_draft.h"
#include "z115_switch.h"
#if Z115_MAPS
#include "z115_layout.h"
int z115_main();
#endif
#include "z114_switch.h"
#if Z114_MAPS
#include "z114_layout.h"
int z114_main();
#endif
#include "z113_switch.h"
#if Z113_MAPS
#include "z113_layout.h"
int z113_main();
#endif
#include "z98_switch.h"
#if Z98_FARM
#include "z70_layout.h"
int z98_farm_main();
#endif
#include "z85_switch.h"
#if Z85_SHORTCUT
#include "z85_layout.h"
int z85_shortcut_main();
#endif
#include "z78_switch.h"
#if Z78_MAPS && (Z78_BIGWORLD || Z78_PACKS)
#include "z78_layout.h"
int z78_m1_main();
int z78_m2_main();
#endif
#include "z70_switch.h"
#if Z70_MAPS
#include "z70_layout.h"
int z70_optimized_main();
#endif
#include "z69_switch.h"
#if Z69_STAMPEDE
#include "z69_layout.h"
int z69_optimized_main();
int z69_interim_optimized_main();
#endif
#include "z68_switch.h"
#if Z68_GOLEM
#include "z68_layout.h"
int z68_legacy_main();
int z68_optimized_main();
int z68_interim_legacy_main();
int z68_interim_optimized_main();
#endif
#include <cambc.h>
#include "z51_layouts.h"
#include "z61_layout.h"
int z61_legacy_main();
int z61_optimized_main();
int z61_interim_legacy_main();
int z61_interim_optimized_main();
int legacy_main();
int optimized_main();
int z51_legacy_main();
int z51_optimized_main();
int main() {
#if CC2_DUMP
 {
  using namespace cambc;
  const int w=map::width(),h=map::height();
  const auto wl=map::walls(),ho=map::holes(),al=map::altars(),sa=map::spawn_region(team::ally),se=map::spawn_region(team::enemy);
  std::cout<<"DIM "<<w<<" "<<h<<"\n";for(int y=0;y<h;++y){std::string r;for(int x=0;x<w;++x){vec2 v{x,y};char c='.';if(wl.is_set(v))c='#';else if(ho.is_set(v))c='o';else if(al.is_set(v))c='A';else if(sa.is_set(v))c='S';else if(se.is_set(v))c='E';r+=c;}std::cout<<"TER "<<r<<"\n";}
  std::cout<<"MEM "<<cambc::is_draft();for(auto b:cambc::draft_memory)std::cout<<" "<<std::to_integer<int>(b);std::cout<<"\n"<<std::flush;
 }
#endif

 // Z451: exact-layout drafts; retain Boss Battle's z51 gameplay marker.
 if(cambc::is_draft()) {
  const int z451_map=z451_::detect();
  if(z451_map && z451_map!=4 && z451_map!=2) { // boss_battle (4) dropped locally; rows (2) dropped after G23 (-2/+1)
   constexpr int z451_orders[][5]={{11,2,1,-1,-1},{11,1,6,0,-1},{11,1,6,2,10},{11,1,6,2,0}};
   do {
    const auto z451_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z451_id:z451_orders[z451_map-1]) {
     if(z451_id<0)break;
     const auto z451_pick=static_cast<cambc::champion_draft>(z451_id);
     if(std::find(z451_chosen.begin(),z451_chosen.end(),z451_pick)!=z451_chosen.end())continue;
     std::vector<std::uint8_t> z451_plan;
     for(auto z451_c:z451_chosen)z451_plan.push_back(static_cast<std::uint8_t>(z451_c));
     z451_plan.push_back(static_cast<std::uint8_t>(z451_id));
     if(z451_map==4){z451_plan.resize(6,0);z451_plan[5]=51;}
     cambc::draft_memory=z451_plan;cambc::draft::pick(z451_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z445: exact-layout drafts; preserve Corridors generic-policy marker.
 if(cambc::is_draft()) {
  const int z445_map=z445_::detect();
  if(z445_map) {
   constexpr int z445_orders[][5]={{11,1,6,2,9},{11,1,6,0,9}};
   do {
    const auto z445_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z445_id:z445_orders[z445_map-1]) {
     if(z445_id<0)break;
     const auto z445_pick=static_cast<cambc::champion_draft>(z445_id);
     if(std::find(z445_chosen.begin(),z445_chosen.end(),z445_pick)!=z445_chosen.end())continue;
     std::vector<std::uint8_t> z445_plan;
     for(auto z445_c:z445_chosen)z445_plan.push_back(static_cast<std::uint8_t>(z445_c));
     z445_plan.push_back(static_cast<std::uint8_t>(z445_id));
     if(z445_map==2){z445_plan.resize(16,255);z445_plan[15]=229;}
     cambc::draft_memory=z445_plan;cambc::draft::pick(z445_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z443: exact-layout drafts. Shortcut must retain its z85 gameplay marker.
 if(cambc::is_draft()) {
  const int z443_map=z443_::detect();
  if(z443_map) {
   constexpr int z443_orders[][5]={{11,1,6,0,2},{11,1,6,0,-1}};
   do {
    const auto z443_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z443_id:z443_orders[z443_map-1]) {
     if(z443_id<0)break;
     const auto z443_pick=static_cast<cambc::champion_draft>(z443_id);
     if(std::find(z443_chosen.begin(),z443_chosen.end(),z443_pick)!=z443_chosen.end())continue;
     std::vector<std::uint8_t> z443_plan;
     for(auto z443_c:z443_chosen)z443_plan.push_back(static_cast<std::uint8_t>(z443_c));
     z443_plan.push_back(static_cast<std::uint8_t>(z443_id));
     if(z443_map==2){z443_plan.resize(14,0);z443_plan[13]=85;}
     cambc::draft_memory=z443_plan;cambc::draft::pick(z443_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z441: preserve Elk Arena z61 gameplay dispatch while changing only drafts.
 if(cambc::is_draft()) {
  const int z441_map=z441_::detect();
  if(z441_map) {
   constexpr int z441_orders[][5]={{1,11,2,-1,-1},{11,1,6,0,2}};
   do {
    const auto z441_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z441_id:z441_orders[z441_map-1]) {
     if(z441_id<0)break;
     const auto z441_pick=static_cast<cambc::champion_draft>(z441_id);
     if(std::find(z441_chosen.begin(),z441_chosen.end(),z441_pick)!=z441_chosen.end())continue;
     std::vector<std::uint8_t> z441_plan;
     for(auto z441_c:z441_chosen)z441_plan.push_back(static_cast<std::uint8_t>(z441_c));
     z441_plan.push_back(static_cast<std::uint8_t>(z441_id));
     if(z441_map==2){z441_plan.resize(7,0);z441_plan[6]=61;}
     cambc::draft_memory=z441_plan;cambc::draft::pick(z441_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z436: exact-layout five-pick Ryth swaps, preserving baseline dispatch markers.
 if(cambc::is_draft()) {
  const int z436_map=z436_::detect();
  if(z436_map) {
   do {
    const int z436_order[]={11,6,1,2,z436_map==4?9:10};
    const auto z436_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z436_id:z436_order) {
     const auto z436_pick=static_cast<cambc::champion_draft>(z436_id);
     if(std::find(z436_chosen.begin(),z436_chosen.end(),z436_pick)!=z436_chosen.end())continue;
     std::vector<std::uint8_t> z436_plan;
     for(auto z436_c:z436_chosen)z436_plan.push_back(static_cast<std::uint8_t>(z436_c));
     z436_plan.push_back(static_cast<std::uint8_t>(z436_id));
     // Concentrated Wealth uses z51_optimized_main; its draft writes byte 5.
     if(z436_map==1){z436_plan.resize(6,0);z436_plan[5]=51;}
     cambc::draft_memory=z436_plan;cambc::draft::pick(z436_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z434: exact-layout draft changes; retain each map's gameplay dispatch marker.
 if(cambc::is_draft()) {
  const int z434_map=z434_::detect();
  if(z434_map && z434_map!=3) { // 01gold_rush (id 3) dropped: lost 2 wins on gauntlet G11
   constexpr int z434_orders[][5]={{11,1,6,2,0},{11,1,2,6,-1},{6,0,2,-1,-1},
    {11,1,6,2,0},{11,1,6,2,0},{11,1,6,2,0},{11,1,6,2,0}};
   do {
    const auto z434_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z434_id:z434_orders[z434_map-1]) {
     if(z434_id<0)break;
     const auto z434_pick=static_cast<cambc::champion_draft>(z434_id);
     if(std::find(z434_chosen.begin(),z434_chosen.end(),z434_pick)!=z434_chosen.end())continue;
     std::vector<std::uint8_t> z434_plan;
     for(auto z434_c:z434_chosen)z434_plan.push_back(static_cast<std::uint8_t>(z434_c));
     z434_plan.push_back(static_cast<std::uint8_t>(z434_id));
     // Gem Fields uses z115; Big World uses z78_m1. Match their padding too.
     if(z434_map==1){z434_plan.resize(16,255);z434_plan[15]=115;}
     if(z434_map==7){z434_plan.resize(11,0);z434_plan[10]=79;}
     cambc::draft_memory=z434_plan;cambc::draft::pick(z434_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }


 // Z431: exact-layout draft changes only. All nine maps use optimized_main
 // without a dispatch marker in the frozen v603 base (see task evidence).
 if(cambc::is_draft()) {
  const int z431_map=z431_::detect();
  if(z431_map && z431_map!=9) { // v605: adgato-gemcombat4 (id 9) back to the v603 draft (lost on gauntlet G8)
   constexpr int z431_orders[][5]={{11,1,6,-1,-1},{11,1,2,-1,-1},{11,1,6,-1,-1},
    {11,1,0,6,-1},{11,1,6,0,-1},{1,11,2,6,-1}, // cc_qdraft: 00quarry (id 6) [Vesp,HoR,Vend,Marc]; G116 +1/0
    {11,1,6,2,0},{11,1,6,2,0},{11,1,6,2,0}};
   do {
    const auto z431_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z431_id:z431_orders[z431_map-1]) {
     if(z431_id<0)break;
     const auto z431_pick=static_cast<cambc::champion_draft>(z431_id);
     if(std::find(z431_chosen.begin(),z431_chosen.end(),z431_pick)!=z431_chosen.end())continue;
     std::vector<std::uint8_t> z431_plan;
     for(auto z431_c:z431_chosen)z431_plan.push_back(static_cast<std::uint8_t>(z431_c));
     z431_plan.push_back(static_cast<std::uint8_t>(z431_id));
     cambc::draft_memory=z431_plan;cambc::draft::pick(z431_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }

 // Z428: Anvil-only draft HoR, Marcella, Vesper; play stays v580's. Other maps unchanged.
 if(cambc::is_draft() && cambc::map::width()==37 && cambc::map::height()==31 && z424_::detect()==1) {
  do {
   constexpr int z428_order[]={11,6,1};
   const auto z428_chosen=cambc::draft::current_draft(cambc::team::ally);
   for(int z428_i:z428_order){
    const auto z428_pick=static_cast<cambc::champion_draft>(z428_i);
    if(std::find(z428_chosen.begin(),z428_chosen.end(),z428_pick)!=z428_chosen.end())continue;
    std::vector<std::uint8_t> z428_plan;
    for(auto c:z428_chosen)z428_plan.push_back(static_cast<std::uint8_t>(c));
    z428_plan.push_back(static_cast<std::uint8_t>(z428_pick));
    cambc::draft_memory=z428_plan;cambc::draft::pick(z428_pick);break;
   }
   cambc::end_turn();
  }while(cambc::is_draft());
 }
#if Z357_DRAFT
 if(cambc::is_draft()){
  const int z357_map=z357_::detect();
  if((z357_map==1 && (Z357_FRAGMENT&1)) || (z357_map==2 && (Z357_STAMPEDE&1))){do{
   constexpr int z357_fragment[]={1,11,2},z357_stampede[]={11,6,0,1,2};
   const int* z357_order=z357_map==1?z357_fragment:z357_stampede;
   const auto z357_chosen=cambc::draft::current_draft(cambc::team::ally);
   for(int z357_i=0;z357_i<(z357_map==1?3:5);++z357_i){
    const auto z357_pick=static_cast<cambc::champion_draft>(z357_order[z357_i]);
    if(std::find(z357_chosen.begin(),z357_chosen.end(),z357_pick)!=z357_chosen.end())continue;
    std::vector<std::uint8_t> z357_plan;for(auto z357_c:z357_chosen)z357_plan.push_back(static_cast<std::uint8_t>(z357_c));
    z357_plan.push_back(static_cast<std::uint8_t>(z357_order[z357_i]));
    const int z357_marker=z357_map==1?9:8;z357_plan.resize(z357_marker+1,0);z357_plan[z357_marker]=z357_map==1?70:69;
    cambc::draft_memory=z357_plan;cambc::draft::pick(z357_pick);break;
   }
   cambc::end_turn();
  }while(cambc::is_draft());}
 }
#endif

#line 95 "main.cpp"
#if Z346_DRAFT
 if(cambc::is_draft()){
  const int z346_map=z346_::detect();
  if((z346_map==1 && (Z346_FARM&1)) || (z346_map==2 && (Z346_EIGHT&1))){do{
   constexpr int z346_farm[]={1,6,11},z346_eight[]={11,6,0,1,2};
   const int* z346_order=z346_map==1?z346_farm:z346_eight;
   const auto z346_chosen=cambc::draft::current_draft(cambc::team::ally);
   for(int z346_i=0;z346_i<(z346_map==1?3:5);++z346_i){
    const auto z346_pick=static_cast<cambc::champion_draft>(z346_order[z346_i]);
    if(std::find(z346_chosen.begin(),z346_chosen.end(),z346_pick)!=z346_chosen.end())continue;
    std::vector<std::uint8_t> z346_plan;for(auto z346_c:z346_chosen)z346_plan.push_back(static_cast<std::uint8_t>(z346_c));
    z346_plan.push_back(static_cast<std::uint8_t>(z346_order[z346_i]));
    cambc::draft_memory=z346_plan;cambc::draft::pick(z346_pick);break;
   }
   cambc::end_turn();
  }while(cambc::is_draft());}
 }
#endif

#line 95
#if Z340_DRAFT
 if(cambc::is_draft()){
  const int z344_map=z344_::detect();
  if((z344_map==1 && (Z340_PILGRIMAGE&1)) || (z344_map==2 && (Z340_SWIRL&1))){do{
   constexpr int z344_pg[]={11,0,2,1,6},z344_sw[]={1,2,11,3};
   const int* z344_order=z344_map==1?z344_pg:z344_sw;
   const auto z344_chosen=cambc::draft::current_draft(cambc::team::ally);
   for(int z344_i=0;z344_i<(z344_map==1?5:4);++z344_i){
    const auto z344_pick=static_cast<cambc::champion_draft>(z344_order[z344_i]);
    if(std::find(z344_chosen.begin(),z344_chosen.end(),z344_pick)!=z344_chosen.end())continue;
    std::vector<std::uint8_t> z344_plan;for(auto z344_c:z344_chosen)z344_plan.push_back(static_cast<std::uint8_t>(z344_c));
    z344_plan.push_back(static_cast<std::uint8_t>(z344_order[z344_i]));
    cambc::draft_memory=z344_plan;cambc::draft::pick(z344_pick);break;
   }
   cambc::end_turn();
  }while(cambc::is_draft());}
 }
#endif

#line 95
#if Z335_DRAFT
 if(cambc::is_draft()){
  const int z335_map=z335_::detect();
  if((z335_map==1 && (Z335_GOLD&1)) || (z335_map==2 && (Z335_ROAD&1))){
   do{
    constexpr int z335_gold[]={11,6,1},z335_road[]={11,6,1,2,9}; // Road [HoR,Marc,Vesp,Vend,Varg]; G130 +1/0
    const int* z335_order=z335_map==1?z335_gold:z335_road;
    const auto z335_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z335_i=0;z335_i<(z335_map==1?3:5);++z335_i){
     const auto z335_pick=static_cast<cambc::champion_draft>(z335_order[z335_i]);
     if(std::find(z335_chosen.begin(),z335_chosen.end(),z335_pick)!=z335_chosen.end())continue;
     std::vector<std::uint8_t> z335_plan;for(auto z335_c:z335_chosen)z335_plan.push_back(static_cast<std::uint8_t>(z335_c));
     z335_plan.push_back(static_cast<std::uint8_t>(z335_order[z335_i]));
     if(z335_map==2){z335_plan.resize(16,255);z335_plan[15]=113;}
     cambc::draft_memory=z335_plan;cambc::draft::pick(z335_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }
#endif

#line 103
#line 103
#if (Z326_CORRIDORS & 1)
 if(cambc::is_draft() && z326_::detect()==1){do{z326_::pick();cambc::end_turn();}while(cambc::is_draft());}
#endif
#if Z328_DRAFT
 if(cambc::is_draft()){
  const int z328_map=z328_::detect();
  if((z328_map==1 && (Z328_FRACTURE&1)) || (z328_map==2 && (Z328_GW&1))){
   do{
    const int z328_f[]={11,6,0,1,2},z328_g[]={11,6,2,0,1};
    const int* z328_order=z328_map==1?z328_f:z328_g;
    const auto z328_chosen=cambc::draft::current_draft(cambc::team::ally);
    for(int z328_i=0;z328_i<5;++z328_i){
     const auto z328_pick=static_cast<cambc::champion_draft>(z328_order[z328_i]);
     if(std::find(z328_chosen.begin(),z328_chosen.end(),z328_pick)!=z328_chosen.end())continue;
     std::vector<std::uint8_t> z328_plan;for(auto z328_c:z328_chosen)z328_plan.push_back(static_cast<std::uint8_t>(z328_c));
     z328_plan.push_back(static_cast<std::uint8_t>(z328_order[z328_i]));
     if(z328_map==2){z328_plan.resize(16,255);z328_plan[15]=114;}
     cambc::draft_memory=z328_plan;cambc::draft::pick(z328_pick);break;
    }
    cambc::end_turn();
   }while(cambc::is_draft());
  }
 }
#endif

#line 91
 if constexpr(z300_::any) if(cambc::is_draft()){const int z300_id=z300_::detect();if(z300_::enabled(z300_id)){do{z300_::pick(z300_id);cambc::end_turn();}while(cambc::is_draft());}}
 if constexpr(z300_::any) if(z300_::generic())return optimized_main();
 if constexpr(z299_::any) if(cambc::is_draft()){const int z299_id=z299_::detect();if(z299_::enabled(z299_id)){do{z299_::pick(z299_id);cambc::end_turn();}while(cambc::is_draft());}}
 if constexpr(z299_::any) if(z299_::generic())return optimized_main();
 if constexpr(z298_::any) if(cambc::is_draft()){const int z298_id=z298_::detect();if(z298_::enabled(z298_id)){do{z298_::pick(z298_id);cambc::end_turn();}while(cambc::is_draft());}}
 if constexpr(z295_::any) if(cambc::is_draft()){const int z295_id=z295_::detect();if(z295_::enabled(z295_id)){do{z295_::pick(z295_id);cambc::end_turn();}while(cambc::is_draft());}}
 if constexpr(z294_::any) if(cambc::is_draft()){const int z294_id=z294_::detect();if(z294_::enabled(z294_id)){do{z294_::pick(z294_id);cambc::end_turn();}while(cambc::is_draft());}}
#if Z267_DRAFT
 if(cambc::is_draft()&&cambc::map::width()==31&&cambc::map::height()==31&&z267_::detect()==1){do{z267_::pick();cambc::end_turn();}while(cambc::is_draft());}
#endif
#if Z260_ON
 if(cambc::is_draft()){const int z260_id=z260_::detect();if(z260_::enabled(z260_id)){do{z260_::pick(z260_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z254_ON
 if(cambc::is_draft()){const int z254_id=z254_::detect();if(z254_::enabled(z254_id)){do{z254_::pick(z254_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z195_ON
 if(cambc::is_draft()){const int z195_id=z195_::detect();if(z195_::enabled(z195_id)){do{z195_::pick(z195_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif
#if Z226_ON
 if(cambc::is_draft()){const int z226_id=z226_::detect();if(z226_::enabled(z226_id)){do{z226_::pick(z226_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z180_ON
 if(cambc::is_draft()){const int z180_id=z180_::detect();if(z180_::enabled(z180_id)){do{z180_::pick(z180_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif
#if Z182_ON
 if(cambc::is_draft()){const int z182_id=z182_::detect();if(z182_::enabled(z182_id)){do{z182_::pick(z182_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z169_ON
 if(cambc::is_draft()){const int z169_id=z169_::detect();if(z169_::enabled(z169_id)){do{z169_::pick(z169_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z160_ON
 if(cambc::is_draft()){const int z160_id=z160_::detect();if(z160_::enabled(z160_id)){do{z160_::pick(z160_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif
#if Z161_ON
 if(cambc::is_draft()){const int z161_id=z161_::detect();if(z161_::enabled(z161_id)){do{z161_::pick(z161_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z158_ON
 if(cambc::is_draft()){const int z158_id=z158_::detect();
#if Z251_ON
 if(z158_id==1){do{z251_::pick();cambc::end_turn();}while(cambc::is_draft());}
 else
#endif
 if(z158_::enabled(z158_id)){do{z158_::pick(z158_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z138_ON
 if(cambc::is_draft()){const int z138_id=z138_::detect();if(z138_::enabled(z138_id)){do{z138_::pick(z138_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z135_ON
 if(cambc::is_draft()){const int z135_id=z135_::detect();if(z135_::enabled(z135_id)){do{z135_::pick(z135_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z134_ON
 if(cambc::is_draft()){const int z134_id=z134_::detect();if(z134_::enabled(z134_id)){do{z134_::pick(z134_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z131_ON
 if(cambc::is_draft()){const int z131_id=z131_::detect();if(z131_::enabled(z131_id)){do{z131_::pick(z131_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z129_ON
 if(cambc::is_draft()){const int z129_id=z129_::detect();if(z129_::enabled(z129_id)){do{z129_::pick(z129_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z128_ON
 if(cambc::is_draft()){const int z128_id=z128_::detect();if(z128_::enabled(z128_id)){do{z128_::pick(z128_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z127_ON
 if(cambc::is_draft()){const int z127_id=z127_::detect();if(z127_::enabled(z127_id)){do{z127_::pick(z127_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z123_ON
 if(cambc::is_draft()){const int z123_id=z123_::detect();if(z123_::enabled(z123_id)){do{z123_::pick(z123_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z118_ON
 if(cambc::is_draft()){const int z118_id=z118_::detect();if(z118_::enabled(z118_id)){do{z118_::pick(z118_id);cambc::end_turn();}while(cambc::is_draft());}}
#endif

#if Z117_ON
 if(cambc::is_draft()){
  const int z117_id=z117_::detect();
  if(z117_::enabled(z117_id)){
   do{z117_::pick(z117_id);cambc::end_turn();}while(cambc::is_draft());
  }
 }
#endif

#if Z116_ON
 if(cambc::is_draft()){
  const int z116_id=z116_::detect();
  if(z116_::enabled(z116_id)){
   do{z116_::pick(z116_id);cambc::end_turn();}while(cambc::is_draft());
  }
 }
#endif

#if Z115_MAPS
 const bool z115map=cambc::is_draft()?z115::detect()!=0:(cambc::draft_memory.size()>15 && std::to_integer<int>(cambc::draft_memory[15])==115);
 if(z115map)return z115_main();
#endif

#if Z114_MAPS
 const bool z114map=cambc::is_draft()?z114::detect()!=0:(cambc::draft_memory.size()>15 && std::to_integer<int>(cambc::draft_memory[15])==114);
#if (Z328_GW & 2)
 if(z114map)return z328_gw_main();
#else
#line 204
 if(z114map)return z114_main();
#endif
#line 205
#endif

#if Z113_MAPS
 const bool z113map=cambc::is_draft()?z113::detect()!=0:(cambc::draft_memory.size()>15 && std::to_integer<int>(cambc::draft_memory[15])==113);
#if (Z335_ROAD & 2)
 if(z113map && z335_::detect()==2)return z335_road_main();
#endif
#line 253
 if(z113map)return z113_main();
#endif

#if Z98_FARM
    const bool z98map=cambc::is_draft()?z70::detect()==3:(cambc::draft_memory.size()>14 && std::to_integer<int>(cambc::draft_memory[14])==98);
    if(z98map)return z98_farm_main();
#endif

#if Z85_SHORTCUT
    const bool z85_sc=cambc::is_draft()?z85::detect()!=0:(cambc::draft_memory.size()>13 && std::to_integer<int>(cambc::draft_memory[13])==85);
    if(z85_sc)return z85_shortcut_main();
#endif

#if Z78_MAPS && (Z78_BIGWORLD || Z78_PACKS)
    const int z78map=cambc::is_draft()?z78::detect():(cambc::draft_memory.size()>10?std::to_integer<int>(cambc::draft_memory[10])-78:0);
#if Z78_BIGWORLD
    if(z78map==1)return z78_m1_main();
#endif
#if Z78_PACKS
    if(z78map==2)return z78_m2_main();
#endif
#endif

#if Z69_STAMPEDE
    const bool stampede = cambc::is_draft() ? z69::detect() != 0
        : (cambc::draft_memory.size() > 8 && std::to_integer<int>(cambc::draft_memory[8]) == 69);
    if (stampede) return z69_optimized_main();
#endif
#if Z70_MAPS
    const bool weakmap = cambc::is_draft() ? z70::detect() != 0
        : (cambc::draft_memory.size() > 9 && std::to_integer<int>(cambc::draft_memory[9]) == 70);
    if (weakmap) return z70_optimized_main();
#endif
    // Detect exactly during draft; preserve gameplay initialization fuel.
    const bool original = cambc::is_draft() ? z51::detect() != 0
        : (cambc::draft_memory.size() > 5 && std::to_integer<int>(cambc::draft_memory[5]) == 51);
    const bool legacy = cambc::map::width() == 100 && cambc::map::height() == 100;
    const bool corrected = cambc::is_draft() ? z61::detect() != 0
        : (cambc::draft_memory.size() > 6 && std::to_integer<int>(cambc::draft_memory[6]) == 61);
#if Z68_GOLEM
    const bool golem = cambc::is_draft() ? z68::detect() != 0
        : (cambc::draft_memory.size() > 7 && std::to_integer<int>(cambc::draft_memory[7]) == 68);
    if (golem) {
        if (original) return legacy ? z68_interim_legacy_main() : z68_interim_optimized_main();
        return legacy ? z68_legacy_main() : z68_optimized_main();
    }
#endif
    if (corrected) {
        if (original) return legacy ? z61_interim_legacy_main() : z61_interim_optimized_main();
        return legacy ? z61_legacy_main() : z61_optimized_main();
    }
    if (original) return legacy ? z51_legacy_main() : z51_optimized_main();
    return legacy ? legacy_main() : optimized_main();
}
