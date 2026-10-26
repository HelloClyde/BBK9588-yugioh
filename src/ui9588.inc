/* All visible coordinates and touch hitboxes in this file are 240 x 320. */
static void back(void);
static void request_exit(void);
static unsigned option_list9588(void);
static unsigned secondary9588,menu9588,browse9588,browse_owner9588,browse_index9588;
static const uint16_t *phase_labels9588[]={u"抽牌",u"准备",u"主一",u"战斗",u"主二",u"结束"};
static void card_name9588(int x,int y,unsigned card,unsigned width,unsigned height,unsigned lines,uint32_t color){uint16_t s[160];NativeCard *c=&native_cards[card<1115?card:0];unsigned n=native_string(c->name,c->name_length,s,160);native_wrapped(x,y,width,s,n,height,lines,color);}
static void title9588(const uint16_t *label){native_fill(0,0,240,320,0x1083);native_panel(4,4,232,312);native_label(14,12,label,18,0x513819);}
static void scene_menu(void){
    scene_background(0);
    const uint16_t *labels[]={u"单局决斗",u"卡组整理",u"卡片鉴赏",u"对局记录",u"退出游戏",u"关于"};
    for(unsigned i=0;i<6;++i)portrait_button(35,140+i*27,170,24,labels[i],native_menu_focus==i);
    native_fill(0,305,240,15,0x1083);native_label(32,306,u"方向选择  确定进入",12,0xf9e2b8);
}
static void scene_about(void){
    title9588(u"关于 游戏王");
    native_label(16,48,u"移植作者：HelloClyde",16,0x35230f);
    native_label(16,78,u"混沌力量·城之内篇",16,0x35230f);
    native_label(16,104,u"BBK 9588 / 240 x 320",14,0x35230f);
    native_label(16,140,u"原版开发与发行：KONAMI",14,0x35230f);
    native_label(16,168,u"汉化：游戏王汉化制作组",14,0x35230f);
    native_label(16,204,u"原图、音乐、卡片等版权",14,0x513819);
    native_label(16,225,u"归原权利人所有。",14,0x513819);
    portrait_button(40,275,160,30,u"返回菜单",1);
}
static void details_draw(void){
    title9588(u"卡片详情");portrait_button(185,8,46,25,u"返回",0);
    native_card(12,43,native_selected,0);card_name9588(110,46,native_selected,116,16,3,0x35230f);
    NativeCard *c=&native_cards[native_selected];
    if(native_selected&&duel_card_definitions[native_selected].family==DUEL_MONSTER){
        int attack=c->attack,defense=c->defense;
        if(scene==1&&!option_list9588()&&native_focus>=105&&native_focus<115){unsigned row=(native_focus-100)/5,col=(native_focus-100)%5,player=row<2?1:0,index=player?4-col:col;attack=duel_attack_value(&joey_duel_state,player,index);defense=duel_defense_value(&joey_duel_state,player,index);}
        native_label(110,112,u"攻击",14,0x513819);native_number(151,112,attack<0?0:attack,14,0x35230f);
        native_label(110,137,u"守备",14,0x513819);native_number(151,137,defense<0?0:defense,14,0x35230f);
    }
    uint16_t s[640];unsigned count=native_string(c->description,c->description_length,s,640),starts[160],lines=1,col=0;starts[0]=0;
    for(unsigned i=0;i<count;++i){unsigned w=s[i]<128?7:14;if(s[i]=='\n'||col+w>208){if(lines<160)starts[lines++]=i+(s[i]=='\n');col=0;if(s[i]=='\n')continue;}col+=w;}
    unsigned maximum=lines>6?lines-6:0;if(detail_scroll>maximum)detail_scroll=maximum;
    for(unsigned row=0;row<6&&detail_scroll+row<lines;++row){unsigned line=detail_scroll+row,start=starts[line],end=line+1<lines?starts[line+1]:count;while(end>start&&s[end-1]=='\n')--end;native_text(16,182+row*17,s+start,end-start,14,0x35230f);}
    native_label(16,292,u"上下滚动说明",12,0x513819);portrait_button(150,284,36,25,u"上",0);portrait_button(190,284,36,25,u"下",0);
}
/* Keep the original wood as a frame; flat interiors give small text a clear
 * background, without the shadow overlapping the bitmap font's strokes. */
static void editor_button9588(int x,int y,unsigned w,unsigned h,const uint16_t *label,unsigned selected){
    for(unsigned yy=0;yy<h;++yy)for(unsigned xx=0;xx<w;++xx)screen[(y+yy)*240+x+xx]=native_wood[(yy*28/h)*181+xx*181/w];
    native_fill(x+2,y+2,w-4,h-4,selected?0x21a8:0xff9c);
    unsigned height=14,width=0;for(unsigned i=0;label[i];++i)width+=label[i]<128?height/2:height;
    native_label(x+((int)w-(int)width)/2,y+((int)h-(int)height)/2,label,height,selected?0xffffff:0x111111);
    if(selected)native_outline(x,y,w,h,0xffe0);
}
static void editor_name9588(int x,int y,unsigned card,uint32_t color){
    uint16_t text[160];NativeCard *c=&native_cards[card];unsigned n=native_string(c->name,c->name_length,text,160),width=0;
    for(unsigned i=0;i<n;++i)width+=text[i]<128?6:12;
    if(width>176){unsigned count=0;for(width=0;count<n&&width+(text[count]<128?6:12)<=158;++count)width+=text[count]<128?6:12;n=count;text[n++]='.';text[n++]='.';text[n++]='.';}
    native_text(x,y,text,n,12,color);
}
static void editor_draw(void){
    title9588(scene==4?u"卡片鉴赏":u"卡组整理");
    native_label(172,16,u"已用/持有",12,0x111111);
    const uint16_t *tabs[]={u"主卡组",u"融合",u"副卡组",u"持有卡"};
    for(unsigned i=0;i<4;++i)editor_button9588(7+i*57,37,55,26,tabs[i],editor_tab==i);
    unsigned page=editor_index/5;
    for(unsigned i=0;i<5&&page*5+i<editor_count;++i){unsigned index=page*5+i,card=editor_cards[index],y=69+i*36,selected=index==editor_index;uint32_t ink=selected?0xffffff:0x111111;native_fill(10,y,220,34,selected?0x21a8:0xff9c);scene_card(13,y+2,card,2,21,30);editor_name9588(42,y+1,card,ink);
        native_label(42,y+18,u"攻",10,ink);native_number(58,y+18,native_cards[card].attack,10,ink);native_label(92,y+18,u"守",10,ink);native_number(108,y+18,native_cards[card].defense,10,ink);
        native_number(164,y+18,joey_profile_count(&joey_edit_state,card),10,ink);native_label(176,y+18,u"/",10,ink);native_number(188,y+18,joey_edit_state.owned[card],10,ink);if(selected)native_outline(10,y,220,34,0xffe0);
    }
    if(!editor_count)native_label(50,130,u"此区域没有卡片",14,0x35230f);
    native_label(12,253,u"页",12,0x111111);native_number(28,253,page+1,12,0x111111);native_label(51,253,u"/",12,0x111111);native_number(63,253,(editor_count+4)/5,12,0x111111);
    native_label(100,253,u"主",12,0x111111);native_number(116,253,joey_edit_state.main_count,12,0x111111);native_label(143,253,u"融",12,0x111111);native_number(159,253,joey_edit_state.fusion_count,12,0x111111);native_label(185,253,u"副",12,0x111111);native_number(201,253,joey_edit_state.side_count,12,0x111111);
    editor_button9588(8,272,70,20,u"上一页",0);editor_button9588(85,272,70,20,u"详情",0);editor_button9588(162,272,70,20,u"下一页",0);
    editor_button9588(8,296,54,20,u"返回",0);editor_button9588(65,296,54,20,u"加入",0);editor_button9588(122,296,54,20,u"移出",0);editor_button9588(179,296,53,20,editor_changed?u"保存*":u"保存",0);
    if(editor_leaving){native_panel(12,85,216,170);native_label(25,98,u"牌组尚未保存",18,0x35230f);const uint16_t *labels[]={u"保存并离开",u"放弃修改",u"继续编辑"};for(unsigned i=0;i<3;++i)editor_button9588(28,132+i*37,184,30,labels[i],editor_exit_choice==i);}
}
enum { OPEN_ROW_X=12,OPEN_ROW_Y=74,OPEN_ROW_W=216,OPEN_ROW_H=72,OPEN_ROW_STEP=76,OPEN_ACTION_Y=264,OPEN_ACTION_H=34 };
static void opening_center9588(int center,int y,const uint16_t *label,unsigned h,uint32_t color){unsigned width=0;for(unsigned i=0;label[i];++i)width+=label[i]<128?h/2:h;native_label(center-(int)width/2,y,label,h,color);}
/* The three original tiles are 90 x 90. Use one scale for both axes;
 * the horizontal scene itself must never be squeezed into portrait. */
static void opening_hand9588(int x,int y,unsigned hand,unsigned size){if(hand>=3)return;unsigned sx=34+161*hand;for(unsigned yy=0;yy<size;++yy)for(unsigned xx=0;xx<size;++xx)screen[(y+yy)*240+x+xx]=native_background[(139+yy*90/size)*480+sx+xx*90/size];}
static void opening_draw(void){
    native_background_load(2);
    /* Repeat only the clean cloth above the original gesture illustrations. */
    for(unsigned y=0;y<320;++y)for(unsigned x=0;x<240;++x)screen[y*240+x]=native_background[(y%128)*480+120+x];
    native_panel(8,8,224,38);opening_center9588(120,17,u"猜拳决定先后攻",20,0x513819);
    const uint16_t *hands[]={u"石头",u"剪刀",u"布"};DuelOpening *o=&opening_state;
    if(o->phase==DUEL_OPEN_CHOOSE){
        opening_center9588(120,54,u"请选择手势",14,0xffffff);
        for(unsigned i=0;i<3;++i){unsigned y=OPEN_ROW_Y+i*OPEN_ROW_STEP;opening_hand9588(16,y+2,i,68);portrait_button(99,y+18,120,36,hands[i],0);if(opening_hand_focus==i)native_outline(OPEN_ROW_X,y,OPEN_ROW_W,OPEN_ROW_H,0xffe0);}
    }else{
        opening_center9588(63,64,u"我方",16,0xffffff);opening_center9588(177,64,u"城之内",16,0xffffff);
        opening_hand9588(18,90,o->human,90);opening_hand9588(132,90,o->enemy,90);
        if(o->human<3)opening_center9588(63,189,hands[o->human],14,0xffffff);if(o->enemy<3)opening_center9588(177,189,hands[o->enemy],14,0xffffff);
        native_panel(12,216,216,34);opening_center9588(120,225,o->phase==DUEL_OPEN_DRAW?u"平局，请再猜一次":o->phase==DUEL_OPEN_PLAYER_ORDER?u"你获胜，请选择":o->npc_first?u"城之内选择先攻":u"城之内选择后攻",14,0x513819);
        if(o->phase==DUEL_OPEN_PLAYER_ORDER){portrait_button(24,OPEN_ACTION_Y,90,OPEN_ACTION_H,u"先攻",!opening_order_focus);portrait_button(126,OPEN_ACTION_Y,90,OPEN_ACTION_H,u"后攻",opening_order_focus);}else portrait_button(35,OPEN_ACTION_Y,170,OPEN_ACTION_H,o->phase==DUEL_OPEN_DRAW?u"再猜一次":u"开始决斗",1);
    }native_label(32,305,u"返回取消  确定选择",12,0xffffff);
}
static void result_draw(void){
    title9588(result_winner==2?u"平局":result_winner?u"败北":u"胜利");
    if(result_card){native_card(15,46,result_card,0);card_name9588(112,49,result_card,110,14,3,0x35230f);native_label(112,115,result_new?u"新卡":u"持有卡",16,0x8f3010);native_number(112,145,joey_profile_state.owned[result_card],16,0x35230f);native_label(22,190,result_claimed?u"已加入收藏":u"收下后加入收藏",16,0x35230f);}
    else native_label(35,100,u"下次再来挑战！",20,0x35230f);
    native_label(22,225,u"换卡进度",14,0x513819);native_number(100,225,joey_profile_state.reward_gauge,14,0x35230f);native_label(127,225,u"/20",14,0x35230f);
    if(result_action){native_label(22,253,u"记录未保存，请重试",14,0x8f3010);portrait_button(24,277,192,30,u"重试保存",1);}
    else if(joey_profile_state.reward_flags&JOEY_REWARD_REROLL){portrait_button(14,277,104,30,u"收下卡片",!result_choice);portrait_button(123,277,104,30,u"换一张",result_choice);}
    else portrait_button(30,277,180,30,joey_profile_state.reward_flags&JOEY_REWARD_PENDING?u"收下卡片":u"返回菜单",1);
}
static void records_draw(void){title9588(u"对局记录");unsigned kinds=0,copies=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i){kinds+=joey_profile_state.owned[i]!=0;copies+=joey_profile_state.owned[i];}const uint16_t *labels[]={u"累计胜利",u"累计败北",u"持有种类",u"累计张数",u"换卡进度"};unsigned values[]={joey_profile_state.wins,joey_profile_state.losses,kinds,copies,joey_profile_state.reward_gauge};for(unsigned i=0;i<5;++i){native_label(25,63+i*37,labels[i],16,0x513819);native_number(155,63+i*37,values[i],16,0x35230f);}portrait_button(40,278,160,30,u"返回菜单",1);}

/* Card choices use a list, so long effect labels remain readable on 240 px. */
static const uint16_t *option_label9588(unsigned choice){
    Duel *d=&joey_duel_state;DuelChoiceOption *o=&effect_options[choice];unsigned kind=d->choice.kind;
    if(o->kind==DUEL_OPTION_SKIP)return u"取消 / 不发动";
    if(o->kind!=DUEL_OPTION_CONFIRM)return 0;
    if(pending==PENDING_ATTACK_SELECT||kind==DUEL_CHOICE_ATTACK_REPLAY)return u"直接攻击";
    if(kind==DUEL_CHOICE_CHOSEN_POSITION||kind==DUEL_CHOICE_CYBER_POSITION)return o->index?u"盖放守备":u"攻击表示";
    if(kind==DUEL_CHOICE_RETARGET_MODE)return o->index==1?u"转移卡片目标":u"转移攻击目标";
    if(kind==DUEL_CHOICE_EARTH_FIRST||kind==DUEL_CHOICE_EARTH_SECOND||kind==DUEL_CHOICE_EARTH_OPPONENT||kind==DUEL_CHOICE_ACTIVATION_PARAMETER)return native_trait_label(d->choice.card,o->index);
    if(kind==DUEL_CHOICE_EQUIP_STAT)return o->index?u"守备力+700":u"攻击力+700";
    if(kind==DUEL_CHOICE_ATTACK_COIN||kind==DUEL_CHOICE_FAIRY_BOX_COIN||kind==DUEL_CHOICE_GAMBLE_COIN)return o->index?u"反面":u"正面";
    if(kind==DUEL_CHOICE_PILE_PLAYER)return o->index?u"对方墓地":u"我方墓地";
    if(kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION||kind==DUEL_CHOICE_FUSION_POSITION||kind==DUEL_CHOICE_RITUAL_POSITION||kind==DUEL_CHOICE_IGNITION_POSITION||kind==DUEL_CHOICE_SPECIAL_HAND_POSITION||kind==DUEL_CHOICE_PILE_SUMMON_POSITION||kind==DUEL_CHOICE_GRAVE_PAIR_POSITION)return o->index==DUEL_FACEDOWN_DEFENSE?u"盖放守备":o->index?u"守备表示":u"攻击表示";
    if(kind==DUEL_CHOICE_KUNAI_MODE)return o->index==1?u"攻击者转守备":o->index==2?u"装备加500攻":u"两项都发动";
    if(kind==DUEL_CHOICE_OFFERING_FORM)return o->index?u"盖放守备":u"通常召唤";
    if(kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND)return o->index/5?u"对方区域":u"我方区域";
    if(kind==DUEL_CHOICE_OFFERING_ZONE)return u"怪兽区域";
    if(kind==DUEL_CHOICE_BAIT_BACKUP)return u"完成";
    if(kind==DUEL_CHOICE_UPKEEP){unsigned g=duel_card_definitions[d->choice.card].name_group;return g==1424?u"支付500":g==1131?u"支付100":g==1154?u"支付700":g==1099?u"支付2000":u"支付1000";}
    if(kind==DUEL_CHOICE_REVIVAL_JAM)return u"支付1000";
    if(kind==DUEL_CHOICE_DRILL_OFFER||kind==DUEL_CHOICE_HISTORY_OFFER||kind==DUEL_CHOICE_SEARCH_OFFER||kind==DUEL_CHOICE_FLIP_OFFER||kind==DUEL_CHOICE_DARK_SAGE_OFFER)return u"发动效果";
    if(kind==DUEL_CHOICE_INSPECTION_OFFER||kind==DUEL_CHOICE_OFFERING_PAY||kind==DUEL_CHOICE_EQUIP_RECYCLE||kind==DUEL_CHOICE_MUSHROOM)return u"支付500";
    if(kind==DUEL_CHOICE_DEFUSION||kind==DUEL_CHOICE_BATTLE_REVIVE)return u"特殊召唤";
    return kind==DUEL_CHOICE_PERIODIC?u"加入手牌":u"确认效果";
}
static unsigned option_count9588(void){return pending==PENDING_RESPONSE?response_count+1:pending==PENDING_BACKUP||pending==PENDING_REMODEL?picker_count+1:picker_count;}
static unsigned option_list9588(void){return pending==PENDING_GRAVE||pending==PENDING_EFFECT_CHOICE||pending==PENDING_RESPONSE||pending==PENDING_BACKUP||pending==PENDING_REMODEL||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND;}
/* Resolve from the highlighted option, never from the previous board focus.
 * A zero card is also the rules engine's concealed-card marker. */
static unsigned option_card9588(unsigned choice){
    unsigned card=0;
    if(pending==PENDING_RESPONSE){if(choice<response_count)card=response_options[choice].card;}
    else if(choice<picker_count){
        if((pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND)&&(effect_options[choice].kind==DUEL_OPTION_SKIP||effect_options[choice].kind==DUEL_OPTION_PAGE))return 0;
        card=picker_card(choice);
    }
    return card<DUEL_CARD_COUNT?card:0;
}
static void option_focus9588(unsigned choice){grave_choice=choice;grave_page=choice/5;native_selected=option_card9588(choice);}
static void option_details9588(void){unsigned card=option_card9588(grave_choice);if(card){native_selected=card;details_open=1;detail_scroll=0;dirty=1;}}
static void option_context9588(unsigned choice,int y){
    DuelChoiceOption *o=&effect_options[choice];unsigned kind=joey_duel_state.choice.kind,token=o->index,index=token;const uint16_t *label=0;
    if(kind==DUEL_CHOICE_PATROL_TARGET){label=token<5?u"怪兽区":token<10?u"魔陷区":u"场地区";index=5-token%5;}
    else if(kind==DUEL_CHOICE_TARGET_FLIP){index=token%11;label=token/11?(index<5?u"对方怪兽":index<10?u"对方魔陷":u"对方场地"):(index<5?u"我方怪兽":index<10?u"我方魔陷":u"我方场地");index=token/11?5-index%5:index%5+1;}
    else if(kind==DUEL_CHOICE_ATTACK_REPLAY||pending==PENDING_ATTACK_SELECT){label=u"对方怪兽";index=5-token%5;}
    else if(kind==DUEL_CHOICE_PENGUIN||kind==DUEL_CHOICE_FLIP_TARGET){label=token/5?u"对方怪兽":u"我方怪兽";index=token/5?5-token%5:token%5+1;}
    else if(kind==DUEL_CHOICE_FUSION_MATERIAL||kind==DUEL_CHOICE_RITUAL_MATERIAL||(kind==DUEL_CHOICE_SPECIAL_HAND_COST&&duel_hand_special_cost_is_field(&duel_card_definitions[joey_duel_state.choice.card])==2)){label=token<DUEL_HAND_MAX?u"手牌":u"我方场上";index=token<DUEL_HAND_MAX?token+1:token-DUEL_HAND_MAX+1;}
    else if(kind==DUEL_CHOICE_TRIBUTE_UPKEEP||kind==DUEL_CHOICE_SPECIAL_HAND_COST){label=token<5?u"我方场上":u"对方场上";index=token%5+1;}
    else if(kind==DUEL_CHOICE_OFFERING_ZONE||kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND){label=token/5?u"对方区域":u"我方区域";index=token%5+1;}
    if(label){native_label(43,y,label,10,0x513819);native_number(118,y,index,10,0x35230f);}
}
static void pending_draw9588(void){
    if(pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT){native_panel(8,72,224,174);native_label(24,84,u"怪兽操作",18,0x513819);const uint16_t *labels[]={u"发动效果",pending==PENDING_HAND_EFFECT?u"通常召唤":u"改变表示",u"取消"};if((pending==PENDING_MONSTER_MENU&&duel_monster_deck_special(&joey_duel_state,0,monster_source))||(pending==PENDING_HAND_EFFECT&&duel_hand_special(&duel_card_definitions[joey_duel_state.players[0].hand[source_hand]])))labels[0]=u"特殊召唤";for(unsigned i=0;i<3;++i)portrait_button(24,116+i*39,192,32,labels[i],monster_choice==i);return;}
    if(pending==PENDING_POSITION||pending==PENDING_COIN_GUESS||pending==PENDING_LIFE_TARGET){native_panel(8,90,224,136);native_label(24,105,pending==PENDING_POSITION?u"选择表示形式":pending==PENDING_COIN_GUESS?u"猜硬币正反面":u"请选择回复对象",18,0x513819);unsigned selected=pending==PENDING_POSITION?grave_position:pending==PENDING_COIN_GUESS?coin_guess:life_choice;portrait_button(20,154,96,38,pending==PENDING_POSITION?u"攻击表示":pending==PENDING_COIN_GUESS?u"正面":u"我方",!selected);portrait_button(124,154,96,38,pending==PENDING_POSITION?u"守备表示":pending==PENDING_COIN_GUESS?u"反面":u"对方",selected);return;}
    if(pending==PENDING_TRIBUTE||pending==PENDING_ATTACK||pending==PENDING_SPELL_TARGET||pending==PENDING_MONSTER_TARGET||pending==PENDING_DISCARD_COST)return;
    unsigned count=option_count9588();if(!count)return;
    native_panel(6,33,228,260);native_label(16,43,pending==PENDING_RESPONSE?u"选择连锁响应":pending==PENDING_BACKUP?u"选一至三张卡片":pending==PENDING_REMODEL?u"混沌壶翻牌结果":pending==PENDING_PUBLIC_HAND?u"对方公开手牌":u"选择卡片或效果",16,0x513819);
    unsigned page=grave_choice/5;
    for(unsigned i=0;i<5&&page*5+i<count;++i){unsigned index=page*5+i,y=69+i*36;native_fill(12,y,216,34,index==grave_choice?0x8c4e:0xbd93);const uint16_t *label=0;unsigned card=0;
        if(pending==PENDING_RESPONSE){if(index==response_count)label=u"不发动 / 继续";else card=response_options[index].card;}
        else if((pending==PENDING_BACKUP||pending==PENDING_REMODEL)&&index==picker_count)label=u"完成选择";
        else {card=picker_card(index);if(pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND)label=option_label9588(index);}
        if(card){scene_card(15,y+2,card,2,21,30);card_name9588(43,y+2,card,176,12,1,0x35230f);}else if(!label)label=u"选择区域";
        if(label)native_label(card?43:22,y+(card?17:7),label,12,0x35230f);
        if(pending==PENDING_BACKUP&&index<picker_count&&picker_selected[index])native_label(203,y+16,u"选",12,0x114412);
        if(!label&&(pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT))option_context9588(index,y+21);
        if(label&&pending==PENDING_EFFECT_CHOICE&&(joey_duel_state.choice.kind==DUEL_CHOICE_GROUND_FIRST||joey_duel_state.choice.kind==DUEL_CHOICE_GROUND_SECOND||joey_duel_state.choice.kind==DUEL_CHOICE_OFFERING_ZONE))native_number(185,y+7,effect_options[index].index%5+1,12,0x35230f);
        if(index==grave_choice)native_outline(12,y,216,34,0xffe0);
    }
    portrait_button(12,251,45,16,u"上页",0);native_number(99,253,page+1,12,0x35230f);native_label(118,253,u"/",12,0x513819);native_number(132,253,(count+4)/5,12,0x35230f);portrait_button(183,251,45,16,u"下页",0);
    portrait_button(12,270,68,20,u"返回",0);portrait_button(86,270,68,20,u"详情",0);portrait_button(160,270,68,20,u"确认",1);
    if(!option_card9588(grave_choice)){native_fill(88,272,64,16,0xc618);native_label(106,273,u"详情",14,0x777777);}
}
static void board_draw(void){
    Duel *d=&joey_duel_state;native_backdrop(0);
    /* Reuse a clean cloth patch from the original board in the portrait gaps. */
    for(unsigned y=0;y<320;++y)for(unsigned x=0;x<240;++x)screen[y*240+x]=native_background[(45+y%24)*480+212+x%20];
    native_fill(0,0,240,32,0x1083);
    native_label(4,2,u"城之内 LP",12,0xffd9a0);native_number(66,2,d->players[1].life,14,0xffd9a0);native_label(131,2,d->active?u"对方回合":u"我方回合",12,0xffffff);native_number(207,2,d->turn,12,0xffffff);
    if(ai_stage)native_label(4,19,ai_label,12,0xffdf91);
    else if(native_selected)card_name9588(4,19,native_selected,208,12,1,0xffffff);
    else {native_label(4,19,u"对方手牌",12,0xd0d0d0);native_number(64,19,d->players[1].hand_count,12,0xffffff);native_label(103,19,u"牌组",12,0xd0d0d0);native_number(136,19,d->players[1].deck_count,12,0xffffff);}
    for(unsigned row=0;row<4;++row)for(unsigned col=0;col<5;++col){unsigned player=row<2?1:0,index=player?4-col:col,br=row==0||row==3;int x=20+col*40,y=35+row*43+(row>=2?2:0);DuelSlot *s=br?&d->players[player].backrow[index]:&d->players[player].monsters[index];native_fill(x,y,40,40,row<2?0x1968:0x1987);native_outline(x,y,40,40,0x6b71);
        if(s->card){unsigned quarter=br?(player?2:0):(s->position!=DUEL_FACEUP_ATTACK&&!(s->position==DUEL_FACEDOWN_DEFENSE&&s->facedown_attack))?1:player?2:0;portrait_field(x,y,s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,quarter);}else if(!br&&s->zone_blocked)native_label(x+14,y+10,u"X",18,0xff6644);
        if(native_focus==(int)(100+row*5+col)||(!br&&(tribute_mask&(1u<<(player*5+index)))&&(pending==PENDING_TRIBUTE||pending==PENDING_MONSTER_TARGET)))native_outline(x,y,40,40,0xffe0);
    }
    for(unsigned player=0;player<2;++player){int y=player?45:178;DuelSlot *s=&d->players[player].field;if(s->card)scene_card(1,y,s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,2,17,25);else native_label(2,y,u"场",12,0xe4ce9c);if(native_focus==(player?120:121))native_outline(0,y-2,19,29,0xffe0);native_label(222,y,u"墓",12,0xe4ce9c);native_number(221,y+15,d->players[player].grave_count,10,0xffffff);}
    for(unsigned i=0;i<6;++i){native_fill(i*40,209,40,19,i==d->phase?0x8c4e:0x2926);native_label(8+i*40,211,phase_labels9588[i],12,i==d->phase?0xffe1a0:0xffffff);if(native_focus==(int)(200+i))native_outline(i*40,209,40,19,0xffe0);}
    native_fill(0,230,240,48,0x1083);unsigned shown=hand_visible(),count=d->players[0].hand_count;if(hand_scroll+shown>count)hand_scroll=count-shown;
    for(unsigned i=0;i<shown;++i){unsigned index=hand_scroll+i;scene_card(16+i*35,232,d->players[0].hand[index],1,30,44);if(native_focus==(int)index)native_outline(15+i*35,231,32,46,0xffe0);}
    if(hand_scroll)native_label(1,246,u"<",14,0xffe0b1);if(hand_scroll+shown<count)native_label(227,246,u">",14,0xffe0b1);
    native_fill(0,279,240,17,0x1083);native_label(3,281,u"我方 LP",12,0xc9edff);native_number(53,281,d->players[0].life,14,0xc9edff);native_label(113,281,u"手",12,0xffffff);native_number(129,281,count,12,0xffffff);native_label(163,281,u"牌组",12,0xffffff);native_number(197,281,d->players[0].deck_count,12,0xffffff);
    portrait_button(0,297,59,23,u"详情",0);portrait_button(60,297,59,23,secondary9588?u"盖放*":u"盖放",secondary9588);portrait_button(120,297,59,23,u"确认",0);portrait_button(180,297,60,23,u"菜单",0);
    if(pending)pending_draw9588();
    if(menu9588){native_panel(18,49,204,222);const uint16_t *items[]={u"继续决斗",u"卡片详情",u"盖放 / 切换表示",u"查看墓地",u"结束回合",u"返回主菜单"};for(unsigned i=0;i<6;++i)portrait_button(32,60+i*34,176,29,items[i],menu9588==i+1);}
}
static void browse_draw9588(void){title9588(browse_owner9588?u"对方墓地":u"我方墓地");unsigned count=joey_duel_state.players[browse_owner9588].grave_count;if(browse_index9588>=count)browse_index9588=count?count-1:0;unsigned page=browse_index9588/5;for(unsigned i=0;i<5&&page*5+i<count;++i){unsigned index=page*5+i,card=joey_duel_state.players[browse_owner9588].grave[index],y=60+i*40;scene_card(16,y,card,2,25,36);card_name9588(48,y+6,card,172,14,1,0x35230f);if(index==browse_index9588)native_outline(11,y-1,218,39,0xffe0);}if(!count)native_label(44,120,u"墓地没有卡片",16,0x35230f);native_label(25,37,u"左右切换  上下选择",12,0x513819);portrait_button(12,258,66,16,u"上页",0);native_number(111,260,page+1,12,0x35230f);portrait_button(160,258,66,16,u"下页",0);portrait_button(12,278,66,29,u"返回",0);portrait_button(86,278,66,29,u"详情",1);portrait_button(160,278,66,29,u"切换",0);}
static void redraw(void){
    if(details_open)details_draw();else if(browse9588)browse_draw9588();else if(scene==7)opening_draw();else if(scene==5)result_draw();else if(scene==6)records_draw();else if(scene==3||scene==4)editor_draw();else if(scene==0)scene_menu();else board_draw();
    if(about_open)scene_about();
    if(!about_open&&notice&&(int32_t)(notice_until-h1_timer_read_ms())>0){uint16_t text[80];unsigned i=0;while(notice[i]&&i<79){text[i]=notice[i];++i;}native_panel(4,250,232,44);native_wrapped(12,254,216,text,i,12,2,0x35230f);}
    dirty=0; /* The 9588 frame owner presents this buffer after redraw. */
}
static void details_toggle9588(void){if(native_selected){details_open=!details_open;detail_scroll=0;dirty=1;}}
static void menu_action9588(void){unsigned selected=menu9588;menu9588=0;if(selected==2)details_toggle9588();else if(selected==3)secondary9588^=1;else if(selected==4){browse9588=1;browse_owner9588=browse_index9588=0;}else if(selected==5)command_phase(DUEL_END);else if(selected==6)close_game();dirty=1;}
static void browse_action9588(unsigned key){unsigned count=joey_duel_state.players[browse_owner9588].grave_count;if(key==4)browse9588=0;else if(key==5){if(count){native_selected=joey_duel_state.players[browse_owner9588].grave[browse_index9588];native_focus=-1;details_toggle9588();}}else if(key<2){browse_owner9588^=1;browse_index9588=0;}else if(count)browse_index9588=(browse_index9588+(key==3?count-1:1))%count;dirty=1;}
static void key9588(unsigned key){
    if(option_list9588()&&!details_open&&key==0){option_details9588();return;}
    if(browse9588&&!details_open){browse_action9588(key);return;}
    if(menu9588){if(key==4)menu9588=0;else if(key==5)menu_action9588();else menu9588=(menu9588+(key==1||key==3?4:0))%6+1;dirty=1;return;}
    if(key==4&&scene==1&&!pending&&!details_open&&!about_open){menu9588=1;dirty=1;return;}
    if(key==4){back();return;}if(key==5){activate(secondary9588);secondary9588=0;return;}
    static const unsigned nav[]={H1_KEY_RIGHT,H1_KEY_LEFT,H1_KEY_DOWN,H1_KEY_UP};move(nav[key]);
}
static void portrait_touch(int x,int y){
    dirty=1;if(about_open){about_open=0;return;}
    if(details_open){if(y<37){details_open=0;return;}if(y>=280){if(x>=190)move(H1_KEY_DOWN);else if(x>=150)move(H1_KEY_UP);}return;}
    if(browse9588){unsigned count=joey_duel_state.players[browse_owner9588].grave_count;if(y>=278){browse_action9588(x<80?4:x<158?5:0);}else if(y>=258&&y<276){if(x<80)browse_index9588=browse_index9588>=5?browse_index9588-5:0;else if(x>=160&&browse_index9588/5*5+5<count)browse_index9588=browse_index9588/5*5+5;}else if(y>=60&&y<258){unsigned index=browse_index9588/5*5+(y-60)/40;if(index<count){if(index==browse_index9588)browse_action9588(5);else browse_index9588=index;}}return;}
    if(scene==0){if(y>=140&&y<302&&x>=35&&x<205){native_menu_focus=(y-140)/27;activate(0);}return;}
    if(scene==7){if(opening_state.phase==DUEL_OPEN_CHOOSE){if(x>=OPEN_ROW_X&&x<OPEN_ROW_X+OPEN_ROW_W&&y>=OPEN_ROW_Y){unsigned row=(y-OPEN_ROW_Y)/OPEN_ROW_STEP;if(row<3&&(y-OPEN_ROW_Y)%OPEN_ROW_STEP<OPEN_ROW_H){opening_hand_focus=row;opening_confirm();}}}else if(y>=OPEN_ACTION_Y&&y<OPEN_ACTION_Y+OPEN_ACTION_H){if(opening_state.phase==DUEL_OPEN_PLAYER_ORDER){if(x>=24&&x<114){opening_order_focus=0;opening_confirm();}else if(x>=126&&x<216){opening_order_focus=1;opening_confirm();}}else if(x>=35&&x<205)opening_confirm();}return;}
    if(scene==6){if(y>=277)close_game();return;}
    if(scene==5){if(y>=277)result_activate(x>=120);else if(result_card&&x<106&&y>=46&&y<175)details_toggle9588();return;}
    if(scene==3||scene==4){
        if(editor_leaving){if(y>=132&&y<243&&x>=28&&x<212){editor_exit_choice=(y-132)/37;editor_exit();}return;}
        if(y>=37&&y<63){unsigned tab=(x-7)/57;if(x>=7&&tab<4)editor_select(tab,editor_cursor[tab]);return;}
        if(y>=69&&y<249){unsigned index=editor_index/5*5+(y-69)/36;if(index<editor_count){editor_index=index;editor_cursor[editor_tab]=index;native_selected=editor_cards[index];}return;}
        if(y>=272&&y<292){if(x<80)editor_page(-1);else if(x<160)details_toggle9588();else editor_page(1);return;}
        if(y>=296){if(x<65)editor_leave(0);else if(x<122){if(scene==3)editor_change(1);}else if(x<179){if(scene==3)editor_change(0);}else if(scene==3)profile_save();}return;
    }
    if(menu9588){if(y>=60&&y<264&&x>=32&&x<208){menu9588=(y-60)/34+1;menu_action9588();}return;}
    if(pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT){if(x>=24&&x<216&&y>=116&&y<233){monster_choice=(y-116)/39;confirm_pending();}return;}
    if(pending==PENDING_POSITION||pending==PENDING_COIN_GUESS||pending==PENDING_LIFE_TARGET){if(y>=154&&y<192){if(pending==PENDING_POSITION)grave_position=x>=120;else if(pending==PENDING_COIN_GUESS)coin_guess=x>=120;else life_choice=x>=120;confirm_pending();}return;}
    unsigned list=option_list9588();
    if(list){if(y>=270&&y<292){if(x>=12&&x<80)back();else if(x>=86&&x<154)option_details9588();else if(x>=160&&x<228)confirm_pending();return;}if(y>=297&&x>=0&&x<59){option_details9588();return;}if(y>=251&&y<269){unsigned count=option_count9588(),page=grave_choice/5;if(x>=12&&x<57&&page)option_focus9588((page-1)*5);else if(x>=183&&x<228&&(page+1)*5<count)option_focus9588((page+1)*5);return;}if(x>=12&&x<228&&y>=69&&y<249&&(y-69)%36<34){unsigned index=grave_choice/5*5+(y-69)/36;if(index<option_count9588()){if(index==grave_choice)confirm_pending();else option_focus9588(index);}return;}return;}
    if(y<32&&!pending&&duel_hand_revealed(&joey_duel_state,1)){public_hand_choices();return;}
    if(y>=297){if(x<60)details_toggle9588();else if(x<120)secondary9588^=1;else if(x<180){activate(secondary9588);secondary9588=0;}else menu9588=1;return;}
    if(y>=209&&y<228){command_phase(x/40);return;}
    if(ai_stage==1)return;
    if(y>=230&&y<278){unsigned count=joey_duel_state.players[0].hand_count;if(x<16){hand_scroll=hand_scroll>=6?hand_scroll-6:0;return;}if(x>=225){if(hand_scroll+6<count){hand_scroll+=6;if(hand_scroll+6>count)hand_scroll=count>6?count-6:0;}return;}unsigned index=hand_scroll+(x-16)/35;if(index<count&&(x-16)%35<32){if(native_focus==(int)index){begin_hand(index,secondary9588);secondary9588=0;}else select_card(joey_duel_state.players[0].hand[index],index);}return;}
    if(x>=222&&y>=35&&y<206){browse9588=1;browse_owner9588=y<120;browse_index9588=0;return;}
    if(x<20&&y>=35&&y<206){unsigned player=y<120;DuelSlot *s=&joey_duel_state.players[player].field;int focus=player?120:121;if(pending||native_focus==focus){field_action(player,DUEL_FIELD_SLOT,1,secondary9588);secondary9588=0;}else select_card(player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,focus);return;}
    if(x>=20&&x<220&&y>=35&&y<206){unsigned row=y<78?0:y<123?1:y<166?2:3,col=(x-20)/40,player=row<2?1:0,index=player?4-col:col,br=row==0||row==3;DuelSlot *s=br?&joey_duel_state.players[player].backrow[index]:&joey_duel_state.players[player].monsters[index];int focus=100+row*5+col;if(pending||native_focus==focus){field_action(player,index,br,secondary9588);secondary9588=0;}else select_card(player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,focus);}
}
