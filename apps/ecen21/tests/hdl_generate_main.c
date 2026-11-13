/* Host-only code emission driver; argv parameters are bounded by production. */
#include "hdl.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv)
{
    if(argc!=12)return 2;
    unsigned v[11];for(unsigned i=0;i<11;i++)v[i]=(unsigned)strtoul(argv[i+1],NULL,10);
    hg_config c={.kind=(hg_kind)v[0],.width=v[1],.inputs=v[2],.modulus=v[3],.seed=v[4],.slot=v[5],.falling=v[6]!=0,.reset=(hg_reset)v[7],.reset_high=v[8]!=0,.reset_value=v[9]!=0,.right=v[10]!=0};
    static hg_document doc;if(!hg_generate(&c,&doc)){fprintf(stderr,"%s\n",doc.error);return 1;}
    return fwrite(doc.code,1,doc.length,stdout)==doc.length?0:1;
}
