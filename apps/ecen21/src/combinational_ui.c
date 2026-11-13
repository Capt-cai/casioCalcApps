#include "combinational.h"
#include "combinational_ui.h"
#include "info.h"
#include "keys.h"
#include "utils.h"
#include <gint/display.h>
#include <gint/keyboard.h>
#include <stdio.h>
#include <string.h>
#define ACCENT C_RGB(2,12,17)
#define HI C_RGB(26,29,31)
enum { SETUP,TABLE,EQUATIONS,KMAP,WIRES,MUX,VERIFY };
static cb_model model;
static unsigned view,field,row,page;
static char notice[64];
static void reset_pages(void) { row=page=field=0;notice[0]=0; }
static unsigned fields(void)
{
    if(model.kind==CB_RIPPLE)return 4;
    if(model.kind==CB_ENCODER)return 3;
    return 6+(model.kind==CB_FUNCTION?3:0)+(model.kind==CB_DISPLAY?2:0);
}
static void setting(unsigned index,char *text,size_t cap)
{
    if(model.kind==CB_RIPPLE) {
        static const char *const names[]={"Width (1..8)","Operand A","Operand B","Carry in"};
        unsigned v=index==0?model.width:index==1?model.a:index==2?model.b:model.cin;
        snprintf(text,cap,"%s: %u",names[index],v);return;
    }
    if(model.kind==CB_ENCODER) {
        if(index==0)snprintf(text,cap,"Inputs: %u (ordinary one-hot)",model.encoder_inputs);
        if(index==1)snprintf(text,cap,"Output: Y%u",model.output);
        if(index==2)snprintf(text,cap,"Inspect input pattern: %u",row);
        return;
    }
    if(model.kind==CB_FUNCTION) {
        if(index==0){snprintf(text,cap,"Source: %s",model.expression_source?"expression":"truth table 0/1/X");return;}
        if(index==1){snprintf(text,cap,"Variables: %u (A is MSB)",model.variables);return;}
        if(index==2){snprintf(text,cap,"Expression: EXE opens editor");return;}index-=3;
    }
    if(model.kind==CB_DISPLAY) {
        if(index==0){snprintf(text,cap,"Pins active: %s",model.active_low?"LOW":"HIGH");return;}
        if(index==1){static const char *const p[]={"zero pins","blank display","don't care"};snprintf(text,cap,"BCD10..15: %s",p[model.invalid_policy]);return;}index-=2;
    }
    if(index==0){char out[12];cb_output_name(&model,out,sizeof out);snprintf(text,cap,"Output: %s (EXE cycles)",out);}
    if(index==1)snprintf(text,cap,"Gate form: %s",model.form==EB_SOP?"SOP":"POS");
    if(index==2)snprintf(text,cap,"MUX: %u:1 (%u select bits)",1u<<model.select_count,model.select_count);
    if(index>=3){unsigned i=index-3;char name[8];cb_input_name(&model,model.select[i],name,sizeof name);snprintf(text,cap,"Select slot%u: %s %s",i,name,i<model.select_count?"(first is MSB)":"(unused)");}
}
static bool number(unsigned *value,unsigned max)
{
    char digits[4]="";unsigned length=0;bool bad=false;
    for(;;) {
        dclear(C_WHITE);info_title("Enter whole number",ACCENT);
        dprint(10,45,C_DARK,"Current %u | allowed 0..%u",*value,max);
        dprint(10,78,C_BLACK,"New: %s_",digits);
        dtext(10,115,C_DARK,"EXE applies | EXIT cancels | AC clears");
        if(bad)dtext(10,150,C_RED,"Invalid range or input capacity.");
        dupdate();int key=getkey().key;
        if(key==KEY_EXIT)return false;
        if(key==KEY_ACON){length=0;digits[0]=0;bad=false;continue;}
        if(key==KEY_DEL){if(length)digits[--length]=0;bad=false;continue;}
        if(key==KEY_EXE){unsigned v=0;for(unsigned i=0;i<length;i++)v=v*10+(unsigned)(digits[i]-'0');if(length&&v<=max){*value=v;return true;}bad=true;continue;}
        int d=key_digit(key);if(d>=0){if(length<3){digits[length++]=(char)('0'+d);digits[length]=0;bad=false;}else bad=true;}
    }
}
static char expression_key(int key)
{
    switch(key){case KEY_XOT:return 'A';case KEY_LOG:return 'B';case KEY_LN:return 'C';case KEY_SIN:return 'D';
    case KEY_0:return '0';case KEY_1:return '1';case KEY_ADD:return '+';case KEY_MUL:return '*';case KEY_POWER:return '^';
    case KEY_DIV:return '!';case KEY_DOT:return '\'';case KEY_LEFTP:return '(';case KEY_RIGHTP:return ')';default:return 0;}
}
static bool expression_edit(void)
{
    char text[80];snprintf(text,sizeof text,"%s",model.expression);bool full=false;
    for(;;) {
        dclear(C_WHITE);info_title("Combinational expression",ACCENT);
        for(unsigned i=0;i<2;i++){char line[47]="";if(i*46<strlen(text))snprintf(line,sizeof line,"%.46s",text+i*46);dtext(10,45+(int)i*18,C_BLACK,line);}
        dtext(10,100,C_DARK,"A:x/theta B:log C:ln D:sin | 0/1");
        dtext(10,122,C_DARK,"+ OR | multiply AND | power XOR | / NOT");
        dtext(10,144,C_DARK,"EXE applies | EXIT cancels | DEL / AC");
        if(full)dtext(10,175,C_RED,"Maximum79 characters; delete first.");
        dupdate();int key=getkey().key;size_t n=strlen(text);
        if(key==KEY_EXIT)return false;
        if(key==KEY_EXE){snprintf(model.expression,sizeof model.expression,"%s",text);model.expression_source=true;return true;}
        if(key==KEY_ACON){text[0]=0;full=false;continue;}
        if(key==KEY_DEL){if(n)text[n-1]=0;full=false;continue;}
        char c=expression_key(key);if(c){if(n+1<sizeof text){text[n]=c;text[n+1]=0;}else full=true;}
    }
}
static void edit_setting(void)
{
    bool changed=true;unsigned i=field;
    if(model.kind==CB_RIPPLE) {
        if(i==0){unsigned w=model.width;changed=number(&w,8);if(changed&&w==0){snprintf(notice,sizeof notice,"Width must be1..8; unchanged.");return;}if(changed)model.width=w;}
        else if(i==1)changed=number(&model.a,255);
        else if(i==2)changed=number(&model.b,255);
        else model.cin^=1;
    }
    else if(model.kind==CB_ENCODER) {
        if(i==0){model.encoder_inputs=model.encoder_inputs==4?8:4;model.output=0;row=0;}
        if(i==1)model.output=(model.output+1)%cb_outputs(&model);
        if(i==2){number(&row,(1u<<model.encoder_inputs)-1);return;}
    }
    else {
        if(model.kind==CB_FUNCTION) {
            if(i==0){model.expression_source=!model.expression_source;goto done;}
            if(i==1){model.variables=model.variables==4?2:model.variables+1;model.on=model.dc=0;row=0;goto done;}
            if(i==2){changed=expression_edit();goto done;}i-=3;
        }
        if(model.kind==CB_DISPLAY) {
            if(i==0){model.active_low=!model.active_low;goto done;}
            if(i==1){model.invalid_policy=(model.invalid_policy+1)%3;goto done;}i-=2;
        }
        if(i==0)model.output=(model.output+1)%cb_outputs(&model);
        if(i==1)model.form=model.form==EB_SOP?EB_POS:EB_SOP;
        if(i==2)model.select_count=model.select_count==3?1:model.select_count+1;
        if(i>=3)model.select[i-3]=(model.select[i-3]+1)%cb_inputs(&model);
    }
done:
    if(changed){cb_dirty(&model);page=0;notice[0]=0;}
}
static void draw_setup(void)
{
    unsigned first=field/6*6;char text[64];
    for(unsigned i=first;i<fields()&&i<first+6;i++) {
        int y=46+(int)(i-first)*22;
        if(i==field)drect(7,y-2,387,y+16,HI);
        setting(i,text,sizeof text);dtext(10,y,C_BLACK,text);
    }
    dprint(10,183,ACCENT,"Setting%u/%u | UP/DOWN,EXE edit",field+1,fields());
}
static void bittext(unsigned value,unsigned n,char text[10])
{ for(unsigned i=0;i<n;i++)text[i]=(char)('0'+((value>>(n-1-i))&1u));text[n]=0; }
static void draw_table(void)
{
    if(model.kind==CB_RIPPLE) {
        cb_stage stages[8];unsigned sum,cout;
        if(!model.valid||!cb_ripple(model.width,model.a,model.b,model.cin,stages,&sum,&cout)){dtext(10,65,C_RED,"RUN first; operands must fit width.");return;}
        dprint(10,46,ACCENT,"%u + %u + %u = %u | Cout=%u",model.a,model.b,model.cin,sum,cout);
        dtext(10,68,C_DARK,"FA i   x   y   Cin   Sum   Cout");
        unsigned first=page/5*5;
        for(unsigned i=first;i<model.width&&i<first+5;i++)dprint(10,91+(int)(i-first)*18,C_BLACK," %u     %u   %u    %u     %u     %u",i,stages[i].x,stages[i].y,stages[i].cin,stages[i].sum,stages[i].cout);
        dtext(10,187,C_DARK,"LSB first; next Cin=previous Cout");return;
    }
    if(model.kind==CB_FUNCTION&&model.expression_source&&!model.valid){dtext(10,65,C_RED,"Expression not compiled. Press RUN first.");return;}
    unsigned n=cb_inputs(&model),rows=1u<<n,first=row/6*6;char names[64]="",name[8];
    for(unsigned i=0;i<n;i++){cb_input_name(&model,i,name,sizeof name);size_t used=strlen(names);snprintf(names+used,sizeof names-used,"%s%s",i?",":"",name);}
    dprint(10,46,ACCENT,"Inputs MSB first: %s",names);
    if(model.kind==CB_ENCODER)dtext(10,65,C_DARK,"Row  Inputs     Code  Valid  Status");
    else dtext(10,65,C_DARK,"Row   Inputs    Required  Gate  MUX");
    for(unsigned i=first;i<rows&&i<first+6;i++) {
        int y=84+(int)(i-first)*17;char bits[10];bittext(i,n,bits);
        if(i==row)drect(7,y-1,387,y+14,HI);
        if(model.kind==CB_ENCODER){bool valid;unsigned code=cb_encoder_code(i,n,&valid);dprint(10,y,C_BLACK,"%3u   %s   %u   %u   %s",i,bits,code,valid?1:0,valid?"one-hot":i?"MULTI":"ZERO");}
        else {
            int expected=cb_expected(&model,i);char v=expected<0?'X':(char)('0'+expected);
            if(model.valid)dprint(10,y,C_BLACK,"%2u    %s       %c        %d    %d",i,bits,v,cb_eval_gate(&model,i,false),cb_eval_mux(&model,i));
            else dprint(10,y,C_BLACK,"%2u    %s       %c        -    -",i,bits,v);
        }
    }
    dprint(10,190,C_DARK,"Row%u/%u | arrows%s",row,rows-1,model.kind==CB_FUNCTION&&!model.expression_source?"; EXE edit0/1/X":" scroll rows");
}
static void formula_lines(const char *text,int top,unsigned start,unsigned count)
{
    size_t n=strlen(text);for(unsigned i=0;i<count;i++) {
        size_t offset=(start+i)*46;if(offset>=n)break;
        char line[47];snprintf(line,sizeof line,"%.46s",text+offset);dtext(10,top+(int)i*18,C_BLACK,line);
    }
}
static void draw_equations(void)
{
    if(!model.valid){dtext(10,68,C_RED,"No current equations. Press RUN.");return;}
    if(model.kind==CB_ENCODER) {
        dtext(10,50,C_BLACK,"Yj = OR of Ii where index i has bit j=1");
        dtext(10,76,C_BLACK,"VALID = any input AND no pair asserted");
        dtext(10,102,C_RED,"Code only meaningful when VALID=1.");
        dtext(10,126,C_DARK,"On invalid inputs, code pins still OR indices.");
        dtext(10,150,C_DARK,"No priority; 8input family, not minimization.");return;
    }
    if(model.kind==CB_RIPPLE) {
        dtext(10,52,C_BLACK,"Each FA: P=x XOR y");dtext(10,78,C_BLACK,"Sum=P XOR Cin");dtext(10,104,C_BLACK,"Cout=(x AND y) OR (P AND Cin)");
        dtext(10,140,C_DARK,"Result=sum of FAi.Sum*2^i");dtext(10,164,C_DARK,"No physical delay model.");return;
    }
    char text[256];eb_format(&model.result[model.form],text,sizeof text);
    dprint(10,46,ACCENT,"%s: %s",model.form==EB_SOP?"SOP":"POS",model.kind==CB_FA?"A=x0 B=y0 C=c0":model.kind==CB_DISPLAY?"A=B3 B=B2 C=B1 D=B0":"A..D are MSB-first inputs");
    formula_lines(text,68,page,5);
    if(model.kind==CB_HA||model.kind==CB_FA||model.kind==CB_POPCOUNT) {
        dtext(10,165,C_DARK,"HA: Sum=A^B; Carry=A*B");
        dtext(10,182,C_DARK,"FA: Sum=A^B^C; Carry=A*B+(A^B)*C");
    }else if(model.kind==CB_DISPLAY)dtext(10,181,C_DARK,"a=top,b=UR,c=LR,d=bottom,e=LL,f=UL,g=mid");
}
static void draw_kmap(void)
{
    if(!model.valid||model.kind==CB_ENCODER||model.kind==CB_RIPPLE){dtext(10,65,C_DARK,"K-map requires solved2..4-input function.");return;}
    unsigned n=cb_inputs(&model),height=n==4?4:2,width=n==2?2:4;
    dtext(10,46,ACCENT,"Gray-order map; X is unconstrained");
    for(unsigned y=0;y<height;y++)for(unsigned x=0;x<width;x++) {
        unsigned r=bs_kmap_row(n,y,x);int required=cb_expected(&model,r);
        int px=55+(int)x*76,py=76+(int)y*28;
        unsigned count=model.result[model.form].count;
        eb_cube cube=count?model.result[model.form].cubes[page%count]:(eb_cube){0,0};
        bool member=count&&((r&cube.fixed)==cube.bits);
        drect_border(px,py,px+72,py+25,member?HI:C_WHITE,member?2:1,C_LIGHT);
        dprint(px+3,py+4,C_DARK,"%u:%c",r,required<0?'X':'0'+required);
    }
    dprint(10,194,C_DARK,"%s group%u/%u | UP/DOWN groups (%s)",model.form==EB_SOP?"SOP":"POS",model.result[model.form].count?page%model.result[model.form].count+1:0,model.result[model.form].count,model.form==EB_SOP?"ON":"OFF");
}
static void draw_wires(void)
{
    unsigned count=cb_connection_lines(&model);
    if(!count){dtext(10,65,C_RED,"No current connections. Press RUN.");return;}
    char line[96];unsigned first=page/7*7;
    for(unsigned i=first;i<count&&i<first+7;i++){cb_connection_line(&model,i,line,sizeof line);dtext(10,46+(int)(i-first)*20,C_BLACK,line);}
    dprint(10,191,C_DARK,"Line%u/%u | UP/DOWN page | EXE MUX",first+1,count);
}
static void draw_mux(void)
{
    if(!model.valid||model.kind==CB_ENCODER||model.kind==CB_RIPPLE){dtext(10,65,C_DARK,"MUX requires solved2..4-input function.");return;}
    char names[48]="",name[8];
    for(unsigned i=0;i<model.select_count;i++){cb_input_name(&model,model.select[i],name,sizeof name);size_t n=strlen(names);snprintf(names+n,sizeof names-n,"S%u=%s ",model.select_count-1-i,name);}
    dprint(10,46,ACCENT,"%u:1 MUX | %s",1u<<model.select_count,names);
    unsigned d=page%(1u<<model.select_count);char text[256];eb_format(&model.cofactor[d],text,sizeof text);
    dprint(10,70,ACCENT,"D%u <- N%u ; %s",d,model.mux_data[d],model.kind==CB_FA?"A=x0 B=y0 C=c0":"formula A..D in input order");
    formula_lines(text,94,0,4);
    dtext(10,172,C_DARK,"Data pins use same gate graph in WIRES.");
    dtext(10,191,C_DARK,"UP/DOWN data pins | EXE verify");
}
static void draw_verify(void)
{
    if(!model.valid){dtext(10,70,C_RED,"Not verified. RUN required.");return;}
    dprint(10,49,ACCENT,"PASS: %u %s checked",model.verified,model.kind==CB_RIPPLE?"stages":"cared rows");
    dtext(10,80,C_BLACK,"Gate connections match problem outputs.");
    if(model.kind==CB_ENCODER)dtext(10,106,C_BLACK,"Validity circuit matches exactly-one input.");
    else if(model.kind!=CB_RIPPLE)dtext(10,106,C_BLACK,"MUX and gate match, including DC completion.");
    dtext(10,134,C_DARK,"DC means unspecified; actual values in TABLE.");
    dtext(10,158,C_DARK,"Not a minimum-gate-count or delay claim.");
    dtext(10,188,C_DARK,"EXE returns to connections.");
}
static void draw(void)
{
    dclear(C_WHITE);info_title("ECEN21 - Combinational",ACCENT);
    char out[12];cb_output_name(&model,out,sizeof out);
    dprint(10,26,C_DARK,"%s | %s",cb_name(model.kind),out);
    if(view==SETUP)draw_setup();else if(view==TABLE)draw_table();else if(view==EQUATIONS)draw_equations();
    else if(view==KMAP)draw_kmap();else if(view==WIRES)draw_wires();else if(view==MUX)draw_mux();else draw_verify();
    if(model.error[0]){drect(7,175,389,208,C_WHITE);formula_lines(model.error,177,0,2);}
    else if(notice[0])dtext(10,197,C_RED,notice);
    fkey_button(1,"SET");fkey_button(2,"TABLE");fkey_button(3,"EQ");fkey_button(4,"KMAP");fkey_button(5,"CIRCUIT");fkey_button_color(6,"RUN",ACCENT);dupdate();
}
static void menu(void)
{
    dclear(C_WHITE);info_title("Combinational problem types",ACCENT);
    for(unsigned i=0;i<=CB_RIPPLE;i++)dprint(10,32+(int)i*21,C_BLACK,"%u %s",i+1,cb_name((cb_kind)i));
    dtext(10,185,C_DARK,"9 help | other keys cancel; type replaces.");dupdate();int digit=key_digit(getkey().key);
    if(digit>=1&&digit<=7){cb_init(&model,(cb_kind)(digit-1));reset_pages();view=SETUP;}
    if(digit==9) {
        dclear(C_WHITE);info_title("Combinational help",ACCENT);
        dtext(10,37,C_BLACK,"SET: UP/DOWN field; EXE edit/cycle.");
        dtext(10,60,C_BLACK,"RUN builds selected output; SET chooses it.");
        dtext(10,83,C_BLACK,"TABLE: arrows rows; EXE cycles manual0/1/X.");
        dtext(10,106,C_BLACK,"CIRCUIT: EXE -> MUX -> Verify -> wires.");
        dtext(10,129,C_BLACK,"MUX selectors distinct; first slot is MSB.");
        dtext(10,152,C_BLACK,"BCD a..g: top,UR,LR,bottom,LL,UL,middle.");
        dtext(10,184,C_DARK,"Any key returns; EXIT tool -> home.");dupdate();getkey();
    }
}
void combinational_run(void)
{
    static bool initialized;if(!initialized){cb_init(&model,CB_FA);initialized=true;}
    for(;;) {
        draw();int key=getkey().key;
        if(key==KEY_EXIT)return;
        if(key==INFO_KEY){menu();continue;}
        if(key==KEY_F1){view=SETUP;page=0;continue;}
        if(key==KEY_F2){view=TABLE;page=0;continue;}
        if(key==KEY_F3){view=EQUATIONS;page=0;continue;}
        if(key==KEY_F4){view=KMAP;page=0;continue;}
        if(key==KEY_F5){view=WIRES;page=0;continue;}
        if(key==KEY_F6){if(cb_solve(&model)){if(view==SETUP)view=EQUATIONS;}else view=SETUP;page=0;continue;}
        if(view==SETUP){if(key==KEY_UP)field=(field+fields()-1)%fields();if(key==KEY_DOWN)field=(field+1)%fields();if(key==KEY_EXE)edit_setting();continue;}
        if(view==TABLE) {
            if(model.kind==CB_RIPPLE){if(key==KEY_DOWN)page=(page+5)%10;if(key==KEY_UP)page=page?0:5;continue;}
            unsigned rows=1u<<cb_inputs(&model);
            if(key==KEY_UP)row=(row+rows-1)%rows;
            if(key==KEY_DOWN)row=(row+1)%rows;
            if(key==KEY_LEFT)row=(row+rows-6)%rows;
            if(key==KEY_RIGHT)row=(row+6)%rows;
            if(key==KEY_EXE&&model.kind==CB_FUNCTION&&!model.expression_source) {
                unsigned bit=1u<<row;
                if(model.dc&bit)model.dc&=(uint16_t)~bit;
                else if(model.on&bit){model.on&=(uint16_t)~bit;model.dc|=(uint16_t)bit;}
                else model.on|=(uint16_t)bit;
                cb_dirty(&model);
            }
            continue;
        }
        if(key==KEY_UP||key==KEY_DOWN) {
            unsigned count=view==WIRES?((cb_connection_lines(&model)+6)/7):view==MUX?(1u<<model.select_count):view==EQUATIONS?6:view==KMAP?model.result[model.form].count:1;
            if(!count)count=1;
            if(view==WIRES){unsigned p=page/7;p=(p+count+(key==KEY_DOWN?1:-1))%count;page=p*7;}
            else page=(page+count+(key==KEY_DOWN?1:-1))%count;
        }
        if(key==KEY_EXE){if(view==WIRES)view=MUX;else if(view==MUX)view=VERIFY;else if(view==VERIFY)view=WIRES;page=0;}
    }
}
