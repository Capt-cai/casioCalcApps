#include "exact_boolean.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct { unsigned terms, literals; } cost;
static cost oracle[65536];
static unsigned cases, cared_rows;

static bool less(cost a,cost b)
{ return a.terms<b.terms || (a.terms==b.terms && a.literals<b.literals); }

/* Independent oracle: enumerate ternary variable patterns (not primes),
 * then shortest paths over unions of full truth masks. */
static void make_oracle(unsigned vars)
{
    unsigned masks[81], lits[81], nc=1, states=1u<<(1u<<vars);
    for(unsigned i=0;i<vars;i++) nc*=3;
    for(unsigned code=0;code<nc;code++) {
        masks[code]=0; lits[code]=0;
        unsigned p=code;
        for(unsigned v=0;v<vars;v++,p/=3) if(p%3!=2) lits[code]++;
        for(unsigned row=0;row<(1u<<vars);row++) {
            bool match=true; p=code;
            for(unsigned v=0;v<vars;v++,p/=3)
                if(p%3!=2 && ((row>>v)&1)!=p%3) match=false;
            if(match) masks[code]|=1u<<row;
        }
    }
    for(unsigned s=0;s<states;s++) oracle[s]=(cost){255,255};
    oracle[0]=(cost){0,0};
    for(unsigned s=0;s<states;s++) {
        assert(oracle[s].terms<255);
        for(unsigned c=0;c<nc;c++) {
            unsigned next=s|masks[c];
            cost option={oracle[s].terms+1,oracle[s].literals+lits[c]};
            if(less(option,oracle[next])) oracle[next]=option;
        }
    }
}

static cost with_dontcares(unsigned target,unsigned dc)
{
    cost best={255,255};
    unsigned sub=dc;
    for(;;) {
        cost candidate=oracle[target|sub];
        if(less(candidate,best)) best=candidate;
        if(!sub) break;
        sub=(sub-1)&dc;
    }
    return best;
}

/* Independent evaluator for text emitted by the formatter. */
static int literal(const char **p,unsigned vars,unsigned row)
{
    char name=*(*p)++;
    assert(name>='A' && name<'A'+(int)vars);
    int value=(row>>(vars-1-(unsigned)(name-'A')))&1;
    if(**p=='\'') { value=!value; (*p)++; }
    return value;
}

static int text_value(const char *p,eb_form form,unsigned vars,unsigned row)
{
    if((p[0]=='0'||p[0]=='1') && p[1]=='\0') return p[0]-'0';
    int value=form==EB_SOP?0:1;
    while(*p) {
        int term=form==EB_SOP?1:0;
        if(form==EB_POS) { assert(*p=='('); p++; }
        while(*p && *p!='+' && *p!=')') {
            int v=literal(&p,vars,row);
            if(form==EB_SOP) term &=v;
            else term |=v;
            if(form==EB_POS && *p=='+') p++;
        }
        if(form==EB_POS) { assert(*p==')'); p++; }
        if(form==EB_SOP) value |=term;
        else value &=term;
        if(*p) { assert(*p==(form==EB_SOP?'+':'*')); p++; }
    }
    return value;
}

static void check(unsigned vars,unsigned on,unsigned dc, bool repeat)
{
    unsigned domain=(1u<<(1u<<vars))-1;
    for(int f=EB_SOP;f<=EB_POS;f++) {
        eb_result result;
        assert(eb_minimize(vars,on,dc,(eb_form)f,&result)==0);
        unsigned target=f==EB_SOP?on:domain & ~(on|dc);
        cost expected=with_dontcares(target,dc);
        assert(result.count==expected.terms);
        assert(result.literals==expected.literals);
        for(unsigned i=1;i<result.count;i++) {
            eb_cube a=result.cubes[i-1],b=result.cubes[i];
            assert(a.fixed<b.fixed || (a.fixed==b.fixed && a.bits<b.bits));
        }
        char text[256];
        int n=eb_format(&result,text,sizeof text);
        assert(n>=0 && n==(int)strlen(text));
        assert(eb_format(&result,NULL,0)==n);
        char tiny[2]={'x','x'};
        assert(eb_format(&result,tiny,sizeof tiny)==n && tiny[1]=='\0');
        for(unsigned row=0;row<(1u<<vars);row++) {
            int value=eb_evaluate(&result,row);
            assert(value==0 || value==1);
            assert(text_value(text,(eb_form)f,vars,row)==value);
            if(!(dc & (1u<<row))) {
                assert(value==(int)((on>>row)&1));
                cared_rows++;
            }
        }
        if(repeat) {
            eb_result again;
            assert(eb_minimize(vars,on,dc,(eb_form)f,&again)==0);
            assert(again.count==result.count && again.literals==result.literals);
            for(unsigned i=0;i<again.count;i++)
                assert(again.cubes[i].fixed==result.cubes[i].fixed &&
                       again.cubes[i].bits==result.cubes[i].bits);
        }
        cases++;
    }
}

