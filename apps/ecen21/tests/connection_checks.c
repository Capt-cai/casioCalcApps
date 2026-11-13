#include "connection_checks.h"
#include "seq_connections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(c) do { ++*checks; if(!(c)) return __LINE__; } while(0)

/* Tests execute the actual emitted gate-language lines, independently of the
 * builder's cube/parity implementation and its declarative node interpreter. */
static int execute_text(const cn_netlist *n,unsigned row,unsigned values[CN_MAX_NODES],unsigned *checks)
{
    unsigned next_node=0;
    for(unsigned line=0;line<cn_line_count(n);line++) {
        char text[CN_LINE_CAPACITY];
        CHECK(cn_line(n,line,text,sizeof text));
        CHECK(strlen(text)<=46);
        if(text[0]!='N')continue;
        char *cursor=text+1,*end;
        unsigned id=(unsigned)strtoul(cursor,&end,10);
        CHECK(end!=cursor && *end=='=' && id==next_node && id<n->count);
        next_node++;cursor=end+1;
        if(!strcmp(cursor,"0") || !strcmp(cursor,"1")) values[id]=(unsigned)(cursor[0]-'0');
        else if(cursor[0]=='Q' || cursor[0]=='E') {
            unsigned control_bits=n->variables-n->bits;
            if(cursor[0]=='E') {
                CHECK(!strcmp(cursor,"E") && control_bits==1);
                values[id]=row&1u;
            }
            else {
                unsigned bit=(unsigned)strtoul(cursor+1,&end,10);
                CHECK(end!=cursor+1 && !*end && bit<n->bits);
                values[id]=(row>>(control_bits+bit))&1u;
            }
        }
        else {
            char operation[5]={0};unsigned length=0;
            while(cursor[length] && cursor[length]!='(' && length<4){operation[length]=cursor[length];length++;}
            CHECK(cursor[length]=='(');
            cursor+=length+1;
            unsigned count=0,inputs[4]={0};
            while(*cursor && *cursor!=')') {
                CHECK(*cursor=='N' && count<4);cursor++;
                unsigned input=(unsigned)strtoul(cursor,&end,10);
                CHECK(end!=cursor && input<id);
                inputs[count++]=values[input];cursor=end;
                if(*cursor==',')cursor++;
                else CHECK(*cursor==')');
            }
            CHECK(cursor[0]==')' && cursor[1]=='\0' && count>0);
            unsigned value=inputs[0];
            if(!strcmp(operation,"NOT")){CHECK(count==1);value=!value;}
            else {
                CHECK(!strcmp(operation,"AND") || !strcmp(operation,"OR") || !strcmp(operation,"XOR"));
                for(unsigned i=1;i<count;i++) {
                    if(!strcmp(operation,"AND"))value=value&&inputs[i];
                    else if(!strcmp(operation,"OR"))value=value||inputs[i];
                    else value=value!=inputs[i];
                }
            }
            values[id]=value;
        }
    }
    CHECK(next_node==n->count);
    return 0;
}
static int replay(sq_model *m,unsigned *checks)
{
    static cn_netlist net;
    for(unsigned f=0;f<2;f++) {
        CHECK(cn_build(m,(eb_form)f,&net));
        CHECK(net.valid && net.count<=CN_MAX_NODES);
        CHECK(net.bits==m->bits && net.pin_count==m->pin_count);
        CHECK(net.edge==m->active_edge && net.initial==m->initial);
        for(unsigned row=0;row<m->count;row++) {
            unsigned values[CN_MAX_NODES]={0};
            int line=execute_text(&net,row,values,checks);if(line)return line;
            unsigned pin_lines=0,output_lines=0;
            for(unsigned i=0;i<cn_line_count(&net);i++) {
                char text[CN_LINE_CAPACITY],name=0;unsigned bit=0,node=0,output=0;
                CHECK(cn_line(&net,i,text,sizeof text));
                if(sscanf(text,"FF%u.%c <- N%u",&bit,&name,&node)==3) {
                    unsigned found=m->pin_count;
                    for(unsigned p=0;p<m->pin_count;p++)if(m->pins[p].bit==bit && m->pins[p].name[0]==name)found=p;
                    CHECK(found<m->pin_count && node<net.count);
                    CHECK(values[node]==(unsigned)sq_pin_value(m,(eb_form)f,found,row));
                    pin_lines++;
                }
                else if(sscanf(text,"Z%u <- N%u",&output,&node)==2) {
                    CHECK(output<2 && m->output_present[output] && node<net.count);
                    CHECK(values[node]==((m->output_mask[output]>>row)&1u));
                    output_lines++;
                }
            }
            CHECK(pin_lines==m->pin_count);
            CHECK(output_lines==(unsigned)m->output_present[0]+(unsigned)m->output_present[1]);
        }
    }
    return 0;
}
int cn_run_checks(unsigned *checks)
{
    if(!checks)return __LINE__;
    *checks=0;
    static sq_model model;
    sq_init(&model);CHECK(sq_solve(&model));
    int line=replay(&model,checks);if(line)return line;
    /* Full mixed example, then all-SR/all-JK and both unused policies. */
    CHECK(sq_set_text(&model,0,"031254760"));
    CHECK(sq_set_type(&model,2,FF_D));CHECK(sq_set_type(&model,1,FF_T));CHECK(sq_set_type(&model,0,FF_JK));
    CHECK(sq_solve(&model));line=replay(&model,checks);if(line)return line;
    for(unsigned type=FF_SR;type<=FF_T;type++)for(unsigned policy=SQ_RETURN;policy<=SQ_DC;policy++) {
        sq_init(&model);
        for(unsigned b=0;b<3;b++)CHECK(sq_set_type(&model,b,(ff_type)type));
        CHECK(sq_set_policy(&model,(sq_policy)policy));CHECK(sq_solve(&model));
        line=replay(&model,checks);if(line)return line;
    }
    sq_init(&model);CHECK(sq_set_bits(&model,2));CHECK(sq_set_controlled(&model,true));
    CHECK(sq_set_text(&model,0,"02310"));CHECK(sq_set_text(&model,1,"01320"));
    CHECK(sq_solve(&model));line=replay(&model,checks);if(line)return line;
    /* Required restricted implementation: one shared3-inputXOR and oneNOT. */
    static cn_netlist net;
    for(unsigned f=0;f<2;f++) {
        CHECK(cn_build(&model,(eb_form)f,&net));
        unsigned xor_gates=0,not_gates=0,other_gates=0;
        for(unsigned i=0;i<net.count;i++) {
            xor_gates+=net.nodes[i].op==CN_XOR;not_gates+=net.nodes[i].op==CN_NOT;
            other_gates+=net.nodes[i].op==CN_AND || net.nodes[i].op==CN_OR;
            if(net.nodes[i].op==CN_XOR)CHECK(net.nodes[i].count==3);
        }
        CHECK(xor_gates==1 && not_gates==1 && other_gates==0);
        CHECK(net.pin_drives[0]!=net.pin_drives[1]);
    }
    /* All sixteen two-variable functions as D0, with complementary D1. */
    for(unsigned function=0;function<16;function++) {
        sq_init(&model);CHECK(sq_set_bits(&model,2));
        CHECK(sq_set_type(&model,0,FF_D));CHECK(sq_set_type(&model,1,FF_D));
        unsigned targets[16]={0};
        for(unsigned q=0;q<4;q++)targets[q]=((function>>q)&1u)|((((function>>q)&1u)^1u)<<1);
        CHECK(sq_synthesize_table(&model,targets,15,0));
        line=replay(&model,checks);if(line)return line;
    }
    /* Four-variable mixed functions plus two independently evaluated outputs. */
    for(unsigned sample=0;sample<12;sample++) {
        sq_init(&model);CHECK(sq_set_controlled(&model,true));
        CHECK(sq_set_type(&model,0,(ff_type)(sample%4)));
        CHECK(sq_set_type(&model,1,(ff_type)((sample+1)%4)));
        CHECK(sq_set_type(&model,2,(ff_type)((sample+2)%4)));
        CHECK(sq_set_text(&model,8,"A^D"));CHECK(sq_set_text(&model,9,"A*B+C*D"));
        unsigned targets[16];
        for(unsigned row=0;row<16;row++)targets[row]=(row*row+sample*3+row*sample)%8;
        CHECK(sq_synthesize_table(&model,targets,65535,0));
        for(unsigned row=0;row<16;row++) {
            unsigned a=(row>>3)&1u,b=(row>>2)&1u,c=(row>>1)&1u,d=row&1u;
            CHECK(sq_output_value(&model,0,row)==(int)(a!=d));
            CHECK(sq_output_value(&model,1,row)==(int)((a&&b)||(c&&d)));
        }
        line=replay(&model,checks);if(line)return line;
    }
    char text[CN_LINE_CAPACITY];
    CHECK(cn_build(&model,EB_SOP,&net));
    for(unsigned i=0;i<cn_line_count(&net);i++) {
        CHECK(cn_line(&net,i,text,sizeof text));
        size_t length=strlen(text);char tiny[CN_LINE_CAPACITY];memset(tiny,'x',sizeof tiny);
        CHECK(!cn_line(&net,i,tiny,length));CHECK(!tiny[0]);
        CHECK(cn_line(&net,i,tiny,length+1));CHECK(!strcmp(tiny,text));
    }
    CHECK(!cn_line(&net,cn_line_count(&net),text,sizeof text) && !text[0]);
    CHECK(!cn_line(&net,0,NULL,0));CHECK(!cn_line(NULL,0,text,sizeof text));
    CHECK(cn_line_count(NULL)==0);CHECK(!cn_build(&model,EB_SOP,NULL));
    CHECK(!cn_build(NULL,EB_SOP,&net) && !net.valid);
    CHECK(!cn_build(&model,(eb_form)2,&net));
    sq_invalidate(&model);CHECK(!cn_build(&model,EB_SOP,&net));
    return 0;
}
