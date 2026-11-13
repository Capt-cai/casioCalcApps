#include "numeric.h"
#include "numeric_checks.h"
#include <stdio.h>
#include <string.h>

#define CHECK(condition) do { ++*checks; if(!(condition)) return __LINE__; } while(0)

/* Oracle uses wider signed mathematical arithmetic and range comparisons,
 * independently of production's width masks and sign-bit overflow formula. */
static int64_t interpreted(uint32_t value, unsigned width)
{
    int64_t modulus = INT64_C(1) << width;
    return value >= (uint64_t)modulus / 2 ? (int64_t)value - modulus : value;
}

static int arithmetic_check(unsigned *checks, unsigned width, uint32_t a,
    uint32_t b, bool signed_mode, nm_operation operation)
{
    nm_state s = {.width = width, .base = 16, .signed_mode = signed_mode,
        .operation = operation, .operand = {a,b}, .operand_valid = {true,true}};
    int64_t modulus = INT64_C(1) << width;
    int64_t unsigned_math = operation == NM_ADD ? (int64_t)a + b : (int64_t)a - b;
    int64_t signed_math = operation == NM_ADD ? interpreted(a,width) + interpreted(b,width) :
        interpreted(a,width) - interpreted(b,width);
    uint32_t expected = (uint32_t)((unsigned_math % modulus + modulus) % modulus);
    bool carry = operation == NM_ADD && unsigned_math >= modulus;
    bool borrow = operation == NM_SUB && unsigned_math < 0;
    bool overflow = signed_math < -(modulus / 2) || signed_math >= modulus / 2;
    CHECK(nm_solve(&s));
    CHECK(s.solved && !s.error[0]);
    CHECK(s.result == expected);
    CHECK(s.carry == carry);
    CHECK(s.borrow == borrow);
    CHECK(s.overflow == overflow);
    CHECK(s.wrapped == (signed_mode ? overflow : carry || borrow));
    CHECK((uint64_t)s.result < (uint64_t)modulus);
    return 0;
}

static int conversion_check(unsigned *checks, unsigned width, uint32_t raw,
    unsigned base, bool signed_mode)
{
    nm_state s;
    nm_init(&s);
    CHECK(nm_set_width(&s,width));
    CHECK(nm_set_signed(&s,signed_mode));
    CHECK(nm_set_base(&s,base));
    char text[48], expected[48];
    int length = nm_format(raw,width,base,signed_mode,text,sizeof text);
    if(base == 10) {
        if(signed_mode) snprintf(expected,sizeof expected,"%ld",(long)interpreted(raw,width));
        else snprintf(expected,sizeof expected,"%lu",(unsigned long)raw);
    }
    else {
        unsigned digits = base == 2 ? width : (width + 3) / 4;
        unsigned step = base == 2 ? 1 : 4;
        for(unsigned i=0;i<digits;i++)
            expected[i]="0123456789ABCDEF"[(raw >> ((digits-i-1)*step)) & (base-1)];
        expected[digits]='\0';
    }
    CHECK(length == (int)strlen(expected) && strcmp(text,expected)==0);
    CHECK(nm_format(raw,width,base,signed_mode,NULL,0)==length);
    CHECK(nm_set_input(&s,0,text));
    CHECK(s.operand[0]==raw && s.operand_valid[0] && !s.solved);
    char tiny[2]={'x','x'};
    CHECK(nm_format(raw,width,base,signed_mode,tiny,sizeof tiny)==length);
    CHECK(tiny[0]==text[0] && tiny[1]=='\0');
    char one='x';
    CHECK(nm_format(raw,width,base,signed_mode,&one,1)==length && one=='\0');
    return 0;
}

