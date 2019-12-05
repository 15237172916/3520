/******************************************************************************
  A simple program of Hisilicon HI3531 video encode implementation.
  Copyright (C), 2010-2011, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2011-2 Created
******************************************************************************/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "it6801.h"
#include "sample_comm.h"

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_PAL;
#define SAMPLE_YUV_D1_FILEPATH         "SAMPLE_420_D1.yuv"

/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_VENC_Usage(char *sPrgNm)
{
    printf("Usage : %s <index>\n", sPrgNm);
    printf("index:\n");
    printf("\t 0) 4D1 H264 encode.\n");
    printf("\t 1) 1*720p H264 encode.\n");
    printf("\t 2) 1D1 MJPEG encode.\n");
    printf("\t 3) 4D1 JPEG snap with StartRecvPic.\n");
    printf("\t 4) 4D1 JPEG snap with StartRecvPicEx.\n");
	printf("\t 5) 8*Cif JPEG snap.\n");
    printf("\t 6) 1D1 User send pictures for H264 encode.\n");
    printf("\t 7) 4D1 H264 encode with color2grey.\n");
    return;
}

/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void SAMPLE_VENC_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : to process abnormal case - the case of stream venc
******************************************************************************/
void SAMPLE_VENC_StreamHandleSig(HI_S32 signo)
{

    if (SIGINT == signo || SIGTSTP == signo)
    {
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram exit abnormally!\033[0;39m\n");
    }

    exit(0);
}

/******************************************************************************
* function :  4D1 H264 encode
******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_H264(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    PAYLOAD_TYPE_E enPayLoad[2]= {PT_H264, PT_H264};
    PIC_SIZE_E enSize[2] = {PIC_D1, PIC_CIF};
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;
    
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_8D1_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_8D1_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_8D1_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_8D1_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_8D1_2;
    }

    /******************************************
     step 5: select rc mode
    ******************************************/
    while(1)
    {
        printf("please choose rc mode:\n"); 
        printf("\t0) CBR\n"); 
        printf("\t1) VBR\n"); 
        printf("\t2) FIXQP\n"); 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            enRcMode = SAMPLE_RC_CBR;
            break;
        }
        else if ('1' == ch)
        {
            enRcMode = SAMPLE_RC_VBR;
            break;
        }
        else if ('2' == ch)
        {
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        }
        else
        {
            printf("rc mode invaild! please try again.\n");
            continue;
        }
    }
    /******************************************
     step 5: start stream venc (big + little)
    ******************************************/
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main stream **/
        VencGrp = i*2;
        VencChn = i*2;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
                                       gs_enNorm, enSize[0], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_2;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }

        /*** Sub stream **/
        VencGrp ++;
        VencChn ++;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1], \
                                        gs_enNorm, enSize[1], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_8D1_3;
        }
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt*2);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_8D1_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 7: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();
    
END_VENC_8D1_3:
    for (i=0; i<u32ViChnCnt*2; i++)
    {
        VencGrp = i;
        VencChn = i;
        VpssGrp = i/2;
        VpssChn = (VpssGrp%2)?VPSS_PRE0_CHN:VPSS_BSTR_CHN;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_8D1_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_8D1_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_8D1_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


HI_S32 VI_TEST(HI_VOID)
{
    HI_S32 s32Ret; 
    VI_DEV ViDev = 0; 
    VI_CHN ViChn = 0; 
    VI_DEV_ATTR_S stDevAttr; 
    VI_CHN_ATTR_S stChnAttr;
    stDevAttr.enIntfMode = VI_MODE_BT1120_INTERLEAVED;//VI_MODE_BT656; 
    stDevAttr.enWorkMode = VI_WORK_MODE_1Multiplex; 
    stDevAttr.au32CompMask[0] = 0xFF000000; 
    stDevAttr.au32CompMask[1] = 0x00FF0000; 
    stDevAttr.enScanMode = VI_SCAN_INTERLACED; 
    stDevAttr.s32AdChnId[0] = -1; 
    stDevAttr.s32AdChnId[1] = -1; 
    stDevAttr.s32AdChnId[2] = -1; 
    stDevAttr.s32AdChnId[3] = -1;

    VB_CONF_S stVbConf;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    HI_U32 u32ViChnCnt = 1;
    SIZE_S stSize; 
    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S)); //create video buffer pool 

    //a frame picture size
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    stVbConf.u32MaxPoolCnt = 128; 

    /* video buffer*/ //main video buffer 
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 12;
    memset(stVbConf.astCommPool[0].acMmzName,0,
    sizeof(stVbConf.astCommPool[0].acMmzName));
