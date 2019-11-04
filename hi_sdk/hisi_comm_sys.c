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
    
/******************************************************************************
* function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 HISI_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S *pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width = 176;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?144:120;
            break;
        case PIC_CIF:
            pstSize->u32Width = 352;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?288:240;
            break;
        case PIC_D1:
            pstSize->u32Width = 720;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_960H:
            pstSize->u32Width = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;			
        case PIC_2CIF:
            pstSize->u32Width = 360;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL==enNorm)?576:480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width = 1920;
            pstSize->u32Height = 1080;
            break;
        default:
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}
    
/******************************************************************************
* function : calculate VB Block size of picture.
******************************************************************************/
HI_U32 HISI_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;

    s32Ret = HISI_COMM_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("get picture size[%d] failed!\n", enPicSize);
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt) {
        HISI_PRT("pixel format[%d] input failed!\n", enPixFmt);
        return HI_FAILURE;
    }

    if (16!=u32AlignWidth && 32!=u32AlignWidth && 64!=u32AlignWidth) {
        HISI_PRT("system align width[%d] input failed!\n",\
               u32AlignWidth);
        return HI_FAILURE;
    }
    //SAMPLE_PRT("w:%d, u32AlignWidth:%d\n", CEILING_2_POWER(stSize.u32Width,u32AlignWidth), u32AlignWidth);
    return (CEILING_2_POWER(stSize.u32Width, u32AlignWidth) * \
            CEILING_2_POWER(stSize.u32Height,u32AlignWidth) * \
           ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt)?2:1.5));
}

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 HISI_COMM_SYS_Init(VB_CONF_S *pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32Ret = HI_FAILURE;

    HISI_COMM_SYS_Exit();

    if (NULL == pstVbConf) {
        HISI_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }
    HISI_PRT("pstVbConf->astCommPool[0].u32BlkSize = %d!\n", pstVbConf->astCommPool[0].u32BlkSize);

    s32Ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VB_SetConf failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VB_SetConf SUCCESS with %#x!\n", s32Ret);
    }

    s32Ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VB_Init failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VB_Init SUCCESS with %#x!\n", s32Ret);
    }

    stSysConf.u32AlignWidth = HISI_SYS_ALIGN_WIDTH;
    s32Ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_SYS_SetConf failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_SYS_SetConf SUCCESS with %#x!\n", s32Ret);
    }

    HISI_PRT("HI_MPI_SYS_Init start\n");
    s32Ret = HI_MPI_SYS_Init();
    HISI_PRT("HI_MPI_SYS_Init end\n");
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_SYS_Init failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_SYS_Init SUCCESS with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : vb exit & MPI system exit
******************************************************************************/
//HI_VOID HISI_COMM_SYS_Exit(void)
HI_S32 HISI_COMM_SYS_Exit(void)
{
    HI_S32 s32Ret;
    s32Ret = HI_SUCCESS;

    s32Ret = HI_MPI_SYS_Exit();
    s32Ret = HI_MPI_VB_Exit();
    return s32Ret;
//    HI_MPI_SYS_Exit();
//    HI_MPI_VB_Exit();
//    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif