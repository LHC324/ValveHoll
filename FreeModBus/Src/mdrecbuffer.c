
#include "mdrecbuffer.h"
#include <stdlib.h>
#include <string.h>

#if(USER_MODBUS_LIB)
/*
    mdClearReceiveBuffer
        @handler 句柄
        @return
    复位接收缓冲
*/
mdVOID mdClearReceiveBuffer(ReceiveBufferHandle handler)
{
    handler->count = 0;
    memset(handler->buf,0,MODBUS_PDU_SIZE_MAX);
}

/*
    mdCreateReceiveBuffer
        @handler 句柄
        @return
    创建并初始化接收缓冲
*/
mdSTATUS mdCreateReceiveBuffer(ReceiveBufferHandle *handler)
{
    (*handler) = (ReceiveBufferHandle)malloc(sizeof(struct ReceiveBuffer));
    if(!handler){
        free(handler);
        return mdFALSE;
    }
    mdClearReceiveBuffer(*handler);
    return mdTRUE;
}

/*
    mdDestoryReceiveBuffer
        @handler 句柄
        @return
    销毁接收缓冲，释放内存
*/
mdVOID mdDestoryReceiveBuffer(ReceiveBufferHandle *handler)
{
    free(*handler);
    (*handler) = NULL;
}
#endif

