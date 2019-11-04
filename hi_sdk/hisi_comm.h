/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   hisi_comm.h
 * Author: ussh
 *
 * Created on 17 июня 2019 г., 17:33
 */

#ifndef HISI_COMM_H
#define HISI_COMM_H

#include "tw2865.h"
#include "hi_common.h"
#include "hi_comm_sys.h"
#include "hi_comm_vb.h"
#include "hi_comm_isp.h"
#include "hi_comm_vi.h"
#include "hi_comm_vo.h"
#include "hi_comm_venc.h"
#include "hi_comm_vpss.h"
#include "hi_comm_vdec.h"
#include "hi_comm_vda.h"
#include "hi_comm_region.h"
#include "hi_comm_adec.h"
#include "hi_comm_aenc.h"
#include "hi_comm_ai.h"
#include "hi_comm_ao.h"
#include "hi_comm_aio.h"
#include "hi_comm_isp.h"
#include "hi_defines.h"

#include "mpi_sys.h"
#include "mpi_vb.h"
#include "mpi_vi.h"
#include "mpi_vo.h"
#include "mpi_venc.h"
#include "mpi_vpss.h"
#include "mpi_vdec.h"
#include "mpi_vda.h"
#include "mpi_region.h"
#include "mpi_adec.h"
#include "mpi_aenc.h"
#include "mpi_ai.h"
#include "mpi_ao.h"
#include "mpi_isp.h"

// -DISP_V2
#define ISP_V2

#ifdef ISP_V2
#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_af.h"
#endif
#include "hi_sns_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/*******************************************************
    macro define 
*******************************************************/
#define ALIGN_BACK(x, a)              ((a) * (((x) / (a))))
#define HISI_SYS_ALIGN_WIDTH      64
#define HISI_PIXEL_FORMAT         PIXEL_FORMAT_YUV_SEMIPLANAR_420

#define TW2865_FILE "/dev/tw2865dev"
#define TW2960_FILE "/dev/tw2960dev"
#define TLV320_FILE "/dev/tlv320aic31"

/* Maximum supported audio devices */
#define HISI_MAX_DEV_COUNT               2
/* Maximum supported device name */
#define HISI_MAX_DEV_LENGTH_NAME         256
    
#define HISI_DEFAULT_FREQ                8000
#define HISI_MAX_CHANNELS                2
//#define HISI_MAX_CHANNELS                1
#define HISI_BITS_X_SAMPLE        16                  /**< Bits per sample */
/* Default milliseconds per buffer */
// #define HISI_MSECOND_PER_BUFFER          16
//#define HISI_MSECOND_PER_BUFFER          10
#define HISI_MSECOND_PER_BUFFER          20
/* Only for supported systems */
#define HISI_STARTING_INPUT_VOLUME       50
/* Only for supported systems */
#define HISI_STARTING_OUTPUT_VOLUME      100
    
    
//#define SAMPLE_AUDIO_PTNUMPERFRM   640
//#define SAMPLE_AUDIO_PTNUMPERFRM   320
//#define SAMPLE_AUDIO_PTNUMPERFRM   80
#define SAMPLE_AUDIO_PTNUMPERFRM   160
    
    
static HI_BOOL gs_bAiReSample   = HI_FALSE;
static HI_BOOL gs_bAoReSample   = HI_FALSE;
static HI_BOOL gs_bUserGetMode  = HI_FALSE;
static HI_BOOL gs_bAoVolumeCtrl = HI_FALSE;
// static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_16000;
// static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_16000;


static AUDIO_SAMPLE_RATE_E enAudioSampleRate = AUDIO_SAMPLE_RATE_8000;
static AUDIO_SAMPLE_RATE_E enDeviceAudioSampleRate = AUDIO_SAMPLE_RATE_8000;


#define HISI_AUDIO_AI_DEV 0
#define HISI_AUDIO_AO_DEV 0

#if HICHIP == HI3518_V100 //FIXME
#define HISI_VO_DEV_DSD1 0
#define HISI_VO_DEV_DSD0 HISI_VO_DEV_DSD1
#else
#error HICHIP define may be error
#endif

/*** for init global parameter ***/
#define HISI_ENABLE 		    1
#define HISI_DISABLE 		    0
#define HISI_NOUSE 		    -1


#define HISI_AUDIO_HDMI_AO_DEV 3

static char json_doc1[] = 
"{\
    \"sensor\": {\
        \"lib_name\": \"libsns_imx138.so\",\
        \"sensor_name\": \"imx138\",\
        \"s_enNorm\": 1,\
        \"enSize\": 16,\
        \"enPayLoad\": 96,\
        \"enRcMode\": 0,\
        \"image_attrs\": {\
            \"enBayer\": 2,\
            \"u16FrameRate\": 30\
        },\
        \"enWndMode\": 3,\
        \"u16HorWndStart\": 72,\
        \"u16VerWndStart\": 20,\
        \"vi_dev_attrs\": {\
            \"enIntfMode\": 2,\
            \"enWorkMode\": 0,\
            \"au32CompMask\": [\"0xFFF00000\", \"0x0\"],\
            \"enScanMode\": 1,\
            \"s32AdChnId\": [ -1, -1, -1, -1 ],\
            \"enDataSeq\": 2,\
            \"stSynCfg\": {\
                \"enVsync\": 1,\
                \"enVsyncNeg\": 0,\
                \"enHsync\": 0,\
                \"enHsyncNeg\": 0,\
                \"enVsyncValid\": 1,\
                \"enVsyncValidNeg\": 0,\
                \"stTimingBlank\": {\
                    \"u32HsyncHfb\": 0,\
                    \"u32HsyncHbb\": 0,\
                    \"u32VsyncVfb\": 0,\
                    \"u32VsyncVbb\": 0,\
                    \"u32VsyncVbfb\": 0,\
                    \"u32VsyncVbact\": 0,\
                    \"u32VsyncVbbb\": 0\
                }\
            },\
            \"enDataPath\": 1,\
            \"enInputDataType\": 1,\
            \"bDataRev\": 0\
        },\
        \"stCapRect\": {\
            \"s32X\": 0,\
            \"s32Y\": 0\
        },\
        \"chl_attrs\": {\
            \"bChromaResample\": 0,\
            \"enCapSel\": 2\
        },\
        \"vi_params\": {\
            \"s32ViChnCnt\": 1,\
            \"s32ViChnInterval\": 1,\
            \"s32ViDevCnt\": 1,\
            \"s32ViDevInterval\": 1\
        }\
    },\
    \"stVbConf\": {\
        \"u32MaxPoolCnt\": 128,\
        \"astCommPool\": [ \
            {\
                \"u32BlkSize\": 1474560,\
                \"u32BlkCnt\": 10\
            }, {\
                \"u32BlkSize\": 0,\
                \"u32BlkCnt\":  0\
            }, {\
                \"u32BlkSize\": 0,\
                \"u32BlkCnt\":  0\
            }\
        ]\
    },\
    \"stVpssGrpAttr\": {\
        \"bDrEn\": 0,\
        \"bDbEn\": 0,\
        \"bIeEn\": 1,\
        \"bNrEn\": 1,\
        \"bHistEn\": 0,\
        \"enDieMode\": 0,\
    },\
    \"stVpssChnMode\": {\
        \"enChnMode\": 1,\
        \"bDouble\": 0\
    },\
    \"stVpssChnAttr\": {\
        \"bFrameEn\": 0,\
        \"bSpEn\": 1\
    },\
    \"stVpssExtChnAttr\": {\
        \"s32BindChn\": 1\
    },\
    \"enRotate\": 0,\
    \"enNorm\": 2,\
    \"enViChnSet\": 0,\
    \
    \"user\": \"johndoe\", \
    \"admin\": false, \
    \"uid\": 1000, \
    \"groups\": [\
        \"users\", \
        \"wheel\", \
        \"audio\", \
        \"video\"\
    ]\
}\
\n";


//static char json_doc1[] =
//"{\
//    \"Object\": {\
//       \"Integer\":  800,\
//       \"Negative\":  -12,\
//       \"Float\": -7.2,\
//       \"String\":  \"A\\tString with tab\",\
//       \"Object2\": {\
//           \"True\": true,\
//           \"False\": false,\
//           \"Null\": null\
//       },\
//       \"Array1\": [116, false, \"string\", {}],\
//       \"Array2\": [\
//	    {\
//        	   \"Float\": 123.,\
//	    },\
//	    {\
//		   \"Float\": 123.,\
//	    }\
//       ]\
//     },\
//   \"Integer\":  800,\
//   \"Array1\": [116, false, \"string\"]\
//}\
//";


