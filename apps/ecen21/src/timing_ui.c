#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>
#include "timing.h"
#include "timing_samples.h"
#include "timing_ui.h"
#include "info.h"
#include "keys.h"
#include "utils.h"
#define ACCENT C_RGB(2,12,17)
#define HIGHLIGHT C_RGB(26,29,31)
static tm_model model;
static tm_phase_log phase_log;
static unsigned view,component,field,signal_index,column,event_index,input_index,phase,sample;
static char notice[80];
enum { CONFIG,EVENTS,WAVE,TABLE,DETAIL,PHASE };
static char value_char(unsigned v){return v==TM_X?'X':(char)('0'+v);}
static bool active_signal(unsigned s)
{
    return s<4 || (s<8?model.cells[s-4].enabled:model.gates[s-8].enabled);
}
static char display_value(unsigned s,unsigned value)
{
    return active_signal(s)?value_char(value):'-';
}
static void wrap(const char *text,int y,int color)
{
    for(unsigned line=0;line<3;line++){
        size_t off=line*46,n=strlen(text);if(off>=n)break;n-=off;if(n>46)n=46;
        char part[47];memcpy(part,text+off,n);part[n]='\0';dtext(10,y+(int)line*15,color,part);
    }
}
static void dirty(void){sample=10;phase_log.count=0;notice[0]='\0';}
static unsigned field_count(void){return component==0?4:component<=4?12:4;}
static void field_text(unsigned f,char text[80])
{
    if(!component){snprintf(text,80,"Initial I%u: %u",f,(model.initial_inputs>>f)&1u);return;}
    if(component<=4){
        tm_cell const *c=&model.cells[component-1];
        switch(f){
        case 0:snprintf(text,80,"Enabled: %s",c->enabled?"YES":"NO");break;
        case 1:snprintf(text,80,"Type: %s",tm_type_name(c->type));break;
        case 2:snprintf(text,80,"Initial Q: %c",value_char(c->initial));break;
        case 3:snprintf(text,80,"Input A (D/T/J/S): %s",tm_signal_name(c->a));break;
        case 4:snprintf(text,80,"Input B (K/R): %s",tm_signal_name(c->b));break;
        case 5:snprintf(text,80,"Clock / enable: %s",tm_signal_name(c->clock));break;
        case 6:snprintf(text,80,"FF active edge: %s",c->edge==FF_RISING?"RISE":"FALL");break;
        case 7:snprintf(text,80,"Latch transparent: %s",c->high?"HIGH":"LOW");break;
        case 8:snprintf(text,80,"Reset mode: %s",c->reset_mode==TM_RESET_NONE?"NONE":c->reset_mode==TM_RESET_SYNC?"SYNC":"ASYNC");break;
        case 9:snprintf(text,80,"Reset source: %s",tm_signal_name(c->reset));break;
        case 10:snprintf(text,80,"Reset active: %s",c->reset_high?"HIGH":"LOW");break;
        default:snprintf(text,80,"Reset Q value: %u",c->reset_value);break;
        }
    }
    else{
        tm_gate const *g=&model.gates[component-5];
        if(f==0)snprintf(text,80,"Enabled: %s",g->enabled?"YES":"NO");
        if(f==1)snprintf(text,80,"Gate: %s",tm_gate_name(g->op));
        if(f==2)snprintf(text,80,"Input A: %s",tm_signal_name(g->a));
        if(f==3)snprintf(text,80,"Input B: %s",tm_signal_name(g->b));
    }
}
static void config(void)
{
    unsigned n=field_count();field%=n;
    if(component==0)dtext(10,46,ACCENT,"External initial levels (before column0)");
    else dprint(10,46,ACCENT,"%s configuration",tm_signal_name(component+3));
    for(unsigned i=field/6*6;i<n && i<field/6*6+6;i++){
        int y=68+(int)(i%6)*19;char text[80];field_text(i,text);
        if(i==field){drect(7,y-2,386,y+13,HIGHLIGHT);}
        dtext(10,y,C_BLACK,text);
    }
    dtext(10,186,C_DARK,"LEFT/RIGHT component | UP/DOWN field");
    dtext(10,196,C_DARK,"EXE cycles value | F6 run | OPTN examples");
}
static void edit_field(void)
{
    if(!component)tm_set_initial_inputs(&model,model.initial_inputs^(1u<<field));
    else if(component<=4){
        tm_cell c=model.cells[component-1];
        switch(field){
        case 0:c.enabled=!c.enabled;break;case 1:c.type=(tm_type)((c.type+1)%6);break;
        case 2:c.initial=(c.initial+1)%3;break;case 3:c.a=(c.a+1)%14;break;
        case 4:c.b=(c.b+1)%14;break;case 5:c.clock=(c.clock+1)%14;break;
        case 6:c.edge=c.edge==FF_RISING?FF_FALLING:FF_RISING;break;
        case 7:c.high=!c.high;break;case 8:c.reset_mode=(tm_reset_mode)((c.reset_mode+1)%3);break;
        case 9:c.reset=(c.reset+1)%14;break;case 10:c.reset_high=!c.reset_high;break;
        default:c.reset_value^=1;break;
        }
        tm_set_cell(&model,component-1,&c);
    }
    else{
        tm_gate g=model.gates[component-5];
        if(field==0)g.enabled=!g.enabled;
        if(field==1)g.op=(tm_gate_op)((g.op+1)%5);
        if(field==2)g.a=(g.a+1)%14;
        if(field==3)g.b=(g.b+1)%14;
        tm_set_gate(&model,component-5,&g);
    }
    dirty();
}
static void event_table(void)
{
    if(model.event_count && event_index>=model.event_count)event_index=model.event_count-1;
    unsigned first=event_index/6*6;
    dprint(10,46,ACCENT,"%u/64 events | . holds previous input",model.event_count);
    dtext(10,65,C_DARK,"Event       I0     I1     I2     I3");
    for(unsigned i=first;i<first+6 && i<model.event_count;i++){
        int y=85+(int)(i-first)*16;dprint(10,y,C_BLACK,"%2u",i+1);
        for(unsigned b=0;b<4;b++){
            int x=105+(int)b*58;
            if(i==event_index && b==input_index)drect(x-5,y-2,x+28,y+13,HIGHLIGHT);
            dprint(x,y,C_BLACK,"%c",(model.events[i].mask&(1u<<b))?value_char((model.events[i].values>>b)&1u):'.');
        }
    }
    if(!model.event_count)dtext(10,105,C_DARK,"Empty timeline. F5 adds the first event.");
    if(notice[0])dtext(10,186,C_RED,notice);
    else dtext(10,186,C_DARK,"Arrows select | 0/1 set,2 hold | EXE cycle");
    dtext(10,196,C_DARK,"F5 append | DEL delete | AC clear | F6 run");
}
static void event_edit(int key)
{
    if(key==KEY_F5){
        if(model.event_count==64){strcpy(notice,"Event limit64 reached; no data changed.");return;}
        event_index=model.event_count;tm_set_event_count(&model,model.event_count+1);
        tm_set_event(&model,event_index,0,0);dirty();return;
    }
    if(key==KEY_ACON){tm_set_event_count(&model,0);event_index=0;dirty();return;}
    if(!model.event_count)return;
    if(key==KEY_DEL){
        for(unsigned i=event_index;i+1<model.event_count;i++)model.events[i]=model.events[i+1];
        tm_set_event_count(&model,model.event_count-1);if(event_index && event_index>=model.event_count)event_index--;dirty();return;
    }
    if(key==KEY_UP)event_index=(event_index+model.event_count-1)%model.event_count;
    if(key==KEY_DOWN)event_index=(event_index+1)%model.event_count;
    if(key==KEY_LEFT)input_index=(input_index+3)%4;
    if(key==KEY_RIGHT)input_index=(input_index+1)%4;
    if(key!=KEY_0 && key!=KEY_1 && key!=KEY_2 && key!=KEY_EXE)return;
    tm_event e=model.events[event_index];unsigned bit=1u<<input_index;
    unsigned value=!(e.mask&bit)?2:(e.values&bit)?1:0;
    if(key==KEY_EXE)value=value==2?0:value==0?1:2;
    else value=key==KEY_0?0:key==KEY_1?1:2;
    if(value==2)e.mask&=(uint8_t)~bit;
    else{e.mask|=(uint8_t)bit;if(value)e.values|=(uint8_t)bit;else e.values&=(uint8_t)~bit;}
    e.values&=e.mask;tm_set_event(&model,event_index,e.mask,e.values);dirty();
}
static void ruler(unsigned first)
{
    dtext(10,46,ACCENT,"Signal");
    if(first)dtext(70,46,ACCENT,"<");
    if(first+7<model.event_count)dtext(385,46,ACCENT,">");
    for(unsigned i=0;i<8 && first+i<=model.event_count;i++)dprint(85+(int)i*38,46,C_DARK,"%u",first+i);
}
static void waveform(bool table)
{
    unsigned first=column/8*8,top=signal_index/4*4;ruler(first);
    for(unsigned row=0;row<4;row++){
        unsigned s=top+row;int y=67+(int)row*29;
        if(s==signal_index)drect(7,y-4,386,y+23,HIGHLIGHT);
        dprint(10,y+6,C_BLACK,"%s%s",tm_signal_name(s),active_signal(s)?"":" off");
        for(unsigned i=0;i<8 && first+i<=model.event_count;i++){
            unsigned col=first+i,v=model.values[col][s];int x=82+(int)i*38;
            if(!active_signal(s)){
                if(table)dtext(x+10,y+6,C_DARK,"-");
                else for(unsigned dot=0;dot<35;dot+=6)dline(x+(int)dot,y+12,x+(int)dot+2,y+12,C_LIGHT);
            }
            else if(table)dprint(x+10,y+6,v==TM_X?C_RED:ACCENT,"%c",value_char(v));
            else if(v==TM_X){dline(x,y+3,x+35,y+20,C_RED);dline(x,y+20,x+35,y+3,C_RED);}
            else{
                int level=y+(v?3:20);dline(x,level,x+38,level,ACCENT);
                if(col && model.values[col-1][s]!=TM_X && model.values[col-1][s]!=v)dline(x,y+3,x,y+20,ACCENT);
            }
            if(col==column)drect_border(x-1,y-3,x+36,y+23,C_NONE,1,C_RGB(30,13,2));
        }
    }
    dprint(10,185,C_DARK,"Signal %s | Col%u=%c | Time page %u/%u",tm_signal_name(signal_index),column,display_value(signal_index,model.values[column][signal_index]),first/8+1,(model.event_count+8)/8);
    dtext(10,196,C_DARK,"Arrows scroll | EXE event details | X unknown");
}
static char trigger_mark(unsigned cell)
{
    const tm_cell *c=&model.cells[cell];
    return c->type==TM_LATCH || c->type==TM_SR_LATCH?(c->high?'H':'L'):(c->edge==FF_RISING?'^':'v');
}
static void detail(void)
{
    dprint(10,46,ACCENT,"Column%u | %s=%c | %u settling phases",column,tm_signal_name(signal_index),display_value(signal_index,model.values[column][signal_index]),(unsigned)model.phases[column]);
    if(column)dprint(10,65,C_DARK,"I0:%c>%c I1:%c>%c I2:%c>%c I3:%c>%c",
        value_char(model.values[column-1][0]),value_char(model.values[column][0]),
        value_char(model.values[column-1][1]),value_char(model.values[column][1]),
        value_char(model.values[column-1][2]),value_char(model.values[column][2]),
        value_char(model.values[column-1][3]),value_char(model.values[column][3]));
    else dtext(10,65,C_DARK,"Initialization: no edge invented.");
    for(unsigned c=0;c<4;c++){
        int y=88+(int)c*21;
        if(!model.cells[c].enabled){dprint(10,y,C_DARK,"Q%u: disabled",c);continue;}
        char before=column?value_char(model.values[column-1][4+c]):value_char(model.cells[c].initial);
        dprint(10,y,C_BLACK,"Q%u:%c>%c %s%c %s",c,before,value_char(model.values[column][4+c]),tm_signal_name(model.cells[c].clock),trigger_mark(c),tm_reason_name((tm_reason)model.reasons[column][c]));
    }
    dprint(10,177,C_DARK,"G0=%c G1=%c G2=%c G3=%c",display_value(8,model.values[column][8]),display_value(9,model.values[column][9]),display_value(10,model.values[column][10]),display_value(11,model.values[column][11]));
    dtext(10,196,ACCENT,"EXE phases | ^/v edge,H/L latch | L/R event");
}
static void phases(void)
{
    if(!phase_log.count || phase_log.column!=column){
        if(!tm_explain(&model,column,&phase_log)){dtext(10,80,C_RED,"Cannot replay this event.");return;}
        phase=0;
    }
    phase%=phase_log.count;
    dprint(10,46,ACCENT,"Column%u | Phase point %u/%u",column,phase+1,phase_log.count);
    dtext(10,64,C_DARK,phase==0?"Inputs applied / before state commits":"After simultaneous commit + gate settling");
    for(unsigned s=0;s<12;s++)dprint(10+(int)(s%4)*95,85+(int)(s/4)*19,C_BLACK,"%s=%c",tm_signal_name(s),display_value(s,phase_log.values[phase][s]));
    for(unsigned c=0;c<4;c++)if(model.cells[c].enabled)dprint(10,144+(int)c*14,C_DARK,"Q%u %s%c: %s",c,tm_signal_name(model.cells[c].clock),trigger_mark(c),tm_reason_name((tm_reason)phase_log.reasons[phase][c]));
    dtext(10,196,ACCENT,"UP/DOWN phase | F5 summary | F3 waves");
}
static void draw(void)
{
    dclear(C_WHITE);info_title("ECEN21 - Timing simulator",ACCENT);
    dtext(10,26,C_DARK,sample<10?tm_sample_name(sample):"Custom timing problem (ideal zero delay)");
    if(view==CONFIG)config();
    else if(view==EVENTS)event_table();
    else if(!model.valid){dtext(10,80,C_RED,"No current simulation. F6 validates/runs.");if(model.error[0])wrap(model.error,110,C_RED);}
    else if(view==WAVE || view==TABLE)waveform(view==TABLE);
    else if(view==DETAIL)detail();else phases();
    fkey_button(1,"SETUP");fkey_button(2,"EVENT");fkey_button(3,"WAVE");
    fkey_button(4,"TABLE");fkey_button(5,view==EVENTS?"APPEND":"DETAIL");fkey_button_color(6,"RUN",ACCENT);dupdate();
}
static void help(void)
{
    dclear(C_WHITE);info_title("Ideal timing model / controls",ACCENT);
    dtext(10,28,C_BLACK,"SETUP: L/R component; EXE cycles field.");
    dtext(10,47,C_DARK,"I0..I3 external; Q0..Q3 cells; G0..G3 gates");
    dtext(10,66,C_DARK,"Columns=events; off=-; constants0/1.");
    dtext(10,85,C_DARK,"EVENT: . hold;0/1 value;2 restores hold.");
    dtext(10,104,C_DARK,"Same batch: inputs update before sampling.");
    dtext(10,123,C_DARK,"FFs sample together; latches follow level.");
    dtext(10,142,C_DARK,"SYNC reset waits; ASYNC acts between edges.");
    dtext(10,161,C_DARK,"X unknown; derived clocks settle in phases.");
    dtext(10,180,C_DARK,"F3 waves; EXE event/phase; EXIT home.");
    dtext(10,199,ACCENT,"Any key returns; no physical delay model.");dupdate();getkey();
}
static void examples(void)
{
    dclear(C_WHITE);info_title("Timing examples",ACCENT);
    for(unsigned n=0;n<10;n++)dprint(10,29+(int)n*16,C_BLACK,"%u  %s",n,tm_sample_name(n));
    dtext(10,196,C_DARK,"Digit replaces problem; EXE help; EXIT cancel");dupdate();int key=getkey().key;
    if(key==KEY_EXE){help();return;}
    int n=key_digit(key);if(n<0)return;
    tm_sample(&model,(unsigned)n);sample=(unsigned)n;view=CONFIG;component=field=column=event_index=input_index=phase=0;signal_index=4;phase_log.count=0;notice[0]='\0';
}
static void run(void)
{
    if(tm_run(&model)){view=WAVE;column=0;phase_log.count=0;notice[0]='\0';return;}
    dclear(C_WHITE);info_title("Timing simulation error",ACCENT);
    dprint(10,46,C_RED,"Column%u component%u",model.error_column,model.error_cell);wrap(model.error,70,C_RED);
    dtext(10,140,C_DARK,"No stable waveform is available.");dtext(10,180,ACCENT,"Any key returns; edit settings or reload.");dupdate();getkey();view=CONFIG;phase_log.count=0;
}
void timing_run(void)
{
    static bool initialized;if(!initialized){tm_sample(&model,1);sample=1;signal_index=4;initialized=true;}
    for(;;){
        draw();int key=getkey().key;if(key==KEY_EXIT)return;
        if(key==INFO_KEY){examples();continue;}
        if(key==KEY_F1){view=CONFIG;continue;}if(key==KEY_F2){view=EVENTS;continue;}
        if(key==KEY_F3){view=WAVE;continue;}if(key==KEY_F4){view=TABLE;continue;}
        if(key==KEY_F6){run();continue;}
        if(view==EVENTS){event_edit(key);continue;}
        if(key==KEY_F5){view=DETAIL;continue;}
        if(view==CONFIG){
            if(key==KEY_LEFT || key==KEY_RIGHT){component=key==KEY_LEFT?(component+8)%9:(component+1)%9;field=0;}
            if(key==KEY_UP){field=(field+field_count()-1)%field_count();}
            if(key==KEY_DOWN){field=(field+1)%field_count();}
            if(key==KEY_EXE){edit_field();}
            continue;
        }
        if(!model.valid)continue;
        if(key==KEY_LEFT || key==KEY_RIGHT){column=key==KEY_LEFT?(column+model.event_count)%(model.event_count+1):(column+1)%(model.event_count+1);phase_log.count=0;}
        if(view==WAVE || view==TABLE){
            if(key==KEY_UP){signal_index=(signal_index+11)%12;}
            if(key==KEY_DOWN){signal_index=(signal_index+1)%12;}
            if(key==KEY_EXE)view=DETAIL;
        }
        else if(view==DETAIL && key==KEY_EXE){view=PHASE;phase_log.count=0;}
        else if(view==PHASE && phase_log.count){if(key==KEY_UP)phase=(phase+phase_log.count-1)%phase_log.count;if(key==KEY_DOWN)phase=(phase+1)%phase_log.count;}
    }
}
