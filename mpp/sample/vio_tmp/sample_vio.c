/******************************************************************************
  A simple program of Hisilicon Hi35xx video input and output implementation.
  Copyright (C), 2014-2015, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2015-1 Created
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "i2c.h"
//#include "test.h"
#include "sample_comm.h"
#include "it6801.h"
#include "typedef.h"

extern VTiming CurVTiming; //from it680x.c
extern AVI_InfoFrame aviinfoframe; //from it680x.c

#define HDMI_SUPPORT

#if 0
/******************************************************************************
* function : show usage
******************************************************************************/
void SAMPLE_VIO_Usage(char *sPrgNm)
{
    printf("Usage : %s + <index> (eg: %s 0)\n", sPrgNm,sPrgNm);
    printf("index:\n");
	printf("\t0:  VI 1080P input, HD0(VGA+BT1120),HD1(HDMI)/SD VO \n");
	printf("\t1:  VI 4K input, HD0(VGA+BT1120),HD1(HDMI)/SD VO\n");
	printf("\t2:  VI 4*SDI 1080P input, HD0(VGA+BT1120),HD1(HDMI)/SD VO\n");
	printf("\tq:  quit\n");

    return;
}
#endif
/******************************************************************************
* function : show usage
******************************************************************************/
HI_VOID SAMPLE_VIO_Usage(HI_VOID)
{
    printf("\n\n/************************************/\n");
    printf("please choose the case which you want to run:\n");
    printf("\t1:  VI --> 1080P          VO --> 1080P@60\n");
    printf("\t2:  VI --> 4K             VO --> 1080P@60\n");
    printf("\tq:  quit\n");
    printf("sample command:");
    return;
}

/******************************************************************************
* function : to process abnormal case
******************************************************************************/
void SAMPLE_VIO_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTERM == signo)
    {   SAMPLE_COMM_VO_HdmiStop();
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

/******************************************************************************
* function : VI(1080P: 4 windows) -> VPSS -> HD0(1080P,VGA,BT1120)
                                         -> HD1(1080P,HDMI)
                                         -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_4_1080P(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoHD1 = VPSS_CHN2;
	VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;

	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr_hd0, stVoPubAttr_hd1, stVoPubAttr_sd;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_S32 ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	HI_U32 vichn;

	/******************************************
	 step  1: init variable
	******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 16;

	/******************************************
	 step 2: mpp system init.
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_1080P_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_1080P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_1080P_1;
	}

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    	stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_8_1080P_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_1080P_2;
	}

	/******************************************
	 step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA;
	stVoPubAttr_hd0.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_4;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_5;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_5;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_5;
		}
	}

#if 1
    /******************************************
	 step 5: start vo HD1 (HDMI), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_hd1.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI;
	stVoPubAttr_hd1.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_5;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_6;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_6;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_7;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_7;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_7;
		}
	}
	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_sd.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr_sd.enIntfType = VO_INTF_CVBS;
	stVoPubAttr_sd.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_sd);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_7;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_sd.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_8;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_8;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_9;
	}

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		vichn = 4*i;

		s32Ret = SAMPLE_COMM_VO_BindVi(VoLayer,VoChn,vichn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}
#endif 
	/******************************************
	step 7: HD0 switch mode
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_4MUX;
	while(1)
	{
		enPreVoMode = enVoMode;

		printf("please choose preview mode, press 'q' to exit this sample.\n");
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");

		ch = getchar();
        if (10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_1080P_9:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_8:

	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_7:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD1);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);

END_8_1080P_6:
	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_5:

	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif

    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_4:
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_1080P_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_1080P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_1080P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_1080P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}
/******************************************************************************
* function : VI(1080P: 1 windows) -> VPSS -> HD0(1080P,VGA,BT1120)
                                         -> HD1(1080P,HDMI)
                                         -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_1080P(HI_VOID)
{
	printf("SAMPLE_VIO_1080P \n");
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_1080P;
	//SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_4Kx2K;//SAMPLE_VI_MODE_8_720P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;//VIDEO_ENCODING_MODE_PAL;

	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoHD1 = VPSS_CHN2;
	VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;

	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr_hd0, stVoPubAttr_hd1, stVoPubAttr_sd;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_S32 ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	HI_U32 vichn;

	/******************************************
	 step  1: init variable
	******************************************/
	printf("step 1 \n");
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 16;

	/******************************************
	 step 2: mpp system init.
	******************************************/
	printf("step 2\n");
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_1080P_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	printf("step 3\n");
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_1080P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	printf("step 4\n");
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_UHD4K, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_1080P_1;
	}

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
	stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_8_1080P_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_1080P_2;
	}
#if 0
	/******************************************
	 step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	******************************************/

	printf("step 5\n");
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA;
	stVoPubAttr_hd0.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_4;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_5;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_5;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_5;
		}
	}
#endif
#if 1
    /******************************************
	 step 5: start vo HD1 (HDMI), multi-screen, you can switch mode
	******************************************/
	printf("step 5\n");
	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_hd1.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI;
	stVoPubAttr_hd1.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_5;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_6;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_6;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_7;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_7;
		}
	}
#endif
#if 1
	VoChn = 0; //video output channel
	VpssGrp = 1; //	
	
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		goto END_8_1080P_7;
	}
#endif
#if 0
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_7;
		}
	}
#endif
#endif
#if 0
	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("step 6\n");
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_sd.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr_sd.enIntfType = VO_INTF_CVBS;
	stVoPubAttr_sd.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_sd);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_7;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_sd.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_8;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_8;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_9;
	}

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		vichn = 4*i;

		s32Ret = SAMPLE_COMM_VO_BindVi(VoLayer,VoChn,vichn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}
#endif
 
	/******************************************
	step 7: HD0 switch mode
	******************************************/
	printf("step 7\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_1MUX;
	while(1)
	{
		enPreVoMode = enVoMode;
#if 1
		printf("please choose preview mode, press 'q' to exit this sample.\n");
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");

		ch = getchar();
        if (10 == ch)
        {
            continue;
        }
		getchar();
#endif
		//ch = '0';
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_1080P_9:
	printf("END_8_1080P_9\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_8:
	printf("END_8_1080P_8\n");
	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_7:
	printf("END_8_1080P_7\n");
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD1);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);

END_8_1080P_6:
	printf("END_8_1080P_6\n");
	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_5:
	printf("END_8_1080P_5\n");
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif

    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_4:
	printf("END_8_1080P_4\n");
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_1080P_3:	//vi unbind vpss
	printf("END_8_1080P_3\n");
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_1080P_2:	//vpss stop
	printf("END_8_1080P_2\n");
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_1080P_1:	//vi stop
	printf("END_8_1080P_1\n");
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_1080P_0:	//system exit
	printf("END_8_1080P_0\n");
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}
/******************************************************************************
* function : VI(4K: 1 windows) -> VPSS -> HD0(1080P,VGA,BT1120)
                                         -> HD1(1080P,HDMI)
                                         -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_4K(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_4_4Kx2K;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;

	HI_U32 u32ViChnCnt = 4;
	HI_S32 s32VpssGrpCnt = 4;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoHD1 = VPSS_CHN2;
	VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;

	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr_hd0, stVoPubAttr_hd1, stVoPubAttr_sd;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_S32 ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	HI_U32 vichn;

	/******************************************
	 step  1: init variable
	******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_UHD4K, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 16;

	/******************************************
	 step 2: mpp system init.
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_1080P_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_1080P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_UHD4K, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_1080P_1;
	}

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
        stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_8_1080P_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_1080P_2;
	}
	/******************************************
	 step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA;
	stVoPubAttr_hd0.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_4;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_5;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_5;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_5;
		}
	}

#if 1
    /******************************************
	 step 5: start vo HD1 (HDMI), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr_hd1.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI;
	stVoPubAttr_hd1.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_5;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_6;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_6;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_7;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_7;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_7;
		}
	}
	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_4MUX;

	stVoPubAttr_sd.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr_sd.enIntfType = VO_INTF_CVBS;
	stVoPubAttr_sd.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_sd);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_7;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_sd.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_8;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_8;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_9;
	}

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		vichn = 4*i;

		s32Ret = SAMPLE_COMM_VO_BindVi(VoLayer,VoChn,vichn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}
#endif 
	/******************************************
	step 7: HD0 switch mode
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_4MUX;
	while(1)
	{
		enPreVoMode = enVoMode;

		printf("please choose preview mode, press 'q' to exit this sample.\n");
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");

		ch = getchar();
        if (10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_1080P_9:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_8:

	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_7:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD1);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);

END_8_1080P_6:
	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_5:

	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif

    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_4:
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_1080P_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_1080P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_1080P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_1080P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}
/******************************************************************************
* function : VI(1080P: 8 windows) -> VPSS -> HD0(1080P,VGA,BT1120)
                                         -> HD1(1080P,HDMI)
                                         -> SD0(D1)
******************************************************************************/
HI_S32 SAMPLE_VIO_8_1080P(HI_VOID)
{
	SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

	HI_U32 u32ViChnCnt = 8;
	HI_S32 s32VpssGrpCnt = 8;

	VB_CONF_S stVbConf;
	VPSS_GRP VpssGrp;
	VPSS_GRP_ATTR_S stGrpAttr;
    VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoHD1 = VPSS_CHN2;
	VPSS_CHN VpssChn_VoSD0 = VPSS_CHN3;

	VO_DEV VoDev;
	VO_LAYER VoLayer;
	VO_CHN VoChn;
	VO_PUB_ATTR_S stVoPubAttr_hd0, stVoPubAttr_hd1, stVoPubAttr_sd;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
	SAMPLE_VO_MODE_E enVoMode, enPreVoMode;

	HI_S32 i;
	HI_S32 s32Ret = HI_SUCCESS;
	HI_U32 u32BlkSize;
	HI_S32 ch;
	SIZE_S stSize;
	HI_U32 u32WndNum;
	HI_U32 vichn;

	/******************************************
	 step  1: init variable
	******************************************/
	memset(&stVbConf,0,sizeof(VB_CONF_S));
	u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
				PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
	stVbConf.u32MaxPoolCnt = 128;

	/* video buffer*/
	//todo: vb=15
	stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;

	/******************************************
	 step 2: mpp system init.
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		goto END_8_1080P_0;
	}

	/******************************************
	 step 3: start vi dev & chn
	******************************************/
	s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("start vi failed!\n");
		goto END_8_1080P_0;
	}

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto END_8_1080P_1;
	}

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto END_8_1080P_1;
	}

	s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto END_8_1080P_2;
	}

	/******************************************
	 step 5: start vo HD0 (bt1120+VGA), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;

	stVoPubAttr_hd0.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd0.enIntfType = VO_INTF_BT1120|VO_INTF_VGA;
	stVoPubAttr_hd0.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd0.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_4;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_4;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_5;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd0.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_5;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_5;
		}
	}


    /******************************************
	 step 5: start vo HD1 (HDMI), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;

	stVoPubAttr_hd1.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI;
	stVoPubAttr_hd1.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_5;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_6;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_6;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_7;
	}
#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_8_1080P_7;
		}
	}
#endif
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;

		s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_7;
		}
	}
	/******************************************
	step 6: start vo SD0 (CVBS)
	******************************************/
	printf("start vo SD0\n");
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;

	stVoPubAttr_sd.enIntfSync = VO_OUTPUT_PAL;
	stVoPubAttr_sd.enIntfType = VO_INTF_CVBS;
	stVoPubAttr_sd.u32BgColor = 0x0;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_sd);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_8_1080P_7;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_sd.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_8_1080P_8;
	}

	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    stLayerAttr.stDispRect.s32X       = 0;
    stLayerAttr.stDispRect.s32Y       = 0;
    stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
    stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_8_1080P_8;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_8_1080P_9;
	}

	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		vichn = 4*i;

		s32Ret = SAMPLE_COMM_VO_BindVi(VoLayer,VoChn,vichn);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}

	/******************************************
	step 7: HD0 switch mode
	******************************************/
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	enVoMode = VO_MODE_9MUX;
	while(1)
	{
		enPreVoMode = enVoMode;

		printf("please choose preview mode, press 'q' to exit this sample.\n");
		printf("\t0) 1 preview\n");
		printf("\t1) 4 preview\n");
		printf("\t2) 8 preview\n");
		printf("\tq) quit\n");

		ch = getchar();
        if (10 == ch)
        {
            continue;
        }
		getchar();
		if ('0' == ch)
		{
			u32WndNum = 1;
			enVoMode = VO_MODE_1MUX;
		}
		else if ('1' == ch)
		{
			u32WndNum = 4;
			enVoMode = VO_MODE_4MUX;
		}
		/*Indeed only 8 chns show*/
		else if ('2' == ch)
		{
			u32WndNum = 9;
			enVoMode = VO_MODE_9MUX;
		}
		else if ('q' == ch)
		{
			break;
		}
		else
		{
			SAMPLE_PRT("preview mode invaild! please try again.\n");
			continue;
		}
		SAMPLE_PRT("vo(%d) switch to %d mode\n", VoDev, u32WndNum);

		s32Ret= HI_MPI_VO_SetAttrBegin(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StopChn(VoLayer, enPreVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}

		s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
		s32Ret= HI_MPI_VO_SetAttrEnd(VoLayer);
		if (HI_SUCCESS != s32Ret)
		{
			SAMPLE_PRT("Start VO failed!\n");
			goto END_8_1080P_9;
		}
	}

	/******************************************
	 step 8: exit process
	******************************************/
END_8_1080P_9:
	VoDev = SAMPLE_VO_DEV_DSD0;
	VoLayer = SAMPLE_VO_LAYER_VSD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoSD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_8:

	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_7:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
	VoDev = SAMPLE_VO_DEV_DHD1;
	VoLayer = SAMPLE_VO_LAYER_VHD1;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD1);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);

END_8_1080P_6:
	SAMPLE_COMM_VO_StopDev(VoDev);

END_8_1080P_5:

	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd0.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif

    VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 8;
	enVoMode = VO_MODE_9MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD0);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
END_8_1080P_4:
	SAMPLE_COMM_VO_StopDev(VoDev);
END_8_1080P_3:	//vi unbind vpss
	SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_8_1080P_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_8_1080P_1:	//vi stop
	SAMPLE_COMM_VI_Stop(enViMode);
END_8_1080P_0:	//system exit
	SAMPLE_COMM_SYS_Exit();

	return s32Ret;
}

/******************************************************************************
* function : VI(D1) -> VPSS grp 0 -> VO HD0(1080p)
*            VI(D1) -> VPSS grp 1 -> VO PIP
******************************************************************************/
HI_S32 SAMPLE_VIO_HD_ZoomIn()
{
    SAMPLE_VI_MODE_E enViMode = SAMPLE_VI_MODE_8_1080P;
	VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_NTSC;

    HI_U32 u32ViChnCnt = 8;
    HI_S32 s32VpssGrpCnt = 8;
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP VpssGrp_Clip = 1;
	VPSS_CHN VpssChn_VoHD0 = VPSS_CHN1;
	VPSS_CHN VpssChn_VoPIP = VPSS_CHN2;
    VO_DEV VoDev = SAMPLE_VO_DEV_DHD0;
	VO_LAYER VoLayer = SAMPLE_VO_LAYER_VHD0;
	VO_LAYER VoLayerPip = SAMPLE_VO_LAYER_VPIP;
    VO_CHN VoChn = 0;

    VB_CONF_S stVbConf;
    VPSS_GRP_ATTR_S stGrpAttr;
    VO_PUB_ATTR_S stVoPubAttr;
    SAMPLE_VO_MODE_E enVoMode;
    VPSS_CROP_INFO_S stVpssCropInfo;
	VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SIZE_S stSize;

    HI_S32 i;
    HI_S32 s32Ret = HI_SUCCESS;
    HI_U32 u32BlkSize;
    HI_S32 ch;
    HI_U32 u32WndNum;

    /******************************************
     step  1: init variable
    ******************************************/
    memset(&stVbConf,0,sizeof(VB_CONF_S));
    u32BlkSize = SAMPLE_COMM_SYS_CalcPicVbBlkSize(enNorm,\
                PIC_HD1080, SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH,COMPRESS_MODE_SEG);
    stVbConf.u32MaxPoolCnt = 128;

    /* video buffer*/
    //todo: vb=15
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = u32ViChnCnt * 8;

    /******************************************
     step 2: mpp system init.
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        goto END_HDZOOMIN_0;
    }

    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_Start(enViMode, enNorm);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed!\n");
        goto END_HDZOOMIN_0;
    }

    /******************************************
     step 4: start vpss and vi bind vpss
    ******************************************/
    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_HD1080, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
        goto END_HDZOOMIN_0;
    }

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM,NULL);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Vpss failed!\n");
        goto END_HDZOOMIN_1;
    }

    s32Ret = SAMPLE_COMM_VI_BindVpss(enViMode);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Vi bind Vpss failed!\n");
        goto END_HDZOOMIN_2;
    }

    /******************************************
     step 5: start VO to preview
    ******************************************/
	printf("start vo HD0.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
	stVoPubAttr.enIntfType = VO_INTF_HDMI|VO_INTF_VGA;
	stVoPubAttr.u32BgColor = 0x000000ff;
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto END_HDZOOMIN_3;
	}

	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto END_HDZOOMIN_3;
	}
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
	stLayerAttr.stDispRect.s32X 	  = 0;
	stLayerAttr.stDispRect.s32Y 	  = 0;
	stLayerAttr.stDispRect.u32Width   = stLayerAttr.stImageSize.u32Width;
	stLayerAttr.stDispRect.u32Height  = stLayerAttr.stImageSize.u32Height;
	s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stLayerAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartLayer failed!\n");
		goto END_HDZOOMIN_3;
	}

	s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto END_HDZOOMIN_4;
	}