#define HISI_PRT(fmt...)   \
    do {\
        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
        printf(fmt);\
       }while(0)

/*******************************************************
    enum define 
*******************************************************/

//typedef enum sample_ispcfg_opt_e
//{
//    CFG_OPT_NONE    = 0,
//    CFG_OPT_SAVE    = 1,
//    CFG_OPT_LOAD    = 2,
//    CFG_OPT_BUTT
//}SAMPLE_CFG_OPT_E;

//typedef enum hisi_vi_mode_e
//{
//    APTINA_AR0130_DC_720P_30FPS = 0,
//    APTINA_9M034_DC_720P_30FPS,
//    SONY_ICX692_DC_720P_30FPS,
//    SONY_IMX104_DC_720P_30FPS,
//    SONY_IMX138_DC_720P_30FPS,
//    SONY_IMX122_DC_1080P_30FPS,
//    SONY_IMX236_DC_1080P_30FPS,
//    OMNI_OV9712_DC_720P_30FPS,
//    APTINA_MT9P006_DC_1080P_30FPS,
//    SOI_H22_DC_720P_30FPS,
//    HIMAX_1375_DC_720P_30FPS,
//    APTINA_AR0330_DC_1080P_30FPS,
//    PIXELPLUS_3100K_DC_720P_30FPS,
//    SAMPLE_VI_MODE_1_D1,
//}HISI_VI_MODE_E;
//
//typedef enum 
//{
//    VI_DEV_BT656_D1_1MUX = 0,
//    VI_DEV_BT656_D1_4MUX,
//    VI_DEV_BT656_960H_1MUX,
//    VI_DEV_BT656_960H_4MUX,
//    VI_DEV_720P_HD_1MUX,
//    VI_DEV_1080P_HD_1MUX,
//    VI_DEV_BUTT
//}SAMPLE_VI_DEV_TYPE_E;

typedef enum hisi_vi_chn_set_e
{
    HISI_VI_CHN_SET_NORMAL = 0,      /* mirror, flip close */
    HISI_VI_CHN_SET_MIRROR,          /* open MIRROR */
    HISI_VI_CHN_SET_FLIP,            /* open filp */
    HISI_VI_CHN_SET_FLIP_MIRROR      /* mirror, flip */
} HISI_VI_CHN_SET_E;

//typedef enum sample_vo_mode_e
//{
//    VO_MODE_1MUX = 0,
//    VO_MODE_BUTT
//}SAMPLE_VO_MODE_E;
    
typedef enum hisi_rc_e
{
    HISI_RC_CBR = 0,
    HISI_RC_VBR,
    HISI_RC_FIXQP
}HISI_RC_E;

//typedef enum sample_rgn_change_type_e
//{
//    RGN_CHANGE_TYPE_FGALPHA = 0,
//    RGN_CHANGE_TYPE_BGALPHA,
//    RGN_CHANGE_TYPE_LAYER
//}SAMPLE_RGN_CHANGE_TYPE_EN;
//
//
/*******************************************************
    structure define 
*******************************************************/
typedef struct hisi_vi_param_s
{
    HI_S32 s32ViDevCnt;         // VI Dev Total Count
    HI_S32 s32ViDevInterval;    // Vi Dev Interval
    HI_S32 s32ViChnCnt;         // Vi Chn Total Count
    HI_S32 s32ViChnInterval;    // VI Chn Interval
} HISI_VI_PARAM_S;
//
//typedef struct sample_video_loss_s
//{
//    HI_BOOL bStart;
//    pthread_t Pid;
//    SAMPLE_VI_MODE_E enViMode;
//} SAMPLE_VIDEO_LOSS_S;

typedef struct hisi_venc_getstream_s
{
     HI_BOOL bThreadStart;
     HI_S32  s32Cnt;
} HISI_VENC_GETSTREAM_PARA_S;

