/* Native M5 workflow; computational models and upstream apps remain separate. */
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>
#include "sequential.h"
#include "sequential_ui.h"
#include "seq_connections.h"
#include "solver.h"
#include "keys.h"
#include "info.h"
#include "utils.h"

#define ACCENT C_RGB(2,12,17)
#define HIGHLIGHT C_RGB(26,29,31)
#define ORANGE C_RGB(30,13,2)
enum { SETUP, STATES, EXCITE, EQUATIONS, MAP, TRACE, WIRES, OUTPUTS, RECOVERY };
static sq_model model;
static unsigned view, selected, pin, page, group;
static cn_netlist connections_net;
static eb_form form;
static bool map_details;
static char const *const titles[] = {"Setup", "State table", "Excitation", "Equations", "K-map", "State trace", "Connections", "Output table", "Recovery"};

static void wrapped(const char *text,int y,unsigned first,unsigned lines,int color)
{
    size_t length=strlen(text);
    for(unsigned line=0;line<lines;line++) {
        size_t off=(size_t)(first+line)*46;
        if(off>=length)break;
        size_t n=length-off;if(n>46)n=46;
        char part[47];memcpy(part,text+off,n);part[n]='\0';
        dtext(10,y+(int)line*16,color,part);
    }
}
static void binary(unsigned value,char out[4])
{
    if(value==SQ_UNDEFINED){strcpy(out,"!");return;}
    for(unsigned b=0;b<model.bits;b++)out[b]=(char)('0'+((value>>(model.bits-1-b))&1));
    out[model.bits]='\0';
}
static unsigned fields(unsigned ids[24])
{
    unsigned n=0;
    for(unsigned i=0;i<7;i++)ids[n++]=i;
    if(model.controlled)ids[n++]=7;
    for(unsigned bit=model.bits;bit-->0;)ids[n++]=10-bit;
    if(!model.reverse){ids[n++]=11;if(model.controlled)ids[n++]=12;}
    else for(unsigned bit=model.bits;bit-->0;) {
        ids[n++]=14+bit*2;
        if(model.types[bit]==FF_SR || model.types[bit]==FF_JK)ids[n++]=15+bit*2;
    }
    ids[n++]=20;ids[n++]=21;
    if(model.controlled)ids[n++]=13;
    return n;
}
static int text_field(unsigned id)
{
    if(id==11 || id==12)return (int)id-11;
    if(id==13)return 10;
    if(id>=14 && id<20)return (int)id-12;
    if(id==20 || id==21)return (int)id-12;
    return -1;
}
static void field_label(unsigned id,char out[48])
{
    static char const *const names[]={"Problem mode","State bits","Control E","Clock edge","Unused states","Recovery state","Initial state","Fixed trace E"};
    if(id<8)snprintf(out,48,"%s",names[id]);
    else if(id<=10)snprintf(out,48,"Q%u flip-flop",10-id);
    else if(id==11 || id==12)snprintf(out,48,"Cycle E=%u",id-11);
    else if(id==13)snprintf(out,48,"E timeline (empty=fixed E)");
    else if(id<20) {
        unsigned eq=id-14,bit=eq/2;
        snprintf(out,48,"Input %s%u",ff_input_name(model.types[bit],eq%2),bit);
    }
    else snprintf(out,48,"Output Z%u (empty=none)",id-20);
}
static void field_value(unsigned id,char out[80])
{
    int f=text_field(id);
    if(f>=0){snprintf(out,80,"%s",sq_text(&model,(unsigned)f));return;}
    if(id==0)snprintf(out,80,"%s",model.reverse?"REVERSE equations":"DESIGN from cycles");
    else if(id==1)snprintf(out,80,"%u",model.bits);
    else if(id==2)snprintf(out,80,"%s",model.controlled?"Enabled: two E branches":"None");
    else if(id==3)snprintf(out,80,"%s",model.active_edge==FF_RISING?"RISE":"FALL");
    else if(id==4)snprintf(out,80,"%s",sq_policy_name(model.policy));
    else if(id==5)snprintf(out,80,"%u",model.recovery);
    else if(id==6)snprintf(out,80,"%u",model.initial);
    else if(id==7)snprintf(out,80,"%u",model.fixed_e);
    else snprintf(out,80,"%s",ff_type_name(model.types[10-id]));
}
static void mapping(char out[80])
{
    size_t n=0;unsigned vars=model.bits+(model.controlled?1:0);
    out[0]='\0';
    for(unsigned v=0;v<vars;v++)
        n+=(size_t)snprintf(out+n,80-n,"%c=%s%s",'A'+(int)v,sq_variable_name(&model,v),v+1<vars?"  ":"");
}
static void setup(void)
{
    unsigned ids[24],n=fields(ids);selected%=n;
    unsigned first=selected/6*6;
    dtext(10,43,C_DARK,"UP/DOWN select | EXE edits / cycles setting");
    for(unsigned i=first;i<first+6 && i<n;i++) {
        char name[48],value[80],line[47];field_label(ids[i],name);field_value(ids[i],value);
        int y=63+(int)(i-first)*21;
        if(i==selected)drect(7,y-2,386,y+17,HIGHLIGHT);
        snprintf(line,sizeof line,"%.18s: %.25s",name,value[0]?value:"(empty)");
        dtext(10,y,C_BLACK,line);
    }
    if(model.error[0])wrapped(model.error,190,0,1,C_RED);
    else dprint(10,190,C_DARK,"Field %u/%u | OPTN examples | F6 solve",selected+1,n);
}
static char expression_key(int key)
{
    switch(key){
    case KEY_XOT:return 'A';case KEY_LOG:return 'B';case KEY_LN:return 'C';case KEY_SIN:return 'D';
    case KEY_0:return '0';case KEY_1:return '1';case KEY_ADD:return '+';case KEY_MUL:return '*';
    case KEY_POWER:return '^';case KEY_DIV:return '!';case KEY_DOT:return '\'';
    case KEY_LEFTP:return '(';case KEY_RIGHTP:return ')';default:return 0;
    }
}
static void editor(unsigned id)
{
    unsigned f=(unsigned)text_field(id);char text[80],name[48],map[80];
    snprintf(text,sizeof text,"%s",sq_text(&model,f));field_label(id,name);mapping(map);
    size_t cursor=strlen(text),capacity=f<2?32:f==10?33:80;
    bool full=false;
    for(;;){
        dclear(C_WHITE);info_title("Edit sequential problem",ACCENT);
        dtext(10,29,ACCENT,name);wrapped(map,48,0,1,C_DARK);
        drect_border(7,66,386,113,C_WHITE,1,ACCENT);wrapped(text,73,0,2,C_BLACK);
        char prefix[47];size_t line=cursor/46,col=cursor%46;
        memcpy(prefix,text+line*46,col);prefix[col]='\0';int width=0;dsize(prefix,NULL,&width,NULL);
        drect(10+width,73+(int)line*16,11+width,84+(int)line*16,ACCENT);
        if(f<2){dtext(10,128,C_DARK,"Enter states as digits, e.g. 03570.");dtext(10,146,C_DARK,"Decimal point inserts comma; closing0 OK.");}
        else if(f==10){dtext(10,128,C_DARK,"One 0/1 per active clock, max32 steps.");dtext(10,146,C_DARK,"Empty timeline follows fixed E to a cycle.");}
        else{dtext(10,128,C_DARK,"X/log/ln/sin = A/B/C/D | / NOT");dtext(10,146,C_DARK,"+ OR  x AND  ^ XOR  . postfix NOT");}
        dtext(10,169,full?C_RED:C_DARK,full?"Text capacity reached; edit or cancel.":"LEFT/RIGHT cursor | DEL delete | AC clear");
        dtext(10,190,ACCENT,"EXE saves | EXIT cancels without changes");
        fkey_button(1,"CLEAR");fkey_button_color(6,"SAVE",ACCENT);dupdate();
        int key=getkey().key;if(key==KEY_EXIT)return;
        if(key==KEY_EXE || key==KEY_F6){sq_set_text(&model,f,text);return;}
        if(key==KEY_ACON || key==KEY_F1){text[0]='\0';cursor=0;full=false;continue;}
        if(key==KEY_LEFT){if(cursor)cursor--;continue;}
        if(key==KEY_RIGHT){if(cursor<strlen(text))cursor++;continue;}
        if(key==KEY_DEL){if(cursor){memmove(text+cursor-1,text+cursor,strlen(text+cursor)+1);cursor--;}full=false;continue;}
        char c=0;
        if(f<2 || f==10){int digit=key_digit(key);if(digit>=0)c=(char)('0'+digit);if(f<2 && key==KEY_DOT)c=',';}
        else c=expression_key(key);
        if(c){size_t n=strlen(text);if(n+1>=capacity){full=true;continue;}memmove(text+cursor+1,text+cursor,n-cursor+1);text[cursor++]=c;full=false;}
    }
}
static void edit_setting(void)
{
    unsigned ids[24],n=fields(ids);selected%=n;unsigned id=ids[selected];
    if(text_field(id)>=0){editor(id);return;}
    if(id==0)sq_set_reverse(&model,!model.reverse);
    else if(id==1)sq_set_bits(&model,model.bits==3?2:3);
    else if(id==2)sq_set_controlled(&model,!model.controlled);
    else if(id==3)sq_set_edge(&model,model.active_edge==FF_RISING?FF_FALLING:FF_RISING);
    else if(id==4)sq_set_policy(&model,(sq_policy)((model.policy+1)%3));
    else if(id==5)sq_set_recovery(&model,(model.recovery+1)%(1u<<model.bits));
    else if(id==6)sq_set_initial(&model,(model.initial+1)%(1u<<model.bits));
    else if(id==7)sq_set_fixed_e(&model,model.fixed_e^1);
    else sq_set_type(&model,10-id,(ff_type)((model.types[10-id]+1)%4));
    page=group=0;
}
static void state_table(bool outputs)
{
    unsigned first=page*6;if(first>=model.count){page=0;first=0;}
    dtext(10,47,ACCENT,outputs?"E  Q       Z0 Z1 (present state / pre-edge)":"E  Q     Required  Actual   Source");
    for(unsigned i=first;i<first+6 && i<model.count;i++) {
        sq_row const *r=&model.rows[i];char q[4],target[4],actual[4];binary(r->q,q);binary(r->target,target);binary(r->next[form],actual);
        int y=70+(int)(i-first)*19;
        if(outputs){int a=sq_output_value(&model,0,i),b=sq_output_value(&model,1,i);dprint(10,y,C_BLACK,"%u  %s      %c  %c",r->e,q,a<0?'-':(char)('0'+a),b<0?'-':(char)('0'+b));}
        else dprint(10,y,r->next[form]==SQ_UNDEFINED?C_RED:C_BLACK,"%u  %s      %-3s     %-3s   %s",r->e,q,model.reverse?target:r->care?target:"DC",actual,r->used?"given":"unused");
    }
    if(outputs)dprint(10,188,C_DARK,"Page %u/%u | - = not specified | UP/DOWN",page+1,(model.count+5)/6);
    else dprint(10,188,C_DARK,"Page %u/%u | UP/DOWN | +/- form",page+1,(model.count+5)/6);
}
static void excitation(void)
{
    unsigned first=page*6;if(first>=model.count){page=0;first=0;}
    sq_pin const *p=&model.pins[pin];
    dprint(10,47,ACCENT,"%s: required input vs implemented %s",p->name,form==EB_SOP?"SOP":"POS");
    dtext(10,65,C_DARK,"E  Q     Q+    Required    Actual pin");
    for(unsigned i=first;i<first+6 && i<model.count;i++){
        char q[4],next[4];binary(model.rows[i].q,q);binary(model.rows[i].next[form],next);
        unsigned value=model.rows[i].excitation[pin];
        dprint(10,85+(int)(i-first)*16,C_BLACK,"%u  %s   %-3s      %c           %d",model.rows[i].e,q,next,model.rows[i].next[form]==SQ_UNDEFINED?'!':value==FF_X?'X':(char)('0'+value),sq_pin_value(&model,form,pin,i));
    }
    dprint(10,188,C_DARK,"Page %u/%u | L/R pin | X=any legal, !=invalid",page+1,(model.count+5)/6);
}
static void equations(void)
{
    char text[256],map[80];sq_format_equation(&model,form,pin,text,sizeof text);mapping(map);
    eb_result const *r=&model.pins[pin].solution[form];
    unsigned pages=(unsigned)(strlen(text)+46*5-1)/(46*5);if(!pages)pages=1;page%=pages;
    dprint(10,46,ACCENT,"%s | %u groups, %u literals | %s",model.pins[pin].name,(unsigned)r->count,(unsigned)r->literals,form==EB_SOP?"SOP":"POS");
    wrapped(map,65,0,1,C_DARK);wrapped(text,87,page*5,5,C_BLACK);
    dprint(10,173,ACCENT,"Verified: %u form-row comparisons",model.checked_rows);
    dprint(10,190,C_DARK,"Pin %u/%u Page %u/%u | +/- swaps form",pin+1,model.pin_count,page+1,pages);
}
static void map(void)
{
    const sq_pin *p=&model.pins[pin];const eb_result *r=&p->solution[form];
    unsigned rows=model.variables==4?4:2,cols=model.variables==2?2:4;
    if(group>=r->count)group=0;
    char legend[80];mapping(legend);wrapped(legend,43,0,1,C_DARK);
    dprint(10,63,ACCENT,"%s %s group %u/%u",p->name,form==EB_SOP?"SOP":"POS",r->count?group+1:0,(unsigned)r->count);
    if(map_details){
        if(!r->count){dtext(10,94,C_DARK,"Constant / empty cover, no groups.");return;}
        eb_result one={.variables=(uint8_t)model.variables,.count=1,.form=form};one.cubes[0]=r->cubes[group];
        for(unsigned bits=one.cubes[0].fixed;bits;bits>>=1)one.literals+=bits&1u;
        char term[80];eb_format(&one,term,sizeof term);dprint(10,85,ORANGE,"Term/clause: %s",term);
        char covered[80]="",dc[80]="";size_t n=0,k=0;
        for(unsigned i=0;i<model.count;i++)if((i&r->cubes[group].fixed)==r->cubes[group].bits){
            n+=(size_t)snprintf(covered+n,sizeof covered-n,"%s%u",n?",":"",i);
            if(p->dc&(1u<<i))k+=(size_t)snprintf(dc+k,sizeof dc-k,"%s%u",k?",":"",i);
        }
        dtext(10,110,C_DARK,"Covered row numbers:");wrapped(covered,127,0,2,C_BLACK);
        dtext(10,161,C_DARK,"Excitation/unused don't-care rows:");wrapped(k?dc:"none",177,0,1,ORANGE);return;
    }
    static const unsigned gray[]={0,1,3,2};
    for(unsigned x=0;x<cols;x++){
        if(cols==2)dprint(207+(int)x*46,79,C_DARK,"%u",gray[x]);
        else dprint(199+(int)x*46,79,C_DARK,"%u%u",gray[x]>>1,gray[x]&1);
    }
    for(unsigned y=0;y<rows;y++){
        if(rows==2)dprint(177,98+(int)y*24,C_DARK,"%u",gray[y]);
        else dprint(164,98+(int)y*24,C_DARK,"%u%u",gray[y]>>1,gray[y]&1);
    }
    for(unsigned y=0;y<rows;y++)for(unsigned x=0;x<cols;x++){
        unsigned i=bs_kmap_row(model.variables,y,x);int px=190+(int)x*46,py=92+(int)y*24;
        bool in=r->count && (i&r->cubes[group].fixed)==r->cubes[group].bits;
        drect_border(px,py,px+44,py+22,in?HIGHLIGHT:C_WHITE,in?2:1,in?ORANGE:C_LIGHT);
        dtext(px+17,py+2,(p->dc&(1u<<i))?ORANGE:ACCENT,(p->dc&(1u<<i))?"X":(p->on&(1u<<i))?"1":"0");
        dprint(px+3,py+13,C_DARK,"%u",i);
    }
    dtext(10,97,C_DARK,model.variables==4?"AB / CD":model.variables==3?"A / BC":"A / B");
    dtext(10,117,C_DARK,"Orange = group");
    dtext(10,137,C_DARK,"UP/DOWN group");
    dtext(10,157,C_DARK,"LEFT/RIGHT pin");
    dtext(10,190,C_DARK,"EXE details | +/- form | row numbers shown");
}
static void trace(void)
{
    sq_trace_result result;
    const char *timeline=model.controlled?model.timeline:NULL;
    if(!sq_trace(&model,form,model.initial,model.fixed_e,timeline,&result))return;
    unsigned count=result.steps+1,pages=(count+5)/6;page%=pages;
    dprint(10,46,ACCENT,"Start Q=%u | %s",model.initial,timeline&&timeline[0]?"E timeline":"Fixed E traversal");
    for(unsigned i=page*6;i<count && i<page*6+6;i++){
        char q[4];binary(result.states[i],q);
        if(i<result.steps){
            if(model.controlled)dprint(10,68+(int)(i%6)*18,C_BLACK,"%2u: Q=%s (%u)  -- E=%u -->",i,q,result.states[i],result.controls[i]);
            else dprint(10,68+(int)(i%6)*18,C_BLACK,"%2u: Q=%s (%u)  -- CLK -->",i,q,result.states[i]);
        }
        else dprint(10,68+(int)(i%6)*18,result.invalid?C_RED:C_BLACK,"%2u: Q=%s (%u) %s",i,q,result.states[i],result.invalid?"-> undefined":"");
    }
    if(result.cycle)dprint(10,184,ACCENT,"Prefix %u | Period %u | Page %u/%u",result.prefix,result.period,page+1,pages);
    else dprint(10,184,result.invalid?C_RED:C_DARK,"%s | Page %u/%u",result.invalid?"Invalid SR: trace stops":"Timeline end (no cycle inferred)",page+1,pages);
}
static void recovery(void)
{
    unsigned total=(1u<<model.bits)*(model.controlled?2:1),pages=(total+5)/6;page%=pages;
    dtext(10,46,ACCENT,"All starts / fixed E: prefix,period,end");
    for(unsigned i=page*6;i<total && i<page*6+6;i++){
        unsigned q=i>>(model.controlled?1:0),e=model.controlled?i&1u:0;unsigned row=i;
        sq_trace_result t;sq_trace(&model,form,q,e,NULL,&t);
        unsigned end=t.states[t.steps];bool enters=false;
        for(unsigned k=0;k<=t.steps;k++)if(model.rows[(t.states[k]<<(model.controlled?1:0))|e].used)enters=true;
        dprint(10,68+(int)(i%6)*18,t.invalid?C_RED:C_BLACK,"E%u Q%u %s p%u/%u %s",e,q,model.rows[row].used?"given ":"unused",t.prefix,t.period,t.invalid?"INVALID":enters?"cycle entry":"TRAP");
        (void)end;
    }
    dprint(10,184,model.policy==SQ_DC?ORANGE:C_DARK,"%s | Page %u/%u",model.policy==SQ_DC?"DC: completion, no recovery guarantee":"p = transient length / cycle period",page+1,pages);
}
static void connections(void)
{
    unsigned count=cn_line_count(&connections_net),pages=(count+6)/7;if(!pages)pages=1;page%=pages;
    dprint(10,46,ACCENT,"Whole circuit | Page %u/%u",page+1,pages);
    for(unsigned i=0;i<7;i++){
        char line[80];if(!cn_line(&connections_net,page*7+i,line,sizeof line))break;
        dtext(10,68+(int)i*17,C_BLACK,line);
    }
    dtext(10,193,C_DARK,"UP/DOWN all wiring pages | +/- form");
}
static void draw(void)
{
    if(model.valid && pin>=model.pin_count)pin=0;
    dclear(C_WHITE);char title[48];snprintf(title,sizeof title,"ECEN21 - %s",titles[view]);info_title(title,ACCENT);
    dprint(10,26,C_DARK,"%ubit%s %s | %s %s %s",model.bits,model.controlled?"+E":"",model.reverse?"REVERSE":"DESIGN",sq_policy_name(model.policy),form==EB_SOP?"SOP":"POS",model.active_edge==FF_RISING?"RISE":"FALL");
    if(view==SETUP)setup();
    else if(!model.valid){dtext(10,80,C_RED,"No current result. Edit setup, then F6.");if(model.error[0])wrapped(model.error,110,0,2,C_RED);}
    else if(view==STATES)state_table(false);
    else if(view==EXCITE)excitation();
    else if(view==OUTPUTS)state_table(true);
    else if(view==EQUATIONS)equations();
    else if(view==MAP)map();
    else if(view==TRACE)trace();
    else if(view==RECOVERY)recovery();
    else connections();
    fkey_button(1,"SETUP");fkey_button(2,"TABLE");fkey_button(3,"EQS");
    fkey_button(4,"MAP");fkey_button(5,"MORE");fkey_button_color(6,"SOLVE",ACCENT);dupdate();
}
static void help(void)
{
    dclear(C_WHITE);info_title("Counter / sequential help",ACCENT);
    dtext(10,29,C_BLACK,"Setup EXE edits a field or cycles a setting.");
    dtext(10,48,C_DARK,"F2: state / excitation / output table");
    dtext(10,67,C_DARK,"F3 equations | F4 map / EXE group details");
    dtext(10,86,C_DARK,"F5: trace, connections, recovery, outputs");
    dtext(10,105,C_DARK,"LEFT/RIGHT pin | UP/DOWN page or group");
    dtext(10,124,C_DARK,"+/- switches SOP/POS completed circuits.");
    dtext(10,143,C_DARK,"DC completions can differ; no recovery claim.");
    dtext(10,162,C_DARK,"Shared CLK; all inputs use the same old Q.");
    dtext(10,181,C_DARK,"OPTN examples replace work; EXIT home.");
    dtext(10,198,ACCENT,"Any key returns.");dupdate();getkey();
}
static void more(void)
{
    dclear(C_WHITE);info_title("Sequential result views",ACCENT);
    dtext(10,45,C_BLACK,"1  State trace (initial / E timeline)");
    dtext(10,70,C_BLACK,"2  Circuit connections (selected pin)");
    dtext(10,95,C_BLACK,"3  Recovery / traps from every state");
    dtext(10,120,C_BLACK,"4  Present-state output table");
    dtext(10,145,C_BLACK,"5  Help / key reference");
    dtext(10,185,C_DARK,"Other keys return without changes.");dupdate();
    int n=key_digit(getkey().key);
    if(n>=1 && n<=4){
        static const unsigned result_views[]={TRACE,WIRES,RECOVERY,OUTPUTS};
        view=result_views[n-1];page=0;
    }
    else if(n==5)help();
}
static void load_sample(unsigned n)
{
    sq_init(&model);
    if(n==2){for(unsigned b=0;b<3;b++)sq_set_type(&model,b,FF_JK);sq_set_text(&model,0,"0147350");}
    if(n==3){sq_set_type(&model,2,FF_D);sq_set_type(&model,0,FF_JK);sq_set_text(&model,0,"031254760");}
    if(n==4){sq_set_bits(&model,2);sq_set_controlled(&model,true);sq_set_text(&model,0,"02310");sq_set_text(&model,1,"01320");}
    if(n==5){sq_set_bits(&model,2);sq_set_reverse(&model,true);sq_set_type(&model,0,FF_JK);sq_set_text(&model,4,"B+!A");sq_set_text(&model,2,"!B*A");sq_set_text(&model,3,"!B+A");sq_set_text(&model,8,"B");}
    if(n==6){sq_set_controlled(&model,true);sq_set_type(&model,2,FF_D);sq_set_type(&model,0,FF_JK);sq_set_text(&model,0,"03570");sq_set_text(&model,1,"014763250");sq_set_text(&model,10,"01011010");}
    if(n==7){sq_set_bits(&model,2);sq_set_reverse(&model,true);sq_set_type(&model,0,FF_D);sq_set_type(&model,1,FF_D);sq_set_text(&model,4,"B");sq_set_text(&model,2,"A");sq_set_initial(&model,1);}
    if(n==8){sq_set_bits(&model,2);sq_set_reverse(&model,true);sq_set_type(&model,1,FF_SR);sq_set_type(&model,0,FF_D);sq_set_text(&model,4,"1");sq_set_text(&model,5,"1");sq_set_text(&model,2,"0");}
    view=SETUP;selected=pin=page=group=0;form=EB_SOP;map_details=false;
}
static void samples(void)
{
    dclear(C_WHITE);info_title("Sequential examples",ACCENT);
    dtext(10,28,C_DARK,"Choose 1-8: explicitly replaces this problem");
    dtext(10,49,C_BLACK,"1  T: 0,3,5,7,0 (3 bits)");
    dtext(10,68,C_BLACK,"2  JK: 0,1,4,7,3,5,0 (3 bits)");
    dtext(10,87,C_BLACK,"3  Mixed D/T/JK: 0,3,1,2,5,4,7,6,0");
    dtext(10,106,C_BLACK,"4  Controlled T (E=0 / E=1), 2 bits");
    dtext(10,125,C_BLACK,"5  Reverse T/JK + Z (Review p33)");
    dtext(10,144,C_BLACK,"6  Three-bit + E / mixed / E timeline");
    dtext(10,163,C_BLACK,"7  Simultaneous D swap: 01 -> 10");
    dtext(10,182,C_BLACK,"8  Reverse invalid SR11");
    dtext(10,199,C_DARK,"Other keys cancel. F6 solves after loading.");dupdate();int n=key_digit(getkey().key);if(n>=1 && n<=8)load_sample((unsigned)n);
}
static void solve(void)
{
    dclear(C_WHITE);info_title("Solving sequential design...",ACCENT);dtext(10,70,C_DARK,"Exact per-pin SOP/POS and row verification");dupdate();
    if(sq_solve(&model)){
        if(cn_build(&model,form,&connections_net)){view=STATES;pin=page=group=0;return;}
        sq_invalidate(&model);snprintf(model.error,sizeof model.error,"Connection graph build failed");
    }
    unsigned ids[24],n=fields(ids),want=model.error_field<2?11+model.error_field:model.error_field<8?12+model.error_field:model.error_field<10?12+model.error_field:13;
    for(unsigned i=0;i<n;i++)if(ids[i]==want){selected=i;break;}
    view=SETUP;
    dclear(C_WHITE);info_title("Sequential input error",ACCENT);
    dprint(10,48,C_RED,"Field %u, character %u",model.error_field,model.error_pos+1);wrapped(model.error,78,0,3,C_RED);
    dtext(10,157,C_DARK,"Old tables/equations are invalidated.");dtext(10,185,ACCENT,"Any key returns to setup for repair.");dupdate();getkey();
}
static unsigned page_count(void)
{
    if(view==STATES || view==EXCITE || view==OUTPUTS)return (model.count+5)/6;
    if(view==RECOVERY)return ((1u<<model.bits)*(model.controlled?2:1)+5)/6;
    if(view==WIRES)return (cn_line_count(&connections_net)+6)/7;
    if(view==EQUATIONS){char text[256];sq_format_equation(&model,form,pin,text,sizeof text);unsigned n=(unsigned)(strlen(text)+229)/230;return n?n:1;}
    if(view==TRACE){sq_trace_result t;sq_trace(&model,form,model.initial,model.fixed_e,model.controlled?model.timeline:NULL,&t);return (t.steps+6)/6;}
    return 1;
}
void sequential_run(void)
{
    static bool initialized;if(!initialized){load_sample(1);initialized=true;}
    for(;;){
        draw();int key=getkey().key;if(key==KEY_EXIT)return;
        if(key==INFO_KEY){samples();continue;}
        if(key==KEY_F1){view=SETUP;page=0;continue;}
        if(key==KEY_F2){view=view==STATES?EXCITE:view==EXCITE?OUTPUTS:STATES;page=0;continue;}
        if(key==KEY_F3){view=EQUATIONS;page=0;continue;}
        if(key==KEY_F4){view=MAP;page=group=0;map_details=false;continue;}
        if(key==KEY_F5){more();continue;}
        if(key==KEY_F6){solve();continue;}
        if(view==SETUP){
            unsigned ids[24],n=fields(ids);
            if(key==KEY_UP)selected=(selected+n-1)%n;
            if(key==KEY_DOWN)selected=(selected+1)%n;
            if(key==KEY_EXE)edit_setting();
            continue;
        }
        if(!model.valid)continue;
        if(key==KEY_ADD || key==KEY_SUB){form=form==EB_SOP?EB_POS:EB_SOP;cn_build(&model,form,&connections_net);page=group=0;continue;}
        if(key==KEY_LEFT || key==KEY_RIGHT){pin=key==KEY_LEFT?(pin+model.pin_count-1)%model.pin_count:(pin+1)%model.pin_count;page=group=0;continue;}
        if(view==MAP){unsigned count=model.pins[pin].solution[form].count;if(count && key==KEY_DOWN)group=(group+1)%count;if(count && key==KEY_UP)group=(group+count-1)%count;if(key==KEY_EXE)map_details=!map_details;}
        else if(key==KEY_DOWN)page=(page+1)%page_count();
        else if(key==KEY_UP)page=(page+page_count()-1)%page_count();
    }
}
