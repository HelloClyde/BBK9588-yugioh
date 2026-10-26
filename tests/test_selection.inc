/* Included after the unmodified generated frontend. These are synthetic UI
 * states, not claimed as gameplay or firmware screenshots. */
volatile uint32_t joey_audio_diagnostic[32];
int joey_audio_open(void){return 1;}
void joey_audio_music(unsigned n){(void)n;}
void joey_audio_effect(unsigned n){(void)n;}
void joey_audio_tick(void){}
void joey_audio_close(void){}
static unsigned checks;
#define CHECK(expr) do{assert(expr);++checks;}while(0)
static void fixture(unsigned kind){
    memset(&joey_duel_state,0,sizeof(joey_duel_state));
    memset(picker_selected,0,sizeof(picker_selected));
    scene=1;pending=kind;details_open=about_open=menu9588=browse9588=0;
    notice=0;native_focus=110;native_selected=99;grave_choice=1;grave_page=0;
    picker_count=response_count=7;backup_count=1;picker_selected[1]=1;
    joey_duel_state.winner=-1;joey_duel_state.players[0].life=8000;joey_duel_state.players[1].life=5200;
    joey_duel_state.turn=9;joey_duel_state.phase=DUEL_MAIN1;joey_duel_state.response_player=DUEL_NO_SLOT;
    for(unsigned i=0;i<7;++i){
        unsigned card=10+i;
        picker_owner[i]=0;picker_index[i]=i;
        joey_duel_state.players[0].grave[i]=card;
        joey_duel_state.remodel.cards[0][i]=card;
        effect_options[i]=(DuelChoiceOption){card,i,DUEL_OPTION_CARD};
        response_options[i].card=card;
    }
    joey_duel_state.players[0].grave_count=7;
}
static void ppm(const char *name){
    const char *folder=getenv("YUGIOH_TEST_OUT");if(!folder)folder="build/tests";
    char path[1024];snprintf(path,sizeof(path),"%s/%s.ppm",folder,name);
    FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n240 320\n255\n");
    for(unsigned i=0;i<240*320;++i){uint16_t c=screen[i];unsigned char rgb[3]={(c>>11)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};fwrite(rgb,1,3,f);}fclose(f);
}
int main(void){
    native_resource_drive=1;profile_load();
    CHECK(profile_paths==profile_roots[1]);CHECK(profile_paths[0][0]=='B');
    native_resource_drive=0;profile_load();
    CHECK(profile_paths==profile_roots[0]);CHECK(profile_paths[0][0]=='A');
#ifdef JOEY_UI_WITH_ASSETS
    CHECK(native_load());CHECK(font_assets_valid());CHECK(scene_load());
#endif
    const unsigned kinds[]={PENDING_GRAVE,PENDING_EFFECT_CHOICE,PENDING_RESPONSE,PENDING_BACKUP,PENDING_REMODEL,PENDING_ATTACK_SELECT,PENDING_PUBLIC_HAND};
    for(unsigned k=0;k<sizeof(kinds)/sizeof(kinds[0]);++k){
        fixture(kinds[k]);Duel before=joey_duel_state;unsigned selected=picker_selected[1];
        portrait_touch(120,281);CHECK(details_open);CHECK(native_selected==11);CHECK(pending==kinds[k]);CHECK(grave_choice==1);CHECK(!memcmp(&before,&joey_duel_state,sizeof(before)));CHECK(picker_selected[1]==selected);CHECK(backup_count==1);
        portrait_touch(200,20);CHECK(!details_open);CHECK(grave_choice==1);CHECK(pending==kinds[k]);
        key9588(0);CHECK(details_open);CHECK(native_selected==11);key9588(4);CHECK(!details_open);CHECK(pending==kinds[k]);
        portrait_touch(200,259);CHECK(grave_choice==5);CHECK(grave_page==1);CHECK(native_selected==15);
        portrait_touch(120,281);CHECK(details_open);CHECK(native_selected==15);key9588(4);
        portrait_touch(40,259);CHECK(grave_choice==0);CHECK(native_selected==10);
        portrait_touch(90,118);CHECK(grave_choice==1);CHECK(native_selected==11);CHECK(!details_open);
        portrait_touch(30,305);CHECK(details_open);CHECK(native_selected==11);key9588(4);
        CHECK(!memcmp(&before,&joey_duel_state,sizeof(before)));CHECK(picker_selected[1]==selected);CHECK(backup_count==1);
    }
    /* An action row, hidden target or terminal row has no card to disclose. */
    fixture(PENDING_EFFECT_CHOICE);
    effect_options[1].card=0;portrait_touch(120,281);CHECK(!details_open);
    effect_options[1]=(DuelChoiceOption){11,0,DUEL_OPTION_SKIP};key9588(0);CHECK(!details_open);
    effect_options[1].kind=DUEL_OPTION_PAGE;portrait_touch(30,305);CHECK(!details_open);
    for(unsigned k=0;k<3;++k){fixture(k==0?PENDING_RESPONSE:k==1?PENDING_BACKUP:PENDING_REMODEL);grave_choice=7;portrait_touch(120,281);CHECK(!details_open);}
    fixture(PENDING_GRAVE);portrait_touch(190,281);CHECK(pending==PENDING_POSITION);CHECK(!details_open);
    fixture(PENDING_GRAVE);portrait_touch(40,281);CHECK(pending==0);
    fixture(PENDING_GRAVE);portrait_touch(82,281);CHECK(pending==PENDING_GRAVE);CHECK(!details_open);
#ifdef JOEY_UI_WITH_ASSETS
    fixture(PENDING_EFFECT_CHOICE);redraw();ppm("selection-host-list");
    portrait_touch(120,281);redraw();ppm("selection-host-details");
    portrait_touch(200,20);redraw();ppm("selection-host-return");
#endif
    printf("{\"passed\":true,\"checks\":%u,\"fixture_kind\":\"host synthetic states using production frontend\",\"guest_memory_written\":false,\"selection_modes\":7}\n",checks);
    return 0;
}