typedef struct hisi_sensor_s
{
//    HI_CHAR lib_name[256];
    HI_CHAR *lib_name;
    HI_CHAR *sensor_name; 
//    HI_CHAR sensor_name[256]; 
    
    ISP_IMAGE_ATTR_S image_attrs;
    VI_DEV_ATTR_S vi_dev_attrs;
    HISI_VI_PARAM_S vi_params;
    VI_CHN_ATTR_S chl_attrs;
    
    
    RECT_S stCapRect;
    SIZE_S stTargetSize;
    
//    HI_U16 u16Width;        /*RW, Range: [0x0, 0x780]*/
//    HI_U16 u16Height;       /*RW, Range: [0x0, 0x4B0]*/
//    HI_U16 u16FrameRate;    /*RW, Range: [0x0, 0xFF]*/ 
    
    ISP_WIND_MODE_E enWndMode;
    HI_U16 u16HorWndStart;    /*RW, Range: [0x0, 0x780]*/
    HI_U16 u16HorWndLength;   /*RW, Range: [0x0, 0x780]*/
    HI_U16 u16VerWndStart;    /*RW, Range: [0x0, 0x4B0]*/
    HI_U16 u16VerWndLength;   /*RW, Range: [0x0, 0x4B0]*/
    
//    ISP_BAYER_FORMAT_E  enBayer;
} HISI_SENSOR_S;

typedef struct hisi_vi_config_s
{
//    HISI_VI_MODE_E enViMode;
    HISI_SENSOR_S sensor;
    VIDEO_NORM_E enNorm;           /*DC: VIDEO_ENCODING_MODE_AUTO */    
    ROTATE_E enRotate;
    HISI_VI_CHN_SET_E enViChnSet;  
    PIXEL_FORMAT_E enPixFmt; 
}HISI_VI_CONFIG_S;

//typedef struct hisi_vi_config_s
//{
//    HISI_VI_MODE_E enViMode;
//    VIDEO_NORM_E enNorm;           /*DC: VIDEO_ENCODING_MODE_AUTO */    
//    ROTATE_E enRotate;
//    HISI_VI_CHN_SET_E enViChnSet;    
//}HISI_VI_CONFIG_S;



typedef struct hisi_sdk_app_s
{
    HI_BOOL init_stage_0;
    HI_BOOL init_stage_1;
    HI_BOOL init_stage_2;
    HI_BOOL init_stage_3;
    HI_BOOL init_stage_4;
    HI_BOOL init_stage_5;
    
    PAYLOAD_TYPE_E enPayLoad;
    VB_CONF_S stVbConf;
    HI_U32 u32BlkSize;
    PIC_SIZE_E enSize;
    SIZE_S stSize;
    
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN_ATTR_S stVpssChnAttr;
    VPSS_CHN_MODE_S stVpssChnMode;
    
    VPSS_EXT_CHN_ATTR_S stVpssExtChnAttr;
    VIDEO_NORM_E gs_enNorm;
    HISI_RC_E enRcMode;
    
    HISI_VI_CONFIG_S stViConfig;
} HISI_SDK_APP_S;


