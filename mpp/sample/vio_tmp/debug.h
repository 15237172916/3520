///*****************************************
//  Copyright (C) 2009-2014
//  ITE Tech. Inc. All Rights Reserved
//  Proprietary and Confidential
///*****************************************
//   @file   <debug.h>
//   @author Max.Kao@ite.com.tw
//   @date   2014/09/25
//   @fileversion: ITE_MHLRX_SAMPLE_V1.11
//******************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_



#ifndef Debug_message
#define Debug_message 1
#endif


#if Debug_message
	#include <stdio.h>
	#define MHLRX_DEBUG_PRINTF(x)  printf  x
	#define EQ_DEBUG_PRINTF(x) printf  x
	#define EQ_PORT0_PRINTF(x) printf  x
	#define EQ_PORT1_PRINTF(x) printf  x
	#define VIDEOTIMNG_DEBUG_PRINTF(x)  printf  x
	#define IT6802_DEBUG_INT_PRINTF(x)   printf  x
	#define IT6802_MHL_DEBUG_PRINTF(x) printf x
	#define MHL_MSC_DEBUG_PRINTF(x) printf x
	#define HDMIRX_VIDEO_PRINTF(x)   printf  x
	#define HDMIRX_AUDIO_PRINTF(x)  printf  x
	#define HDMIRX_DEBUG_PRINT(x) printf x
	#define CEC_DEBUG_PRINTF(x) //printf x
	#define EDID_DEBUG_PRINTF(x) //printf  x
	#define IT680X_DEBUG_PRINTF(x) printf  x
	#define VSDB_DEBUG_PRINTF(x) //printf  x
	#define RCP_DEBUG_PRINTF(x)  printf  x
	#define MHL3D_DEBUG_PRINTF(x) //printf  x
	#define CBUS_CAL_PRINTF(x)  printf  x
	#define MHL_INT_PRINTF(x)  printf  x
#else
	#define MHLRX_DEBUG_PRINTF(x)
	#define EQ_DEBUG_PRINTF(x)
	#define EQ_PORT0_PRINTF(x)
	#define EQ_PORT1_PRINTF(x)
	#define VIDEOTIMNG_DEBUG_PRINTF(x)
	#define IT6802_DEBUG_INT_PRINTF(x)
	#define IT6802_MHL_DEBUG_PRINTF(x)
	#define MHL_MSC_DEBUG_PRINTF(x)
	#define HDMIRX_VIDEO_PRINTF(x)
	#define HDMIRX_AUDIO_PRINTF(x)
	#define HDMIRX_DEBUG_PRINT(x)
	#define CEC_DEBUG_PRINTF(x)
	#define EDID_DEBUG_PRINTF(x)
	#define IT680X_DEBUG_PRINTF(x)
	#define VSDB_DEBUG_PRINTF(x)
	#define RCP_DEBUG_PRINTF(x)
	#define MHL3D_DEBUG_PRINTF(x)
	#define CBUS_CAL_PRINTF(x)
	#define MHL_INT_PRINTF(x)
#endif




#endif
