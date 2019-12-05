///*****************************************
//   @file   <ZY_HDMIRX.c>
//   @author limingqiang@zhiyang-tech.com
//   @date   2018/12/21
//   @fileversion: ZY_HDMIRX_V1.00
//******************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "ZY_HDMIRX.h"

VI_DEV_ATTR_S HDMIRX_BT1120_INTERLEAVED =
{
    /*interface mode*/
    VI_MODE_BT1120_INTERLEAVED,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},
    
    /* for single/double edge, must be set when double edge*/
    VI_CLK_EDGE_SINGLE_UP,

    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_UVUV,

    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV

};

VI_DEV_ATTR_S HDMIRX_BT1120_STANDARD =
{
    /*interface mode*/
    VI_MODE_BT1120_STANDARD,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
   {0xFF000000,    0xFF0000},
    /* for single/double edge, must be set when double edge*/
	VI_CLK_EDGE_DOUBLE,
    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_UVUV,
    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV,
  /* Data reverse */
    HI_FALSE
};

VI_DEV_ATTR_S HDMIRX_BT656_1MUX =
{
    /*interface mode*/
    VI_MODE_BT656,
    /*work mode, 1/2/4 multiplex*/
    VI_WORK_MODE_1Multiplex,
    /* r_mask    g_mask    b_mask*/
    {0xFF000000,    0xFF0000},

    /* for single/double edge, must be set when double edge*/
    VI_CLK_EDGE_DOUBLE,

    /*AdChnId*/
    {-1, -1, -1, -1},
    /*enDataSeq, just support yuv*/
    VI_INPUT_DATA_UVUV,

    /*sync info*/
    {
    /*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
    VI_VSYNC_PULSE, VI_VSYNC_NEG_HIGH, VI_HSYNC_VALID_SINGNAL,VI_HSYNC_NEG_HIGH,VI_VSYNC_NORM_PULSE,VI_VSYNC_VALID_NEG_HIGH,

    /*timing info*/
    /*hsync_hfb    hsync_act    hsync_hhb*/
    {0,            1920,        0,
    /*vsync0_vhb vsync0_act vsync0_hhb*/
     0,            1080,        0,
    /*vsync1_vhb vsync1_act vsync1_hhb*/
     0,            0,            0}
    },
    /*whether use isp*/
    VI_PATH_BYPASS,
    /*data type*/
    VI_DATA_TYPE_YUV

};

void PRINTF_HDMIRX_INFO(pHDMIRX_INFO TMP_HDMIRX_INFO)
{
	printf("CHIP_ID:%x\r\n",TMP_HDMIRX_INFO->Chip_ID);
    printf("\n**********************Video Information**************************\n");
	printf("Channel_ID:%x\r\n",TMP_HDMIRX_INFO->Channel_ID);
    printf("PCLK:%d\r\n",TMP_HDMIRX_INFO->PCLK);
    printf("H_Total:%d\r\n",TMP_HDMIRX_INFO->H_Total);
    printf("H_Syncwidth:%d\r\n",TMP_HDMIRX_INFO->H_Syncwidth);
    printf("H_Bkporch:%d\r\n",TMP_HDMIRX_INFO->H_Bkporch);
	printf("H_Active:%d\r\n",TMP_HDMIRX_INFO->H_Active);
    printf("H_Syncpol:%d\r\n",TMP_HDMIRX_INFO->H_Syncpol);
    printf("Color:%d\r\n",TMP_HDMIRX_INFO->Color);
    printf("V_Bkporch:%d\r\n",TMP_HDMIRX_INFO->V_Bkporch);
    printf("V_Syncpol:%d\r\n",TMP_HDMIRX_INFO->V_Syncpol);
    printf("V_Syncwidth:%d\r\n",TMP_HDMIRX_INFO->V_Syncwidth);
    printf("V_Total:%d\r\n",TMP_HDMIRX_INFO->V_Total);
	printf("V_Active:%d\r\n",TMP_HDMIRX_INFO->V_Active);
	printf("FrmRate:%d\r\n",TMP_HDMIRX_INFO->FrmRate);
	printf("ScanMode:%d\r\n",TMP_HDMIRX_INFO->ScanMode);
    printf("\n**********************Audio Information**************************\n");
    printf("AudioBitWidth_IN:%d\r\n",TMP_HDMIRX_INFO->AudioBitWidth_IN);
    printf("AudioBitWidth_OUT:%d\r\n",TMP_HDMIRX_INFO->AudioBitWidth_OUT); 
    printf("AudioSampleRate_IN:%d\r\n",TMP_HDMIRX_INFO->AudioSampleRate_IN); 
    printf("AudioSampleRate_OUT:%d\r\n",TMP_HDMIRX_INFO->AudioSampleRate_OUT); 
    printf("AudioStatus:%d\r\n",TMP_HDMIRX_INFO->AudioStatus); 
    
}
/******************************************************************************
* function : to compare HDMI INFO
******************************************************************************/
HI_BOOL IS_HDMIRX_CHANGE(pHDMIRX_INFO Pre_HDMI_INFO, pHDMIRX_INFO TMP_HDMI_INFO)
{
    HI_BOOL HDMI_CHANGE = HI_FALSE;
    if((Pre_HDMI_INFO->H_Active != TMP_HDMI_INFO->H_Active)||\
        (Pre_HDMI_INFO->V_Active != TMP_HDMI_INFO->V_Active)||\
        (Pre_HDMI_INFO->ScanMode != TMP_HDMI_INFO->ScanMode)||\
        (Pre_HDMI_INFO->FrmRate != TMP_HDMI_INFO->FrmRate))
    {
        HDMI_CHANGE = HI_TRUE;
        memcpy(Pre_HDMI_INFO,TMP_HDMI_INFO,sizeof(HDMIRX_INFO));
        PRINTF_HDMIRX_INFO(TMP_HDMI_INFO);
    }
    else
    {
        HDMI_CHANGE = HI_FALSE;
    }
    return HDMI_CHANGE;
}