#if 0
    /* hist buf*/ //sub video buffer 
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 4;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));
#endif
    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        //goto END_VENC_1HD_0;
    }

    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stDevAttr); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Set dev attributes failed with error code %#x!\n", s32Ret); return HI_FAILURE;
    }
    s32Ret = HI_MPI_VI_EnableDev(ViDev); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Enable dev failed with error code %#x!\n", s32Ret);
        return HI_FAILURE; 
    }

    stChnAttr.stCapRect.s32X = 0; 
    stChnAttr.stCapRect.s32Y = 0; 
    stChnAttr.stCapRect.u32Width = 1920; 
    stChnAttr.stCapRect.u32Height = 1080; 
    stChnAttr.stDestSize.u32Width = 1920; 
    stChnAttr.stDestSize.u32Height = 1080; 
    stChnAttr.enCapSel = VI_CAPSEL_BOTH;
    stChnAttr.enPixFormat = PIXEL_FORMAT_YUV_SEMIPLANAR_422; 
    stChnAttr.bMirror = HI_FALSE; 
    stChnAttr.bFlip = HI_FALSE; 
    stChnAttr.bChromaResample = HI_FALSE;
    stChnAttr.s32SrcFrameRate = -1; 
    stChnAttr.s32FrameRate = -1;
    s32Ret = HI_MPI_VI_SetChnAttr(ViChn,&stChnAttr); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Set chn attributes failed with error code %#x!\n", s32Ret); 
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VI_EnableChn(ViChn); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Enable chn failed with error code %#x!\n", s32Ret); 
        return HI_FAILURE;
    } /* now, vi is capturing images, you can do something else ... */

    getchar();

    s32Ret = HI_MPI_VI_DisableChn(ViChn); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Disable chn failed with error code %#x!\n", s32Ret); 
        return HI_FAILURE;
    }
    s32Ret = HI_MPI_VI_DisableDev(ViDev); 
    if (s32Ret != HI_SUCCESS) 
    {
        printf("Disable dev failed with error code %#x!\n", s32Ret); 
        return HI_FAILURE;
    }


}


/******************************************************************************
* function :  1HD H264 encode
******************************************************************************/
HI_S32 SAMPLE_VENC_1HD_H264(HI_VOID)
{
    //SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_720P;// SAMPLE_VI_MODE_1_720P;
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_1080P;

    HI_U32 u32ViChnCnt = 1;
    HI_S32 s32VpssGrpCnt = 1;
    PAYLOAD_TYPE_E enPayLoad[2]= {PT_H264, PT_H264};
    //PIC_SIZE_E enSize[2] = {PIC_HD720, PIC_D1};
    PIC_SIZE_E enSize[2] = {PIC_HD1080, PIC_HD720}; 
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    printf("step 1 \n");
    memset(&stVbConf,0,sizeof(VB_CONF_S)); //create video buffer pool 

    //get a frame picture size
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);

    stVbConf.u32MaxPoolCnt = 128; 

    /* video buffer*/ //main video buffer 
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 12;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));
#if 0
    /* hist buf*/ //sub video buffer 
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 10;//4;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));
#endif
    /******************************************
     step 2: mpp system init. 
    ******************************************/
    printf("step 2 \n");
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_1HD_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    printf("step 3 \n");
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_1HD_0;
    }
#if 1
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    printf("step 4 \n");
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_HD1080, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_1HD_0;
    }

    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;//VPSS_DIE_MODE_NODIE;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    printf("stGrpAttr.u32MaxW : %d \n", stGrpAttr.u32MaxW); 
    printf("stGrpAttr.u32MaxH : %d \n", stGrpAttr.u32MaxH);
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_1HD_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_1HD_2;
    }

    /******************************************
     step 5: select rc mode
    ******************************************/
    printf("step 5 \n");
    enRcMode = SAMPLE_RC_CBR;
#if 0
    while(1)
    {
        printf("please choose rc mode:\n"); 
        printf("\t0) CBR\n"); 
        printf("\t1) VBR\n"); 
        printf("\t2) FIXQP\n"); 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            enRcMode = SAMPLE_RC_CBR;
            break;
        }
        else if ('1' == ch)
        {
            enRcMode = SAMPLE_RC_VBR;
            break;
        }
        else if ('2' == ch)
        {
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        }
        else
        {
            printf("rc mode invaild! please try again.\n");
            continue;
        }
    }
#endif
    /******************************************
     step 6: start stream venc (big + little)
    ******************************************/
    printf("step 6 \n");
    i = 0;
    VencGrp = i*2;
    VencChn = i*2;
    VpssGrp = i;

    printf("VencGrp : %d \n", VencGrp);
    printf("VencChn : %d \n", VencChn);
    printf("VpssGrp : %d \n", VpssGrp); 
    s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
                                    gs_enNorm, enSize[0], enRcMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1HD_3;
    }
    printf("venc bind vpss \n");
    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1HD_3;
    }

#if 0
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VencGrp = i*2;
        VencChn = i*2;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
                                        gs_enNorm, enSize[0], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }

        /*** Sub frame **/
        VencGrp ++;
        VencChn ++;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1], \
                                     gs_enNorm, enSize[1], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_1HD_3;
        }
    }
#endif   
#endif 
    //getchar();
    /******************************************
     step 7: stream venc process -- get stream, then save it to file. 
    ******************************************/
    //s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt*2);
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_1HD_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    
    getchar();

    /******************************************
     step 8: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();
    
END_VENC_1HD_3:
    for (i=0; i<u32ViChnCnt*2; i++)
    {
        VencGrp = i;
        VencChn = i;
        VpssGrp = i/2;
        VpssChn = (VpssGrp%2)?VPSS_PRE0_CHN:VPSS_BSTR_CHN;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_1HD_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_1HD_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_1HD_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function :  1D1 MJPEG encode
******************************************************************************/
HI_S32 SAMPLE_VENC_1D1_MJPEG(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_1_D1;

    HI_U32 u32ViChnCnt = 1;
    HI_S32 s32VpssGrpCnt = 1;
    PAYLOAD_TYPE_E enPayLoad = PT_MJPEG;
    PIC_SIZE_E enSize = PIC_D1;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_MJPEG_0;
    }
 
    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_MJPEG_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_MJPEG_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_MJPEG_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_MJPEG_2;
    }

    /******************************************
     step 5: select rc mode
    ******************************************/
    while(1)
    {
        printf("please choose rc mode:\n"); 
        printf("\t0) CBR\n"); 
        printf("\t1) VBR\n"); 
        printf("\t2) FIXQP\n");
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            enRcMode = SAMPLE_RC_CBR;
            break;
        }
        else if ('1' == ch)
        {
            enRcMode = SAMPLE_RC_VBR;
            break;
        }
        else if ('2' == ch)
        {
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        }
        else
        {
            printf("rc mode invaild! please try again.\n");
            continue;
        }
    }
    /******************************************
     step 5: start stream venc
    ******************************************/
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VencGrp = i;
        VencChn = i;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad,\
                                        gs_enNorm, enSize, enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_MJPEG_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_MJPEG_3;
        }
    }

    /******************************************
     step 6: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_MJPEG_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 8: exit process
    ******************************************/
    SAMPLE_COMM_VENC_StopGetStream();
    
END_VENC_MJPEG_3:
    for (i=0; i<u32ViChnCnt; i++)
    {
        VencGrp = i;
        VencChn = i;
        VpssGrp = i;
        VpssChn =VPSS_BSTR_CHN;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_MJPEG_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_MJPEG_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_MJPEG_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function :  4D1 SNAP
******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_Snap(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    PIC_SIZE_E enSize = PIC_D1;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 15;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 15;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_SNAP_0;
    }
	
    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_SNAP_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_SNAP_2;
    }

    /******************************************
     step 5: snap process
    ******************************************/
    VencGrp = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_SNAP_3;
    }
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VpssGrp = i;

        s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("snap process failed!\n");
            goto END_VENC_SNAP_4;
        }
        printf("snap chn %d ok!\n", i);

        sleep(1);
    }

    /******************************************
     step 8: exit process
    ******************************************/
    printf("snap over!\n");
    
END_VENC_SNAP_4:
    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Stop snap failed!\n");
        goto END_VENC_SNAP_3;
    }
END_VENC_SNAP_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_SNAP_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_SNAP_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_SNAP_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function :  16*Cif SNAP
******************************************************************************/
HI_S32 SAMPLE_VENC_8_Cif_Snap(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_2Cif;

    HI_U32 u32ViChnCnt = 8;
    HI_S32 s32VpssGrpCnt = 8;
    PIC_SIZE_E enSize = PIC_2CIF;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_SNAP_0;
    }
	
    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_SNAP_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_SNAP_2;
    }

    /******************************************
     step 5: snap process
    ******************************************/
    VencGrp = 0;
    VencChn = 0;
	/*snap Cif pic*/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_CIF, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_SNAP_0;
    }
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_SNAP_3;
    }
    
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VpssGrp = i;

        s32Ret = SAMPLE_COMM_VENC_SnapProcess(VencGrp, VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("snap process failed!\n");
            goto END_VENC_SNAP_4;
        }
        printf("snap chn %d ok!\n", i);

    }

    /******************************************
     step 8: exit process
    ******************************************/
    printf("snap over!\n");
    
END_VENC_SNAP_4:
    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Stop snap failed!\n");
        goto END_VENC_SNAP_3;
    }
END_VENC_SNAP_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_SNAP_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_SNAP_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_SNAP_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


/******************************************************************************
* function :  1D1 User send pictures for H264 encode
******************************************************************************/
HI_S32 SAMPLE_VENC_1D1_USER_SEND_PICTURES(HI_VOID)
{  
    VB_CONF_S stVbConf;
    VENC_GRP VencGrp = 0;
    VENC_CHN VencChn = 0;
    PIC_SIZE_E enSize = PIC_D1;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;
    VB_POOL hPool  = VB_INVALID_POOLID;
    FILE *pfp_img = HI_NULL;  
    HI_U32 u32PicLStride            = 0;
    HI_U32 u32PicCStride            = 0;
    HI_U32 u32LumaSize              = 0;
    HI_U32 u32ChrmSize              = 0;
    HI_U32 u32Cnt                   = 0;
    HI_U32 u32ChnCnt                = 1;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 10;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    
    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_USER_0;
    }

   
    /******************************************
     step 3: open yuv file
    ******************************************/
    if (pfp_img != HI_NULL)
    {
        fclose(pfp_img);
        pfp_img = HI_NULL;
    }
    

    pfp_img = fopen(SAMPLE_YUV_D1_FILEPATH, "rb" ); 
    if (pfp_img == HI_NULL)
    {
        SAMPLE_PRT("Open yuv file failed!Check if the file %s exit\n",SAMPLE_YUV_D1_FILEPATH);
        goto END_VENC_USER_0;
    }
    
    /******************************************
     step 4: create private pool on ddr0
    ******************************************/
    hPool   = HI_MPI_VB_CreatePool( u32BlkSize, 10,NULL );
    if (hPool == VB_INVALID_POOLID)
    {
        SAMPLE_PRT("HI_MPI_VB_CreatePool failed! \n");
        goto END_VENC_USER_1;
    }
    
    /******************************************
     step 5: encode process
    ******************************************/
    
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_USER_2;
    }
    
    s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, PT_H264, gs_enNorm, enSize, SAMPLE_RC_CBR);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_USER_2;
    }


   /******************************************
     step 6: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ChnCnt);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("StartGetStream failed!\n");
        goto END_VENC_USER_3;
    }

    u32PicLStride = CEILING_2_POWER(stSize.u32Width, SAMPLE_SYS_ALIGN_WIDTH);
    u32PicCStride = CEILING_2_POWER(stSize.u32Width, SAMPLE_SYS_ALIGN_WIDTH);
    u32LumaSize = (u32PicLStride * stSize.u32Height);
    u32ChrmSize = (u32PicCStride * stSize.u32Height) >> 2;


    while(0 == feof(pfp_img))
    { 
        SAMPLE_MEMBUF_S stMem = {0};
        VIDEO_FRAME_INFO_S stFrmInfo;

        stMem.hPool = hPool;
        u32Cnt ++;
        while((stMem.hBlock = HI_MPI_VB_GetBlock(stMem.hPool, u32BlkSize,NULL)) == VB_INVALID_HANDLE)
        {
             ;
        }
  
        stMem.u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(stMem.hBlock);


        stMem.pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( stMem.u32PhyAddr, u32BlkSize );
        if(stMem.pVirAddr == NULL)
        {
            SAMPLE_PRT("Mem dev may not open\n");
            goto END_VENC_USER_4;
        }
    
        memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
        stFrmInfo.stVFrame.u32PhyAddr[0] = stMem.u32PhyAddr;
        stFrmInfo.stVFrame.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.u32PhyAddr[2] = stFrmInfo.stVFrame.u32PhyAddr[1] + u32ChrmSize;
        
        stFrmInfo.stVFrame.pVirAddr[0] = stMem.pVirAddr;
        stFrmInfo.stVFrame.pVirAddr[1] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.pVirAddr[2] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[1] + u32ChrmSize;

        stFrmInfo.stVFrame.u32Width     = stSize.u32Width;
        stFrmInfo.stVFrame.u32Height    = stSize.u32Height;
        stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[1] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[2] = u32PicLStride;

        stFrmInfo.stVFrame.u64pts     = (u32Cnt * 40);
        stFrmInfo.stVFrame.u32TimeRef = (u32Cnt * 2);

        /*  Different channsel with different picture sequence  */
        SAMPLE_COMM_VENC_ReadOneFrame( pfp_img, stFrmInfo.stVFrame.pVirAddr[0], 
            stFrmInfo.stVFrame.pVirAddr[1], stFrmInfo.stVFrame.pVirAddr[2],
            stFrmInfo.stVFrame.u32Width, stFrmInfo.stVFrame.u32Height, 
            stFrmInfo.stVFrame.u32Stride[0], stFrmInfo.stVFrame.u32Stride[1] >> 1 );

        if(0 != feof(pfp_img))
        {
            break;
        }
         
        SAMPLE_COMM_VENC_PlanToSemi( stFrmInfo.stVFrame.pVirAddr[0], stFrmInfo.stVFrame.u32Stride[0],
        stFrmInfo.stVFrame.pVirAddr[1], stFrmInfo.stVFrame.u32Stride[1],
        stFrmInfo.stVFrame.pVirAddr[2], stFrmInfo.stVFrame.u32Stride[1],
        stFrmInfo.stVFrame.u32Width,    stFrmInfo.stVFrame.u32Height );

        stFrmInfo.stVFrame.enPixelFormat = SAMPLE_PIXEL_FORMAT;
        stFrmInfo.stVFrame.u32Field = VIDEO_FIELD_FRAME;
    
        stMem.u32PoolId = HI_MPI_VB_Handle2PoolId( stMem.hBlock );
        stFrmInfo.u32PoolId = stMem.u32PoolId;
        
        s32Ret = HI_MPI_VENC_SendFrame(VencGrp, &stFrmInfo);

        HI_MPI_SYS_Munmap( stMem.pVirAddr, u32BlkSize );
        HI_MPI_VB_ReleaseBlock(stMem.hBlock);

    }

    /******************************************
     step 7: exit process
    ******************************************/
END_VENC_USER_4:
    SAMPLE_COMM_VENC_StopGetStream();
END_VENC_USER_3:
    
    s32Ret = SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);;
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Stop encode failed!\n");
        goto END_VENC_USER_2;
    }
END_VENC_USER_2:    
    //before destroy private pool,must stop venc
    HI_MPI_VB_DestroyPool( hPool );

END_VENC_USER_1:	
    //close the yuv file
    fclose( pfp_img );
    pfp_img = HI_NULL;
END_VENC_USER_0:	
    //system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function : 4D1 H264 encode with color2grey
******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_H264_COLOR2GREY(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    PAYLOAD_TYPE_E enPayLoad[2]= {PT_H264, PT_H264};
    PIC_SIZE_E enSize[2] = {PIC_D1, PIC_CIF};
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_CHN VpssChn;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    SAMPLE_RC_E enRcMode;
    GROUP_COLOR2GREY_CONF_S stGrpColor2GreyConf;
    GROUP_COLOR2GREY_S stGrpColor2Grey;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_CHAR ch;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                PIC_D1, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;
    
    /*ddr0 video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 6;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /*ddr0 hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 3;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

 
    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_16D1_0;
    }

    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_16D1_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, PIC_D1, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_16D1_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_16D1_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_16D1_2;
    }


    /******************************************
     step 5: Set color2grey conf
     ******************************************/
    stGrpColor2GreyConf.bEnable = HI_TRUE;
    stGrpColor2GreyConf.u32MaxWidth = 720;
    stGrpColor2GreyConf.u32MaxHeight = 576;
    s32Ret = HI_MPI_VENC_SetColor2GreyConf(&stGrpColor2GreyConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SetColor2GreyConf failed!\n");
        goto END_VENC_16D1_2;
    }
    /******************************************
     step 6: select rc mode
    ******************************************/
    while(1)
    {
        printf("please choose rc mode:\n"); 
        printf("\t0) CBR\n"); 
        printf("\t1) VBR\n"); 
        printf("\t2) FIXQP\n"); 
        ch = getchar();
        getchar();
        if ('0' == ch)
        {
            enRcMode = SAMPLE_RC_CBR;
            break;
        }
        else if ('1' == ch)
        {
            enRcMode = SAMPLE_RC_VBR;
            break;
        }
        else if ('2' == ch)
        {
            enRcMode = SAMPLE_RC_FIXQP;
            break;
        }
        else
        {
            printf("rc mode invaild! please try again.\n");
            continue;
        }
    }
    /******************************************
     step 7: start stream venc (big + little)
    ******************************************/
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VencGrp = i*2;
        VencChn = i*2;
        VpssGrp = i;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[0],\
                                       gs_enNorm, enSize[0], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_16D1_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencGrp, VpssGrp, VPSS_BSTR_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_16D1_3;
        }
        
        /*** Enable a grp with color2grey **/
        stGrpColor2Grey.bColor2Grey = HI_TRUE;
        s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SetGrpColor2Grey failed!\n");
            goto END_VENC_16D1_3;
        }

        /*** Sub frame **/
        VencGrp ++;
        VencChn ++;
        s32Ret = SAMPLE_COMM_VENC_Start(VencGrp, VencChn, enPayLoad[1], \
                                        gs_enNorm, enSize[1], enRcMode);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_16D1_3;
        }

        s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("Start Venc failed!\n");
            goto END_VENC_16D1_3;
        }

        /*** Enable a grp with color2grey **/
        stGrpColor2Grey.bColor2Grey = HI_TRUE;
        s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SetGrpColor2Grey failed!\n");
            goto END_VENC_16D1_3;
        }
    }

    /******************************************
     step 8: stream venc process -- get stream, then save it to file. 
    ******************************************/
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(u32ViChnCnt*2);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
        goto END_VENC_16D1_3;
    }

    printf("please press twice ENTER to exit this sample\n");
    getchar();
    getchar();

    /******************************************
     step 9: exit process
    ******************************************/
    for (i=0; i<u32ViChnCnt; i++)
    {
        VencGrp = i*2;
        /*** Enable a grp with color2grey **/
        stGrpColor2Grey.bColor2Grey = HI_FALSE;
        s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SetGrpColor2Grey failed!\n");
            goto END_VENC_16D1_3;
        }

        VencGrp = i*2 + 1;
        /*** Enable a grp with color2grey **/
        stGrpColor2Grey.bColor2Grey = HI_FALSE;
        s32Ret = HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("SetGrpColor2Grey failed!\n");
            goto END_VENC_16D1_3;
        }
    }

    
    SAMPLE_COMM_VENC_StopGetStream();
    
END_VENC_16D1_3:
    for (i=0; i<u32ViChnCnt*2; i++)
    {
        VencGrp = i;
        VencChn = i;
        VpssGrp = i/2;
        VpssChn = (VpssGrp%2)?VPSS_PRE0_CHN:VPSS_BSTR_CHN;
        SAMPLE_COMM_VENC_UnBindVpss(VencGrp, VpssGrp, VpssChn);
        stGrpColor2Grey.bColor2Grey = HI_FALSE;
        HI_MPI_VENC_SetGrpColor2Grey(VencGrp, &stGrpColor2Grey);
        SAMPLE_COMM_VENC_Stop(VencGrp,VencChn);
    }
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_16D1_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_16D1_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_16D1_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}

/******************************************************************************
* function :  4D1 SNAP with StartRecvPicEx
******************************************************************************/
HI_S32 SAMPLE_VENC_4D1_SnapEx(HI_VOID)
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_D1;

    HI_U32 u32ViChnCnt = 4;
    HI_S32 s32VpssGrpCnt = 4;
    PIC_SIZE_E enSize = PIC_D1;
    
    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
    VENC_GRP VencGrp;
    VENC_CHN VencChn;
    
    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    SIZE_S stSize;

    /******************************************
     step  1: init variable 
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));

    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(gs_enNorm,\
                enSize, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 15;
    memset(stVbConf.astCommPool[0].acMmzName,0,
        sizeof(stVbConf.astCommPool[0].acMmzName));

    /* hist buf*/
    stVbConf.astCommPool[1].u32BlkSize = (196*4);
    stVbConf.astCommPool[1].u32BlkCnt = u32ViChnCnt * 15;
    memset(stVbConf.astCommPool[1].acMmzName,0,
        sizeof(stVbConf.astCommPool[1].acMmzName));

    /******************************************
     step 2: mpp system init. 
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_VENC_SNAP_0;
    }
	
    /******************************************
     step 3: start vi dev & chn to capture
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, gs_enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(gs_enNorm, enSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_VENC_SNAP_0;
    }
    
    stGrpAttr.u32MaxW = stSize.u32Width;
    stGrpAttr.u32MaxH = stSize.u32Height;
    stGrpAttr.bDrEn = HI_FALSE;
    stGrpAttr.bDbEn = HI_FALSE;
    stGrpAttr.bIeEn = HI_TRUE;
    stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.bHistEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;
    stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;

    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_VENC_SNAP_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_VENC_SNAP_2;
    }

    /******************************************
     step 5: snap process
    ******************************************/
    VencGrp = 0;
    VencChn = 0;
    s32Ret = SAMPLE_COMM_VENC_SnapStart(VencGrp, VencChn, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start snap failed!\n");
        goto END_VENC_SNAP_3;
    }
    for (i=0; i<u32ViChnCnt; i++)
    {
        /*** main frame **/
        VpssGrp = i;

        s32Ret = SAMPLE_COMM_VENC_SnapProcessEx(VencGrp, VencChn, VpssGrp, VPSS_PRE0_CHN);
        if (HI_SUCCESS != s32Ret)
        {
            SAMPLE_PRT("snap process failed!\n");
            goto END_VENC_SNAP_4;
        }
        printf("snap chn %d ok!\n", i);

        sleep(1);
    }

    /******************************************
     step 8: exit process
    ******************************************/
    printf("snap over!\n");
    