int nm_run_checks(unsigned *checks)
{
    *checks=0;
    nm_state s; nm_init(&s);
    CHECK(s.width==8 && s.base==10 && !s.signed_mode && s.operation==NM_ADD);
    CHECK(s.operand_valid[0] && s.operand_valid[1] && s.operand[0]==255 && s.operand[1]==1);
    CHECK(!s.solved && !s.error[0]);
    CHECK(nm_solve(&s) && s.result==0 && s.carry && !s.borrow && !s.overflow && s.wrapped);
    CHECK(nm_set_signed(&s,true) && strcmp(s.input[0],"-1")==0 && !s.solved);
    CHECK(nm_set_input(&s,0,"127") && nm_solve(&s));
    CHECK(s.result==128 && s.overflow && !s.carry && s.wrapped);
    CHECK(nm_set_input(&s,0,"-128"));
    nm_set_operation(&s,NM_SUB);
    CHECK(!s.solved && !s.overflow && !s.carry && !s.borrow);
    CHECK(nm_solve(&s) && s.result==127 && s.overflow && !s.borrow && s.wrapped);
    CHECK(nm_set_input(&s,0,"0") && nm_solve(&s));
    CHECK(s.result==255 && s.borrow && !s.carry && !s.overflow && !s.wrapped);
    CHECK(nm_set_signed(&s,false) && nm_solve(&s) && s.wrapped);

    nm_init(&s); CHECK(nm_set_signed(&s,true));
    CHECK(nm_set_input(&s,0,"-5"));
    char text[48];
    CHECK(nm_format(s.operand[0],8,16,true,text,sizeof text)==2 && !strcmp(text,"FB"));
    CHECK(nm_format(s.operand[0],8,2,true,text,sizeof text)==8 && !strcmp(text,"11111011"));
    CHECK(nm_set_signed(&s,false) && !strcmp(s.input[0],"251"));
    CHECK(s.operand[0]==251 && nm_set_base(&s,16) && !strcmp(s.input[0],"FB"));
    CHECK(nm_set_signed(&s,true) && nm_set_width(&s,16));
    CHECK(s.operand[0]==65531 && !strcmp(s.input[0],"FFFB") && !s.truncated);
    CHECK(nm_set_width(&s,8) && s.operand[0]==251 && !s.truncated);
    CHECK(nm_set_signed(&s,false) && nm_set_width(&s,16));
    CHECK(s.operand[0]==251 && !strcmp(s.input[0],"00FB"));
    CHECK(nm_set_signed(&s,true) && nm_set_width(&s,8));
    CHECK(s.operand[0]==251 && s.truncated);
    CHECK(nm_solve(&s) && s.truncated);
    CHECK(nm_solve(&s) && s.truncated);
    CHECK(nm_set_width(&s,8) && s.truncated);
    CHECK(nm_set_base(&s,10) && !s.truncated && !strcmp(s.input[0],"-5"));
    CHECK(nm_set_width(&s,16) && nm_set_input(&s,0,"32767") && nm_solve(&s));
    CHECK(s.result==32768 && s.overflow && !s.carry);
    CHECK(nm_set_width(&s,32) && nm_set_input(&s,0,"-2147483648"));
    CHECK(s.operand[0]==UINT32_C(0x80000000));
    CHECK(nm_format(s.operand[0],32,10,true,text,sizeof text)==11 && !strcmp(text,"-2147483648"));
    CHECK(nm_set_signed(&s,false) && !strcmp(s.input[0],"2147483648"));
    CHECK(nm_set_input(&s,0,"4294967295"));
    nm_set_operation(&s,NM_ADD);
    CHECK(nm_solve(&s) && !s.result && s.carry && !s.overflow);
    CHECK(nm_set_signed(&s,true) && nm_set_input(&s,0,"2147483647") && nm_solve(&s));
    CHECK(s.result==UINT32_C(0x80000000) && s.overflow && !s.carry);

    struct { unsigned width, base; bool sign; const char *bad; } invalid[] = {
        {8,10,false,""}, {8,10,false,"-1"}, {8,10,false,"256"},
        {8,10,true,"128"}, {8,10,true,"-129"}, {8,10,true,"-"},
        {8,10,true,"+1"}, {8,10,true,"1 "}, {8,10,true,"1.0"},
        {8,2,false,"2"}, {8,2,false,"000000000"}, {8,2,true,"-1"},
        {8,16,false,"100"}, {8,16,true,"0xFF"}, {8,16,false,"GG"},
        {16,10,false,"65536"}, {16,10,true,"-32769"},
        {32,10,false,"4294967296"}, {32,10,true,"2147483648"},
        {32,10,true,"-2147483649"}, {32,16,false,"0FFFFFFFF"},
    };
    for(unsigned i=0;i<sizeof invalid/sizeof *invalid;i++) {
        nm_init(&s);
        CHECK(nm_set_width(&s,invalid[i].width));
        CHECK(nm_set_signed(&s,invalid[i].sign));
        CHECK(nm_set_base(&s,invalid[i].base));
        CHECK(nm_solve(&s));
        CHECK(!nm_set_input(&s,0,invalid[i].bad));
        CHECK(!s.operand_valid[0] && !s.operand[0] && !s.solved && !s.result);
        CHECK(!s.carry && !s.borrow && !s.overflow && !s.wrapped && s.error[0]);
        CHECK(!nm_solve(&s) && !s.solved);
        unsigned base=s.base,width=s.width; bool sign=s.signed_mode;
        CHECK(!nm_set_base(&s,base==10?16:10) && s.base==base);
        CHECK(!nm_set_width(&s,width==8?16:8) && s.width==width);
        CHECK(!nm_set_signed(&s,!sign) && s.signed_mode==sign);
        CHECK(nm_set_input(&s,0,"0") && s.operand_valid[0]);
        CHECK(nm_solve(&s));
    }
    nm_init(&s); nm_clear(&s,0);
    CHECK(!s.operand_valid[0] && !s.input[0][0] && !s.solved);
    CHECK(!nm_set_base(&s,2) && s.base==10);
    CHECK(nm_append(&s,0,'1') && nm_append(&s,0,'2') && nm_append(&s,0,'7'));
    CHECK(!strcmp(s.input[0],"127") && s.operand[0]==127);
    CHECK(nm_solve(&s));
    CHECK(!nm_append(&s,0,'x') && !strcmp(s.input[0],"127x") && !s.operand_valid[0]);
    nm_backspace(&s,0);
    CHECK(s.operand_valid[0] && s.operand[0]==127 && !s.solved);
    CHECK(nm_set_base(&s,16)); nm_clear(&s,0);
    CHECK(nm_append(&s,0,'a') && s.operand[0]==10);
    CHECK(nm_append(&s,0,'f') && s.operand[0]==175);
    CHECK(!nm_append(&s,0,'0') && !s.operand_valid[0]);
    nm_backspace(&s,0);
    CHECK(s.operand_valid[0] && s.operand[0]==175);
    nm_init(&s); CHECK(nm_set_signed(&s,true)); nm_clear(&s,0);
    CHECK(!nm_append(&s,0,'-') && !s.operand_valid[0]);
    CHECK(nm_append(&s,0,'5') && s.operand[0]==251);
    nm_init(&s); char overlong[64]; memset(overlong,'0',63); overlong[63]='\0';
    CHECK(!nm_set_input(&s,0,overlong) && strlen(s.input[0])==34);
    CHECK(!s.operand_valid[0] && !nm_solve(&s));
    nm_backspace(&s,0);
    CHECK(s.operand_valid[0] && s.operand[0]==0);
    CHECK(nm_append(&s,0,'0') && strlen(s.input[0])==34);
    CHECK(!nm_append(&s,0,'0') && !s.operand_valid[0]);
    CHECK(!nm_set_input(&s,2,"1") && s.error[0]);
    CHECK(!nm_set_input(&s,0,NULL) && !s.operand_valid[0]);
    CHECK(!nm_set_input(NULL,0,"1") && !nm_solve(NULL));
    nm_init(&s); CHECK(!nm_set_base(&s,8) && s.base==10);
    CHECK(!nm_set_width(&s,64) && s.width==8);
    CHECK(nm_solve(&s)); nm_set_operation(&s,(nm_operation)99);
    CHECK(!s.solved && s.operation==NM_ADD && s.error[0]);
    CHECK(nm_mask(0)==0 && nm_mask(64)==0 && nm_mask(32)==UINT32_MAX);
    CHECK(nm_signed_value(128,8)==-128 && nm_signed_value(UINT32_MAX,32)==-1);
    CHECK(nm_signed_value(UINT32_C(0x80000000),32)==INT64_C(-2147483648));
    CHECK(nm_format(0,33,10,false,text,sizeof text)==-1);
    CHECK(nm_format(0,8,8,false,text,sizeof text)==-1);
    CHECK(nm_format(0,8,10,false,NULL,1)==-1);
    CHECK(nm_format(UINT32_MAX,8,16,false,text,sizeof text)==2 && !strcmp(text,"FF"));

    unsigned bases[]={2,10,16};
    for(unsigned raw=0;raw<256;raw++) for(unsigned b=0;b<3;b++)
        for(unsigned sign=0;sign<2;sign++) {
            int failed=conversion_check(checks,8,raw,bases[b],sign!=0);
            if(failed) return failed;
        }
    uint32_t rng=UINT32_C(0xec210003);
    for(unsigned w=1;w<=32;w++) {
        uint32_t mask=nm_mask(w),sign=UINT32_C(1)<<(w-1);
        uint32_t values[]={0,1,sign-1,sign,mask-1,mask};
        for(unsigned i=0;i<6;i++) {
            for(unsigned b=0;b<3;b++) for(unsigned mode=0;mode<2;mode++) {
                int failed=conversion_check(checks,w,values[i],bases[b],mode!=0);
                if(failed) return failed;
            }
            for(unsigned j=0;j<6;j++) for(unsigned op=0;op<2;op++)
                for(unsigned mode=0;mode<2;mode++) {
                    int failed=arithmetic_check(checks,w,values[i],values[j],mode!=0,(nm_operation)op);
                    if(failed) return failed;
                }
        }
        for(unsigned i=0;i<32;i++) {
            rng=rng*UINT32_C(1664525)+UINT32_C(1013904223); uint32_t a=rng&mask;
            rng=rng*UINT32_C(1664525)+UINT32_C(1013904223); uint32_t b=rng&mask;
            for(unsigned op=0;op<2;op++) for(unsigned mode=0;mode<2;mode++) {
                int failed=arithmetic_check(checks,w,a,b,mode!=0,(nm_operation)op);
                if(failed) return failed;
            }
        }
    }
    /* Every width: parsing limits, partial HEX nibble, extension/truncation. */
    for(unsigned w=1; w<=32; w++) {
        uint64_t modulus=UINT64_C(1)<<w;
        nm_init(&s); CHECK(nm_set_width(&s,w));
        snprintf(text,sizeof text,"%llu",(unsigned long long)(modulus-1));
        CHECK(nm_set_input(&s,0,text));
        snprintf(text,sizeof text,"%llu",(unsigned long long)modulus);
        CHECK(!nm_set_input(&s,0,text) && !s.operand_valid[0] && !s.solved);
        CHECK(nm_set_input(&s,0,"0") && nm_set_signed(&s,true));
        snprintf(text,sizeof text,"%lld",(long long)(modulus/2-1));
        CHECK(nm_set_input(&s,0,text));
        snprintf(text,sizeof text,"%lld",(long long)(modulus/2));
        CHECK(!nm_set_input(&s,0,text));
        snprintf(text,sizeof text,"-%llu",(unsigned long long)(modulus/2));
        CHECK(nm_set_input(&s,0,text));
        snprintf(text,sizeof text,"-%llu",(unsigned long long)(modulus/2+1));
        CHECK(!nm_set_input(&s,0,text));
        CHECK(nm_set_input(&s,0,"-1") && nm_set_base(&s,16));
        CHECK(strlen(s.input[0])==(w+3)/4);
        if(w%4) {
            memset(text,'F',(w+3)/4); text[(w+3)/4]='\0';
            CHECK(!nm_set_input(&s,0,text));
            CHECK(nm_set_input(&s,0,"0"));
        }
        CHECK(nm_set_base(&s,10) && nm_set_input(&s,0,"-1"));
        CHECK(nm_set_width(&s,32) && s.operand[0]==UINT32_MAX && !s.truncated);
        CHECK(nm_set_width(&s,w) && s.operand[0]==modulus-1 && !s.truncated);
        CHECK(nm_set_signed(&s,false) && nm_set_width(&s,32));
        CHECK(s.operand[0]==modulus-1);
    }
    nm_init(&s); CHECK(nm_set_width(&s,5) && s.truncated);
    CHECK(nm_set_signed(&s,true) && nm_set_input(&s,0,"-8") && nm_set_input(&s,1,"-9"));
    CHECK(nm_solve(&s) && s.result==15 && s.overflow && s.carry);
    CHECK(nm_set_width(&s,1) && s.truncated && !s.solved);
    /* Literal review examples, independent of production-derived formatting. */
    struct { unsigned width; const char *decimal, *binary; } review[] = {
        {4,"-5","1011"}, {10,"478","0111011110"},
        {10,"-281","1011100111"}, {10,"-2","1111111110"},
        {12,"73","000001001001"}, {12,"1906","011101110010"},
        {12,"-95","111110100001"}, {12,"-63","111111000001"}
    };
    for(unsigned i=0;i<sizeof review/sizeof *review;i++) {
        nm_init(&s); CHECK(nm_set_width(&s,review[i].width) && nm_set_signed(&s,true));
        CHECK(nm_set_input(&s,0,review[i].decimal) && nm_set_base(&s,2));
        CHECK(!strcmp(s.input[0],review[i].binary));
        CHECK(nm_set_input(&s,0,review[i].binary) && nm_set_base(&s,10));
        CHECK(!strcmp(s.input[0],review[i].decimal));
    }
    nm_init(&s); CHECK(!nm_set_width(&s,0) && s.width==8);
    CHECK(!nm_set_width(&s,33) && s.width==8);
    return 0;
}

#ifdef NM_FULL_HOST_TESTS
int nm_run_exhaustive(unsigned *checks)
{
    *checks=0;
    for(unsigned width=1;width<=8;width++)
    for(unsigned a=0;a<(1u<<width);a++) for(unsigned b=0;b<(1u<<width);b++)
        for(unsigned op=0;op<2;op++) for(unsigned sign=0;sign<2;sign++) {
            int failed=arithmetic_check(checks,width,a,b,sign!=0,(nm_operation)op);
            if(failed) return failed;
        }
    uint32_t rng=UINT32_C(0x3281abcd);
    for(unsigned width=9;width<=32;width++) for(unsigned i=0;i<4096;i++) {
        rng=rng*UINT32_C(1664525)+UINT32_C(1013904223); uint32_t a=rng&nm_mask(width);
        rng=rng*UINT32_C(1664525)+UINT32_C(1013904223); uint32_t b=rng&nm_mask(width);
        for(unsigned op=0;op<2;op++) for(unsigned sign=0;sign<2;sign++) {
            int failed=arithmetic_check(checks,width,a,b,sign!=0,(nm_operation)op);
            if(failed) return failed;
        }
    }
    return 0;
}
#endif