static void invalid_inputs(void)
{
    eb_result sentinel,copy;
    memset(&sentinel,0xa5,sizeof sentinel); copy=sentinel;
    assert(eb_minimize(1,0,0,EB_SOP,&sentinel)==-1);
    assert(memcmp(&sentinel,&copy,sizeof copy)==0);
    assert(eb_minimize(5,0,0,EB_SOP,&sentinel)==-1);
    assert(eb_minimize(2,1,1,EB_SOP,&sentinel)==-1);
    assert(eb_minimize(2,16,0,EB_SOP,&sentinel)==-1);
    assert(eb_minimize(2,0,16,EB_SOP,&sentinel)==-1);
    assert(eb_minimize(2,0,0,(eb_form)2,&sentinel)==-1);
    assert(eb_minimize(2,0,0,EB_SOP,NULL)==-1);
    assert(memcmp(&sentinel,&copy,sizeof copy)==0);
    assert(eb_evaluate(NULL,0)==-1);
    assert(eb_evaluate(&sentinel,0)==-1);
    assert(eb_format(NULL,NULL,0)==-1);
    assert(eb_format(&sentinel,NULL,0)==-1);
    eb_result good; assert(eb_minimize(2,0,0,EB_SOP,&good)==0);
    assert(eb_evaluate(&good,4)==-1);
    assert(eb_format(&good,NULL,1)==-1);
    char byte='x'; assert(eb_format(&good,&byte,1)==1 && byte=='\0');
}

int main(void)
{
    invalid_inputs();
    /* Independent all-cube subset enumeration for 0x3d yields exactly two
     * optima with cost (3,6): [(3,0),(6,2),(6,4)] and
     * [(5,0),(6,2),(6,4)] in (fixed,bits) order. */
    for(int f=EB_SOP;f<=EB_POS;f++) {
        eb_result tie;
        assert(eb_minimize(3,f==EB_SOP?0x3d:0xc2,0,(eb_form)f,&tie)==0);
        assert(tie.count==3 && tie.literals==6);
        assert(tie.cubes[0].fixed==3 && tie.cubes[0].bits==0);
        assert(tie.cubes[1].fixed==6 && tie.cubes[1].bits==2);
        assert(tie.cubes[2].fixed==6 && tie.cubes[2].bits==4);
    }
    for(unsigned vars=2;vars<=4;vars++) {
        make_oracle(vars);
        unsigned states=1u<<(1u<<vars);
        for(unsigned on=0;on<states;on++) check(vars,on,0,on%997==0);
        printf("%u variables: all %u fully specified functions, SOP/POS optimal\n",vars,states);
        if(vars<4) {
            unsigned ternary=1;
            for(unsigned row=0;row<(1u<<vars);row++) ternary*=3;
            for(unsigned code=0;code<ternary;code++) {
                unsigned on=0,dc=0,p=code;
                for(unsigned row=0;row<(1u<<vars);row++,p/=3) {
                    if(p%3==1) on |=1u<<row;
                    if(p%3==2) dc |=1u<<row;
                }
                check(vars,on,dc,code%991==0);
            }
            printf("%u variables: all %u ternary ON/OFF/DC specifications, SOP/POS optimal\n",vars,ternary);
        }
    }
    unsigned rng=0xec21;
    for(unsigned i=0;i<4096;i++) {
        rng=rng*1664525u+1013904223u;
        unsigned dc=(rng>>8)&65535;
        rng=rng*1664525u+1013904223u;
        unsigned on=(rng>>8)&65535;
        check(4,on & ~dc,dc,i%97==0);
    }
    printf("4 variables: 4096 deterministic randomized ON/OFF/DC specifications, SOP/POS optimal\n");
    eb_result example;
    assert(eb_minimize(4,0x007e,0,EB_SOP,&example)==0);
    assert(example.count==3 && example.literals==9);
    char text[256]; eb_format(&example,text,sizeof text);
    printf("greedy counterexample 0x007e: %s (%u terms, %u literals)\n",text,example.count,example.literals);
    assert(eb_minimize(4,0,65535,EB_SOP,&example)==0);
    assert(example.count==0 && eb_evaluate(&example,0)==0);
    assert(eb_minimize(4,0,65535,EB_POS,&example)==0);
    assert(example.count==0 && eb_evaluate(&example,0)==1);
    printf("PASS: %u minimizations; %u cared-row checks; constants, polarity, invalid inputs, formatting, deterministic ties\n",cases,cared_rows);
    return 0;
}