#ifdef HDMI_SUPPORT
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto END_HDZOOMIN_5;
		}
	}
#endif

	VoChn = 0;
	VpssGrp = 0;
	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		goto END_HDZOOMIN_5;
	}

    /******************************************
     step 6: Clip process
    ******************************************/
    printf("press any key to show hd zoom.\n");
    getchar();
    /*** enable vo pip layer ***/
    stLayerAttr.bClusterMode = HI_TRUE;
    stLayerAttr.bDoubleFrame = HI_FALSE;
    stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    //default, PIP bind dev0
    HI_MPI_VO_UnBindVideoLayer(VoLayerPip, 0);
	s32Ret = HI_MPI_VO_BindVideoLayer(VoLayerPip, VoDev);
	if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VO_BindVideoLayer failed with %#x!\n", s32Ret);
        goto END_HDZOOMIN_5;
    }

	memset(&stLayerAttr, 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
    s32Ret = SAMPLE_COMM_VO_GetWH(VO_OUTPUT_PAL, \
        &stLayerAttr.stDispRect.u32Width, \
        &stLayerAttr.stDispRect.u32Height, \
        &stLayerAttr.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_HDZOOMIN_6;
    }
    stLayerAttr.stImageSize.u32Width = stLayerAttr.stDispRect.u32Width;
    stLayerAttr.stImageSize.u32Height = stLayerAttr.stDispRect.u32Height;
	stLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayerPip, &stLayerAttr);
    if (HI_SUCCESS != s32Ret)
    {
       SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer failed!\n");
       goto END_HDZOOMIN_6;
    }

	enVoMode = VO_MODE_1MUX;
    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayerPip,enVoMode);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        goto  END_HDZOOMIN_7;
    }

	s32Ret = SAMPLE_COMM_VO_BindVpss(VoLayerPip, VoChn, VpssGrp_Clip, VpssChn_VoPIP);//VpssGrp_Clip
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_BindVpss failed!\n");
        goto END_HDZOOMIN_8;
    }

	/*** enable vpss group clip ***/
    stVpssCropInfo.bEnable = HI_TRUE;
    stVpssCropInfo.enCropCoordinate = VPSS_CROP_ABS_COOR;
    stVpssCropInfo.stCropRect.s32X = 0;
    stVpssCropInfo.stCropRect.s32Y = 0;
    stVpssCropInfo.stCropRect.u32Height = 400;
    stVpssCropInfo.stCropRect.u32Width = 400;
    s32Ret = HI_MPI_VPSS_SetGrpCrop(VpssGrp_Clip, &stVpssCropInfo);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HI_MPI_VPSS_SetGrpCrop failed with %#x!\n", s32Ret);
        goto END_HDZOOMIN_8;
    }

    printf("\npress 'q' to stop sample ... ... \n");
    while('q' != (ch = getchar()) )  {}
    goto END_HDZOOMIN_9;
    /******************************************
     step 7: exit process
    ******************************************/

END_HDZOOMIN_9:
	SAMPLE_COMM_VO_UnBindVpss(SAMPLE_VO_LAYER_VHD0, VoChn, VpssGrp_Clip, VpssChn_VoPIP);
END_HDZOOMIN_8:
	enVoMode = VO_MODE_1MUX;
	SAMPLE_COMM_VO_StopChn(VoLayerPip,enVoMode);
END_HDZOOMIN_7:
	SAMPLE_COMM_VO_StopLayer(VoLayerPip);
END_HDZOOMIN_6:
	HI_MPI_VO_UnBindVideoLayer(VoLayerPip,SAMPLE_VO_DEV_DHD0);
END_HDZOOMIN_5:
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
    u32WndNum = 1;
    enVoMode = VO_MODE_1MUX;
	for(i=0;i<u32WndNum;i++)
    {
        VoChn = i;
        VpssGrp = i;
        SAMPLE_COMM_VO_UnBindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD0);
    }
    SAMPLE_COMM_VO_StopChn(VoDev, enVoMode);
END_HDZOOMIN_4:
	SAMPLE_COMM_VO_StopLayer(SAMPLE_VO_LAYER_VHD0);
	#ifdef HDMI_SUPPORT
    if (stVoPubAttr.enIntfType & VO_INTF_HDMI)
    {
        SAMPLE_COMM_VO_HdmiStop();
    }
	#endif
	SAMPLE_COMM_VO_StopDev(VoDev);
END_HDZOOMIN_3:
    SAMPLE_COMM_VI_UnBindVpss(enViMode);
END_HDZOOMIN_2:
    SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
END_HDZOOMIN_1:
    SAMPLE_COMM_VI_Stop(enViMode);
END_HDZOOMIN_0:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}

#if 1
/******************************************************************************
* function    : main()
* Description : video preview sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_CHAR ch, ret;
    HI_BOOL bExit = HI_FALSE;
	pthread_t it6801Handle;

    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
	//char buffer[][10] = {"0", "0", "0x90", "0xa5", "0xa6"};
	
	#if 0//it6801
	ret = pthread_create(&it6801Handle, NULL, it6801_main, NULL);
	if (ret) 
	{
		printf("Failed to Create Config Handle Thread\n");
		//printf("%d reboot",__LINE__);
		return ret;
	}
	#endif
    while (1)
    {
		char *p, i;
		
		//printf("main is running \n");
		sleep(1);
/*
        SAMPLE_VIO_Usage();
        ch = getchar();
        if (10 == ch)
        {
            continue;
        }
        //getchar();
*/

		SAMPLE_VIO_1080P();
#if 0
		ch = '1';
        switch (ch)
        {
            case '1':   
            {
                s32Ret = SAMPLE_VIO_1080P();
                break;
            }
            case '2':   
            {
                s32Ret = SAMPLE_VIO_4K();
                break;
            }
            case 'q':
            case 'Q':
            {
                bExit = HI_TRUE;
                break;
            }
            default :
            {
                printf("input invaild! please try again.\n");
                break;
            }
        }
        
        if (bExit)
        {
            break;
        }
#endif
    }

    return s32Ret;
}
#endif
#if 0
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_FAILURE;

    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
    
    s32Ret = SAMPLE_VIO_1080P();
			
    if (HI_SUCCESS == s32Ret)
        printf("program exit normally!\n");
    else
        printf("program exit abnormally!\n");
    exit(s32Ret);
}
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

