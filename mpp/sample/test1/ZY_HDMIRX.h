///*****************************************
//   @file   <ZY_HDMIRX.h>
//   @author limingqiang@zhiyang-tech.com
//   @date   2018/12/21
//   @fileversion: ZY_HDMIRX_V1.00
//******************************************/
#ifndef _ZY_HDMIRX_H_
#define _ZY_HDMIRX_H_

#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vdec.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vdec.h"

#include "sample_comm.h"
#include "HDMIRX.h"

#define ZY_Hi3531DV100_1_HDMI   1
//#define ZY_Hi3531DV100_2_HDMI   1
//#define ZY_Hi3531DV100_4_HDMI   1
//#define ZY_Hi3531DV100_8_HDMI   1

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

typedef enum HDMIRX_vi_mode_e
{
    HDMIRX_VI_MODE_1_BT656,
    HDMIRX_VI_MODE_2_BT656,
    HDMIRX_VI_MODE_4_BT656,
    HDMIRX_VI_MODE_8_BT656,
    HDMIRX_VI_MODE_16_BT656,
    HDMIRX_VI_MODE_1_BT1120,
    HDMIRX_VI_MODE_2_BT1120,
    HDMIRX_VI_MODE_4_BT1120,
    HDMIRX_VI_MODE_8_BT1120,
    HDMIRX_VI_MODE_1_BT1120I,
    HDMIRX_VI_MODE_2_BT1120I,
    HDMIRX_VI_MODE_4_BT1120I,
    HDMIRX_VI_MODE_8_BT1120I,
}HDMIRX_VI_MODE_E;

HI_BOOL IS_HDMI_CHANGE(pHDMIRX_INFO Pre_HDMI_INFO, pHDMIRX_INFO TMP_HDMI_INFO);
HI_S32 HDMIRX_VI_Mode2Param(HDMIRX_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam);
HI_S32 HDMIRX_VI_Mode2Size(pHDMIRX_INFO TMP_HDMI_INFO, RECT_S *pstCapRect, SIZE_S *pstDestSize);
HI_S32 HDMIRX_VI_StartDev(VI_DEV ViDev, HDMIRX_VI_MODE_E enViMode);
HI_S32 HDMIRX_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, 
    pHDMIRX_INFO TMP_HDMI_INFO, SAMPLE_VI_CHN_SET_E enViChnSet);
HI_S32 HDMIRX_VI_Start(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO);
HI_S32 HDMIRX_VI_Stop(HDMIRX_VI_MODE_E HDMIRXViMode);
HI_S32 HDMIRX_VI_BindVpss(HDMIRX_VI_MODE_E HDMIRXViMode);
HI_S32 HDMIRX_VI_UnBindVpss(HDMIRX_VI_MODE_E HDMIRXViMode);
HI_S32 HDMIRX_VI_ReSetChn(HDMIRX_VI_MODE_E HDMIRXViMode, pHDMIRX_INFO TMP_HDMI_INFO);









#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif
























