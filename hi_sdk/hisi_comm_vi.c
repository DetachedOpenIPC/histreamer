#if PJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO

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

#include "hisi_comm.h"
    
//HI_BOOL hisiIsSensorInput(SAMPLE_VI_MODE_E enViMode)
//{
//    HI_BOOL bRet = HI_TRUE;
//
//    switch(enViMode)
//    {
//        case SAMPLE_VI_MODE_1_D1:
//            bRet = HI_FALSE;
//            break;
//        default:
//            break;
//    }
//
//    return bRet;
//}
    
/*****************************************************************************
* function : star vi dev (cfg vi_dev_attr; set_dev_cfg; enable dev)
*****************************************************************************/
HI_S32 HISI_COMM_VI_StartDev(VI_DEV ViDev, HISI_SENSOR_S sensor)
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S    stViDevAttr;
    memset(&stViDevAttr,0,sizeof(stViDevAttr));
    memcpy(&stViDevAttr,&sensor.vi_dev_attrs,sizeof(stViDevAttr));
    
//    switch (enViMode)
//    {
//        case SAMPLE_VI_MODE_1_D1:
//            memcpy(&stViDevAttr,&DEV_ATTR_BT656D1_1MUX,sizeof(stViDevAttr));
//            break;
//
//        case APTINA_AR0130_DC_720P_30FPS:
//        case SONY_ICX692_DC_720P_30FPS:
//        case SONY_IMX104_DC_720P_30FPS:
//        case SONY_IMX138_DC_720P_30FPS:
//        case APTINA_9M034_DC_720P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_AR0130_DC_720P,sizeof(stViDevAttr));
//            break;
//
//        case OMNI_OV9712_DC_720P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_OV9712_DC_720P,sizeof(stViDevAttr));
//            break;
//
//        case SOI_H22_DC_720P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_SOIH22_DC_720P,sizeof(stViDevAttr));
//            break;
//
//		case HIMAX_1375_DC_720P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_HIMAX1375_DC_720P,sizeof(stViDevAttr));
//            break;
//
//        case SONY_IMX122_DC_1080P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_IMX122_DC_1080P,sizeof(stViDevAttr));
//            break;
//
//        case APTINA_MT9P006_DC_1080P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_MT9P006_DC_1080P,sizeof(stViDevAttr));
//            break;
//
//        case APTINA_AR0330_DC_1080P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_AR0330_DC_1080P,sizeof(stViDevAttr));
//            break;
//
//        case SONY_IMX236_DC_1080P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_IMX236_DC_1080P,sizeof(stViDevAttr));
//            break;
//
//        case PIXELPLUS_3100K_DC_720P_30FPS:
//            memcpy(&stViDevAttr,&DEV_ATTR_PO3100K_DC_720P,sizeof(stViDevAttr));
//            break;
//
//        default:
//            memcpy(&stViDevAttr,&DEV_ATTR_AR0130_DC_720P,sizeof(stViDevAttr));
//    }

    s32Ret = HI_MPI_VI_SetDevAttr(ViDev, &stViDevAttr);
    if (s32Ret != HI_SUCCESS)
    {
        HISI_PRT("HI_MPI_VI_SetDevAttr failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VI_SetDevAttr SUCCESS with %#x!\n", s32Ret);
    }

    s32Ret = HI_MPI_VI_EnableDev(ViDev);
    if (s32Ret != HI_SUCCESS)
    {
        HISI_PRT("HI_MPI_VI_EnableDev failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VI_SetDevAttr SUCCESS with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;
}

/*****************************************************************************
* function : star vi chn
*****************************************************************************/
//HI_S32 HISI_COMM_VI_StartChn(VI_CHN ViChn, RECT_S *pstCapRect, SIZE_S *pstTarSize, HISI_VI_CONFIG_S* pstViConfig)
HI_S32 HISI_COMM_VI_StartChn(VI_CHN ViChn, HISI_VI_CONFIG_S* pstViConfig)
{
    HI_S32 s32Ret;
    VI_CHN_ATTR_S stChnAttr;
    ROTATE_E enRotate = ROTATE_NONE;
    HISI_VI_CHN_SET_E enViChnSet = HISI_VI_CHN_SET_NORMAL;

    if(pstViConfig) {
        enViChnSet = pstViConfig->enViChnSet;
        enRotate = pstViConfig->enRotate;
//        enViChnSet = pstViConfig->enViChnSet;
    }

    /* step  5: config & start vicap dev */
//    memcpy(&stChnAttr.stCapRect, &pstViConfig->sensor.stCapRect, sizeof(RECT_S));
    stChnAttr.stCapRect.s32X = pstViConfig->sensor.stCapRect.s32X;
    stChnAttr.stCapRect.s32Y = pstViConfig->sensor.stCapRect.s32Y;
    stChnAttr.stCapRect.u32Height = pstViConfig->sensor.stCapRect.u32Height;
    stChnAttr.stCapRect.u32Width = pstViConfig->sensor.stCapRect.u32Width;
    stChnAttr.enCapSel = pstViConfig->sensor.chl_attrs.enCapSel;
    /* to show scale. this is a sample only, we want to show dist_size = D1 only */
    stChnAttr.stDestSize.u32Width = pstViConfig->sensor.stTargetSize.u32Width;
    stChnAttr.stDestSize.u32Height = pstViConfig->sensor.stTargetSize.u32Height;
    stChnAttr.enPixFormat = pstViConfig->enPixFmt;   /* sp420 or sp422 */

//    stChnAttr.bMirror = HI_FALSE;
//    stChnAttr.bFlip = HI_FALSE;

//    switch(enViChnSet) {
//        case HISI_VI_CHN_SET_MIRROR:
//            stChnAttr.bMirror = HI_TRUE;
//            break;
//
//        case HISI_VI_CHN_SET_FLIP:
//            stChnAttr.bFlip = HI_TRUE;
//            break;
//
//        case HISI_VI_CHN_SET_FLIP_MIRROR:
//            stChnAttr.bMirror = HI_TRUE;
//            stChnAttr.bFlip = HI_TRUE;
//            break;
//
//        default:
//            break;
//    }
    stChnAttr.bMirror         = pstViConfig->sensor.chl_attrs.bMirror;
    stChnAttr.bFlip           = pstViConfig->sensor.chl_attrs.bFlip;
    stChnAttr.bChromaResample = pstViConfig->sensor.chl_attrs.bChromaResample;
    stChnAttr.s32SrcFrameRate = pstViConfig->sensor.chl_attrs.s32FrameRate;
    stChnAttr.s32FrameRate    = pstViConfig->sensor.chl_attrs.s32FrameRate;
//    stChnAttr.s32SrcFrameRate = 30;
//    stChnAttr.s32FrameRate = 30;

    s32Ret = HI_MPI_VI_SetChnAttr(ViChn, &stChnAttr);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_VI_SetChnAttr failed with %#x!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("%s: HI_MPI_VI_SetChnAttr Success with %#x!\n", __FUNCTION__, s32Ret);
    }

    if(ROTATE_NONE != enRotate) {
        s32Ret = HI_MPI_VI_SetRotate(ViChn, enRotate);
        if (s32Ret != HI_SUCCESS)
        {
            HISI_PRT("%s: HI_MPI_VI_SetRotate failed with %#x!\n", __FUNCTION__, s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("%s: HI_MPI_VI_SetRotate Success with %#x!\n", __FUNCTION__, s32Ret);
        }
    }

    s32Ret = HI_MPI_VI_EnableChn(ViChn);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_VI_EnableChn failed with %#x!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("%s: HI_MPI_VI_EnableChn Success with %#x!\n", __FUNCTION__, s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 HISI_COMM_VI_StartIsp(HISI_VI_CONFIG_S* pstViConfig)
{
    HI_S32 i, s32Ret = HI_SUCCESS;
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;
//    SIZE_S stTargetSize;
//    RECT_S stCapRect;
//    HISI_VI_MODE_E enViMode;
    ROTATE_E enRotate;
    HISI_VI_CHN_SET_E enViChnSet;

    if(!pstViConfig) {
        HISI_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
//    enViMode = pstViConfig->enViMode;
    enViChnSet = pstViConfig->enViChnSet;
    enRotate = pstViConfig->enRotate;

    /******************************************
     step 1: configure sensor.
     note: you can jump over this step, if you do not use Hi3518 interal isp.
    ******************************************/
    s32Ret = HISI_COMM_ISP_SensorInit(pstViConfig->sensor);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("%s: Sensor init failed!\n", __FUNCTION__);
        return HI_FAILURE;
    } else {
        HISI_PRT("%s: Sensor init Success!\n", __FUNCTION__);
    }

    /******************************************
     step 2: configure & run isp thread
     note: you can jump over this step, if you do not use Hi3518 interal isp.
    ******************************************/
    s32Ret = HISI_COMM_ISP_Run(pstViConfig->sensor);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("%s: ISP init failed with %#x!\n", __FUNCTION__, s32Ret);
	    /* disable videv */
        return HI_FAILURE;
    } else {
        HISI_PRT("%s: Sensor init Success!\n", __FUNCTION__);
    }

    /******************************************************
     step 3 : config & start vicap dev
    ******************************************************/
    for (i = 0; i < u32DevNum; i++) {
        ViDev = i;
        s32Ret = HISI_COMM_VI_StartDev(ViDev, pstViConfig->sensor);
        if (HI_SUCCESS != s32Ret) {
            HISI_PRT("%s: start vi dev[%d] failed!\n", __FUNCTION__, i);
            return HI_FAILURE;
        }
    }

    /******************************************************
    * Step 4: config & start vicap chn (max 1)
    ******************************************************/
    for (i = 0; i < u32ChnNum; i++)
    {
        ViChn = i;

//        stCapRect.s32X = pstViConfig->sensor.stCapRect.s32X;
//        stCapRect.s32Y = pstViConfig->sensor.stCapRect.s32Y;
//        stCapRect.u32Width = pstViConfig->sensor.stCapRect.u32Width;
//        stCapRect.u32Height = pstViConfig->sensor.stCapRect.u32Height;
        

//        stTargetSize.u32Width = stCapRect.u32Width;
//        stTargetSize.u32Height = stCapRect.u32Height;

        s32Ret = HISI_COMM_VI_StartChn(ViChn, pstViConfig);
//        s32Ret = HISI_COMM_VI_StartChn(ViChn, &stCapRect, &stTargetSize, pstViConfig);
        if (HI_SUCCESS != s32Ret) {
            HISI_COMM_ISP_Stop();
            return HI_FAILURE;
        }
    }

    return s32Ret;
}

HI_S32 HISI_COMM_VI_StopIsp(HISI_VI_CONFIG_S* pstViConfig)
{
    VI_DEV ViDev;
    VI_CHN ViChn;
    HI_S32 i;
    HI_S32 s32Ret;
    HI_U32 u32DevNum = 1;
    HI_U32 u32ChnNum = 1;

    if(!pstViConfig) {
        HISI_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }

    /*** Stop VI Chn ***/
    for(i=0;i < u32ChnNum; i++) {
        /* Stop vi phy-chn */
        ViChn = i;
        s32Ret = HI_MPI_VI_DisableChn(ViChn);
        if (HI_SUCCESS != s32Ret) {
            HISI_PRT("HI_MPI_VI_DisableChn failed with %#x\n",s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_VI_DisableChn Success with %#x\n", s32Ret);
        }
    }

    /*** Stop VI Dev ***/
    for(i=0; i < u32DevNum; i++) {
        ViDev = i;
        s32Ret = HI_MPI_VI_DisableDev(ViDev);
        if (HI_SUCCESS != s32Ret) {
            HISI_PRT("HI_MPI_VI_DisableDev failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_VI_DisableDev Success with %#x\n", s32Ret);
        }
    }

    HISI_COMM_ISP_Stop();
    return HI_SUCCESS;
}
    
HI_S32 HISI_COMM_VI_StartVi(HISI_VI_CONFIG_S* pstViConfig) {
    HI_S32 s32Ret = HI_SUCCESS;
//    SAMPLE_VI_MODE_E enViMode;
    VIDEO_NORM_E enNorm;

    if(!pstViConfig) {
        HISI_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
//    enViMode = pstViConfig->enViMode;
    enNorm = pstViConfig->enNorm;

//    if(!IsSensorInput(enViMode))
//    {
//        s32Ret = SAMPLE_COMM_VI_StartBT656(enViMode, enNorm);
//    }
//    else
//    {
        s32Ret = HISI_COMM_VI_StartIsp(pstViConfig);
//    }


    return s32Ret;
}

    

HI_S32 HISI_COMM_VI_StopVi(HISI_VI_CONFIG_S* pstViConfig)
{
    HI_S32 s32Ret = HI_SUCCESS;
//    SAMPLE_VI_MODE_E enViMode;

    if(!pstViConfig)
    {
        HISI_PRT("%s: null ptr\n", __FUNCTION__);
        return HI_FAILURE;
    }
//    enViMode = pstViConfig->enViMode;

//    if(!IsSensorInput(enViMode))
//    {
//        s32Ret = SAMPLE_COMM_VI_StopBT656(enViMode);
//    }
//    else
//    {
        s32Ret = HISI_COMM_VI_StopIsp(pstViConfig);
//    }

    return s32Ret;
}

/*****************************************************************************
* function : Vi chn unbind vpss group
*****************************************************************************/
HI_S32 HISI_COMM_VI_UnBindVpss(HISI_VI_PARAM_S stViParam)
{
    HI_S32 i, j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
//    SAMPLE_VI_PARAM_S stViParam;
    VI_DEV ViDev;
    VI_CHN ViChn;

//    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
//    if (HI_SUCCESS !=s32Ret) {
//        HISI_PRT("SAMPLE_COMM_VI_Mode2Param failed!\n");
//        return HI_FAILURE;
//    } else {
//        HISI_PRT("%s: Sensor init Success!\n", __FUNCTION__);
//    }

    VpssGrp = 0;
    for (i=0; i<stViParam.s32ViDevCnt; i++) {
        ViDev = i * stViParam.s32ViDevInterval;

        for (j=0; j<stViParam.s32ViChnCnt; j++) {
            ViChn = j * stViParam.s32ViChnInterval;

            stSrcChn.enModId = HI_ID_VIU;
            stSrcChn.s32DevId = ViDev;
            stSrcChn.s32ChnId = ViChn;

            stDestChn.enModId = HI_ID_VPSS;
            stDestChn.s32DevId = VpssGrp;
            stDestChn.s32ChnId = 0;

            s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
            if (s32Ret != HI_SUCCESS) {
                HISI_PRT("failed with %#x!\n", s32Ret);
                return HI_FAILURE;
            }

            VpssGrp ++;
        }
    }
    return HI_SUCCESS;
}

//HI_S32 HISI_COMM_VI_Mode2Param(SAMPLE_VI_MODE_E enViMode, SAMPLE_VI_PARAM_S *pstViParam)
//{
//    switch (enViMode)
//    {
//        case SAMPLE_VI_MODE_1_D1:
//        case APTINA_AR0130_DC_720P_30FPS:
//            pstViParam->s32ViDevCnt = 1;
//            pstViParam->s32ViDevInterval = 1;
//            pstViParam->s32ViChnCnt = 1;
//            pstViParam->s32ViChnInterval = 1;
//            break;
//
//        default:
//            pstViParam->s32ViDevCnt = 1;
//            pstViParam->s32ViDevInterval = 1;
//            pstViParam->s32ViChnCnt = 1;
//            pstViParam->s32ViChnInterval = 1;
//            break;
//    }
//    return HI_SUCCESS;
//}

/*****************************************************************************
* function : Vi chn bind vpss group
*****************************************************************************/
HI_S32 HISI_COMM_VI_BindVpss(HISI_VI_PARAM_S stViParam)
{
    HI_S32 j, s32Ret;
    VPSS_GRP VpssGrp;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
//    SAMPLE_VI_PARAM_S stViParam;
    VI_CHN ViChn;

//    s32Ret = SAMPLE_COMM_VI_Mode2Param(enViMode, &stViParam);
//    if (HI_SUCCESS !=s32Ret) {
//        HISI_PRT("SAMPLE_COMM_VI_Mode2Param failed with %#x!\n", __FUNCTION__, s32Ret);
//        return HI_FAILURE;
//    }

    VpssGrp = 0;
    for (j=0; j<stViParam.s32ViChnCnt; j++) {
        ViChn = j * stViParam.s32ViChnInterval;

        stSrcChn.enModId = HI_ID_VIU;
        stSrcChn.s32DevId = 0;
        stSrcChn.s32ChnId = ViChn;

        stDestChn.enModId = HI_ID_VPSS;
        stDestChn.s32DevId = VpssGrp;
        stDestChn.s32ChnId = 0;

        s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
        if (s32Ret != HI_SUCCESS) {
            HISI_PRT("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_SYS_Bind Success with %#x!\n", s32Ret);
        }

        VpssGrp ++;
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif