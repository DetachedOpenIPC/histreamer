#if PJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <pj/log.h>



#include "hisi_comm.h"
#include "acodec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */
    
#define THIS_FILE           "hisi_comm_audio.c"

#define ACODEC_FILE     "/dev/acodec"

#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4/* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define G726_BPS MEDIA_G726_40K         /* MEDIA_G726_16K, MEDIA_G726_24K ... */
    
/******************************************************************************
* function : Start Ao
******************************************************************************/
HI_S32 HISI_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, AO_CHN AoChn,
        AIO_ATTR_S *pstAioAttr, AUDIO_RESAMPLE_ATTR_S *pstAoReSmpAttr)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_AO_SetPubAttr(AoDevId, pstAioAttr);
    if(HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!", __FUNCTION__, AoDevId, s32Ret));
//        printf("%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", __FUNCTION__, \
               AoDevId,s32Ret);
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_SetPubAttr(%d) success with %#x!", __FUNCTION__, AoDevId, s32Ret));
    }
    s32Ret = HI_MPI_AO_Enable(AoDevId);
    if(HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_Enable(%d) with with %#x!", __FUNCTION__, AoDevId, s32Ret));
//        printf("%s: HI_MPI_AO_Enable(%d) failed with %#x!\n", __FUNCTION__, \
               AoDevId, s32Ret);
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_Enable(%d) success with %#x!", __FUNCTION__, AoDevId, s32Ret));
    }
    s32Ret = HI_MPI_AO_EnableChn(AoDevId, AoChn);
    if(HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_EnableChn(%d) failed with %#x!", __FUNCTION__, AoChn, s32Ret));
//        printf("%s: HI_MPI_AO_EnableChn(%d) failed with %#x!\n", __FUNCTION__,\
               AoChn, s32Ret);
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_EnableChn(%d) success with %#x!", __FUNCTION__, AoChn, s32Ret));
    }
    
    if (NULL != pstAoReSmpAttr)
    {
        s32Ret = HI_MPI_AO_DisableReSmp(AoDevId, AoChn);
        s32Ret |= HI_MPI_AO_EnableReSmp(AoDevId, AoChn, pstAoReSmpAttr);
        if(HI_SUCCESS != s32Ret) {
            PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x!", __FUNCTION__, AoDevId, AoChn, s32Ret));
//            printf("%s: HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x!\n", \
                   __FUNCTION__, AoDevId, AoChn, s32Ret);
            return HI_FAILURE;
        } else {
            PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AO_EnableReSmp(%d,%d) success with %#x!", __FUNCTION__, AoDevId, AoChn, s32Ret));
        }
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Ai
******************************************************************************/
HI_S32 HISI_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        HI_BOOL bAnrEn, HI_BOOL bResampleEn)
{
    HI_S32 i;    
    for (i=0; i<s32AiChnCnt; i++) {
        if (HI_TRUE == bResampleEn) {
            HI_MPI_AI_DisableReSmp(AiDevId, i);
        }
        if (HI_TRUE == bAnrEn) {
            HI_MPI_AI_DisableAnr(AiDevId, i);
        }
        HI_MPI_AI_DisableChn(AiDevId, i);
    }  
    HI_MPI_AI_Disable(AiDevId);
    return HI_SUCCESS;
}

/******************************************************************************
* function : Start Ai
******************************************************************************/
HI_S32 HISI_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
        AIO_ATTR_S *pstAioAttr, HI_BOOL bAnrEn, AUDIO_RESAMPLE_ATTR_S *pstAiReSmpAttr)
{
    HI_S32 i, s32Ret;
    
    s32Ret = HI_MPI_AI_SetPubAttr(AiDevId, pstAioAttr);
    if (s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x", __FUNCTION__, AiDevId, s32Ret));
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_SetPubAttr(%d) success with %#x!", __FUNCTION__, AiDevId, s32Ret));
    }
    if (HI_MPI_AI_Enable(AiDevId)) {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_Enable(%d) failed with %#x!", __FUNCTION__, AiDevId, s32Ret));
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_Enable(%d) success with %#x!", __FUNCTION__, AiDevId, s32Ret));
    }          
    for (i=0; i<s32AiChnCnt; i++) {
        if (HI_MPI_AI_EnableChn(AiDevId, i)) {
            PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableChn(%d,%d) failed with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
            return HI_FAILURE;
        } else {
            PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableChn(%d,%d) success with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
        } 

        if (HI_TRUE == bAnrEn) {
            if (HI_MPI_AI_EnableAnr(AiDevId, i)) {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableAnr(%d,%d) failed with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
                return HI_FAILURE;
            } else {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableAnr(%d,%d) success with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
            } 
        }

        if (NULL != pstAiReSmpAttr) {
            if (HI_MPI_AI_EnableReSmp(AiDevId, i, pstAiReSmpAttr)) {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableReSmp(%d,%d) failed with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
                return HI_FAILURE;
            } else {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_EnableReSmp(%d,%d) success with %#x!", __FUNCTION__, AiDevId, i, s32Ret));
            }
        }
    }
    
    return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Ao
******************************************************************************/
HI_S32 HISI_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, AO_CHN AoChn, HI_BOOL bResampleEn)
{
    if (HI_TRUE == bResampleEn) {
        HI_MPI_AO_DisableReSmp(AoDevId, AoChn);
    }
    HI_MPI_AO_DisableChn(AoDevId, AoChn);
    HI_MPI_AO_Disable(AoDevId);
    return HI_SUCCESS;
}