END_VENC_SNAP_4:
    s32Ret = SAMPLE_COMM_VENC_SnapStop(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Stop snap failed!\n");
        goto END_VENC_SNAP_3;
    }
END_VENC_SNAP_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_VENC_SNAP_2:	//vpss stop
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_VENC_SNAP_1:	//vi stop
    SAMPLE_COMM_VI_Stop(enViMode);
END_VENC_SNAP_0:	//system exit
    SAMPLE_COMM_SYS_Exit();
    
    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : video venc sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret;
    pthread_t it6801_thread;
/*
    if ( (argc < 2) || (1 != strlen(argv[1])))
    {
        SAMPLE_VENC_Usage(argv[0]);
        return HI_FAILURE;
    }
*/
    signal(SIGINT, SAMPLE_VENC_HandleSig);
    signal(SIGTERM, SAMPLE_VENC_HandleSig);
    //pthread_create(&it6801_thread, NULL, it6801_main, NULL);
    //sleep(5);
    //SAMPLE_VENC_4D1_H264();
    //VI_TEST();
    //SAMPLE_VENC_1D1_MJPEG();
    SAMPLE_VENC_1HD_H264();   
    //SAMPLE_VENC_4D1_H264_COLOR2GREY();
    //SAMPLE_VENC_1D1_USER_SEND_PICTURES();

    #if 0 
    switch (*argv[1])
    {
        case '0':/* 4D1 H264 encode */
            s32Ret = SAMPLE_VENC_4D1_H264();
            break;
        case '1':/* 1*720p H264 encode */
            s32Ret = SAMPLE_VENC_1HD_H264();
            break;
        case '2':/* 1D1 MJPEG encode */
            s32Ret = SAMPLE_VENC_1D1_MJPEG();
            break;
        case '3':/* 4D1 JPEG snap */
            s32Ret = SAMPLE_VENC_4D1_Snap();
            break;
        case '4':/* 4D1 JPEG snap */
            s32Ret = SAMPLE_VENC_4D1_SnapEx();
            break;            
        case '5':/* 8*Cif JPEG snap */
            s32Ret = SAMPLE_VENC_8_Cif_Snap();
            break;
        case '6':/* 1D1 User send pictures for H264 encode */
            s32Ret = SAMPLE_VENC_1D1_USER_SEND_PICTURES();
            break;
        case '7':/* 4D1 H264 encode with color2grey */
            s32Ret = SAMPLE_VENC_4D1_H264_COLOR2GREY();
            break;
        default:
            printf("the index is invaild!\n");
            SAMPLE_VENC_Usage(argv[0]);
            return HI_FAILURE;
    }
    #endif
    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
