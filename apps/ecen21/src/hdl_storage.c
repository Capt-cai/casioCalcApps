/* Thin gint/BFile adapter; callbacks run in OS world. Physical USB not implied. */
#include "hdl_storage.h"
#include <gint/bfile.h>
#include <gint/gint.h>
#include <stdio.h>
static const hg_config *pending_config;
static const hg_document *pending_document;
static hx_result result;
static void path(const char *name,uint16_t out[32])
{
    char ascii[32];snprintf(ascii,sizeof ascii,"\\\\fls0\\%s",name);
    unsigned i=0;for(;ascii[i]&&i<31;i++)out[i]=(unsigned char)ascii[i];out[i]=0;
}
static int lookup(void *context,const char *name)
{
    (void)context;uint16_t p[32];path(name,p);int fd=BFile_Open(p,BFile_ReadOnly);
    if(fd==BFile_EntryNotFound)return 0;
    if(fd<0)return fd;
    int closed=BFile_Close(fd);return closed<0?closed:1;
}
static int create(void *context,const char *name,unsigned bytes)
{ (void)context;uint16_t p[32];path(name,p);int n=(int)bytes;return BFile_Create(p,BFile_File,&n); }
static int open_file(void *context,const char *name)
{ (void)context;uint16_t p[32];path(name,p);return BFile_Open(p,BFile_WriteOnly); }
static int write_file(void *context,int fd,const char *data,unsigned bytes)
{ (void)context;return BFile_Write(fd,data,(int)bytes); }
static int close_file(void *context,int fd)
{ (void)context;return BFile_Close(fd); }
static int remove_file(void *context,const char *name)
{ (void)context;uint16_t p[32];path(name,p);return BFile_Remove(p); }
static int save_file(void)
{
    hx_io io={NULL,lookup,create,open_file,write_file,close_file,remove_file};
    result=hx_export(pending_config,pending_document,&io);return 0;
}
hx_result hdl_storage_export(const hg_config *config,const hg_document *document)
{
    pending_config=config;pending_document=document;
    result=(hx_result){HX_BAD,0,false};gint_world_switch(GINT_CALL(save_file));return result;
}