/******************************************************************************
* function : Acodec config [ s32Samplerate(0:8k, 1:16k ) ]
******************************************************************************/
static HI_S32 HISI_Acodec_CfgAudio(AUDIO_SAMPLE_RATE_E enSample, HI_BOOL bMicIn)
{
    HI_S32 fdAcodec = -1;
    unsigned int i2s_fs_sel = 0;
//    unsigned int mixer_mic_ctrl = 0;
    unsigned int gain_mic = 0;

    PJ_LOG(4, (THIS_FILE, "%s: enSample = %d", __FUNCTION__, enSample));
    
    fdAcodec = open(ACODEC_FILE,O_RDWR);
    if (fdAcodec < 0) {
        PJ_LOG(4, (THIS_FILE, "%s: can't open acodec, %s", __FUNCTION__, ACODEC_FILE));
//        printf("%s: can't open acodec,%s\n", __FUNCTION__, ACODEC_FILE);
        return HI_FAILURE;     
    }
    
    if(ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL)) {
        PJ_LOG(4, (THIS_FILE, "%s: Reset audio codec error", __FUNCTION__));
//    	printf("Reset audio codec error\n");
    	close(fdAcodec);
    	return HI_FAILURE;
    }
    
    ACODEC_VOL_CTRL vol_ctrl;
    vol_ctrl.vol_ctrl_mute = 0;
    vol_ctrl.vol_ctrl = 12;
//    vol_ctrl.vol_ctrl = 0x0C;
    if (ioctl(fdAcodec, ACODEC_SET_DACR_VOL, &vol_ctrl)) {
        PJ_LOG(4, (THIS_FILE, "%s: set acodec output volume failed", __FUNCTION__));
        close(fdAcodec);
        return HI_FAILURE;
    }
    if (ioctl(fdAcodec, ACODEC_SET_DACL_VOL, &vol_ctrl)) {
        PJ_LOG(4, (THIS_FILE, "%s: set acodec output volume failed", __FUNCTION__));
        close(fdAcodec);
        return HI_FAILURE;
    }
;
    if ((AUDIO_SAMPLE_RATE_8000 == enSample)
        || (AUDIO_SAMPLE_RATE_11025 == enSample)
        || (AUDIO_SAMPLE_RATE_12000 == enSample))
    {
//        i2s_fs_sel = 0x18;
        i2s_fs_sel = ACODEC_FS_8000;
    } else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
        || (AUDIO_SAMPLE_RATE_22050 == enSample)
        || (AUDIO_SAMPLE_RATE_24000 == enSample))
    {
//        i2s_fs_sel = 0x19;
        i2s_fs_sel = ACODEC_FS_16000;
    } else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
        || (AUDIO_SAMPLE_RATE_44100 == enSample)
        || (AUDIO_SAMPLE_RATE_48000 == enSample))
    {
//        i2s_fs_sel = 0x1a;
        i2s_fs_sel = ACODEC_FS_32000;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: not support enSample:%d", __FUNCTION__, enSample));
//        printf("%s: not support enSample:%d\n", __FUNCTION__, enSample);
        close(fdAcodec);
        return HI_FAILURE;
    }
    
    if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel)) {
        PJ_LOG(4, (THIS_FILE, "%s: set acodec sample rate failed", __FUNCTION__));
//        printf("%s: set acodec sample rate failed\n", __FUNCTION__);
        close(fdAcodec);
        return HI_FAILURE;
    }
    
    ACODEC_MIXER_E mixer_mic_ctrl;
//    mixer_mic_ctrl = ACODEC_MIXER_LINEIN;
//    if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl)) {
//        PJ_LOG(4, (THIS_FILE, "%s: set acodec ACODEC_MIXER_LINEIN failed", __FUNCTION__));
////        printf("%s: set acodec sample rate failed\n", __FUNCTION__);
//        close(fdAcodec);
//        return HI_FAILURE;
//    }

    if (HI_TRUE == bMicIn) {
  		/*do not select micin */  
#if 0
        mixer_mic_ctrl = 1;
        if (ioctl(fdAcodec, ACODEC_SET_MIXER_MIC, &mixer_mic_ctrl))
        {
            printf("%s: set acodec micin failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }
#endif		
        /* set volume plus (0~0x1f,default 0x0c) */
        gain_mic = 0x1f;
//        gain_mic = 0x18;
//        gain_mic = 0x0c;
//        gain_mic = 0x1f;
//        gain_mic = 0x11;
        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICL, &gain_mic)) {
            PJ_LOG(4, (THIS_FILE, "%s: set acodec micin volume failed", __FUNCTION__));
//            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }

        if (ioctl(fdAcodec, ACODEC_SET_GAIN_MICR, &gain_mic)) {
            PJ_LOG(4, (THIS_FILE, "%s: set acodec micin volume failed", __FUNCTION__));
//            printf("%s: set acodec micin volume failed\n", __FUNCTION__);
            close(fdAcodec);
            return HI_FAILURE;
        }

    }
//    
//    int db;
////    db = 0x30;
//    db = 0x1E;
//    if (ioctl(fdAcodec, ACODEC_SET_INPUT_VOL, &db)) {
//        printf("ioctl err!\n");
//    }


    close(fdAcodec);    

    return HI_SUCCESS;
}

HI_S32 HISI_COMM_AUDIO_CfgAcodec(AIO_ATTR_S *pstAioAttr, HI_BOOL bMacIn)
{
    HI_S32 s32Ret = HI_FAILURE;
    
    PJ_LOG(4, (THIS_FILE, "%s: pstAioAttr->u32ChnCnt = %d", __FUNCTION__, pstAioAttr->u32ChnCnt));
    
    s32Ret = HISI_Acodec_CfgAudio(pstAioAttr->enSamplerate, bMacIn);
    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif 
#endif /* End of #ifdef __cplusplus */


#endif