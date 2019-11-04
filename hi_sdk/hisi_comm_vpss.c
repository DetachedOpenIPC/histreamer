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

HI_S32 HISI_COMM_VPSS_StartGroup(VPSS_GRP VpssGrp, VPSS_GRP_ATTR_S *pstVpssGrpAttr)
{
    HI_S32 s32Ret;
    VPSS_GRP_PARAM_S stVpssParam;
    
    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM) {
        printf("VpssGrp%d is out of rang. \n", VpssGrp);
        return HI_FAILURE;
    }

    if (HI_NULL == pstVpssGrpAttr) {
        printf("null ptr,line%d. \n", __LINE__);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_CreateGrp(VpssGrp, pstVpssGrpAttr);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_CreateGrp failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_CreateGrp Success with %#x!\n", s32Ret);
    }

    /*** set vpss param ***/
    s32Ret = HI_MPI_VPSS_GetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_GetGrpParam failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_GetGrpParam Success with %#x!\n", s32Ret);
    }
    
    stVpssParam.u32MotionThresh = 0;
    
    s32Ret = HI_MPI_VPSS_SetGrpParam(VpssGrp, &stVpssParam);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_SetGrpParam failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_SetGrpParam Success with %#x!\n", s32Ret);
    }

    s32Ret = HI_MPI_VPSS_StartGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_StartGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_StartGrp Success with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 HISI_COMM_VPSS_EnableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn, 
                                                  VPSS_CHN_ATTR_S *pstVpssChnAttr,
                                                  VPSS_CHN_MODE_S *pstVpssChnMode,
                                                  VPSS_EXT_CHN_ATTR_S *pstVpssExtChnAttr)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM) {
        printf("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    if (VpssChn < 0 || VpssChn > VPSS_MAX_CHN_NUM) {
        printf("VpssChn%d is out of rang[0,%d]. \n", VpssChn, VPSS_MAX_CHN_NUM);
        return HI_FAILURE;
    }

    if (HI_NULL == pstVpssChnAttr && HI_NULL == pstVpssExtChnAttr) {
        printf("null ptr,line%d. \n", __LINE__);
        return HI_FAILURE;
    }

    if (VpssChn < VPSS_MAX_PHY_CHN_NUM) {
        s32Ret = HI_MPI_VPSS_SetChnAttr(VpssGrp, VpssChn, pstVpssChnAttr);
        if (s32Ret != HI_SUCCESS) {
            HISI_PRT("HI_MPI_VPSS_SetChnAttr failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_VPSS_SetChnAttr Success with %#x!\n", s32Ret);
        }
    } else {
        s32Ret = HI_MPI_VPSS_SetExtChnAttr(VpssGrp, VpssChn, pstVpssExtChnAttr);
        if (s32Ret != HI_SUCCESS) {
            HISI_PRT("HI_MPI_VPSS_SetExtChnAttr failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_VPSS_SetExtChnAttr Success with %#x!\n", s32Ret);
        }
    }
    
    if (VpssChn < VPSS_MAX_PHY_CHN_NUM && HI_NULL != pstVpssChnMode) {
        s32Ret = HI_MPI_VPSS_SetChnMode(VpssGrp, VpssChn, pstVpssChnMode);
        if (s32Ret != HI_SUCCESS) {
            HISI_PRT("HI_MPI_VPSS_SetChnMode failed with %#x\n", s32Ret);
            return HI_FAILURE;
        } else {
            HISI_PRT("HI_MPI_VPSS_SetChnMode Success with %#x!\n", s32Ret);
        }     
    }
    
    s32Ret = HI_MPI_VPSS_EnableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_EnableChn failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_EnableChn Success with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;
}

HI_S32 HISI_COMM_VPSS_DisableChn(VPSS_GRP VpssGrp, VPSS_CHN VpssChn)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM) {
        HISI_PRT("VpssGrp%d is out of rang[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    if (VpssChn < 0 || VpssChn > VPSS_MAX_CHN_NUM) {
        HISI_PRT("VpssChn%d is out of rang[0,%d]. \n", VpssChn, VPSS_MAX_CHN_NUM);
        return HI_FAILURE;
    }
    
    s32Ret = HI_MPI_VPSS_DisableChn(VpssGrp, VpssChn);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s failed with %#x\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HISI_COMM_VPSS_StopGroup(VPSS_GRP VpssGrp)
{
    HI_S32 s32Ret;

    if (VpssGrp < 0 || VpssGrp > VPSS_MAX_GRP_NUM) {
        HISI_PRT("VpssGrp%d is out of range[0,%d]. \n", VpssGrp, VPSS_MAX_GRP_NUM);
        return HI_FAILURE;
    }

    s32Ret = HI_MPI_VPSS_StopGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_StopGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_StopGrp Success with %#x!\n", s32Ret);
    }

    s32Ret = HI_MPI_VPSS_DestroyGrp(VpssGrp);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_VPSS_DestroyGrp failed with %#x\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VPSS_DestroyGrp Success with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif