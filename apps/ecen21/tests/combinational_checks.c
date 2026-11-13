#include "combinational.h"
#include "combinational_checks.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { ++*checks;if(!(x))return __LINE__; } while(0)
/* Independently execute the emitted gate graph, including each MUX data pin. */
static int replay(const cb_model *m,unsigned row,unsigned target)
{
    bool values[CB_NODES]={false};
    unsigned n=cb_inputs(m);
    for(unsigned i=0;i<m->node_count;i++) {
        cb_node q=m->nodes[i];
        switch(q.op) {
        case CB_ZERO:values[i]=false;break;case CB_ONE:values[i]=true;break;
        case CB_INPUT:if(q.a>=n)return -1;values[i]=(row/(1u<<(n-1-q.a)))%2;break;
        case CB_NOT:if(q.a>=i)return -1;values[i]=!values[q.a];break;
        case CB_AND:case CB_OR:case CB_XOR:
            if(q.a>=i||q.b>=i)return -1;
            values[i]=q.op==CB_AND?(values[q.a]&&values[q.b]):q.op==CB_OR?(values[q.a]||values[q.b]):values[q.a]!=values[q.b];break;
        default:return -1;
        }
    }
    return target<m->node_count?values[target]:-1;
}
static int mapped(unsigned *checks,cb_model *m)
{
    CHECK(cb_solve(m));CHECK(m->valid);
    unsigned n=cb_inputs(m);
    for(unsigned row=0;row<(1u<<n);row++) {
        int expected=cb_expected(m,row),y=replay(m,row,m->gate_output);
        if(m->kind==CB_FUNCTION)expected=(m->dc&(1u<<row))?-1:(int)((m->on>>row)&1u);
        CHECK(y>=0);if(expected>=0)CHECK(y==expected);
        CHECK(cb_eval_gate(m,row,false)==y);
        if(m->kind!=CB_ENCODER) {
            unsigned address=0;
            for(unsigned i=0;i<m->select_count;i++)address=2*address+(row/(1u<<(n-1-m->select[i])))%2;
            CHECK(replay(m,row,m->mux_data[address])==y);
            CHECK(eb_evaluate(&m->cofactor[address],row)==y);
            CHECK(cb_eval_mux(m,row)==y);
        }
    }
    char line[96];
    for(unsigned i=0;i<cb_connection_lines(m);i++) {
        CHECK(cb_connection_line(m,i,line,sizeof line));CHECK(strlen(line)<=46);
    }
    CHECK(!cb_connection_line(m,cb_connection_lines(m),line,sizeof line));
    return 0;
}
int cb_run_checks(unsigned *checks)
{
    *checks=0;static cb_model m;
    for(unsigned kind=CB_HA;kind<=CB_POPCOUNT;kind++)for(unsigned out=0;out<2;out++) {
        cb_init(&m,(cb_kind)kind);m.output=out;
        for(unsigned f=0;f<2;f++) {
            m.form=(eb_form)f;int rc=mapped(checks,&m);if(rc)return rc;
            for(unsigned row=0;row<(1u<<cb_inputs(&m));row++) {
                unsigned sum=0;for(unsigned bit=0;bit<cb_inputs(&m);bit++)sum+=(row>>bit)&1;
                CHECK(cb_expected(&m,row)==(int)((sum>>out)&1));
            }
        }
    }
    /* P4.12 two MUXes, S1=x0/S0=y0; exact literal pin functions. */
    for(unsigned out=0;out<2;out++) {
        cb_init(&m,CB_FA);m.output=out;CHECK(cb_solve(&m));
        for(unsigned row=0;row<8;row++)for(unsigned d=0;d<4;d++) {
            int c=(int)(row%2),expect=out?(d==0?0:d==3?1:c):(d==0||d==3?c:!c);
            CHECK(replay(&m,row,m.mux_data[d])==expect);
        }
    }
    for(unsigned n=4;n<=8;n+=4)for(unsigned out=0;out<(n==4?2:3);out++) {
        cb_init(&m,CB_ENCODER);m.encoder_inputs=n;m.output=out;
        int rc=mapped(checks,&m);if(rc)return rc;
        for(unsigned row=0;row<(1u<<n);row++) {
            unsigned count=0,code=0;
            for(unsigned i=0;i<n;i++)if(row&(1u<<i)){count++;code|=i;}
            bool valid;CHECK(cb_encoder_code(row,n,&valid)==code);CHECK(valid==(count==1));
            CHECK(replay(&m,row,m.valid_output)==(int)(count==1));
            CHECK(replay(&m,row,m.gate_output)==(int)((code>>out)&1));
        }
    }
    /* Independent decimal shapes as a-g strings, not production hex masks. */
    static const char *const glyph[]={"1111110","0110000","1101101","1111001","0110011","1011011","1011111","1110000","1111111","1111011"};
    for(unsigned low=0;low<2;low++)for(unsigned policy=0;policy<3;policy++)for(unsigned out=0;out<7;out++) {
        cb_init(&m,CB_DISPLAY);m.active_low=low!=0;m.invalid_policy=policy;m.output=out;
        int rc=mapped(checks,&m);if(rc)return rc;
        for(unsigned row=0;row<16;row++) {
            int expected=row<10?(glyph[row][out]-'0')^(int)low:policy==2?-1:policy==0?0:(int)low;
            CHECK(cb_expected(&m,row)==expected);
            if(expected>=0)CHECK(replay(&m,row,m.gate_output)==expected);
        }
    }
    for(unsigned f=0;f<2;f++) {
        cb_init(&m,CB_FUNCTION);m.variables=4;m.on=0;m.dc=UINT16_MAX;m.form=(eb_form)f;
        int rc=mapped(checks,&m);if(rc)return rc;CHECK(m.verified==0);
    }
    uint32_t random=0x721;
    for(unsigned i=0;i<48;i++) {
        cb_init(&m,CB_FUNCTION);m.variables=4;
        random=random*1664525u+1013904223u;m.on=(uint16_t)random;
        random=random*1664525u+1013904223u;m.dc=(uint16_t)random&~m.on;
        m.select_count=1+i%3;m.select[0]=3;m.select[1]=1;m.select[2]=0;m.form=(eb_form)(i%2);
        int rc=mapped(checks,&m);if(rc)return rc;
    }
    cb_init(&m,CB_FUNCTION);m.expression_source=true;snprintf(m.expression,sizeof m.expression,"A^B^C");
    CHECK(cb_solve(&m));CHECK(m.truth==0x96);
    snprintf(m.expression,sizeof m.expression,"A+");CHECK(!cb_solve(&m)&&!m.valid&&m.node_count==0);
    cb_init(&m,CB_FUNCTION);m.select[1]=0;CHECK(!cb_solve(&m)&&!m.valid);
    cb_init(&m,CB_FUNCTION);m.select_count=4;CHECK(!cb_solve(&m));
    cb_init(&m,CB_FUNCTION);m.on=1;m.dc=1;CHECK(!cb_solve(&m));
    cb_init(&m,CB_FUNCTION);m.variables=2;m.on=0x10;CHECK(!cb_solve(&m));
    CHECK(!cb_solve(NULL));
    for(unsigned w=1;w<=8;w++) {
        unsigned max=(1u<<w)-1;
        for(unsigned a=0;a<=max;a+=max)for(unsigned b=0;b<=max;b+=max)for(unsigned cin=0;cin<2;cin++) {
            cb_stage stages[8];unsigned sum,cout;CHECK(cb_ripple(w,a,b,cin,stages,&sum,&cout));
            CHECK(sum+(cout<<w)==a+b+cin);
            unsigned carry=cin;
            for(unsigned i=0;i<w;i++) {
                unsigned total=((a>>i)&1)+((b>>i)&1)+carry;
                CHECK(stages[i].cin==carry&&stages[i].sum==total%2&&stages[i].cout==total/2);
                carry=total/2;
            }
        }
    }
    for(unsigned w=1;w<=8;w++) {
        cb_init(&m,CB_RIPPLE);m.width=w;m.a=(1u<<w)-1;m.b=1;m.cin=0;
        CHECK(cb_solve(&m)&&m.valid&&m.verified==w);
        CHECK(cb_connection_lines(&m)==w+3);
        char text[96],expected[96];
        for(unsigned i=1;i<w;i++) {
            CHECK(cb_connection_line(&m,i+1,text,sizeof text));
            snprintf(expected,sizeof expected,"FA%u: x=A[%u],y=B[%u],Cin=FA%u.Cout",i,i,i,i-1);
            CHECK(!strcmp(text,expected));
        }
        m.a=1u<<w;CHECK(!cb_solve(&m)&&!m.valid&&cb_connection_lines(&m)==0);
    }
    cb_init(&m,CB_FUNCTION);m.variables=1;CHECK(!cb_solve(&m));
    cb_init(&m,CB_FUNCTION);m.select_count=3;m.variables=2;m.on=0;CHECK(!cb_solve(&m));
    cb_init(&m,CB_ENCODER);m.encoder_inputs=7;CHECK(!cb_solve(&m));
    cb_init(&m,CB_DISPLAY);m.invalid_policy=3;CHECK(!cb_solve(&m));
    cb_init(&m,CB_FA);m.output=2;CHECK(!cb_solve(&m));
    cb_init(&m,(cb_kind)99);CHECK(!cb_solve(&m));
    cb_stage stages[8];unsigned sum,cout;
    CHECK(!cb_ripple(0,0,0,0,stages,&sum,&cout));CHECK(!cb_ripple(9,0,0,0,stages,&sum,&cout));
    CHECK(!cb_ripple(4,16,0,0,stages,&sum,&cout));CHECK(!cb_ripple(4,0,0,2,stages,&sum,&cout));
#ifdef CB_FULL_HOST_TESTS
    /* Every2/3-input function and legal ordered selector configuration. */
    for(unsigned n=2;n<=3;n++)for(unsigned truth=0;truth<(1u<<(1u<<n));truth++)
    for(unsigned k=1;k<=n;k++)for(unsigned a=0;a<n;a++)for(unsigned b=0;b<(k>1?n:1);b++)for(unsigned c=0;c<(k>2?n:1);c++) {
        if((k>1&&a==b)||(k>2&&(a==c||b==c)))continue;
        for(unsigned f=0;f<2;f++) {
            cb_init(&m,CB_FUNCTION);m.variables=n;m.on=(uint16_t)truth;m.dc=0;m.select_count=k;
            m.select[0]=a;m.select[1]=b;m.select[2]=c;m.form=(eb_form)f;
            int rc=mapped(checks,&m);if(rc)return rc;
        }
    }
    for(unsigned w=1;w<=8;w++)for(unsigned a=0;a<(1u<<w);a++)for(unsigned b=0;b<(1u<<w);b++)for(unsigned cin=0;cin<2;cin++) {
        CHECK(cb_ripple(w,a,b,cin,stages,&sum,&cout));CHECK(sum+(cout<<w)==a+b+cin);
        unsigned carry=cin;
        for(unsigned i=0;i<w;i++) {
            unsigned total=((a>>i)&1)+((b>>i)&1)+carry;
            CHECK(stages[i].x==((a>>i)&1)&&stages[i].y==((b>>i)&1));
            CHECK(stages[i].cin==carry&&stages[i].sum==total%2&&stages[i].cout==total/2);carry=total/2;
        }
    }
#endif
    return 0;
}
