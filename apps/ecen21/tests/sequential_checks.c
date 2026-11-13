#include "sequential.h"
#include "sequential_checks.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>

#define CHECK(x) do { ++*checks; if(!(x)) return __LINE__; } while(0)
/* Literal independent device truth tables indexed by Q*4+a*2+b. */
static int device(ff_type type,unsigned q,unsigned a,unsigned b)
{
    static const int truth[4][8]={{0,0,1,-1,1,0,1,-1},{0,0,1,1,0,0,1,1},
        {0,0,1,1,1,0,1,0},{0,0,1,1,1,1,0,0}};
    return truth[type][q*4+a*2+b];
}
/* Independent Boolean evaluator for cube data, without eb_evaluate. */
static unsigned cube_value(const eb_result *r,unsigned row)
{
    bool in_union=false;
    for(unsigned i=0;i<r->count;i++) {
        bool match=true;
        for(unsigned bit=0;bit<r->variables;bit++)
            if(((r->cubes[i].fixed>>bit)&1u) && ((row>>bit)&1u)!=((r->cubes[i].bits>>bit)&1u))match=false;
        if(match)in_union=true;
    }
    return r->form==EB_SOP?(unsigned)in_union:(unsigned)!in_union;
}
static unsigned count_bits(unsigned n)
{unsigned count=0;while(n){count+=n&1u;n>>=1;}return count;}
/* Independent union-DP over all implicants (not production prime search).
 * Three-variable representative costs need256 states, kept host/target bounded. */
static unsigned oracle_cost(unsigned variables,unsigned on,unsigned dc,unsigned form)
{
    unsigned rows=1u<<variables,full=(1u<<rows)-1u;
    unsigned target=form==EB_SOP?on:(full&~(on|dc));
    unsigned forbidden=full&~(target|dc),dp[256];
    for(unsigned mask=0;mask<(1u<<rows);mask++)dp[mask]=UINT_MAX/2;
    dp[0]=0;
    for(unsigned covered=0;covered<(1u<<rows);covered++) {
        if(dp[covered]==UINT_MAX/2)continue;
        for(unsigned fixed=0;fixed<(1u<<variables);fixed++)
        for(unsigned bits=0;bits<(1u<<variables);bits++) {
            if(bits&~fixed)continue;
            unsigned subset=0;
            for(unsigned row=0;row<rows;row++)if((row&fixed)==bits)subset|=1u<<row;
            if(subset&forbidden)continue;
            unsigned next=covered|(subset&target),cost=dp[covered]+32+count_bits(fixed);
            if(cost<dp[next])dp[next]=cost;
        }
    }
    return dp[target];
}
static int replay(unsigned *checks,const sq_model *m,const unsigned *expected)
{
    CHECK(m->valid);CHECK(m->count==(1u<<(m->bits+(m->controlled?1u:0u))));
    for(unsigned row=0;row<m->count;row++)for(unsigned form=0;form<2;form++) {
        unsigned a[3]={0},b[3]={0},next=0;bool valid=true;
        for(unsigned i=0;i<m->pin_count;i++) {
            const sq_pin *p=&m->pins[i];unsigned value=cube_value(&p->solution[form],row);
            CHECK(sq_pin_value(m,(eb_form)form,i,row)==(int)value);
            if(p->input)b[p->bit]=value;else a[p->bit]=value;
            if(m->rows[row].care) {
                unsigned q=(m->rows[row].q>>p->bit)&1u,desired=(expected[row]>>p->bit)&1u;
                unsigned allowed=m->rows[row].excitation[i];
                CHECK(allowed==FF_X || allowed==value);
                if(allowed!=FF_X)CHECK(allowed<=1);
                /* Independent enumeration verifies excitation-X assignments. */
                bool any=false;
                for(unsigned aa=0;aa<2;aa++)for(unsigned bb=0;bb<2;bb++) {
                    if(p->input && bb!=value)continue;
                    if(!p->input && aa!=value)continue;
                    if(device(m->types[p->bit],q,aa,bb)==(int)desired)any=true;
                }
                CHECK(any);
            }
        }
        for(unsigned bit=0;bit<m->bits;bit++) {
            int v=device(m->types[bit],(m->rows[row].q>>bit)&1u,a[bit],b[bit]);
            if(v<0){valid=false;break;}next|=(unsigned)v<<bit;
        }
        CHECK(m->rows[row].next[form]==(valid?next:SQ_UNDEFINED));
        if(m->rows[row].care)CHECK(valid && next==expected[row]);
    }
    return 0;
}
static int pin_costs(unsigned *checks,const sq_model *m)
{
    if(m->variables>3) { return 0; }
    for(unsigned i=0;i<m->pin_count;i++)for(unsigned form=0;form<2;form++) {
        const sq_pin *p=&m->pins[i];
        CHECK((unsigned)p->solution[form].count*32+p->solution[form].literals==oracle_cost(m->variables,p->on,p->dc,form));
    }
    return 0;
}
static int fixtures(unsigned *checks)
{
    sq_model m;sq_init(&m);CHECK(sq_solve(&m));
    const unsigned t1[16]={3,0,0,5,0,7,0,0};int line=replay(checks,&m,t1);if(line) { return line; }
    line=pin_costs(checks,&m);if(line) { return line; }
    const unsigned tex[8][3]={{0,1,1},{0,0,1},{0,1,0},{1,1,0},{1,0,0},{0,1,0},{1,1,0},{1,1,1}};
    for(unsigned row=0;row<8;row++)for(unsigned p=0;p<3;p++)CHECK(m.rows[row].excitation[p]==tex[row][p]);
    CHECK(m.checked_rows==16);
    sq_trace_result trace;CHECK(sq_trace(&m,EB_SOP,0,0,NULL,&trace));
    CHECK(trace.cycle && trace.prefix==0 && trace.period==4 && trace.steps==4);
    for(unsigned i=0;i<=4;i++)CHECK((trace.states[i]==(unsigned[]){0,3,5,7,0}[i]));
    for(unsigned start=1;start<7;start++)if(start!=3 && start!=5) {
        CHECK(sq_trace(&m,EB_POS,start,0,NULL,&trace));CHECK(trace.cycle && trace.prefix==1 && trace.period==4);
    }
    for(unsigned bit=0;bit<3;bit++)CHECK(sq_set_type(&m,bit,FF_JK));
    CHECK(sq_set_text(&m,0,"0147350"));CHECK(sq_solve(&m));
    const unsigned jk[16]={1,4,0,5,7,0,0,3};line=replay(checks,&m,jk);if(line) { return line; }
    line=pin_costs(checks,&m);if(line) { return line; }
    CHECK(sq_trace(&m,EB_SOP,0,0,NULL,&trace));CHECK(trace.period==6 && !trace.prefix);
    CHECK(sq_set_type(&m,2,FF_D));CHECK(sq_set_type(&m,1,FF_T));CHECK(sq_set_text(&m,0,"031254760"));
    CHECK(sq_solve(&m));const unsigned mixed[16]={3,2,5,1,7,4,0,6};line=replay(checks,&m,mixed);if(line) { return line; }
    CHECK(m.pin_count==4);CHECK(!strcmp(m.pins[0].name,"D2") && !strcmp(m.pins[3].name,"K0"));
    CHECK(m.rows[0].excitation[0]==0 && m.rows[0].excitation[1]==1 && m.rows[0].excitation[2]==1 && m.rows[0].excitation[3]==FF_X);
    CHECK(m.rows[7].excitation[0]==1 && m.rows[7].excitation[1]==0 && m.rows[7].excitation[2]==FF_X && m.rows[7].excitation[3]==1);
    CHECK(sq_trace(&m,EB_POS,0,0,NULL,&trace));CHECK(trace.period==8);
    sq_init(&m);CHECK(sq_set_bits(&m,2));CHECK(sq_set_controlled(&m,true));
    CHECK(sq_set_text(&m,0,"02310"));CHECK(sq_set_text(&m,1,"01320"));CHECK(sq_solve(&m));
    const unsigned controlled[16]={2,1,0,3,3,0,1,2};line=replay(checks,&m,controlled);if(line) { return line; }
    line=pin_costs(checks,&m);if(line) { return line; }
    for(unsigned p=0;p<2;p++)for(unsigned f=0;f<2;f++) {
        CHECK(m.pins[p].parity_mask[f]==7);CHECK(m.pins[p].parity_invert[f]==(p==0));
    }
    CHECK(!strcmp(sq_variable_name(&m,0),"Q1") && !strcmp(sq_variable_name(&m,1),"Q0") && !strcmp(sq_variable_name(&m,2),"E"));
    CHECK(sq_trace(&m,EB_SOP,0,0,"0101",&trace));CHECK(trace.limited && !trace.cycle && trace.steps==4);
    for(unsigned i=0;i<=4;i++)CHECK(trace.states[i]==(i%2?2u:0u));
    char timeline[33];memset(timeline,'1',32);timeline[32]='\0';
    CHECK(sq_trace(&m,EB_SOP,0,0,timeline,&trace));CHECK(trace.steps==32 && trace.limited && !trace.cycle);
    CHECK(sq_set_bits(&m,3));CHECK(sq_set_type(&m,2,FF_D));CHECK(sq_set_type(&m,1,FF_JK));CHECK(sq_set_type(&m,0,FF_SR));
    CHECK(sq_set_text(&m,0,"03570"));CHECK(sq_solve(&m));
    const unsigned controlled3[16]={3,1,0,3,0,0,5,2,0,0,7,0,0,0,0,0};
    line=replay(checks,&m,controlled3);if(line) { return line; }CHECK(m.count==16 && m.variables==4);
    sq_init(&m);CHECK(sq_set_bits(&m,2));CHECK(sq_set_reverse(&m,true));
    CHECK(sq_set_type(&m,1,FF_T));CHECK(sq_set_type(&m,0,FF_JK));
    CHECK(sq_set_text(&m,4,"B+!A"));CHECK(sq_set_text(&m,2,"!B*A"));CHECK(sq_set_text(&m,3,"!B+A"));
    CHECK(sq_set_text(&m,8,"B"));CHECK(sq_solve(&m));
    const unsigned reverse[16]={2,3,3,0};line=replay(checks,&m,reverse);if(line) { return line; }
    for(unsigned row=0;row<4;row++)CHECK(sq_output_value(&m,0,row)==(int)(row&1u));
    CHECK(sq_output_value(&m,1,0)==-1);CHECK(sq_trace(&m,EB_POS,0,0,NULL,&trace));
    CHECK(trace.period==3 && trace.prefix==0 && trace.steps==3);
    CHECK(sq_trace(&m,EB_SOP,1,0,NULL,&trace));CHECK(trace.prefix==1 && trace.period==3);
    CHECK(sq_set_type(&m,0,FF_D));CHECK(sq_set_type(&m,1,FF_D));
    CHECK(sq_set_text(&m,2,"A"));CHECK(sq_set_text(&m,4,"B"));CHECK(sq_solve(&m));
    unsigned next=99;CHECK(sq_next(&m,EB_SOP,1,0,FF_RISING,&next) && next==2);
    CHECK(sq_next(&m,EB_POS,1,0,FF_NONE,&next) && next==1);
    CHECK(sq_next(&m,EB_POS,1,0,FF_FALLING,&next) && next==1);
    CHECK(sq_set_edge(&m,FF_FALLING));CHECK(sq_solve(&m));
    CHECK(sq_next(&m,EB_SOP,1,0,FF_FALLING,&next) && next==2);
    CHECK(sq_set_type(&m,0,FF_SR));CHECK(sq_set_text(&m,2,"1"));CHECK(sq_set_text(&m,3,"1"));CHECK(sq_solve(&m));
    for(unsigned row=0;row<4;row++)CHECK(m.rows[row].next[0]==SQ_UNDEFINED && m.rows[row].next[1]==SQ_UNDEFINED);
    next=99;CHECK(!sq_next(&m,EB_SOP,0,0,FF_FALLING,&next) && next==99);
    CHECK(sq_trace(&m,EB_SOP,0,0,NULL,&trace) && trace.invalid && trace.steps==0);
    return 0;
}
static int policies_errors(unsigned *checks)
{
    sq_model m;sq_init(&m);CHECK(sq_set_policy(&m,SQ_HOLD));CHECK(sq_solve(&m));
    CHECK(m.rows[1].target==1 && m.rows[6].target==6);
    CHECK(sq_set_policy(&m,SQ_DC));CHECK(sq_solve(&m));
    CHECK(!m.rows[1].care && m.rows[1].target==SQ_UNDEFINED && m.rows[0].care);
    const unsigned wanted[16]={3,0,0,5,0,7,0,0};int line=replay(checks,&m,wanted);if(line) { return line; }
    CHECK(sq_set_policy(&m,SQ_RETURN));CHECK(sq_set_recovery(&m,2));CHECK(sq_solve(&m));
    sq_trace_result trace;CHECK(sq_trace(&m,EB_SOP,1,0,NULL,&trace));CHECK(trace.prefix==1 && trace.period==1 && trace.states[1]==2);
    sq_init(&m);CHECK(sq_set_bits(&m,2));unsigned blank[16]={0};
    CHECK(sq_synthesize_table(&m,blank,0,15));CHECK(m.rows[0].next[EB_SOP]!=m.rows[0].next[EB_POS]);
    CHECK(m.rows[0].next[EB_SOP]==0 && m.rows[0].next[EB_POS]==3);
    CHECK(sq_trace(&m,EB_SOP,0,0,NULL,&trace) && trace.period==1);
    CHECK(sq_trace(&m,EB_POS,0,0,NULL,&trace) && trace.period==2);
    sq_init(&m);CHECK(sq_solve(&m));CHECK(sq_set_text(&m,0,"0,3,0,5"));CHECK(!m.valid && !sq_solve(&m));
    CHECK(m.error_field==0 && m.error_pos==4);
    CHECK(sq_set_text(&m,0,"0->3->5->7->0"));CHECK(sq_solve(&m));
    CHECK(sq_set_text(&m,0,"0"));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,0,""));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,0,"0,9"));CHECK(!sq_solve(&m) && m.error_pos==2);
    CHECK(sq_set_text(&m,0,"0A"));CHECK(!sq_solve(&m) && m.error_pos==1);
    CHECK(sq_set_text(&m,0,"0123456701"));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,0,"03570"));CHECK(sq_set_text(&m,1,"bad"));CHECK(sq_solve(&m));
    CHECK(sq_set_controlled(&m,true));CHECK(!sq_solve(&m) && m.error_field==1);
    CHECK(sq_set_text(&m,1,"01320"));CHECK(sq_set_text(&m,10,"01X"));CHECK(!sq_solve(&m) && m.error_field==10 && m.error_pos==2);
    CHECK(sq_set_controlled(&m,false));CHECK(sq_solve(&m));
    CHECK(sq_set_reverse(&m,true));CHECK(sq_set_text(&m,6,"A+"));CHECK(!sq_solve(&m) && m.error_field==6);
    CHECK(sq_set_text(&m,6,"A"));CHECK(sq_set_text(&m,8,"D"));CHECK(!sq_solve(&m) && m.error_field==8);
    CHECK(sq_set_text(&m,8,""));CHECK(sq_solve(&m));
    CHECK(sq_set_text(&m,6,""));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,6,"A"));
    char long_text[90];memset(long_text,'A',89);long_text[89]='\0';
    CHECK(!sq_set_text(&m,6,long_text));CHECK(!m.valid && !sq_solve(&m));
    CHECK(sq_set_text(&m,8,"0"));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,6,"A"));CHECK(sq_solve(&m));
    CHECK(!sq_set_text(&m,1,long_text));CHECK(sq_solve(&m)); /* Inactivecycle field is ignored in reverse. */
    CHECK(sq_set_reverse(&m,false));CHECK(sq_solve(&m)); /* Branch1 inactive withoutE. */
    CHECK(sq_set_controlled(&m,true));CHECK(!sq_solve(&m));
    CHECK(sq_set_text(&m,1,"01320"));CHECK(sq_set_text(&m,10,""));CHECK(sq_solve(&m));
    CHECK(!sq_set_bits(&m,4) && !m.valid);CHECK(m.bits==3);
    CHECK(!sq_set_type(&m,3,FF_D));CHECK(!sq_set_type(&m,0,(ff_type)-1));
    CHECK(!sq_set_edge(&m,FF_NONE));CHECK(!sq_set_policy(&m,(sq_policy)-1));
    CHECK(!sq_set_initial(&m,8));CHECK(!sq_set_recovery(&m,8));CHECK(!sq_set_fixed_e(&m,2));
    CHECK(!sq_set_text(&m,11,"0"));CHECK(!sq_set_text(&m,0,NULL));
    CHECK(sq_solve(&m));CHECK(!sq_next(&m,(eb_form)-1,0,0,FF_RISING,&blank[0]));
    CHECK(!sq_next(&m,EB_SOP,0,0,(ff_event)-1,&blank[0]));
    CHECK(!sq_trace(&m,(eb_form)-1,0,0,NULL,&trace));
    CHECK(!sq_trace(&m,EB_SOP,8,0,NULL,&trace));CHECK(!sq_trace(&m,EB_SOP,0,2,NULL,&trace));
    CHECK(!sq_trace(&m,EB_SOP,0,0,"0X",&trace));
    char long_timeline[34];memset(long_timeline,'0',33);long_timeline[33]='\0';
    CHECK(!sq_trace(&m,EB_SOP,0,0,long_timeline,&trace));
    CHECK(sq_pin_value(&m,(eb_form)-1,0,0)==-1);CHECK(sq_pin_value(&m,EB_SOP,99,0)==-1);CHECK(sq_pin_value(&m,EB_SOP,0,16)==-1);
    char buf[2];CHECK(sq_format_equation(&m,EB_SOP,0,buf,sizeof buf)>=1 && buf[1]=='\0');
    CHECK(sq_format_equation(&m,(eb_form)-1,0,buf,sizeof buf)==-1);
    CHECK(sq_output_value(&m,2,0)==-1);CHECK(!strcmp(sq_variable_name(&m,4),"?"));
    CHECK(!strcmp(sq_policy_name((sq_policy)-1),"?"));
    CHECK(!sq_synthesize_table(&m,blank,1,1));
    CHECK(sq_set_bits(&m,2));CHECK(sq_set_controlled(&m,false));
    CHECK(!sq_synthesize_table(&m,blank,0,0x10));
    CHECK(!sq_synthesize_table(NULL,blank,0,0));
    CHECK(!sq_synthesize_table(&m,NULL,0,0));
    CHECK(!sq_set_bits(NULL,2));CHECK(!sq_set_controlled(NULL,true));
    CHECK(!sq_set_reverse(NULL,true));CHECK(!sq_set_type(NULL,0,FF_D));
    CHECK(!sq_set_edge(NULL,FF_RISING));CHECK(!sq_set_policy(NULL,SQ_RETURN));
    CHECK(!sq_set_initial(NULL,0));CHECK(!sq_set_recovery(NULL,0));CHECK(!sq_set_fixed_e(NULL,0));
    CHECK(!sq_set_text(NULL,0,"0"));CHECK(sq_text(NULL,0)==NULL);
    CHECK(!sq_solve(NULL));CHECK(sq_pin_value(NULL,EB_SOP,0,0)==-1);
    CHECK(sq_format_equation(NULL,EB_SOP,0,buf,sizeof buf)==-1);
    CHECK(sq_output_value(NULL,0,0)==-1);
    sq_init(&m);CHECK(sq_set_reverse(&m,true));
    char deepest[80];memset(deepest,'(',79);deepest[79]='\0';
    CHECK(sq_set_text(&m,6,deepest));CHECK(!sq_solve(&m));
    CHECK(!m.valid && m.error_field==6 && m.error_pos==79);
    memset(deepest,'!',79);
    CHECK(sq_set_text(&m,6,deepest));CHECK(!sq_solve(&m));
    CHECK(!m.valid && m.error_field==6 && m.error_pos==79);
    deepest[78]='A';CHECK(sq_set_text(&m,6,deepest));CHECK(sq_solve(&m));
    CHECK(m.pins[0].on==0xf0 && m.valid);
    sq_init(NULL);sq_invalidate(NULL);
    return 0;
}
int sq_run_checks(unsigned *checks)
{
    if(!checks) { return __LINE__; }
    *checks=0;
    int line=fixtures(checks);if(!line)line=policies_errors(checks);return line;
}
#ifdef SQ_FULL_HOST_TESTS
int sq_run_exhaustive(unsigned *checks)
{
    if(!checks) { return __LINE__; }
    *checks=0;
    sq_model m;unsigned targets[16]={0};
    for(unsigned map=0;map<256;map++)for(unsigned types=0;types<16;types++) {
        sq_init(&m);m.bits=2;m.types[0]=(ff_type)(types%4);m.types[1]=(ff_type)(types/4);
        for(unsigned row=0;row<4;row++)targets[row]=(map>>(row*2))&3u;
        CHECK(sq_synthesize_table(&m,targets,15,0));
        int line=replay(checks,&m,targets);if(line) { return line; }
        line=pin_costs(checks,&m);if(line) { return line; }
    }
    for(unsigned bits=2;bits<=3;bits++)for(unsigned types=0;types<(1u<<(bits*2));types++)
    for(unsigned desired=0;desired<(1u<<bits);desired++) {
        sq_init(&m);m.bits=bits;
        for(unsigned bit=0;bit<bits;bit++)m.types[bit]=(ff_type)((types>>(bit*2))&3u);
        for(unsigned row=0;row<(1u<<bits);row++)targets[row]=desired;
        CHECK(sq_synthesize_table(&m,targets,(uint16_t)((1u<<(1u<<bits))-1u),0));
        int line=replay(checks,&m,targets);if(line) { return line; }
    }
    for(unsigned bits=2;bits<=3;bits++)for(unsigned types=0;types<(1u<<(bits*2));types++)
    for(unsigned policy=SQ_RETURN;policy<=SQ_DC;policy++)for(unsigned controlled=0;controlled<2;controlled++) {
        sq_init(&m);m.bits=bits;m.controlled=controlled!=0;m.policy=(sq_policy)policy;
        unsigned max=(1u<<bits)-1u;
        for(unsigned bit=0;bit<bits;bit++)m.types[bit]=(ff_type)((types>>(bit*2))&3u);
        snprintf(m.cycle_text[0],sizeof m.cycle_text[0],"0%u",max);
        snprintf(m.cycle_text[1],sizeof m.cycle_text[1],"1%u",max-1);
        CHECK(sq_solve(&m));
        for(unsigned row=0;row<m.count;row++) {
            unsigned q=row>>(controlled?1:0),e=controlled?row&1u:0;
            unsigned src=e?1u:0u,dst=e?max-1:max;
            bool used=q==src || q==dst;
            targets[row]=q==src?dst:q==dst?src:policy==SQ_HOLD?q:0;
            CHECK(m.rows[row].used==used);
            CHECK(m.rows[row].care==(used || policy!=SQ_DC));
        }
        int line=replay(checks,&m,targets);if(line) { return line; }
    }
    uint32_t random=UINT32_C(0x12345678);
    for(unsigned sample=0;sample<256;sample++) {
        sq_init(&m);m.controlled=(sample&1u)!=0;
        for(unsigned bit=0;bit<3;bit++)m.types[bit]=(ff_type)((sample>>(bit*2))&3u);
        unsigned count=1u<<(3+(m.controlled?1u:0u));
        for(unsigned row=0;row<count;row++){random=random*1664525u+1013904223u;targets[row]=(random>>24)&7u;}
        CHECK(sq_synthesize_table(&m,targets,(uint16_t)((UINT32_C(1)<<count)-1u),0));
        int line=replay(checks,&m,targets);if(line) { return line; }
    }
    return 0;
}
#endif
