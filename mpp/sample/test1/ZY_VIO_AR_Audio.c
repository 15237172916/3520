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

#include "sample_comm.h"

#include "HDMIRX.h"
#include "ZY_HDMIRX.h"
#include "typedef.h"
#include "Mhlrx.h"
#include "it6801.h"
//OSD
#include "loadbmp.h"
#include "hi_tde_api.h"
#include "hi_tde_type.h"

//#define HDMI_SUPPORT
IT6801_INFO_S IT6801_INFO;
//HDMIRX_INFO GET_HDMI_INFO;

//HDMIRX_INFO GET_HDMI_INFO_CHN2;

HI_BOOL HDMI_INPUT_CHANGE = HI_FALSE;
HI_BOOL HDMI_INPUT_CHANGE_CHN2 = HI_FALSE;
HI_BOOL RE_START_FLAG = HI_FALSE; 
#define SAMPLE_DBG(s32Ret)\
do{\
    printf("s32Ret=%#x,fuc:%s,line:%d\n", s32Ret, __FUNCTION__, __LINE__);\
}while(0)

/******************************************************************************
* function : to delay x ms 
******************************************************************************/
void delay_ms( unsigned int ms)
{
	usleep( ms*1000);
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
#if 0
/******************************************************************************
* function :Thread to Detect HDMI
******************************************************************************/
void *HDMI_Detect_Thread(HI_VOID)
{
    fd_set RDS; 
    HI_S32 s32Ret = 0;
    int HDMI_Device_FD = 0;
    HI_BOOL HDMI_CHANGE = HI_FALSE;
    HDMIRX_INFO TMP_HDMI_INFO;
    
    memset(&TMP_HDMI_INFO, 0, sizeof(HDMIRX_INFO));

    HDMI_Device_FD = open(HDMIRX_DEVICE,O_RDWR);
    if(HDMI_Device_FD < 0)
    {
        printf("HDMI_Device Open Fail!\r\n");
        return ;
    }
    
    ioctl(HDMI_Device_FD, IOC_GET_HDMIRX_INFO, &TMP_HDMI_INFO);
    memcpy(&GET_HDMI_INFO,&TMP_HDMI_INFO,sizeof(HDMIRX_INFO));
    
    while(1)
    {
    	FD_ZERO(&RDS);   
        FD_SET(HDMI_Device_FD, &RDS);
    	s32Ret = select(HDMI_Device_FD + 1, &RDS, NULL, NULL, NULL);
    	if (s32Ret < 0)  
    	{
    	    printf("Select Error!\r\n");
    	    return;
    	}
    	else if(s32Ret > 0)
    	{
    	        ioctl(HDMI_Device_FD, IOC_GET_HDMIRX_INFO, &TMP_HDMI_INFO);
    		HDMI_CHANGE = IS_HDMIRX_CHANGE(&GET_HDMI_INFO, &TMP_HDMI_INFO);
    		if(HDMI_CHANGE == HI_TRUE)
    		{
                    HDMI_INPUT_CHANGE = HI_TRUE;
    		}
    		else
    		{
                    HDMI_INPUT_CHANGE = HI_FALSE;
    		}
    	}
    	else
    	{
    		printf("Time Out!\r\n");
    	}		
    	
        if(HDMI_INPUT_CHANGE == HI_TRUE)
        {
            HDMI_INPUT_CHANGE = HI_FALSE;
            RE_START_FLAG = HI_TRUE; 
            printf("HDMI INPUT CHANGE !!! \r\n");
            HDMIRX_VI_ReSetChn(HDMIRX_VI_MODE_1_BT1120, &GET_HDMI_INFO);
        }
    }
}
#endif
#if 0
void *HDMI_CHN2_Detect_Thread(HI_VOID)
{
    fd_set RDS; 
    HI_S32 s32Ret = 0;
    int HDMI_Device_FD = 0;
    HI_BOOL HDMI_CHANGE = HI_FALSE;
    HDMIRX_INFO TMP_HDMI_INFO;
    
    memset(&TMP_HDMI_INFO, 0, sizeof(HDMIRX_INFO));

    HDMI_Device_FD = open(HDMIRX_DEVICE_CHN2,O_RDWR);
    if(HDMI_Device_FD < 0)
    {
        printf("HDMIRX_DEVICE_CHN2 Open Fail!\r\n");
        return ;
    }
    
    ioctl(HDMI_Device_FD, IOC_GET_HDMIRX_INFO, &TMP_HDMI_INFO);
    memcpy(&GET_HDMI_INFO_CHN2,&TMP_HDMI_INFO,sizeof(HDMIRX_INFO));
    
    while(1)
    {
    	FD_ZERO(&RDS);   
        FD_SET(HDMI_Device_FD, &RDS);
    	s32Ret = select(HDMI_Device_FD + 1, &RDS, NULL, NULL, NULL);
    	if (s32Ret < 0)  
    	{
    	    printf("Select Error!\r\n");
    	    return;
    	}
    	else if(s32Ret > 0)
    	{
    	        ioctl(HDMI_Device_FD, IOC_GET_HDMIRX_INFO, &TMP_HDMI_INFO);
    		HDMI_CHANGE = IS_HDMIRX_CHANGE(&GET_HDMI_INFO_CHN2, &TMP_HDMI_INFO);
    		if(HDMI_CHANGE == HI_TRUE)
    		{
                    HDMI_INPUT_CHANGE_CHN2 = HI_TRUE;
    		}
    		else
    		{
                    HDMI_INPUT_CHANGE_CHN2 = HI_FALSE;
    		}
    	}
    	else
    	{
    		printf("Time Out!\r\n");
    	}		    
    }
}
#endif

typedef struct CONFIG_PARAM
{
    /*********** video ***********/
    PIC_SIZE_E PIC_SIZE; //hdmi input picture size
    VO_INTF_SYNC_E VO_INTF_SYNC; //output video's format
    /*********** audio **********/
    AUDIO_SAMPLE_RATE_E AUDIO_RARE;
    AUDIO_BIT_WIDTH_E AUDIO_BIT_WIDTH;

}CONFIG_PARAM_S;

int GetAiViConfigParam(pIT6801_INFO_S pIT6801Info, CONFIG_PARAM_S *AiViConfigParam)
{
    GetIt6801Info(pIT6801Info);
    printf("---------pIT6801Info->V_Active: %d -----------\n", pIT6801Info->V_Active);
    printf("*********pIT6801Info->H_Active: %d ***********\n", pIT6801Info->H_Active);
    
    /*************************** video information *********************************/
    if ((768==pIT6801Info->V_Active) && (1024==pIT6801Info->H_Active))
    {
        AiViConfigParam->PIC_SIZE = PIC_XGA;  /* 1024 * 768 */ 
        AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1024x768_60;
    }
    else if ((1050==pIT6801Info->V_Active) && (1400==pIT6801Info->H_Active))
    {
        AiViConfigParam->PIC_SIZE = PIC_SXGA;  /* 1400 * 1050 */   
        AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1680x1050_60;
    }
    else if ((1200==pIT6801Info->V_Active) && (1600==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_UXGA;   /* 1600 * 1200 */    
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1600x1200_60;
    }
    else if ((1536==pIT6801Info->V_Active) && (2048==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_QXGA;   /* 2048 * 1536 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_2560x1600_60;
    }
    else if ((480==pIT6801Info->V_Active) && (854==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_WVGA;   /* 854 * 480 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_480P60;
    }
    else if ((1050==pIT6801Info->V_Active) && (1680==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_WSXGA;  /* 1680 * 1050 */   
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1680x1050_60;
    }
    else if ((1200==pIT6801Info->V_Active) && (1920==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_WUXGA;  /* 1920 * 1200 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1920x1200_60;
    }
    else if ((1600==pIT6801Info->V_Active) && (2560==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_WQXGA;  /* 2560 * 1600 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_2560x1600_60;
    }
    else if ((720==pIT6801Info->V_Active) && (1280==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_HD720;  /* 1280 * 720 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_720P60;
    }
    else if ((1080==pIT6801Info->V_Active) && (1920==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_HD1080;  /* 1920 * 1080 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_1080P60;
    }
    else if ((2160==pIT6801Info->V_Active) && (3840==pIT6801Info->H_Active))
    {
       AiViConfigParam->PIC_SIZE = PIC_UHD4K;  /* 3840*2160 */
       AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_3840x2160_60;
    }
    else
    {
        AiViConfigParam->PIC_SIZE = PIC_HD720;  /* 1280 * 720 */
        AiViConfigParam->VO_INTF_SYNC = VO_OUTPUT_720P60;
        printf("unsupport video resolution \n");
    }
    
    /****************************** audio information end********************************/



    return 0;
}

//#define VENC_ENABLE
//#define AUDIO_ENABLE
#define VO_LOOP_ENABLE
//#define VDEC_ENABLE
#define OSD_ENABLE
int main(int argc, char *argv[])
{
    HDMIRX_VI_MODE_E HDMIRXViMode = HDMIRX_VI_MODE_1_BT1120; //HDMIRX_VI_MODE_1_BT1120;
    
    HI_S32 s32Ret = HI_SUCCESS;
    CONFIG_PARAM_S HDMI_INFO_S;
    pthread_t ZY_HDMI_thread;
   
    signal(SIGINT, SAMPLE_VIO_HandleSig);
    signal(SIGTERM, SAMPLE_VIO_HandleSig);
    
    //memset(&GET_HDMI_INFO, 0, sizeof(HDMIRX_INFO));
#if 0
    pthread_create(&ZY_HDMI_thread,NULL,HDMI_Detect_Thread,NULL);
#endif
#if 1
    printf("it8601 \n");
    pthread_create(&ZY_HDMI_thread,NULL,it6801_main,NULL);
    //while (1) 
        sleep(1);
#endif
#if 1
    delay_ms(100);
ReStart:
    GetAiViConfigParam(&IT6801_INFO, &HDMI_INFO_S); 

    VIDEO_NORM_E enNorm = VIDEO_ENCODING_MODE_PAL;

    HI_U32 u32ViChnCnt = 1;
    HI_S32 s32VpssGrpCnt = 1;

    VB_CONF_S stVbConf;
    VPSS_GRP VpssGrp;
    VPSS_GRP_ATTR_S stGrpAttr;
   
    HI_S32 i;
    HI_U32 u32BlkSize;
    HI_S32 ch;
    SIZE_S stSize;
    HI_U32 u32WndNum;
    HI_U32 vichn;
    PIC_SIZE_E inSize = HDMI_INFO_S.PIC_SIZE;

    SAMPLE_VI_PARAM_S stViParam_HDMIRX;
    SIZE_S stTargetSize_HDMIRX;
    RECT_S stCapRect_HDMIRX;

#ifdef VO_LOOP_ENABLE
    VO_DEV VoDev;
    VO_LAYER VoLayer;
    VO_CHN VoChn;
	VPSS_CHN VpssChn_VoHD1 = VPSS_CHN2;
    VO_PUB_ATTR_S stVoPubAttr_hd1;
    VO_VIDEO_LAYER_ATTR_S stLayerAttr;
    SAMPLE_VO_MODE_E enVoMode, enPreVoMode;
#endif

#ifdef VENC_ENABLE
	SAMPLE_RC_E enRcMode= SAMPLE_RC_CBR;
	HI_S32 s32ChnNum = 1;
    VPSS_CHN VpssChn_Venc = 1;
	VENC_CHN VencChn;
	PAYLOAD_TYPE_E enPayLoad = PT_H265;
	PIC_SIZE_E enSize = inSize;
	HI_U32 u32Profile = 2; //0:Baseline, 1:MP 2:HP
	VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;
#endif

#ifdef AUDIO_ENABLE
    HI_S32 s32AiChnCnt, s32AoChnCnt;
    AUDIO_DEV   AiDev = SAMPLE_AUDIO_AI_DEV;
    AI_CHN      AiChn = 0;
    AUDIO_DEV   AoDev = 2;
    AO_CHN      AoChn = 0;
    AUDIO_RESAMPLE_ATTR_S stAiReSampleAttr;
    AUDIO_RESAMPLE_ATTR_S stAoReSampleAttr;
    
    AIO_ATTR_S stAioAttr;
    AIO_ATTR_S stHdmiAoAttr;

    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_SLAVE;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_STEREO;
    stAioAttr.u32EXFlag      = 1;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 2;
    stAioAttr.u32ClkChnCnt   = 2;
    stAioAttr.u32ClkSel      = 0; 

    stHdmiAoAttr.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
    stHdmiAoAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stHdmiAoAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stHdmiAoAttr.enSoundmode    = AUDIO_SOUND_MODE_STEREO;
    stHdmiAoAttr.u32EXFlag      = 1;
    stHdmiAoAttr.u32FrmNum      = 30;
    stHdmiAoAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stHdmiAoAttr.u32ChnCnt      = 2;
    stHdmiAoAttr.u32ClkChnCnt   = 2;
    stHdmiAoAttr.u32ClkSel      = 0;  
#endif
#ifdef OSD_ENABLE
    HI_S32 u32RgnOverlayNum;
    HI_S32 u32RgnCoverNum;
    BITMAP_S stBitmap;
    RGN_HANDLE OverlayHandle;
    VDEC_SENDPARAM_S stVdesSendPram;
    RGN_ATTR_INFO_S stRgnAttrInfo0;
    RGN_ATTR_INFO_S stRgnAttrInfo1;
#endif
    //MPP_VERSION_S * p = malloc(sizeof(MPP_VERSION_S));
    //HI_MPI_SYS_GetVersion(p);
    //printf("%s \n", *p);
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
    	goto ZY_END_0;
    }

#ifdef AUDIO_ENABLE
    s32AiChnCnt = stAioAttr.u32ChnCnt >> stAioAttr.enSoundmode;
	//printf("s32AiChnCnt:%d \n", s32AiChnCnt); //s32AiChnCnt: 1
    s32Ret = SAMPLE_COMM_AUDIO_StartAi(AiDev, s32AiChnCnt, &stAioAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE, NULL, 0);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return HI_FAILURE;
    }
#endif
#ifdef OSD_ENABLE
    OverlayHandle    = 0;
    u32RgnOverlayNum = 1; //OVERLAY_MAX_NUM_VPSS / 2; // 8 / 2
    s32Ret = SAMPLE_RGN_CreateOverlayForVpss(OverlayHandle, u32RgnOverlayNum); //creat overlay atten
    if(HI_SUCCESS != s32Ret)
    {
        printf("SAMPLE_RGN_CreateOverlayForVpss failed! s32Ret: 0x%x.\n", s32Ret);
        //goto END_OC_VPSS1;
    }
#endif
    /******************************************
     step 3: start vi dev & chn
    ******************************************/
    //  For HDMIRX 
    s32Ret = HDMIRX_VI_Start(HDMIRXViMode, &IT6801_INFO);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("HDMIRX_VI_Start failed!\n");
        goto ZY_END_0;
    }

	/******************************************
	 step 4: start vpss and vi bind vpss
	******************************************/
	//s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, PIC_UHD4K, &stSize);
	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enNorm, inSize, &stSize);	
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed!\n");
		goto ZY_END_1;
	}

	memset(&stGrpAttr,0,sizeof(VPSS_GRP_ATTR_S));
	stGrpAttr.u32MaxW = stSize.u32Width;
	stGrpAttr.u32MaxH = stSize.u32Height;
	stGrpAttr.bNrEn = HI_TRUE;
    stGrpAttr.enDieMode = VPSS_DIE_MODE_AUTO;//VPSS_DIE_MODE_DIE;//VPSS_DIE_MODE_NODIE;
	stGrpAttr.enPixFmt = SAMPLE_PIXEL_FORMAT;
	s32Ret = SAMPLE_COMM_VPSS_Start(s32VpssGrpCnt, &stSize, VPSS_MAX_CHN_NUM, &stGrpAttr);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start Vpss failed!\n");
		goto ZY_END_1;
	}

    s32Ret = HDMIRX_VI_BindVpss(HDMIRXViMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Vi bind Vpss failed!\n");
		goto ZY_END_2;
	}
#ifdef AUDIO_ENABLE
	s32Ret = SAMPLE_COMM_AUDIO_StartAo(AoDev, s32AiChnCnt, &stHdmiAoAttr, AUDIO_SAMPLE_RATE_BUTT, HI_FALSE, NULL, 0);
	if (s32Ret != HI_SUCCESS)
	{
		SAMPLE_PRT("system init failed with %d!\n", s32Ret);
		return HI_FAILURE;
	}
    /* AI to AO channel */
	#if 1
	for (i=0; i<s32AiChnCnt; i++)
	{
		AiChn = i;
		AoChn = i;
		s32Ret = SAMPLE_COMM_AUDIO_CreatTrdAiAo(AiDev, AiChn, AoDev, AoChn);
		//s32Ret = SAMPLE_COMM_AUDIO_CreatTrdFileAdec(AoChn, fopen("./test.pcm","r"));
	    if (s32Ret != HI_SUCCESS)
	    {
	        SAMPLE_DBG(s32Ret);
	        return HI_FAILURE;
	    }
	}
	#endif
#endif
#ifdef VO_LOOP_ENABLE
    /******************************************
	 step 5: start vo HD1 (HDMI), multi-screen, you can switch mode
	******************************************/
	printf("start vo HD1.\n");
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;

	stVoPubAttr_hd1.enIntfSync = HDMI_INFO_S.VO_INTF_SYNC; //VO_OUTPUT_720P60;//VO_OUTPUT_1080P60;
	stVoPubAttr_hd1.enIntfType = VO_INTF_HDMI;//|VO_INTF_VGA;
	stVoPubAttr_hd1.u32BgColor = 0x0;
#ifndef AUDIO_ENABLE
//#if 1
	s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr_hd1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartDev failed!\n");
		goto ZY_END_6;
	}
#endif
	memset(&(stLayerAttr), 0 , sizeof(VO_VIDEO_LAYER_ATTR_S));
	s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr_hd1.enIntfSync, \
		&stLayerAttr.stImageSize.u32Width, \
		&stLayerAttr.stImageSize.u32Height, \
		&stLayerAttr.u32DispFrmRt);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_GetWH failed!\n");
		goto ZY_END_6;
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
		goto ZY_END_6;
	}

	s32Ret = HDMIRX_VO_StartChn(VoLayer, enVoMode);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start SAMPLE_COMM_VO_StartChn failed!\n");
		goto ZY_END_7;
	}
#ifndef AUDIO_ENABLE
//#if 1
	/* if it's displayed on HDMI, we should start HDMI */
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr_hd1.enIntfSync))
		{
			SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
			goto ZY_END_7;
		}
	}
#endif
	VoChn = 0;
	VpssGrp = 0;

	s32Ret = SAMPLE_COMM_VO_BindVpss(VoDev,VoChn,VpssGrp,VpssChn_VoHD1);
	if (HI_SUCCESS != s32Ret)
	{
		SAMPLE_PRT("Start VO failed!\n");
		goto ZY_END_7;
	}
#endif

#ifdef VENC_ENABLE
    /******************************************
     step 6: start stream venc
    ******************************************/
    /*** 4K **/
    enRcMode = SAMPLE_RC_CBR;
	/*** enSize[0] **/
	if (s32ChnNum >= 1)
	{
        VpssGrp = 0; //
	    VencChn = 0; //video encode channel

		printf("sample stream venc \n");
	    s32Ret = SAMPLE_COMM_VENC_Start(VencChn, enPayLoad,\
	                                   gs_enNorm, enSize, enRcMode,u32Profile);
	    if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Start Venc failed!\n");
	    }

        //venc bind 
        printf("venc bind vpss \n");
	    s32Ret = SAMPLE_COMM_VENC_BindVpss(VencChn, VpssGrp, VpssChn_Venc);
	    if (HI_SUCCESS != s32Ret)
	    {
	        SAMPLE_PRT("Start Venc failed!\n");
	    
	    }
    }

    /******************************************
     step 7: stream venc process -- get stream, then save it to file. 
    ******************************************/
    printf("get stream venc process and save it to file \n");
    s32Ret = SAMPLE_COMM_VENC_StartGetStream(s32ChnNum);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("Start Venc failed!\n");
    }
#endif 

#ifdef VDEC_ENABLE
	VO_DEV VoDev;
	VO_PUB_ATTR_S stVoPubAttr;
	VO_VIDEO_LAYER_ATTR_S stVoLayerAttr;
	VO_LAYER VoLayer;

	HI_U32 u32VdCnt = 4, u32GrpCnt = 1;
	VDEC_CHN_ATTR_S stVdecChnAttr[VDEC_MAX_CHN_NUM];
	VDEC_CHN_PARAM_S stChnParam;
	HI_BOOL bVdecCompress = HI_FALSE;
	VdecThreadParam stVdecSend[VDEC_MAX_CHN_NUM];
	pthread_t	VdecThread[2*VDEC_MAX_CHN_NUM];

    /************************************************
    step 8:  start VDEC
    *************************************************/

    SAMPLE_COMM_VDEC_ChnAttr(u32GrpCnt, &stVdecChnAttr[0], PT_H265, &stSize);

    s32Ret = SAMPLE_COMM_VDEC_Start(u32GrpCnt, &stVdecChnAttr[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        //goto END2;
    }
    /***  set the protocol param (should stop to receive stream first) ***/
    for(i=0; i<u32GrpCnt; i++)
    {
        s32Ret = HI_MPI_VDEC_StopRecvStream(i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("stop VDEC fail for %#x!\n", s32Ret);
            //goto END2;
        }
        VDEC_PRTCL_PARAM_S stPrtclParam;
        HI_MPI_VDEC_GetProtocolParam(i, &stPrtclParam);
        stPrtclParam.enType = PT_H265;
        stPrtclParam.stH265PrtclParam.s32MaxSpsNum   = 2;
        stPrtclParam.stH265PrtclParam.s32MaxPpsNum   = 55;
        stPrtclParam.stH265PrtclParam.s32MaxSliceSegmentNum = 100;
        stPrtclParam.stH265PrtclParam.s32MaxVpsNum = 10;
        s32Ret = HI_MPI_VDEC_SetProtocolParam(i, &stPrtclParam);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("stop VDEC fail for %#x!\n", s32Ret);
            //goto END2;
        }

        HI_MPI_VDEC_GetChnParam(i, &stChnParam);
        stChnParam.enCompressMode = bVdecCompress ? COMPRESS_MODE_SEG128 : COMPRESS_MODE_NONE;
        s32Ret = HI_MPI_VDEC_SetChnParam(i, &stChnParam);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("set compress fail for %#x!\n", s32Ret);
            //goto END2;
        }

        s32Ret = HI_MPI_VDEC_StartRecvStream(i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
            //goto END2;
        }
    }

	/************************************************
    step 9:  start VO
    *************************************************/

    VoDev = SAMPLE_VO_DEV_DHD0;
    VoLayer = SAMPLE_VO_LAYER_VHD0;

    stVoPubAttr.enIntfSync = VO_OUTPUT_1080P60;
    stVoPubAttr.enIntfType = VO_INTF_VGA |VO_INTF_HDMI;

    s32Ret = SAMPLE_COMM_VO_StartDev(VoDev, &stVoPubAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
        //goto END4_1;
    }

    if (HI_SUCCESS != SAMPLE_COMM_VO_HdmiStart(stVoPubAttr.enIntfSync))
    {
        SAMPLE_PRT("Start SAMPLE_COMM_VO_HdmiStart failed!\n");
        //goto END4_2;
    }

    s32Ret = SAMPLE_COMM_VO_GetWH(stVoPubAttr.enIntfSync, \
        &stVoLayerAttr.stDispRect.u32Width, &stVoLayerAttr.stDispRect.u32Height, &stVoLayerAttr.u32DispFrmRt);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("failed with %#x!\n", s32Ret);
        //goto  END4_2;
    }
    stVoLayerAttr.stImageSize.u32Width = stVoLayerAttr.stDispRect.u32Width;
    stVoLayerAttr.stImageSize.u32Height = stVoLayerAttr.stDispRect.u32Height;
    stVoLayerAttr.bClusterMode = HI_FALSE;
    stVoLayerAttr.bDoubleFrame = HI_FALSE;
    stVoLayerAttr.enPixFormat = SAMPLE_PIXEL_FORMAT;
    s32Ret = SAMPLE_COMM_VO_StartLayer(VoLayer, &stVoLayerAttr);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartLayer fail for %#x!\n", s32Ret);
        //goto END4_3;
    }

    //s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, VO_MODE_1MUX);
    s32Ret = SAMPLE_COMM_VO_StartChn(VoLayer, VO_MODE_1MUX);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("SAMPLE_COMM_VO_StartChn fail for %#x!\n", s32Ret);
        //goto END4_4;
    }

    /************************************************
    step 10:  VDEC bind VPSS
    *************************************************/
    for(i=0; i<u32GrpCnt; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_BindVpss(i, 2+i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
            //goto END5;
        }
    }

 	/************************************************
    step 11:  VPSS bind VO
    *************************************************/
    s32Ret = SAMPLE_COMM_VO_BindVpss(VoLayer, 0, 2, VPSS_CHN0); //2, 3 is enable
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("vpss bind vo fail for %#x!\n", s32Ret);
        //goto END6;
    }

	/************************************************
    step 12:  send stream to VDEC
    *************************************************/
    SAMPLE_COMM_VDEC_ThreadParam(u32GrpCnt, &stVdecSend[0], &stVdecChnAttr[0], SAMPLE_4K_H265_File_PATH);
    SAMPLE_COMM_VDEC_StartSendStream(u32GrpCnt, &stVdecSend[0], &VdecThread[0]);

    /***  get the stat info of luma pix  ***/
    SAMPLE_COMM_VDEC_StartGetLuma(u32GrpCnt, &stVdecSend[0], &VdecThread[0]);
#endif
	
    //PRINTF_HDMIRX_INFO(&GET_HDMI_INFO);	
    //printf("\nplease press twice ENTER to exit this sample\n");
    //getchar();
    //getchar();
    while (1)
    {
        //printf("\n\n--------input change : %d \n", HDMI_INPUT_CHANGE);
        if (HI_TRUE == RE_START_FLAG)
        {
            RE_START_FLAG = HI_FALSE;
            #ifdef VENC_ENABLE
            SAMPLE_COMM_VENC_StopGetStream();
            #endif
            sleep(1);
            goto ReStart;
            //goto ZY_END_7;
        }
        
        usleep(100000);
    }
#ifdef AUDIO_ENABLE
   for (i=0; i<s32AiChnCnt; i++)
    {
    	AiChn = i;
        s32Ret = SAMPLE_COMM_AUDIO_DestoryTrdAi(AiDev, AiChn);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_DBG(s32Ret);
            return HI_FAILURE;
        }
    }

    s32Ret = SAMPLE_COMM_AUDIO_StopAi(AiDev, s32AiChnCnt, HI_FALSE, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }

    s32Ret = SAMPLE_COMM_AUDIO_StopAo(AoDev, stHdmiAoAttr.u32ChnCnt, HI_TRUE, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        SAMPLE_DBG(s32Ret);
        return HI_FAILURE;
    }
#endif
    pthread_kill(ZY_HDMI_thread, SIGUSR2);
    pthread_join(ZY_HDMI_thread, NULL);
#ifdef VDENC_ENABLE
    SAMPLE_COMM_VDEC_StopSendStream(u32GrpCnt, &stVdecSend[0], &VdecThread[0]);

    SAMPLE_COMM_VDEC_StopGetLuma(u32GrpCnt, &stVdecSend[0], &VdecThread[0]);
#endif

#ifdef VENC_ENABLE
   SAMPLE_COMM_VENC_StopGetStream();
#endif
/******************************************
 step 7: exit process
******************************************/

ZY_END_7:
	#ifdef HDMI_SUPPORT
	if (stVoPubAttr_hd1.enIntfType & VO_INTF_HDMI)
	{
		SAMPLE_COMM_VO_HdmiStop();
	}
	#endif
#ifdef VENC_ENABLE
    VpssGrp = 0; //
	VencChn = 0;
    VpssChn_Venc = 1;
    SAMPLE_COMM_VENC_UnBindVpss(VencChn, VpssGrp, VpssChn_Venc);
    SAMPLE_COMM_VENC_Stop(VencChn);

#endif
#ifdef VO_LOOP_ENABLE
	VoDev = SAMPLE_VO_DEV_DHD0;
	VoLayer = SAMPLE_VO_LAYER_VHD0;
	u32WndNum = 1;
	enVoMode = VO_MODE_1MUX;
	SAMPLE_COMM_VO_StopChn(VoLayer, enVoMode);
	for(i=0;i<u32WndNum;i++)
	{
		VoChn = i;
		VpssGrp = i;
		SAMPLE_COMM_VO_UnBindVpss(VoLayer,VoChn,VpssGrp,VpssChn_VoHD1);
	}
	SAMPLE_COMM_VO_StopLayer(VoLayer);
#endif
ZY_END_6:
#ifdef VO_LOOP_ENABLE
	SAMPLE_COMM_VO_StopDev(VoDev);
#endif
ZY_END_4:
#ifdef VO_LOOP_ENABLE
	SAMPLE_COMM_VO_StopDev(VoDev);
#endif
ZY_END_3:	//vi unbind vpss
	HDMIRX_VI_UnBindVpss(HDMIRXViMode);
ZY_END_2:	//vpss stop
	SAMPLE_COMM_VPSS_Stop(s32VpssGrpCnt, VPSS_MAX_CHN_NUM);
ZY_END_1:	//vi stop
	HDMIRX_VI_Stop(HDMIRXViMode);
ZY_END_0:	//system exit
	SAMPLE_COMM_SYS_Exit();
 #endif   
	return s32Ret;
	
    exit(s32Ret);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

