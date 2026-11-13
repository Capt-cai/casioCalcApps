#include "seq_connections.h"
#include <stdio.h>
#include <string.h>

static uint16_t node_add(cn_netlist *n, cn_op op, const uint16_t *inputs, unsigned count)
{
    if(count>4 || n->count>=CN_MAX_NODES) return CN_ABSENT;
    for(unsigned i=0;i<n->count;i++) {
        cn_node const *p=&n->nodes[i];
        if(p->op!=op || p->count!=count) continue;
        bool same=true;
        for(unsigned j=0;j<count;j++) if(p->input[j]!=inputs[j]) same=false;
        if(same) return (uint16_t)i;
    }
    uint16_t result=(uint16_t)n->count++;
    cn_node *p=&n->nodes[result];p->op=op;p->count=count;
    for(unsigned i=0;i<count;i++) p->input[i]=inputs[i];
    return result;
}
static uint16_t invert(cn_netlist *n,uint16_t input)
{
    if(input==CN_ABSENT) return CN_ABSENT;
    if(input<2) return (uint16_t)(1-input);
    if(n->nodes[input].op==CN_NOT) return n->nodes[input].input[0];
    return node_add(n,CN_NOT,&input,1);
}
static uint16_t combine(cn_netlist *n,cn_op op,const uint16_t *inputs,unsigned count)
{
    if(!count) return op==CN_AND?1:0;
    uint16_t current=inputs[0];
    unsigned offset=1;
    while(offset<count) {
        uint16_t part[4]={current,0,0,0};unsigned size=1;
        while(size<4 && offset<count) part[size++]=inputs[offset++];
        for(unsigned i=0;i<size;i++) if(part[i]==CN_ABSENT) return CN_ABSENT;
        current=node_add(n,op,part,size);
        if(current==CN_ABSENT) return current;
    }
    return current;
}
static unsigned parity(unsigned row,unsigned mask)
{
    unsigned value=0;
    for(unsigned bits=row&mask;bits;bits>>=1) value^=bits&1;
    return value;
}
static uint16_t function(cn_netlist *n,const eb_result *r)
{
    if(r->variables!=n->variables || r->count>16 ||
        (r->form!=EB_SOP && r->form!=EB_POS)) return CN_ABSENT;
    /* Recognize the actual completed function, not just cared rows. This
     * preserves selected-form don't-care completions exactly. */
    unsigned rows=1u<<n->variables;
    for(unsigned mask=1;mask<rows;mask++) for(unsigned negative=0;negative<2;negative++) {
        bool equal=true;
        for(unsigned row=0;row<rows;row++) {
            int actual=eb_evaluate(r,row);
            if(actual<0 || (unsigned)actual!=(parity(row,mask)^negative)) {equal=false;break;}
        }
        if(equal) {
            uint16_t inputs[4];unsigned count=0;
            for(unsigned v=0;v<n->variables;v++)
                if(mask&(1u<<(n->variables-1-v))) inputs[count++]=(uint16_t)(2+v);
            uint16_t value=count==1?inputs[0]:node_add(n,CN_XOR,inputs,count);
            return negative?invert(n,value):value;
        }
    }
    uint16_t groups[16];
    for(unsigned g=0;g<r->count;g++) {
        eb_cube cube=r->cubes[g];uint16_t literals[4];unsigned count=0;
        if(cube.fixed>=rows || (cube.bits&~cube.fixed)) return CN_ABSENT;
        for(unsigned v=0;v<n->variables;v++) {
            unsigned bit=1u<<(n->variables-1-v);
            if(!(cube.fixed&bit)) continue;
            bool negative=r->form==EB_SOP ? !(cube.bits&bit) : (cube.bits&bit)!=0;
            uint16_t value=(uint16_t)(2+v);
            literals[count++]=negative?invert(n,value):value;
        }
        groups[g]=combine(n,r->form==EB_SOP?CN_AND:CN_OR,literals,count);
        if(groups[g]==CN_ABSENT) return CN_ABSENT;
    }
    return combine(n,r->form==EB_SOP?CN_OR:CN_AND,groups,r->count);
}
bool cn_build(const sq_model *m,eb_form form,cn_netlist *n)
{
    if(!n) return false;
    memset(n,0,sizeof *n);
    for(unsigned p=0;p<6;p++) n->pin_drives[p]=CN_ABSENT;
    for(unsigned o=0;o<2;o++) n->output_drives[o]=CN_ABSENT;
    if(!m || !m->valid || (form!=EB_SOP && form!=EB_POS) ||
        m->bits<2 || m->bits>3 || m->variables<2 || m->variables>4 || m->pin_count>6) return false;
    n->form=form;n->bits=m->bits;n->variables=m->variables;n->pin_count=m->pin_count;
    n->reverse=m->reverse;n->initial=m->initial;n->policy=m->policy;n->recovery=m->recovery;n->edge=m->active_edge;
    for(unsigned b=0;b<m->bits;b++) n->types[b]=m->types[b];
    node_add(n,CN_ZERO,NULL,0);node_add(n,CN_ONE,NULL,0);
    for(unsigned v=0;v<m->variables;v++) {
        uint16_t id=(uint16_t)v;
        snprintf(n->variable_names[v],sizeof n->variable_names[v],"%s",sq_variable_name(m,v));
        node_add(n,CN_INPUT,&id,1);
    }
    for(unsigned p=0;p<m->pin_count;p++) {
        snprintf(n->pin_names[p],sizeof n->pin_names[p],"%s",m->pins[p].name);
        n->pin_bits[p]=m->pins[p].bit;
        n->pin_drives[p]=function(n,&m->pins[p].solution[form]);
        if(n->pin_drives[p]==CN_ABSENT) return false;
    }
    for(unsigned o=0;o<2;o++) if(m->output_present[o]) {
        eb_result result;
        if(eb_minimize(m->variables,m->output_mask[o],0,form,&result)) return false;
        n->output_drives[o]=function(n,&result);
        if(n->output_drives[o]==CN_ABSENT) return false;
    }
    n->valid=true;return true;
}
unsigned cn_line_count(const cn_netlist *n)
{
    if(!n || !n->valid) return 0;
    unsigned outputs=(n->output_drives[0]!=CN_ABSENT)+(n->output_drives[1]!=CN_ABSENT);
    return 4+n->bits+n->count+n->pin_count+outputs;
}
bool cn_line(const cn_netlist *n,unsigned index,char *text,size_t capacity)
{
    if(text && capacity) text[0]='\0';
    if(!text || !capacity || index>=cn_line_count(n)) return false;
    char line[CN_LINE_CAPACITY];int length=0;
    if(index==0) length=snprintf(line,sizeof line,"CLK -> all FFs (%s edge)",n->edge==FF_RISING?"RISE":"FALL");
    else if(index==1) length=snprintf(line,sizeof line,"Initialize Q=%u before clocking",n->initial);
    else if(index==2) {
        if(n->reverse) length=snprintf(line,sizeof line,"Supplied equations; no hidden reset pin");
        else if(n->policy==SQ_RETURN) length=snprintf(line,sizeof line,"Unused: RETURN -> Q=%u",n->recovery);
        else length=snprintf(line,sizeof line,"Unused: %s",sq_policy_name(n->policy));
    }
    else if(index==3) length=snprintf(line,sizeof line,"%s: %s",n->form==EB_SOP?"SOP":"POS",
        !n->reverse && n->policy==SQ_DC?"DC gives no recovery guarantee":"All pin connections below are explicit");
    else {
        index-=4;
        if(index<n->bits) {
            unsigned bit=n->bits-1-index;
            length=snprintf(line,sizeof line,"FF%u: %s, output Q%u; common CLK",bit,ff_type_name(n->types[bit]),bit);
        }
        else {
            index-=n->bits;
            if(index<n->count) {
                cn_node const *node=&n->nodes[index];
                if(node->op==CN_ZERO || node->op==CN_ONE) length=snprintf(line,sizeof line,"N%u=%u",index,node->op==CN_ONE?1u:0u);
                else if(node->op==CN_INPUT) length=snprintf(line,sizeof line,"N%u=%s",index,n->variable_names[node->input[0]]);
                else {
                    const char *op=node->op==CN_NOT?"NOT":node->op==CN_AND?"AND":node->op==CN_OR?"OR":"XOR";
                    length=snprintf(line,sizeof line,"N%u=%s(",index,op);
                    for(unsigned i=0;i<node->count;i++) {
                        int written=snprintf(line+(size_t)length,sizeof line-(size_t)length,"%sN%u",i?",":"",(unsigned)node->input[i]);
                        if(written<0 || (size_t)written>=sizeof line-(size_t)length) return false;
                        length+=written;
                    }
                    int written=snprintf(line+(size_t)length,sizeof line-(size_t)length,")");
                    if(written!=1) return false;
                    length++;
                }
            }
            else {
                index-=n->count;
                if(index<n->pin_count) length=snprintf(line,sizeof line,"FF%u.%c <- N%u",n->pin_bits[index],n->pin_names[index][0],(unsigned)n->pin_drives[index]);
                else {
                    index-=n->pin_count;
                    for(unsigned output=0;output<2;output++) if(n->output_drives[output]!=CN_ABSENT) {
                        if(!index) {length=snprintf(line,sizeof line,"Z%u <- N%u (present Q/E)",output,(unsigned)n->output_drives[output]);break;}
                        index--;
                    }
                }
            }
        }
    }
    if(length<0 || (size_t)length>=sizeof line || (size_t)length>=capacity) return false;
    memcpy(text,line,(size_t)length+1);return true;
}