/*******************************************************
    function announce  
*******************************************************/
HI_S32 HISI_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize);
HI_U32 HISI_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth);
//HI_S32 SAMPLE_COMM_SYS_MemConfig(HI_VOID);
HI_S32 HISI_COMM_SYS_Exit(void);
//HI_VOID HISI_COMM_SYS_Exit(void);
HI_S32 HISI_COMM_SYS_Init(VB_CONF_S *pstVbConf);
//HI_S32 SAMPLE_COMM_SYS_Payload2FilePostfix(PAYLOAD_TYPE_E enPayload, HI_CHAR* szFilePostfix);
//
HI_S32 HISI_COMM_ISP_Init(HISI_SENSOR_S sensor);
HI_VOID HISI_COMM_ISP_Stop(void);
HI_S32 HISI_COMM_ISP_Run(HISI_SENSOR_S sensor);
HI_S32 HISI_COMM_ISP_SensorInit(HISI_SENSOR_S sensor);
//HI_S32 SAMPLE_COMM_ISP_ChangeSensorMode(HI_U8 u8Mode);
//
//HI_S32 SAMPLE_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam);
//HI_S32 SAMPLE_COMM_VI_Mode2Size(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm, SIZE_S *pstSize);
//VI_DEV SAMPLE_COMM_VI_GetDev(SAMPLE_VI_MODE_E enViMode, VI_CHN ViChn);
HI_S32 HISI_COMM_VI_StartDev(VI_DEV ViDev, HISI_SENSOR_S sensor);
HI_S32 HISI_COMM_VI_StartChn(VI_CHN ViChn, HISI_VI_CONFIG_S* pstViConfig);
//HI_S32 HISI_COMM_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, HISI_VI_CONFIG_S* pstViConfig);
//HI_S32 SAMPLE_COMM_VI_StartBT656(SAMPLE_VI_MODE_E enViMode, VIDEO_NORM_E enNorm);
//HI_S32 SAMPLE_COMM_VI_StopBT656(SAMPLE_VI_MODE_E enViMode);
HI_S32 HISI_COMM_VI_BindVpss(HISI_VI_PARAM_S stViParam);
HI_S32 HISI_COMM_VI_UnBindVpss(HISI_VI_PARAM_S enViMode);
//HI_S32 SAMPLE_COMM_VI_BindVenc(SAMPLE_VI_MODE_E enViMode);
//HI_S32 SAMPLE_COMM_VI_UnBindVenc(SAMPLE_VI_MODE_E enViMode);
//HI_S32 SAMPLE_COMM_VI_MemConfig(SAMPLE_VI_MODE_E enViMode);
//HI_S32 SAMPLE_COMM_VI_GetVFrameFromYUV(FILE *pYUVFile, HI_U32 u32Width, HI_U32 u32Height,HI_U32 u32Stride, VIDEO_FRAME_INFO_S *pstVFrameInfo);
//HI_S32 SAMPLE_COMM_VI_ChangeCapSize(VI_CHN ViChn, HI_U32 u32CapWidth, HI_U32 u32CapHeight,HI_U32 u32Width, HI_U32 u32Height);
HI_S32 HISI_COMM_VI_StartVi(HISI_VI_CONFIG_S* pstViConfig);
HI_S32 HISI_COMM_VI_StopVi(HISI_VI_CONFIG_S* pstViConfig);
//	
//HI_S32 SAMPLE_COMM_VPSS_MemConfig();
//HI_S32 SAMPLE_COMM_VPSS_Start(HI_S32 s32GrpCnt, SIZE_S *pstSize, HI_S32 s32ChnCnt,VPSS_GRP_ATTR_S *pstVpssGrpAttr);
//HI_S32 SAMPLE_COMM_VPSS_Stop(HI_S32 s32GrpCnt, HI_S32 s32ChnCnt) ;
HI_S32 HISI_COMM_VPSS_StartGroup(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstVpssGrpAttr);
HI_S32 HISI_COMM_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                                                  VPSS_CHN_ATTR_S *pstVpssChnAttr,
                                                  VPSS_CHN_MODE_S *pstVpssChnMode,
                                                  VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr);
