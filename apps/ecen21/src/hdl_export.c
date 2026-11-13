#include "hdl_export.h"
#include <string.h>
const char *hx_message(hx_status s)
{
    static const char *const message[]={"Exported source bytes successfully","Generate code before export","File exists; select another slot","Cannot inspect storage","Cannot create file; storage may be full","Cannot open new file","Incomplete file write","Cannot close file"};
    return (unsigned)s<=HX_CLOSE?message[s]:"Unknown export error";
}
hx_result hx_export(const hg_config *c,const hg_document *d,const hx_io *io)
{
    hx_result result={HX_BAD,0,false};char name[16];
    if(!d||!hg_matches(c,d)||d->length==0||d->length>=HG_CAPACITY||(d->length&1u)||(d->code[d->length]!=0||memchr(d->code,0,d->length)!=NULL)||!hg_filename(c,name,sizeof name)||!io||!io->exists||!io->create||!io->open||!io->write||!io->close||!io->remove)return result;
    int rc=io->exists(io->context,name);
    if(rc!=0){result.status=rc>0?HX_EXISTS:HX_LOOKUP;result.io_error=rc;return result;}
    rc=io->create(io->context,name,d->length);
    if(rc<0){result.status=HX_CREATE;result.io_error=rc;return result;}
    int fd=io->open(io->context,name);
    if(fd<0){result.status=HX_OPEN;result.io_error=fd;}
    else {
        int written=io->write(io->context,fd,d->code,d->length);
        int closed=io->close(io->context,fd);
        if(written<0||(unsigned)written!=d->length){result.status=HX_WRITE;result.io_error=written;}
        else if(closed<0){result.status=HX_CLOSE;result.io_error=closed;}
        else {result.status=HX_OK;return result;}
    }
    result.cleanup_failed=io->remove(io->context,name)<0;
    return result;
}
