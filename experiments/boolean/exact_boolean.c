#include "exact_boolean.h"
#include <stdbool.h>
#include <string.h>

typedef struct { eb_cube cube; uint16_t rows; unsigned literals; } candidate;
typedef struct {
    candidate primes[81];
    unsigned count, best_count, best_literals;
    unsigned chosen[16], best[16];
} search;

static unsigned pop(unsigned x)
{
    unsigned n=0;
    for(; x; x &= x-1) n++;
    return n;
}

static uint16_t cube_rows(unsigned variables, unsigned fixed, unsigned bits)
{
    uint16_t rows=0;
    for(unsigned r=0; r<(1u<<variables); r++)
        if((r & fixed)==bits) rows |= (uint16_t)(1u<<r);
    return rows;
}

static bool canonical_less(const unsigned *a, const unsigned *b, unsigned n)
{
    for(unsigned i=0;i<n;i++) {
        if(a[i]!=b[i]) return a[i]<b[i];
    }
    return false;
}

static void visit(search *s, uint16_t need, unsigned depth, unsigned literals)
{
    if(!need) {
        unsigned ordered[16];
        memcpy(ordered,s->chosen,depth*sizeof *ordered);
        for(unsigned i=1;i<depth;i++) {
            unsigned v=ordered[i],j=i;
            while(j && ordered[j-1]>v) { ordered[j]=ordered[j-1]; j--; }
            ordered[j]=v;
        }
        if(depth<s->best_count || (depth==s->best_count &&
           (literals<s->best_literals || (literals==s->best_literals &&
            canonical_less(ordered,s->best,depth))))) {
            s->best_count=depth; s->best_literals=literals;
            memcpy(s->best,ordered,depth*sizeof *ordered);
        }
        return;
    }
    if(depth>=s->best_count) return;
    unsigned max_cover=0;
    for(unsigned i=0;i<s->count;i++) {
        unsigned n=pop(s->primes[i].rows & need);
        if(n>max_cover) max_cover=n;
    }
    if(!max_cover || depth+(pop(need)+max_cover-1)/max_cover>s->best_count) return;

    /* Every complete cover must choose some cube covering this row.
     * Choosing the most constrained remaining row shrinks branching. */
    unsigned row=0, fewest=82;
    for(unsigned r=0;r<16;r++) if(need & (1u<<r)) {
        unsigned n=0;
        for(unsigned i=0;i<s->count;i++) if(s->primes[i].rows & (1u<<r)) n++;
        if(n<fewest) { row=r; fewest=n; }
    }
    for(unsigned i=0;i<s->count;i++) if(s->primes[i].rows & (1u<<row)) {
        s->chosen[depth]=i;
        visit(s,(uint16_t)(need & ~s->primes[i].rows),depth+1,
              literals+s->primes[i].literals);
    }
}

int eb_minimize(unsigned variables, uint32_t on, uint32_t dc,
                eb_form form, eb_result *output)
{
    if(!output || variables<2 || variables>4 || (form!=EB_SOP && form!=EB_POS)) return -1;
    uint32_t domain=(1u<<(1u<<variables))-1u;
    if((on & dc) || ((on|dc) & ~domain)) return -1;
    uint16_t target=(uint16_t)(form==EB_SOP ? on : domain & ~(on|dc));
    uint16_t allowed=(uint16_t)(target|dc);
    eb_result result={.variables=(uint8_t)variables,.form=form};
    if(!target) { *output=result; return 0; }
    search s={.best_count=17,.best_literals=65};
    unsigned full=(1u<<variables)-1u;
    for(unsigned fixed=0;fixed<=full;fixed++)
        for(unsigned bits=0;bits<=full;bits++) {
            if(bits & ~fixed) continue;
            uint16_t rows=cube_rows(variables,fixed,bits);
            if((rows & ~allowed) || !(rows & target)) continue;
            bool prime=true;
            for(unsigned bit=1;bit<=full;bit<<=1) if(fixed & bit) {
                if(!(cube_rows(variables,fixed & ~bit,bits & ~bit) & ~allowed)) {
                    prime=false; break;
                }
            }
            if(prime) s.primes[s.count++]=(candidate){
                .cube={(uint8_t)bits,(uint8_t)fixed},.rows=rows,.literals=pop(fixed)};
        }
    visit(&s,target,0,0);
    result.count=(uint8_t)s.best_count; result.literals=(uint8_t)s.best_literals;
    for(unsigned i=0;i<s.best_count;i++) result.cubes[i]=s.primes[s.best[i]].cube;
    *output=result;
    return 0;
}

static bool valid_result(const eb_result *r)
{
    if(!r || r->variables<2 || r->variables>4 || r->count>16 ||
       (r->form!=EB_SOP && r->form!=EB_POS)) return false;
    unsigned full=(1u<<r->variables)-1u;
    unsigned literals=0;
    for(unsigned i=0;i<r->count;i++) {
        if((r->cubes[i].fixed & ~full) || (r->cubes[i].bits & ~r->cubes[i].fixed)) return false;
        literals+=pop(r->cubes[i].fixed);
    }
    return literals==r->literals;
}

int eb_evaluate(const eb_result *r, unsigned row)
{
    if(!valid_result(r) || row>=(1u<<r->variables)) return -1;
    bool covered=false;
    for(unsigned i=0;i<r->count;i++)
        if((row & r->cubes[i].fixed)==r->cubes[i].bits) covered=true;
    return r->form==EB_SOP ? covered : !covered;
}

static void append(char c, char *buffer, size_t capacity, size_t *n)
{
    if(*n+1<capacity) buffer[*n]=c;
    (*n)++;
}

int eb_format(const eb_result *r, char *buffer, size_t capacity)
{
    if(!valid_result(r) || (!buffer && capacity)) return -1;
    size_t n=0;
    if(!r->count) append(r->form==EB_SOP?'0':'1',buffer,capacity,&n);
    for(unsigned i=0;i<r->count;i++) {
        eb_cube c=r->cubes[i];
        if(i) append(r->form==EB_SOP?'+':'*',buffer,capacity,&n);
        if(!c.fixed) { append(r->form==EB_SOP?'1':'0',buffer,capacity,&n); continue; }
        if(r->form==EB_POS) append('(',buffer,capacity,&n);
        bool first=true;
        for(unsigned v=0;v<r->variables;v++) {
            unsigned bit=1u<<(r->variables-1-v);
            if(!(c.fixed & bit)) continue;
            if(!first && r->form==EB_POS) append('+',buffer,capacity,&n);
            append((char)('A'+v),buffer,capacity,&n);
            bool complemented=r->form==EB_SOP ? !(c.bits & bit) : (c.bits & bit)!=0;
            if(complemented) append('\'',buffer,capacity,&n);
            first=false;
        }
        if(r->form==EB_POS) append(')',buffer,capacity,&n);
    }
    if(capacity) buffer[n<capacity?n:capacity-1]='\0';
    return (int)n;
}
