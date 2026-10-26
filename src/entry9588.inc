static uint8_t vx9588[24+240*320*2];
static bda_handle_t frame9588,draw9588,back9588,owner9588;
static void *brush9588;
static volatile unsigned detached9588,touch_active9588,touch_pending9588;
static volatile uint32_t touch_value9588;
static unsigned escape_suppressed9588,escape_pending9588;
static uint32_t resume9588,escape_due9588,previous9588,repeat9588,repeat_due9588;
static void suppress_escape9588(void){escape_suppressed9588=1;escape_pending9588=0;previous9588&=~16u;}
static void release_draw9588(void){if(draw9588&&(int32_t)draw9588!=-1)bda_gui_end_draw(draw9588);draw9588=owner9588=0;}
static void acquire9588(bda_handle_t h){if(draw9588&&owner9588==h)return;release_draw9588();draw9588=bda_gui_current_draw(h);if((int32_t)draw9588==-1)draw9588=0;owner9588=h;}
static int proc9588(bda_handle_t h,u32 msg,u32 wp,u32 lp){
    if(msg==BDA_MSG_DRAW_CONTEXT_ATTACH){acquire9588(h);dirty=1;}
    if(msg==BDA_MSG_DRAW_CONTEXT_DETACH){release_draw9588();detached9588=1;}
    if(msg==BDA_MSG_REDRAW_INPUT)dirty=1;
    if(msg==0x21){suppress_escape9588();resume9588=bda_gui_tick_count_25ms()+4;}
    if(msg==BDA_MSG_TOUCH_COORDINATE){touch_active9588=1;suppress_escape9588();resume9588=bda_gui_tick_count_25ms()+2;return 1;}
    if(msg==BDA_MSG_TOUCH_RELEASE){touch_active9588=0;suppress_escape9588();resume9588=bda_gui_tick_count_25ms()+2;touch_value9588=lp;touch_pending9588=1;return 1;}
    return bda_gui_default_proc(h,msg,wp,lp);
}
static void present9588(void){
    audio_scene();redraw();joey_audio_tick();uint16_t *pixels=(uint16_t *)(vx9588+24);
    for(unsigned i=0;i<240*320;++i)pixels[i]=bda_gui_rgb565_avoid_black_key(screen[i]);
    bda_gui_draw_vx(back9588,0,0,vx9588);bda_gui_draw_guard_begin();void *old=bda_gui_select_draw_object(draw9588,brush9588);
    bda_gui_context_copy(back9588,0,0,240,320,draw9588,0,0,0);bda_gui_select_draw_object(draw9588,old);bda_gui_draw_guard_end();joey_audio_tick();
}
static void poll9588(uint32_t now){
    bda_gui_input_packet_t p;uint32_t current=0;bda_gui_input_packet(&p);for(unsigned i=0;i<6;++i)if(p.bytes[i]==1)current|=1u<<i;
    if(escape_suppressed9588){if(!touch_active9588&&!(current&16))escape_suppressed9588=0;current&=~16u;escape_pending9588=0;}
    uint32_t pressed=current&~previous9588;previous9588=current;
    if(pressed&16){escape_pending9588=1;escape_due9588=now+4;}pressed&=~16u;
    if(escape_pending9588&&(int32_t)(now-escape_due9588)>=0){escape_pending9588=0;pressed|=16;}
    unsigned direction=current&15u;
    if(menu9588||browse9588||scene==0||scene==7||pending||!direction||(direction&(direction-1))||touch_active9588){repeat9588=0;}
    else if(direction!=repeat9588){repeat9588=direction;repeat_due9588=now+14;}
    else if((int32_t)(now-repeat_due9588)>=0){pressed|=direction;repeat_due9588=now+6;}
    if(!touch_active9588&&(int32_t)(now-resume9588)>=0)for(unsigned i=0;i<6;++i)if(pressed&(1u<<i)){key9588(i);break;}
}
static void game_tick9588(void){
    uint32_t now=h1_timer_read_ms();if(!menu9588&&!browse9588)ai_tick(now);else ai_clock=now;
    if(scene==1&&!details_open&&!menu9588&&!browse9588&&!ai_stage&&pending!=PENDING_REMODEL&&pending!=PENDING_PUBLIC_HAND&&joey_duel_state.winner<0&&(int32_t)(now-next_ai)>=0){
        Duel *d=&joey_duel_state;DuelCommand c;
        if(d->response_player<2){if(d->response_player==1&&duel_ai_decide(d,1,&c))ai_queue(&c);}
        else if(d->choice.kind){if(d->choice.player==1&&duel_ai_decide(d,1,&c))ai_queue(&c);}
        else if(d->active==1){if(duel_ai_decide(d,1,&c))ai_queue(&c);}
        else if(d->phase<DUEL_MAIN1&&!duel_standby_has_action(d,0))command_phase(d->phase+1);
        next_ai=now+230;
    }
    if(notice&&(int32_t)(now-notice_until)>=0){notice=0;dirty=1;}
    if(scene==1){remodel_report();effect_choices();response_choices();}
    diagnostic();audio_scene();
}
extern uint8_t __bss_start[],__bss_end[];
__attribute__((section(".text.bda_main"))) int bda_main(void){
    /* Initialize all static state again on every firmware launch. */
    for(volatile uint8_t *p=__bss_start;p<__bss_end;++p)*p=0;
    dirty=1;native_large_key=native_background_key=native_menu_key=native_hand_count=0xffffffffu;native_focus=native_stat_focus=-1;profile_slot=-1;
    if(!native_load()||!font_assets_valid()||!scene_load()){
        if(native_file)h1_fclose(native_file);if(native_art_file)h1_fclose(native_art_file);if(scene_file)h1_fclose(scene_file);
        bda_msgbox("Yugioh9588","Missing or incompatible assets.\nCopy FONT.BIN, H1UI.PAK, H1ART.PAK,\nH1SCN.PAK to A:\\YUGIOH\\");return 1;
    }
    profile_load();
    bda_frame_desc_t desc;bda_gui_message_t msg;bda_memset(&desc,0,sizeof(desc));bda_memset(&msg,0,sizeof(msg));
    desc.title="YUGIOH9588";desc.wndproc=proc9588;desc.height=240;desc.width=320;
    frame9588=bda_gui_register_frame_desc(&desc);if(!frame9588||(int32_t)frame9588==-1)goto files_cleanup;
    bda_gui_frame_activate(frame9588,0x100);acquire9588(frame9588);brush9588=bda_gui_draw_object_create(7);
    if(!draw9588||!brush9588||(int32_t)(uintptr_t)brush9588==-1)goto frame_cleanup;
    back9588=bda_gui_compatible_context_create(draw9588);if(!back9588||(int32_t)back9588==-1){back9588=0;goto frame_cleanup;}
    vx9588[0]='V';vx9588[1]='X';for(unsigned i=2;i<6;++i)vx9588[i]=204;vx9588[6]=240;vx9588[10]=64;vx9588[11]=1;for(unsigned i=14;i<20;++i)vx9588[i]=204;for(unsigned i=20;i<24;++i)vx9588[i]=255;
    joey_audio_open();if(joey_profile_state.reward_flags&JOEY_REWARD_PENDING)result_resume();
    present9588();
    {unsigned closing=0,wait=0;while(!detached9588){int pump=bda_gui_event_pump_frame_once(&msg,frame9588);
        if(!closing){if(touch_pending9588){uint32_t v=touch_value9588;touch_pending9588=0;unsigned x=v&65535,y=v>>16;if(x<240&&y<320)portrait_touch(x,y);}
            poll9588(bda_gui_tick_count_25ms());game_tick9588();if(dirty&&draw9588)present9588();
            if(exit_requested){joey_audio_close();bda_gui_frame_stop(frame9588);bda_gui_frame_release(frame9588);closing=1;}}
        else if(!pump||++wait>=128)break;bda_sys_delay(1);
    }if(!closing){bda_gui_frame_stop(frame9588);bda_gui_frame_release(frame9588);}}
    joey_audio_close();if(back9588)bda_gui_compatible_context_free(back9588);release_draw9588();bda_gui_close_frame(frame9588);goto files_cleanup;
frame_cleanup:
    bda_gui_frame_stop(frame9588);bda_gui_frame_release(frame9588);release_draw9588();bda_gui_close_frame(frame9588);
files_cleanup:
    if(native_file)h1_fclose(native_file);if(native_art_file)h1_fclose(native_art_file);if(font_file)h1_fclose(font_file);if(scene_file)h1_fclose(scene_file);
    if(font_glyph_cache)h1_free(font_glyph_cache);for(unsigned i=1;i<3;++i)if(font_books[i])h1_free(font_books[i]);return 0;
}