HI_S32 HISI_COMM_VPSS_StopGroup(VPSS_GRP VpssGrp);
HI_S32 HISI_COMM_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn);
//HI_S32 SAMPLE_COMM_DisableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize);
//HI_S32 SAMPLE_COMM_EnableVpssPreScale(VPSS_GRP VpssGrp,SIZE_S stSize);
//
//
//HI_S32 SAMPLE_COMM_VO_MemConfig(VO_DEV VoDev, HI_CHAR *pcMmzName);
//HI_S32 SAMPLE_COMM_VO_StartDevLayer(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr, HI_U32 u32SrcFrmRate);
//HI_S32 SAMPLE_COMM_VO_StopDevLayer(VO_DEV VoDev);
//HI_S32 SAMPLE_COMM_VO_StartChn(VO_DEV VoDev,VO_PUB_ATTR_S *pstPubAttr,SAMPLE_VO_MODE_E enMode);
//HI_S32 SAMPLE_COMM_VO_StopChn(VO_DEV VoDev,SAMPLE_VO_MODE_E enMode);
//HI_S32 SAMPLE_COMM_VO_BindVi(VO_DEV VoDev, VO_CHN VoChn, VI_CHN ViChn);
//HI_S32 SAMPLE_COMM_VO_UnBindVi(VO_DEV VoDev, VO_CHN VoChn);
//
//HI_S32 SAMPLE_COMM_VENC_MemConfig(HI_VOID);
HI_S32 HISI_COMM_VENC_Start(VENC_GRP VencGrp,VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, HISI_RC_E enRcMode);
HI_S32 HISI_COMM_VENC_Stop(VENC_GRP VencGrp,VENC_CHN VencChn);
//HI_S32 SAMPLE_COMM_VENC_SnapStart(VENC_GRP VencGrp,VENC_CHN VencChn, SIZE_S *pstSize);
//HI_S32 SAMPLE_COMM_VENC_SnapProcess(VENC_GRP VencGrp, VENC_CHN VencChn);
//HI_S32 SAMPLE_COMM_VENC_SnapStop(VENC_GRP VencGrp,VENC_CHN VencChn);
HI_S32 HISI_COMM_VENC_StartGetStream(HI_S32 s32Cnt);
HI_S32 HISI_COMM_VENC_StopGetStream();
HI_S32 HISI_COMM_VENC_BindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
HI_S32 HISI_COMM_VENC_UnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn);
//
//HI_S32 SAMLE_COMM_VDEC_BindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp);
//HI_S32 SAMLE_COMM_VDEC_UnBindVpss(VDEC_CHN VdChn, VPSS_GRP VpssGrp);
//HI_S32 SAMLE_COMM_VDEC_BindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn);
//HI_S32 SAMLE_COMM_VDEC_UnBindVo(VDEC_CHN VdChn, VO_DEV VoDev, VO_CHN VoChn);
//HI_S32 SAMPLE_COMM_VDEC_MemConfig(HI_VOID);
//
//HI_S32 SAMPLE_COMM_VDA_MdStart(VDA_CHN VdaChn, VI_CHN ViChn, SIZE_S *pstSize);
////HI_S32 SAMPLE_COMM_VDA_MdStart(VDA_CHN VdaChn, VPSS_GRP VpssGrp, VPSS_CHN VpssChn, SIZE_S *pstSize);
//HI_S32 SAMPLE_COMM_VDA_OdStart(VDA_CHN VdaChn, VI_CHN ViChn, SIZE_S *pstSize);
//HI_VOID SAMPLE_COMM_VDA_MdStop(VDA_CHN VdaChn, VI_CHN ViChn);
//HI_VOID SAMPLE_COMM_VDA_OdStop(VDA_CHN VdaChn, VI_CHN ViChn);
//
//HI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAo(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
//HI_S32 SAMPLE_COMM_AUDIO_CreatTrdAiAenc(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
//HI_S32 SAMPLE_COMM_AUDIO_CreatTrdAencAdec(AENC_CHN AeChn, ADEC_CHN AdChn, FILE *pAecFd);
//HI_S32 SAMPLE_COMM_AUDIO_CreatTrdFileAdec(ADEC_CHN AdChn, FILE *pAdcFd);
//HI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAi(AUDIO_DEV AiDev, AI_CHN AiChn);
//HI_S32 SAMPLE_COMM_AUDIO_DestoryTrdAencAdec(AENC_CHN AeChn);
//HI_S32 SAMPLE_COMM_AUDIO_DestoryTrdFileAdec(ADEC_CHN AdChn);
//HI_S32 SAMPLE_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
//HI_S32 SAMPLE_COMM_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn);
//HI_S32 SAMPLE_COMM_AUDIO_AoBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
//HI_S32 SAMPLE_COMM_AUDIO_AoUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AUDIO_DEV AoDev, AO_CHN AoChn);
//HI_S32 SAMPLE_COMM_AUDIO_AencBindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
//HI_S32 SAMPLE_COMM_AUDIO_AencUnbindAi(AUDIO_DEV AiDev, AI_CHN AiChn, AENC_CHN AeChn);
HI_S32 HISI_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr, HI_BOOL bMacIn);
//HI_S32 SAMPLE_COMM_AUDIO_DisableAcodec();
HI_S32 HISI_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        AIO_ATTR_S *pstAioAttr, HI_BOOL bAnrEn, AUDIO_RESAMPLE_ATTR_S *pstAiReSmpAttr);
HI_S32 HISI_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        HI_BOOL bAnrEn, HI_BOOL bResampleEn);
HI_S32 HISI_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, AO_CHN AoChn,
        AIO_ATTR_S *pstAioAttr, AUDIO_RESAMPLE_ATTR_S *pstAiReSmpAttr);
HI_S32 HISI_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, AO_CHN AoChn, HI_BOOL bResampleEn);
//HI_S32 SAMPLE_COMM_AUDIO_StartAenc(HI_S32 s32AencChnCnt, PAYLOAD_TYPE_E enType);
//HI_S32 SAMPLE_COMM_AUDIO_StopAenc(HI_S32 s32AencChnCnt);
//HI_S32 SAMPLE_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType);
//HI_S32 SAMPLE_COMM_AUDIO_StopAdec(ADEC_CHN AdChn);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* HISI_COMM_H */