/*****************************************************************************
* function : according HDMI parameter, to set vi type
*****************************************************************************/
HI_S32 HDMIRX_VI_Mode2Param(HDMIRX_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
{
    switch (enViMode)
    {
        case HDMIRX_VI_MODE_1_BT656:
            pstViParam->s32ViDevCnt = 1;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 1;
            pstViParam->s32ViChnInterval = 4;
            break;
            
        case HDMIRX_VI_MODE_2_BT656:
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 2;
            pstViParam->s32ViChnInterval = 8;
            break;
            
        case HDMIRX_VI_MODE_4_BT656:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 4;
            break;
            
        case HDMIRX_VI_MODE_8_BT656:
            pstViParam->s32ViDevCnt = 8;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 4;
            break;
            
        case HDMIRX_VI_MODE_16_BT656:
            pstViParam->s32ViDevCnt = 8;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 16;
            pstViParam->s32ViChnInterval = 2;
            break;
            
        case HDMIRX_VI_MODE_1_BT1120:
            pstViParam->s32ViDevCnt = 1;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 1;
            pstViParam->s32ViChnInterval = 8;
            break;
            
        case HDMIRX_VI_MODE_2_BT1120:
            pstViParam->s32ViDevCnt = 2;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 2;
            pstViParam->s32ViChnInterval = 8;
            break;
            
        case HDMIRX_VI_MODE_4_BT1120:
            pstViParam->s32ViDevCnt = 4;
            pstViParam->s32ViDevInterval = 2;
            pstViParam->s32ViChnCnt = 4;
            pstViParam->s32ViChnInterval = 8;
            break;
            
        case HDMIRX_VI_MODE_8_BT1120:
            pstViParam->s32ViDevCnt = 8;
            pstViParam->s32ViDevInterval = 1;
            pstViParam->s32ViChnCnt = 8;
            pstViParam->s32ViChnInterval = 4;
            break;
            
        default:
            SAMPLE_PRT("ViMode invaild!\n");
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/*****************************************************************************
* function : set VI Size, according HDMIRX parameter
*****************************************************************************/
HI_S32 HDMIRX_VI_Mode2Size(pHDMIRX_INFO TMP_HDMI_INFO, RECT_S *pstCapRect, SIZE_S *pstDestSize)
{
    pstCapRect->s32X = 0;
    pstCapRect->s32Y = 0;

    if((TMP_HDMI_INFO->H_Active > 0) && (TMP_HDMI_INFO->V_Active > 0))
    {
        pstDestSize->u32Height = TMP_HDMI_INFO->V_Active;
        pstDestSize->u32Width  = TMP_HDMI_INFO->H_Active;
        pstCapRect->u32Height  = TMP_HDMI_INFO->V_Active;
        pstCapRect->u32Width = TMP_HDMI_INFO->H_Active;
    }
    else
    {
        pstDestSize->u32Height = 1080;
        pstDestSize->u32Width  = 1920;
        pstCapRect->u32Height  = 1080;
        pstCapRect->u32Width = 1920;
        SAMPLE_PRT("HDMIRX parameter invaild! Size set to 1080P\n");
        //return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
*****************************************************************************/
HI_S32 HDMIRX_VI_StartDev(VI_DEV ViDev, HDMIRX_VI_MODE_E enViMode)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S stViDevAttr;
    memset(&stViDevAttr,0,sizeof(stViDevAttr));

    switch (enViMode)
    {

        case HDMIRX_VI_MODE_1_BT656:
        case HDMIRX_VI_MODE_2_BT656:
        case HDMIRX_VI_MODE_4_BT656:
        case HDMIRX_VI_MODE_8_BT656:
        case HDMIRX_VI_MODE_16_BT656:
            memcpy(&stViDevAttr,&HDMIRX_BT656_1MUX,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        
        case HDMIRX_VI_MODE_1_BT1120:
        case HDMIRX_VI_MODE_2_BT1120:
        case HDMIRX_VI_MODE_4_BT1120:
            memcpy(&stViDevAttr,&HDMIRX_BT1120_STANDARD,sizeof(stViDevAttr));
            //SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
            
        case HDMIRX_VI_MODE_1_BT1120I:
        case HDMIRX_VI_MODE_2_BT1120I:
        case HDMIRX_VI_MODE_4_BT1120I:
        case HDMIRX_VI_MODE_8_BT1120I:
            memcpy(&stViDevAttr,&HDMIRX_BT1120_INTERLEAVED,sizeof(stViDevAttr));
            SAMPLE_COMM_VI_SetMask(ViDev,&stViDevAttr);
            break;
        
        default:
            SAMPLE_PRT("vi input type[%d] is invalid!\n", enViMode);
            return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi chn
*****************************************************************************/
HI_S32 HDMIRX_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, 
    pHDMIRX_INFO TMP_HDMI_INFO, SAMPLE_VI_CHN_SET_E enViChnSet)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;

    /* step  5: config & start vicap dev */
    memcpy(&stChnAttr.stCapRect, pstCapRect, sizeof(RECT_S));
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstTarSize->u32Width;
    stChnAttr.stDestSize.u32Height = pstTarSize->u32Height;
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    stChnAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;   /* sp420 or sp422 */
    stChnAttr.bMirror = (VI_CHN_SET_MIRROR == enViChnSet)?HI_TRUE:HI_FALSE;
    stChnAttr.bFlip = (VI_CHN_SET_FILP == enViChnSet)?HI_TRUE:HI_FALSE;
    stChnAttr.s32SrcFrameRate = -1;
    stChnAttr.s32DstFrameRate = -1;

    if(TMP_HDMI_INFO->ScanMode == ZY_HDMIRX_SCAN_INTERLACED)
    {
        stChnAttr.enScanMode = VI_SCAN_INTERLACED;
    }
    else
    {
        stChnAttr.enScanMode = VI_SCAN_PROGRESSIVE;
    }

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}



/*****************************************************************************
* function : reset vi chn
*****************************************************************************/
HI_S32 HDMIRX_VI_ReSetChn(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam_HDMIRX);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = HDMIRX_VI_Mode2Size(TMP_HDMI_INFO, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Size failed!\n");
        return HI_FAILURE;
    }
#if ZY_Hi3531DV100_1_HDMI
    ViChn = 8;

    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }
#elif ZY_Hi3531DV100_2_HDMI
    ViChn = 8;

    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }
#else
    /*** Stop VI Chn ***/
    for(i=0;i<stViParam_HDMIRX.s32ViChnCnt;i++)
    {
        /* Stop vi phy-chn */
        ViChn = i * stViParam_HDMIRX.s32ViChnInterval;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SAMPLE_COMM_VI_StopChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
    }
#endif

    /*** Start VI Chn ***/
#if ZY_Hi3531DV100_1_HDMI
    ViChn = 8;
    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
#elif ZY_Hi3531DV100_2_HDMI
    ViChn = 8;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
#else 
    for(i=0; i<stViParam_HDMIRX.s32ViChnCnt; i++)
    {
        ViChn = i * stViParam_HDMIRX.s32ViChnInterval;
        
        s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
    }
#endif
    return HI_SUCCESS;
}

/*****************************************************************************
* function : reset vi chn2
*****************************************************************************/
HI_S32 HDMIRX_CHN2_VI_ReSetChn(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam_HDMIRX);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = HDMIRX_VI_Mode2Size(TMP_HDMI_INFO, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Size failed!\n");
        return HI_FAILURE;
    }

#if ZY_Hi3531DV100_2_HDMI
    ViChn = 16;

    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }


    /*** Start VI Chn ***/
    ViChn = 16;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
#endif
    return HI_SUCCESS;
}
#if 0
/*****************************************************************************
* function : star CHN2 vi according to HDMIRX_CHN2
*****************************************************************************/
HI_S32 HDMIRX_CHN2_VI_Start(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam_HDMIRX);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = HDMIRX_VI_Mode2Size(TMP_HDMI_INFO, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
    ViDev = 4;
    s32Ret = HDMIRX_VI_StartDev(ViDev, HDMIRXViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_StartDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }

    /*** Start VI Chn ***/
    ViChn = 16;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
    return HI_SUCCESS;
}
#endif


/*****************************************************************************
* function : star vi according to HDMIRX
*****************************************************************************/
HI_S32 HDMIRX_VI_Start(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam_HDMIRX);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = HDMIRX_VI_Mode2Size(TMP_HDMI_INFO, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
#if ZY_Hi3531DV100_1_HDMI
    ViDev = 2;
    s32Ret = HDMIRX_VI_StartDev(ViDev, HDMIRXViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_StartDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
    
#elif ZY_Hi3531DV100_2_HDMI
    ViDev = 2;
    s32Ret = HDMIRX_VI_StartDev(ViDev, HDMIRXViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_StartDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
#else 
    for(i=0; i<stViParam_HDMIRX.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam_HDMIRX.s32ViDevInterval;
        s32Ret = HDMIRX_VI_StartDev(ViDev, HDMIRXViMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HDMIRX_VI_StartDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
#endif

    /*** Start VI Chn ***/
#if ZY_Hi3531DV100_1_HDMI
    ViChn = 8;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
#elif ZY_Hi3531DV100_2_HDMI
    ViChn = 8;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 
#else 
    for(i=0; i<stViParam_HDMIRX.s32ViChnCnt; i++)
    {
        ViChn = i * stViParam_HDMIRX.s32ViChnInterval;
        
        s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } 
    }
#endif
    return HI_SUCCESS;
}


/*****************************************************************************
* function : star vi according to HDMIRX (CHN2)
*****************************************************************************/
HI_S32 HDMIRX_CHN2_VI_Start(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize;
    RECT_S stCapRect;
    VI_CHN_BIND_ATTR_S stChnBindAttr;
    
    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam_HDMIRX);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    s32Ret = HDMIRX_VI_Mode2Size(TMP_HDMI_INFO, &stCapRect, &stTargetSize);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Size failed!\n");
        return HI_FAILURE;
    }
    
    /*** Start VI Dev ***/
#if ZY_Hi3531DV100_2_HDMI
    ViDev = 4;
    s32Ret = HDMIRX_VI_StartDev(ViDev, HDMIRXViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_StartDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }


    /*** Start VI Chn ***/

    ViChn = 16;

    s32Ret = HDMIRX_VI_StartChn(ViChn, &stCapRect, &stTargetSize, TMP_HDMI_INFO, VI_CHN_SET_NORMAL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("call HDMIRX_VI_StartChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } 

#endif
    return HI_SUCCESS;
}
/*****************************************************************************
* function : stop vi accroding to HDMIRX
*****************************************************************************/
HI_S32 HDMIRX_VI_Stop(HDMIRX_VI_MODE_E HDMIRXViMode)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;

    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
#if ZY_Hi3531DV100_1_HDMI
    ViChn = 8;
    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }
#elif ZY_Hi3531DV100_2_HDMI
    ViChn = 8;
    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }
#else 
    for(i=0;i<stViParam.s32ViChnCnt;i++)
    {
        /* Stop vi phy-chn */
        ViChn = i * stViParam.s32ViChnInterval;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        }
    }
#endif

    /*** Stop VI Dev ***/
#if ZY_Hi3531DV100_1_HDMI
    ViDev = 2;
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
#elif ZY_Hi3531DV100_2_HDMI
    ViDev = 2;
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
#else 
    for(i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        }
    }
#endif
    return HI_SUCCESS;
}

/*****************************************************************************
* function : stop vi accroding to HDMIRX(CHN2)
*****************************************************************************/
HI_S32 HDMIRX_CHN2_VI_Stop(HDMIRX_VI_MODE_E HDMIRXViMode)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    SAMPLE_VI_PARAM_S stViParam;

    /*** get parameter from Sample_Vi_Mode ***/
    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
#if ZY_Hi3531DV100_2_HDMI
    ViChn = 16;
    s32Ret = HI_MPI_VI_DisableChn(ViChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
        return HI_FAILURE;
    }

    /*** Stop VI Dev ***/
    ViDev = 4;
    s32Ret = HI_MPI_VI_DisableDev(ViDev);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
        return HI_FAILURE;
    }
#endif
    return HI_SUCCESS;
}
/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 HDMIRX_VI_BindVpss(HDMIRX_VI_MODE_E HDMIRXViMode)
{
    HI_S32 j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    SAMPLE_VI_PARAM_S stViParam;
    VI_CHN ViChn;

    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    VpssGrp = 0;
#if ZY_Hi3531DV100_1_HDMI
    ViChn = 8;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#elif ZY_Hi3531DV100_2_HDMI
    ViChn = 8;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    VpssGrp ++;
    ViChn = 16;

    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#else
    for (j=0; j<stViParam.s32ViChnCnt; j++)
    {
        ViChn = j * stViParam.s32ViChnInterval;

        stSrcChn.enModId = HI_ID_VIU;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = ViChn;
    
        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = VpssGrp;
        stDestChn.s32ChnId = 0;
    
        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
        
        VpssGrp ++;
    }
#endif
    return HI_SUCCESS;
}


/*****************************************************************************
* function : Vi chn unbind vpss group
*****************************************************************************/
HI_S32 HDMIRX_VI_UnBindVpss(HDMIRX_VI_MODE_E HDMIRXViMode)
{
    HI_S32 i, j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    SAMPLE_VI_PARAM_S stViParam;
    VI_DEV ViDev;
    VI_CHN ViChn;

    s32Ret = HDMIRX_VI_Mode2Param(HDMIRXViMode, &stViParam);
    if (HI_SUCCESS !=s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Mode2Param failed!\n");
        return HI_FAILURE;
    }
    
    VpssGrp = 0;    
#if ZY_Hi3531DV100_1_HDMI
    ViDev = 2;
    ViChn = 8;
    
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = ViDev;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#elif ZY_Hi3531DV100_2_HDMI
    ViDev = 2;
    ViChn = 8;
    
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = ViDev;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    VpssGrp ++;
    ViDev = 4;
    ViChn = 16;
    
    stSrcChn.enModId = HI_ID_VIU;
    stSrcChn.s32DevId = ViDev;
    stSrcChn.s32ChnId = ViChn;

    stDestChn.enModId = HI_ID_VPSS;
    stDestChn.s32DevId = VpssGrp;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
#else
    for (i=0; i<stViParam.s32ViDevCnt; i++)
    {
        ViDev = i * stViParam.s32ViDevInterval;

        for (j=0; j<stViParam.s32ViChnCnt; j++)
        {
            ViChn = j * stViParam.s32ViChnInterval;
            
            stSrcChn.enModId = HI_ID_VIU;
            stSrcChn.s32DevId = ViDev;
            stSrcChn.s32ChnId = ViChn;
        
            stDestChn.enModId = HI_ID_VPSS;
            stDestChn.s32DevId = VpssGrp;
            stDestChn.s32ChnId = 0;
        
            s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }
            
            VpssGrp ++;
        }
    }
#endif
    return HI_SUCCESS;
}
HI_S32 HDMIRX_VO_StartChn(VO_LAYER VoLayer, SAMPLE_VO_MODE_E enMode)
{
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32WndNum = 0;
    HI_U32 u32Square = 0;
    HI_U32 u32Width = 0;
    HI_U32 u32Height = 0;
    VO_CHN_ATTR_S stChnAttr;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    
    switch (enMode)
    {
        case VO_MODE_1MUX:
            u32WndNum = 1;
            u32Square = 1;
            break;
        case VO_MODE_4MUX:
            u32WndNum = 4;
            u32Square = 2;
            break;
        case VO_MODE_2MUX:
            u32WndNum = 2;
            u32Square = 1;
            break;
        case VO_MODE_9MUX:
            u32WndNum = 9;
            u32Square = 3;
            break;
        case VO_MODE_16MUX:
            u32WndNum = 16;
            u32Square = 4;
            break;            
        default:
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_GetVideoLayerAttr(VoLayer, &stLayerAttr);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }
    u32Width = stLayerAttr.stImageSize.u32Width;
    u32Height = stLayerAttr.stImageSize.u32Height;
#if ZY_Hi3531DV100_2_HDMI
    i = 0;
    
    stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/2) * (i%2), 2);
    stChnAttr.stRect.s32Y       = 540/2;
    stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/2, 2);
    stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/2, 2);
    stChnAttr.u32Priority       = 0;
    stChnAttr.bDeflicker        = (SAMPLE_VO_LAYER_VSD0 == VoLayer) ? HI_TRUE : HI_FALSE;

    s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s(%d):failed with %#x!\n",\
               __FUNCTION__,__LINE__,  s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    }

    i++;
    stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/2) * (i%2), 2);
    stChnAttr.stRect.s32Y       = 540/2;
    stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/2, 2);
    stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/2, 2);
    stChnAttr.u32Priority       = 0;
    stChnAttr.bDeflicker        = (SAMPLE_VO_LAYER_VSD0 == VoLayer) ? HI_TRUE : HI_FALSE;

    s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
    if (s32Ret != HI_SUCCESS)
    {
    printf("%s(%d):failed with %#x!\n",\
           __FUNCTION__,__LINE__,  s32Ret);
    return HI_FAILURE;
    }

    s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
    if (s32Ret != HI_SUCCESS)
    {
    SAMPLE_PRT("failed with %#x!\n", s32Ret);
    return HI_FAILURE;
    }
#else
    for (i=0; i<u32WndNum; i++)
    {
        stChnAttr.stRect.s32X       = ALIGN_BACK((u32Width/u32Square) * (i%u32Square), 2);
        stChnAttr.stRect.s32Y       = ALIGN_BACK((u32Height/u32Square) * (i/u32Square), 2);
        stChnAttr.stRect.u32Width   = ALIGN_BACK(u32Width/u32Square, 2);
        stChnAttr.stRect.u32Height  = ALIGN_BACK(u32Height/u32Square, 2);
        stChnAttr.u32Priority       = 0;
        stChnAttr.bDeflicker        = (SAMPLE_VO_LAYER_VSD0 == VoLayer) ? HI_TRUE : HI_FALSE;

        s32Ret = HI_MPI_VO_SetChnAttr(VoLayer, i, &stChnAttr);
        if (s32Ret != HI_SUCCESS)
        {
            printf("%s(%d):failed with %#x!\n",\
                   __FUNCTION__,__LINE__,  s32Ret);
            return HI_FAILURE;
        }

        s32Ret = HI_MPI_VO_EnableChn(VoLayer, i);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        }
    }
#endif
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */











