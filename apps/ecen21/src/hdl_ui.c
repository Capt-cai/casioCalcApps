#include "hdl.h"
#include "hdl_storage.h"
#include "hdl_ui.h"
#include "info.h"
#include "keys.h"
#include "utils.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>
#define ACCENT C_RGB(2,12,17)
#define HI C_RGB(26,29,31)
enum { SET,CODE,PORTS,EXPORT };
enum { WIDTH,INPUTS,MODULUS,SEED,DIRECTION,EDGE,RESET,POLARITY,VALUE,SLOT };
static hg_config config;
static hg_document document;
static unsigned view,field,page,export_return;
static bool export_attempted;
static hx_result export_result;
static char notice[64];
static bool sequential(void) { return config.kind>=HG_D; }
static void dirty(void)
{ document.valid=false;document.length=0;document.code[0]=0;document.error[0]=0;page=0;notice[0]=0;export_attempted=false; }
static unsigned properties(unsigned out[10])
{
    unsigned n=0;
    if(config.kind<=HG_STRUCTURAL||config.kind>=HG_SHIFT)out[n++]=WIDTH;
    if(config.kind==HG_ENCODER)out[n++]=INPUTS;
    if(config.kind==HG_MODULO)out[n++]=MODULUS;
    if(config.kind==HG_RING){out[n++]=SEED;out[n++]=DIRECTION;}
    if(sequential()){out[n++]=EDGE;out[n++]=RESET;out[n++]=POLARITY;}
    if(config.kind>=HG_D&&config.kind<=HG_JK)out[n++]=VALUE;
    out[n++]=SLOT;return n;
}
static const char *reset_name(void)
{ return config.reset==HG_NO_RESET?"NONE":config.reset==HG_SYNC_RESET?"SYNC":"ASYNC"; }
static void setting(unsigned p,char *out,size_t cap)
{
    switch(p) {
    case WIDTH:snprintf(out,cap,"Width: %u (1..32)",config.width);break;
    case INPUTS:snprintf(out,cap,"Ordinary encoder: %u inputs",config.inputs);break;
    case MODULUS:snprintf(out,cap,"Modulus: %u (min width%u)",config.modulus,hg_min_width(config.modulus));break;
    case SEED:snprintf(out,cap,"One-hot seed bit: %u (LSB=0)",config.seed);break;
    case DIRECTION:snprintf(out,cap,"Rotate: %s",config.right?"RIGHT toward LSB":"LEFT toward MSB");break;
    case EDGE:snprintf(out,cap,"Clock: %s edge",config.falling?"falling":"rising");break;
    case RESET:snprintf(out,cap,"Reset: %s%s",reset_name(),config.kind==HG_MODULO?" (required)":"");break;
    case POLARITY:snprintf(out,cap,"Reset active: %s",config.reset_high?"HIGH":"LOW");break;
    case VALUE:snprintf(out,cap,"Reset value: %u",config.reset_value?1:0);break;
    default:snprintf(out,cap,"Export slot: %02u (0..99)",config.slot);break;
    }
}
static bool number(unsigned *value,unsigned min,unsigned max)
{
    char text[6]="";unsigned length=0;bool bad=false;
    for(;;) {
        dclear(C_WHITE);info_title("HDL parameter",ACCENT);
        dprint(10,44,C_DARK,"Current%u | allowed%u..%u",*value,min,max);
        dprint(10,77,C_BLACK,"New: %s_",text);
        dtext(10,113,C_DARK,"EXE apply | EXIT cancel | DEL / AC");
        if(bad)dtext(10,151,C_RED,"Outside range or too many digits.");
        dupdate();int key=getkey().key;
        if(key==KEY_EXIT)return false;
        if(key==KEY_ACON){length=0;text[0]=0;bad=false;continue;}
        if(key==KEY_DEL){if(length)text[--length]=0;bad=false;continue;}
        if(key==KEY_EXE) {
            unsigned v=0;for(unsigned i=0;i<length;i++)v=v*10+(unsigned)(text[i]-'0');
            if(length&&v>=min&&v<=max){*value=v;return true;}bad=true;continue;
        }
        int digit=key_digit(key);
        if(digit>=0){if(length<5){text[length++]=(char)('0'+digit);text[length]=0;bad=false;}else bad=true;}
    }
}
static void edit(void)
{
    unsigned fields[10],n=properties(fields);if(field>=n)field=0;
    unsigned p=fields[field];bool changed=true;
    if(p==WIDTH)changed=number(&config.width,1,32);
    if(p==INPUTS)config.inputs=config.inputs==4?8:4;
    if(p==MODULUS)changed=number(&config.modulus,1,65535);
    if(p==SEED)changed=number(&config.seed,0,31);
    if(p==DIRECTION)config.right=!config.right;
    if(p==EDGE)config.falling=!config.falling;
    if(p==RESET) {
        if(config.kind==HG_MODULO){snprintf(notice,sizeof notice,"Modulo counter requires synchronous reset.");return;}
        if(config.kind==HG_RING)config.reset=config.reset==HG_SYNC_RESET?HG_ASYNC_RESET:HG_SYNC_RESET;
        else config.reset=(hg_reset)((config.reset+1)%3);
    }
    if(p==POLARITY)config.reset_high=!config.reset_high;
    if(p==VALUE)config.reset_value=!config.reset_value;
    if(p==SLOT){if(number(&config.slot,0,99)){notice[0]=0;export_attempted=false;}return;}
    if(changed)dirty();
}
static void setup(void)
{
    unsigned fields[10],n=properties(fields),first=field/6*6;char text[64];
    for(unsigned i=first;i<n&&i<first+6;i++) {
        int y=46+(int)(i-first)*22;if(i==field)drect(7,y-2,387,y+16,HI);
        setting(fields[i],text,sizeof text);dtext(10,y,C_BLACK,text);
    }
    dprint(10,182,ACCENT,"Setting%u/%u | arrows,EXE edit",field+1,n);
    dtext(10,198,C_DARK,"F6 generates; new parameters clear old code.");
}
static void code(void)
{
    if(!document.valid){dtext(10,68,C_RED,"No current code. Press F6 BUILD.");return;}
    char line[47];unsigned count=hg_pages_lines(&document),pages=(count+6)/7;
    if(page>=pages)page=0;
    for(unsigned i=0;i<7&&page*7+i<count;i++){hg_line(&document,page*7+i,line,sizeof line);dtext(10,46+(int)i*20,C_BLACK,line);}
    dprint(10,191,C_DARK,"Page%u/%u | %ubytes | arrows",page+1,pages,document.length);
}
static void ports(void)
{
    dprint(10,45,ACCENT,"module %s",hg_module(config.kind));
    if(config.kind<=HG_STRUCTURAL) {
        dtext(10,72,C_BLACK,"a,b: WIDTH-bit inputs; cin: carry input");
        dtext(10,96,C_BLACK,"sum: WIDTH bits; cout: final carry");
        dtext(10,120,C_DARK,"Bit0=LSB; structural file includes FA cell.");
        dtext(10,147,C_DARK,"WIDTH parameter can be overridden1..32.");
    }
    else if(config.kind==HG_ENCODER) {
        dtext(10,72,C_BLACK,"in: I[n-1]..I0; code: OR of active indices");
        dtext(10,96,C_BLACK,"valid: exactly one active input");
        dtext(10,120,C_RED,"Use code only when valid=1; no priority.");
        dtext(10,147,C_DARK,"INPUTS parameter4or8; actual invalid code shown.");
    }
    else {
        dprint(10,70,C_BLACK,"clk:%s | rst:%s %s",config.falling?"falling":"rising",reset_name(),config.reset_high?"high":"low");
        if(config.kind==HG_D)dtext(10,96,C_BLACK,"d input -> q on selected edge");
        if(config.kind==HG_T)dtext(10,96,C_BLACK,"t=0 hold; t=1 toggles q");
        if(config.kind==HG_JK)dtext(10,96,C_BLACK,"jk00 hold,01 clear,10 set,11 toggle");
        if(config.kind==HG_SHIFT){dtext(10,96,C_BLACK,"Reset > load > shift_right > shift_left > hold");dtext(10,119,C_BLACK,"Right inserts serial_left at MSB; left at LSB.");}
        if(config.kind==HG_MODULO){dtext(10,96,C_BLACK,"enable=0 hold; q wraps/recover at MODULUS");dtext(10,119,C_DARK,"WIDTH auto+EXTRA_BITS; selected width retained.");}
        if(config.kind==HG_RING){dtext(10,96,C_BLACK,"enable rotates one-hot q; reset selects seed.");dtext(10,119,C_DARK,"SEED_INDEX0=LSB; reset required to initialize.");}
        dtext(10,147,C_DARK,"q has no assumed power-up value.");
    }
    dtext(10,178,C_DARK,"Generated source; no Verilog compiler here.");
    dtext(10,198,C_DARK,"Only include one source per module family.");
}
static void export_page(void)
{
    char name[16];hg_filename(&config,name,sizeof name);
    if(!document.valid){dtext(10,65,C_RED,"Generate valid code before export.");return;}
    dprint(10,47,ACCENT,"USB disk root: %s",name);
    dprint(10,73,C_BLACK,"%ubytes | exact source, not wrapped display",document.length);
    dtext(10,99,C_DARK,"Existing files are never overwritten.");
    dtext(10,122,C_DARK,"Change export slot in SET to choose a new name.");
    if(export_attempted) {
        const char *message=hx_message(export_result.status);char a[47];snprintf(a,sizeof a,"%.46s",message);
        dtext(10,153,export_result.status==HX_OK?ACCENT:C_RED,a);
        if(export_result.status!=HX_OK)dprint(10,176,C_RED,"IO code%d | partial cleanup:%s",export_result.io_error,export_result.cleanup_failed?"FAILED":"OK");
    }
    else dtext(10,158,ACCENT,"F6 SAVE confirms export | EXIT cancels");
    dtext(10,198,C_DARK,"Then USB Flash copies .v to your computer.");
}
static void draw(void)
{
    dclear(C_WHITE);info_title("ECEN21 - Verilog generator",ACCENT);
    dprint(10,26,C_DARK,"%s | %s",hg_name(config.kind),document.valid?"generated":"edit / unsolved");
    if(view==SET)setup();else if(view==CODE)code();else if(view==PORTS)ports();else export_page();
    if(document.error[0]) {
        drect(7,172,389,208,C_WHITE);char line[47];snprintf(line,sizeof line,"%.46s",document.error);dtext(10,174,C_RED,line);
        if(strlen(document.error)>46)dtext(10,191,C_RED,document.error+46);
    }else if(notice[0])dtext(10,198,C_RED,notice);
    fkey_button(1,"SET");fkey_button(2,"CODE");fkey_button(3,"PORTS");fkey_button(4,"EXPORT");fkey_button(5,"TYPE");
    fkey_button_color(6,view==EXPORT&&document.valid?"SAVE":"BUILD",ACCENT);dupdate();
}
static void menu(void)
{
    dclear(C_WHITE);info_title("Verilog design family",ACCENT);
    for(unsigned i=0;i<=HG_RING;i++)dprint(10,27+(int)i*19,C_BLACK,"%u %s",i+1,hg_name((hg_kind)i));
    dtext(10,201,C_DARK,"0 help | other keys cancel; type replaces.");dupdate();int choice=key_digit(getkey().key);
    if(choice>=1&&choice<=9){hg_init(&config,(hg_kind)(choice-1));dirty();view=SET;field=0;}
    if(choice==0) {
        dclear(C_WHITE);info_title("Verilog help",ACCENT);
        dtext(10,38,C_BLACK,"SET: arrows select; EXE edit parameters.");
        dtext(10,61,C_BLACK,"BUILD generates code; CODE arrows page all.");
        dtext(10,84,C_BLACK,"EXPORT then F6 saves exact .v source bytes.");
        dtext(10,107,C_BLACK,"Slot00..99; existing files are protected.");
        dtext(10,130,C_BLACK,"Wrong parameters clear old code; fix/build.");
        dtext(10,153,C_BLACK,"No initial q assumed; reset/load first.");
        dtext(10,190,C_DARK,"Any key returns; EXIT tool -> home.");dupdate();getkey();
    }
}
void hdl_run(void)
{
    static bool initialized;if(!initialized){hg_init(&config,HG_BEHAVIORAL);initialized=true;}
    for(;;) {
        draw();int key=getkey().key;
        if(key==KEY_EXIT){if(view==EXPORT){view=export_return;export_attempted=false;continue;}return;}
        if(key==INFO_KEY||key==KEY_F5){menu();continue;}
        if(key==KEY_F1){view=SET;continue;}
        if(key==KEY_F2){view=CODE;continue;}
        if(key==KEY_F3){view=PORTS;continue;}
        if(key==KEY_F4){if(view!=EXPORT)export_return=view;view=EXPORT;export_attempted=false;continue;}
        if(key==KEY_F6) {
            if(view==EXPORT&&document.valid){export_result=hdl_storage_export(&config,&document);export_attempted=true;}
            else {if(hg_generate(&config,&document))view=CODE;else view=SET;page=0;}
            continue;
        }
        if(view==SET) {
            unsigned fields[10],n=properties(fields);
            if(key==KEY_UP)field=(field+n-1)%n;
            if(key==KEY_DOWN)field=(field+1)%n;
            if(key==KEY_EXE)edit();
        }
        if(view==CODE&&document.valid) {
            unsigned pages=(hg_pages_lines(&document)+6)/7;
            if(key==KEY_DOWN||key==KEY_RIGHT)page=(page+1)%pages;
            if(key==KEY_UP||key==KEY_LEFT)page=(page+pages-1)%pages;
        }
    }
}
