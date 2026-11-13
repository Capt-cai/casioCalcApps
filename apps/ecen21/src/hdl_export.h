#ifndef ECEN21_HDL_EXPORT_H
#define ECEN21_HDL_EXPORT_H
#include "hdl.h"
/* IO callbacks return negative errors. write returns actual bytes; never overwrite. */
typedef struct {
    void *context;
    int (*exists)(void *,const char *);
    int (*create)(void *,const char *,unsigned);
    int (*open)(void *,const char *);
    int (*write)(void *,int,const char *,unsigned);
    int (*close)(void *,int);
    int (*remove)(void *,const char *);
} hx_io;
typedef enum { HX_OK,HX_BAD,HX_EXISTS,HX_LOOKUP,HX_CREATE,HX_OPEN,HX_WRITE,HX_CLOSE } hx_status;
typedef struct { hx_status status;int io_error;bool cleanup_failed; } hx_result;
hx_result hx_export(const hg_config *config,const hg_document *doc,const hx_io *io);
const char *hx_message(hx_status status);
#endif
