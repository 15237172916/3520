#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "loadbmp.h"
#include "sample_comm.h"
#include "hi_tde_api.h"
#include "hi_tde_type.h"
#include "osd.h"

HI_S32 SAMPLE_RGN_CreateOverlayForVpss(RGN_HANDLE Handle, HI_U32 u32Num)
{
    HI_S32 i;
    HI_S32 s32Ret;
    MPP_CHN_S stChn;
    RGN_ATTR_S stRgnAttrSet;
    RGN_CHN_ATTR_S stChnAttr;
    
    /*attach the OSD to the vpss*/
    stChn.enModId  = HI_ID_VPSS;
    stChn.s32DevId = 0;
    stChn.s32ChnId = 0;

    for (i=Handle; i<(Handle + u32Num); i++)
    {
        stRgnAttrSet.enType = OVERLAY_RGN;
        stRgnAttrSet.unAttr.stOverlay.enPixelFmt       = PIXEL_FORMAT_RGB_1555;
        stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 1000;
        stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
        stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0;//0x000003e0;

        if (1 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 100;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x0000001f;
        }
        else if (2 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 100;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 100;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x00007c00;
        }
        else if (3 == i%u32Num)
        {
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Width  = 120;
            stRgnAttrSet.unAttr.stOverlay.stSize.u32Height = 120;
            stRgnAttrSet.unAttr.stOverlay.u32BgColor       = 0x000007ff;
        }


        s32Ret = HI_MPI_RGN_Create(i, &stRgnAttrSet);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_Create failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }

        stChnAttr.bShow  = HI_TRUE;
        stChnAttr.enType = OVERLAY_RGN;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 20;
        stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 20;
        stChnAttr.unChnAttr.stOverlayChn.u32BgAlpha   = 0;//255;
        stChnAttr.unChnAttr.stOverlayChn.u32FgAlpha   = 255;
        stChnAttr.unChnAttr.stOverlayChn.u32Layer     = i;
        if (1 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 130;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 330;
        }
        else if (2 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 270;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 300;
        }
        else if (3 == i%4)
        {
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32X = 180;
            stChnAttr.unChnAttr.stOverlayChn.stPoint.s32Y = 400;
        }
        s32Ret = HI_MPI_RGN_AttachToChn(i, &stChn, &stChnAttr);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_RGN_AttachToChn failed! s32Ret: 0x%x.\n", s32Ret);
            return s32Ret;
        }
    }

    return HI_SUCCESS;

}