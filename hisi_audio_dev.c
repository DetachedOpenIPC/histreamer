/* $Id: null_dev.c 5534 2017-01-19 07:41:25Z nanang $ */
/*
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <pjmedia-audiodev/audiodev_imp.h>
#include <pj/assert.h>
#include <pj/log.h>
#include <pj/os.h>


#if PJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO

#include "hi_sdk/hisi_comm.h"

#define THIS_FILE		"null_dev.c"

/* null_audio device info */
/* HISI device info */
struct hisi_audio_dev_info
{
    pjmedia_aud_dev_info	 info;
    unsigned			 dev_id;
};

/* HISI factory */
struct hisi_audio_factory
{
    pjmedia_aud_dev_factory  base;
//    pj_pool_t                *base_pool;
    pj_pool_t                *pool;
    pj_pool_factory          *pf;
    unsigned                 dev_count;
    struct hisi_audio_dev_info        *dev_info;
};

/* Sound stream. */
struct hisi_audio_stream
{
    /** Base stream.                    */
    pjmedia_aud_stream   base;
    /** Settings.                       */
    pjmedia_aud_param    param;
    /** Memory pool.                    */
    pj_pool_t           *pool;

    /** Capture callback.               */
    pjmedia_aud_rec_cb   rec_cb;
    /** Playback callback.              */
    pjmedia_aud_play_cb  play_cb;
    /** Application data.               */
    void                *user_data;

    int          channel_count;

    /** Extended frame buffer           */
    pjmedia_frame_ext   *xfrm;
    /** Total ext frm size              */
    unsigned             xfrm_size;

    /** Timestamp iterator for capture  */
    pj_timestamp         timestampCapture;
    /** Timestamp iterator for playback */
    pj_timestamp         timestampPlayback;

    
    pj_bool_t        quit;

    pj_bool_t        rec_thread_exited;
    pj_bool_t        rec_thread_initialized;
    pj_thread_desc   rec_thread_desc;
    pj_thread_t     *rec_thread;

    pj_bool_t        play_thread_exited;
    pj_bool_t        play_thread_initialized;
    pj_thread_desc   play_thread_desc;
    pj_thread_t     *play_thread;

    
    pj_int16_t      *rec_buf;
    unsigned         rec_buf_count;
    unsigned long        ca_frames;     /* samples_per_frame        */

    
    pj_int16_t      *play_buf;
    unsigned         play_buf_count;
    unsigned long        pb_frames;     /* samples_per_frame        */

    AUDIO_DEV   AiDev;
    AI_CHN      AiChn;
    AENC_CHN    AeChn;
    AUDIO_DEV   AoDev;
    AO_CHN      AoChn;
    AIO_ATTR_S  stAioAttr;
    HI_U32 s32AoChnCnt;
    HI_U32 s32AiChnCnt;
};



/* Prototypes */
static pj_status_t null_factory_init(pjmedia_aud_dev_factory *f);
static pj_status_t null_factory_destroy(pjmedia_aud_dev_factory *f);
static pj_status_t null_factory_refresh(pjmedia_aud_dev_factory *f);
static unsigned    null_factory_get_dev_count(pjmedia_aud_dev_factory *f);
static pj_status_t null_factory_get_dev_info(pjmedia_aud_dev_factory *f,
					     unsigned index,
					     pjmedia_aud_dev_info *info);
static pj_status_t null_factory_default_param(pjmedia_aud_dev_factory *f,
					      unsigned index,
					      pjmedia_aud_param *param);
static pj_status_t null_factory_create_stream(pjmedia_aud_dev_factory *f,
					      const pjmedia_aud_param *param,
					      pjmedia_aud_rec_cb rec_cb,
					      pjmedia_aud_play_cb play_cb,
					      void *user_data,
					      pjmedia_aud_stream **p_aud_strm);

static pj_status_t null_stream_get_param(pjmedia_aud_stream *strm,
					 pjmedia_aud_param *param);
static pj_status_t null_stream_get_cap(pjmedia_aud_stream *strm,
				       pjmedia_aud_dev_cap cap,
				       void *value);
static pj_status_t null_stream_set_cap(pjmedia_aud_stream *strm,
				       pjmedia_aud_dev_cap cap,
				       const void *value);
static pj_status_t null_stream_start(pjmedia_aud_stream *strm);
static pj_status_t null_stream_stop(pjmedia_aud_stream *strm);
static pj_status_t null_stream_destroy(pjmedia_aud_stream *strm);

/* Operations */
static pjmedia_aud_dev_factory_op factory_op =
{
    &null_factory_init,
    &null_factory_destroy,
    &null_factory_get_dev_count,
    &null_factory_get_dev_info,
    &null_factory_default_param,
    &null_factory_create_stream,
    &null_factory_refresh
};

static pjmedia_aud_stream_op stream_op =
{
    &null_stream_get_param,
    &null_stream_get_cap,
    &null_stream_set_cap,
    &null_stream_start,
    &null_stream_stop,
    &null_stream_destroy
};

//#define HISI_PRT(fmt...)   \
//    do {\
//        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
//        printf(fmt);\
//       }while(0)

#define PJ_LOG2(level,arg)	do { \
				    if (level <= pj_log_get_level()) { \
					pj_log_wrapper_##level(arg); \
				    } \
				} while (0)

//#define PJ_HISI_PRT(fmt...)\
    

#define PJ_HISI_PRT(fmt...)   \
    do {\
        PJ_LOG(4, (THIS_FILE, "[%s]-%d: " fmt, __FUNCTION__, __LINE__));\
       }while(0)
//
//        PJ_LOG(4, (THIS_FILE, "[%s]-%d:", __FUNCTION__, __LINE__, fmt));\
//#define HISI_PRT(fmt...)   \
//    do {\
//        PJ_LOG(4, (THIS_FILE, "[%s]-%d: ", __FUNCTION__, __LINE__, fmt));\
//        printf("[%s]-%d: ", __FUNCTION__, __LINE__);\
//        printf(fmt);\
//       }while(0)
/****************************************************************************
 * Factory operations
 */
/*
 * Init null_audio audio driver.
 */
pjmedia_aud_dev_factory* pjmedia_null_audio_factory(pj_pool_factory *pf)
{
//    PJ_HISI_PRT("pjmedia_null_audio_factory");
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: pjmedia_null_audio_factory", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *f;
    pj_pool_t *pool;

    pool = pj_pool_create(pf, "HISI_DRIVER", 1000, 1000, NULL);
    f = PJ_POOL_ZALLOC_T(pool, struct hisi_audio_factory);
    f->pf = pf;
    f->pool = pool;
//    f->base_pool = pool;
    f->base.op = &factory_op;

    return &f->base;
}

//static pj_status_t hisi_audio_factory_init(pjmedia_aud_dev_factory *f) {
//    PJ_LOG(4, (THIS_FILE, "hisi_audio_factory_init"));
//    pj_status_t ret = null_factory_refresh(f);
//    if (ret != PJ_SUCCESS) return ret;
//
//    PJ_LOG(4, (THIS_FILE, "HISI initialized"));
//
//    return PJ_SUCCESS;
//}

/* API: init factory */
static pj_status_t null_factory_init(pjmedia_aud_dev_factory *f)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_init", __FUNCTION__, __LINE__));
//    PJ_HISI_PRT("[%s]-%d: null_factory_init", __FUNCTION__, __LINE__);
//    struct hisi_audio_factory *nf = (struct hisi_audio_factory*)f;
//    struct hisi_audio_dev_info *ndi;
//    struct hisi_audio_dev_info *ndi;
    
    pj_status_t ret = null_factory_refresh(f);
    if (ret != PJ_SUCCESS) return ret;

    PJ_LOG(4, (THIS_FILE, "[%s]-%d: HISI initialized", __FUNCTION__, __LINE__));
//    PJ_HISI_PRT("[%s]-%d: HISI initialized", __FUNCTION__, __LINE__);
    
    return PJ_SUCCESS;
}

/* API: destroy factory */
static pj_status_t null_factory_destroy(pjmedia_aud_dev_factory *f)
{
//    PJ_HISI_PRT("[%s]-%d: null_factory_destroy", __FUNCTION__, __LINE__);
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_destroy", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *wf = (struct hisi_audio_factory*)f;

    HI_S32 s32Ret;
    
    s32Ret = HISI_COMM_SYS_Exit();
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s-%d: system destroy failed with %d!", __FUNCTION__, __LINE__, s32Ret));
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s-%d: system destroy success with %d!", __FUNCTION__, __LINE__, s32Ret));
    }
    
//    pj_pool_t *pool = wf->base_pool;

//    pj_pool_release(wf->pool);
//    wf->base_pool = NULL;
//    pj_pool_release(pool);
    
    pj_pool_safe_release(&wf->pool);

    return PJ_SUCCESS;
}



/* API: refresh the list of devices */
static pj_status_t null_factory_refresh(pjmedia_aud_dev_factory *f)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_refresh", __FUNCTION__, __LINE__));
//    PJ_HISI_PRT("[%s]-%d: null_factory_refresh", __FUNCTION__, __LINE__);
    struct hisi_audio_factory *wf = (struct hisi_audio_factory*)f;
    
//    if(wf->pool != NULL) {
////        pj_pool_safe_release(&wf->pool);
//        pj_pool_release(wf->pool);
//        wf->pool = NULL;
//    }
        
    struct hisi_audio_dev_info *ndi;
    
//    wf->pool = pj_pool_create(wf->pf, "HISI_DRIVER", 1000, 1000, NULL);
////    wf->pool = pj_pool_create(wf->pf, "HISI_DEVICES", 1000, 1000, NULL);
//    if (wf->pool == NULL) {
////        my_perror("Error creating pool", PJ_ENOMEM);
//        PJ_LOG(4, (THIS_FILE, "[%s]-%d: Error creating pool", __FUNCTION__, __LINE__));
////        PJ_HISI_PRT("[%s]-%d: Error creating pool", __FUNCTION__, __LINE__);
//        return PJ_ENOMEM;
//    }
    
    HI_S32 s32Ret = HI_SUCCESS;
    VB_CONF_S stVbConf;
    
    pj_memset(&stVbConf, 0, sizeof(VB_CONF_S));
    s32Ret = HISI_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: system init failed with %#x!", __FUNCTION__, s32Ret));
        return HI_FAILURE;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: system init success with %#x!", __FUNCTION__, s32Ret));
    }
    
//    AUDIO_OUT();

    /* Initialize input and output devices here */
    wf->dev_count = 1;
    wf->dev_info = (struct hisi_audio_dev_info*)
            pj_pool_calloc(wf->pool, wf->dev_count,
            sizeof(struct hisi_audio_dev_info));
    
    wf->dev_info[0].dev_id = 0;
    wf->dev_info[0].info.caps = 0;
    // bdIMADpj_getDeviceCapabilities(HISI_CAPTURE_DEVICES,&wf->dev_info[i].info.caps);
//    wf->dev_info[0].info.default_samples_per_sec = 16000;
    wf->dev_info[0].info.default_samples_per_sec = 48000;
    strcpy(wf->dev_info[0].info.driver, "HISI");
    wf->dev_info[0].info.ext_fmt_cnt = 0;
    wf->dev_info[0].info.input_count = 2;
    wf->dev_info[0].info.output_count = 2;
    strcpy(wf->dev_info[0].info.name, "HISI device");
    wf->dev_info[0].info.routes = 0;
    
    
//    ndi = &wf->dev_info[0];
//    pj_bzero(ndi, sizeof(*ndi));
//    strcpy(ndi->info.name, "null device");
//    strcpy(ndi->info.driver, "null");
//    ndi->info.input_count = 1;
//    ndi->info.output_count = 1;
//    ndi->info.default_samples_per_sec = 16000;
//    /* Set the device capabilities here */
//    ndi->info.caps = 0;

//    PJ_HISI_PRT("[%s]-%d: HISI found %d devices:", __FUNCTION__, __LINE__, wf->dev_count);
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: HISI found %d devices:", __FUNCTION__, __LINE__, wf->dev_count));
    
    for(int i=0; i<wf->dev_count; i++) {
        
        PJ_LOG(4,
           (THIS_FILE, "dev_id %d: %s  (in=%d, out=%d)",
               i,
               wf->dev_info[i].info.name,
               wf->dev_info[i].info.input_count,
               wf->dev_info[i].info.output_count));
    }
//    PJ_LOG(4, (THIS_FILE, "null audio initialized"));
        
    PJ_UNUSED_ARG(f);
    return PJ_SUCCESS;
}

/* API: get number of devices */
static unsigned null_factory_get_dev_count(pjmedia_aud_dev_factory *f)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_get_dev_count", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *wf = (struct hisi_audio_factory*)f;
    return wf->dev_count;
}

/* API: get device info */
static pj_status_t null_factory_get_dev_info(pjmedia_aud_dev_factory *f,
					     unsigned index,
					     pjmedia_aud_dev_info *info)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_get_dev_info", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *nf = (struct hisi_audio_factory*)f;

    PJ_ASSERT_RETURN(index < nf->dev_count, PJMEDIA_EAUD_INVDEV);

    pj_memcpy(info, &nf->dev_info[index].info, sizeof(*info));

    return PJ_SUCCESS;
}

/* API: create default device parameter */
static pj_status_t null_factory_default_param(pjmedia_aud_dev_factory *f,
					      unsigned index,
					      pjmedia_aud_param *param)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_default_param", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *nf = (struct hisi_audio_factory*)f;
    struct hisi_audio_dev_info *di = &nf->dev_info[index];

    PJ_ASSERT_RETURN(index < nf->dev_count, PJMEDIA_EAUD_INVDEV);

    pj_bzero(param, sizeof(*param));
    if (di->info.input_count && di->info.output_count) {
	param->dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
	param->rec_id = index;
	param->play_id = index;
        param->channel_count = di->info.output_count;
    } else if (di->info.input_count) {
	param->dir = PJMEDIA_DIR_CAPTURE;
	param->rec_id = index;
	param->play_id = PJMEDIA_AUD_INVALID_DEV;
        param->channel_count = di->info.input_count;
    } else if (di->info.output_count) {
	param->dir = PJMEDIA_DIR_PLAYBACK;
	param->play_id = index;
	param->rec_id = PJMEDIA_AUD_INVALID_DEV;
        param->channel_count = di->info.output_count;
    } else {
	return PJMEDIA_EAUD_INVDEV;
    }

    /* Set the mandatory settings here */
    /* The values here are just some examples */
    param->clock_rate = di->info.default_samples_per_sec;
    param->channel_count = 1;
    param->samples_per_frame = di->info.default_samples_per_sec * 20 / 1000;
//    param->samples_per_frame = param->clock_rate * param->channel_count * HISI_MSECOND_PER_BUFFER / 1000;
    param->bits_per_sample = 16;

    /* Set the device capabilities here */
//    param->flags = 0;
    param->flags = di->info.caps;

    return PJ_SUCCESS;
}

AIO_ATTR_S HISI_COMM_GET_AIO_ATTR(unsigned clock_rate) {
    AIO_ATTR_S stAioAttr;

    if (clock_rate != 8000 && clock_rate != 16000
       && clock_rate != 32000 && clock_rate != 44100 && clock_rate != 48000) {
        PJ_LOG(4, (THIS_FILE,
           "HISI support 8000 Hz, 16000 Hz, 32000 Hz, 44100 Hz and 48000 Hz "
           "frequency."));
        stAioAttr.enSamplerate   = 0;
        stAioAttr.enBitwidth     = 0;
        stAioAttr.enWorkmode     = 0;
        stAioAttr.enSoundmode    = 0;
        stAioAttr.u32EXFlag      = 0;
        stAioAttr.u32FrmNum      = 0;
        stAioAttr.u32PtNumPerFrm = 0;
        stAioAttr.u32ChnCnt      = 0;
        stAioAttr.u32ClkSel      = 0;
        return stAioAttr;
    }
    // if (clock_rate == 8000) {

    // }

#ifdef HI_ACODEC_TYPE_TLV320AIC31
    // printf("%s: HI_ACODEC_TYPE_TLV320AIC31\n\n", __FUNCTION__);
    // PJ_LOG(4, (THIS_FILE, "%s: HI_ACODEC_TYPE_TLV320AIC31", __FUNCTION__));
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_16000;
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enSamplerate   = clock_rate;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    // stAioAttr.u32PtNumPerFrm = HISI_AUDIO_PTNUMPERFRM;

    stAioAttr.u32PtNumPerFrm = clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 1;
#else //  inner acodec
    
//    #define SAMPLE_AUDIO_PTNUMPERFRM   320
        /* init stAio. all of cases will use it */
#if 0
    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 0;
    stAioAttr.u32EXFlag = 1;
//    stAioAttr.u32FrmNum = 10;
    stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 1;
#else
    /*
//    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_16000;
    clock_rate = 8000;
    stAioAttr.enSamplerate = clock_rate;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_8;
//    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enWorkmode = AIO_MODE_I2S_SLAVE;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 1;
    stAioAttr.u32FrmNum = 10;
//    stAioAttr.u32FrmNum = 30;
    stAioAttr.u32PtNumPerFrm = 320;
//    stAioAttr.u32PtNumPerFrm = clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 0;
    */
//    clock_rate = clock_rate * 2;
    stAioAttr.enSamplerate = clock_rate;
    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_8;
    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag = 1;
    stAioAttr.u32FrmNum = 30;
//    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32PtNumPerFrm = clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000;
//    stAioAttr.u32PtNumPerFrm = (clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000)/3;
    stAioAttr.u32ChnCnt = 2;
    stAioAttr.u32ClkSel = 0;
    
//    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
//    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
//    stAioAttr.u32EXFlag = 1;
//    stAioAttr.u32FrmNum = 30;
//    stAioAttr.u32PtNumPerFrm = 320;
//    stAioAttr.u32ChnCnt = 2;
//    stAioAttr.u32ClkSel = 1;
//    
//    stAioAttr.u32PtNumPerFrm = 640;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.u32EXFlag = 0;
//    stAioAttr.u32ClkSel = 0;
    
#endif
    /*
    // printf("%s: NO HI_ACODEC_TYPE_TLV320AIC31\n\n", __FUNCTION__);
    // PJ_LOG(4, (THIS_FILE, "%s: NO HI_ACODEC_TYPE_TLV320AIC31", __FUNCTION__));
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    // stAioAttr.enSamplerate   = enAudioSampleRate;
    stAioAttr.enSamplerate   = clock_rate;
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_32000;
    // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_16000;
    // stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_8;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    // stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_STEREO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30; // Value range: [2, MAX_AUDIO_FRAME_NUM 50]
    // stAioAttr.u32PtNumPerFrm = HISI_AUDIO_PTNUMPERFRM;

    stAioAttr.u32PtNumPerFrm = clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;
    */
    
        PJ_LOG(4, (THIS_FILE, "%s: stAioAttr.enSamplerate = %d", __FUNCTION__, stAioAttr.enSamplerate));
        PJ_LOG(4, (THIS_FILE, "%s: stAioAttr.enBitwidth = %d", __FUNCTION__, stAioAttr.enBitwidth));
        PJ_LOG(4, (THIS_FILE, "%s: stAioAttr.enSoundmode = %d", __FUNCTION__, stAioAttr.enSoundmode));
        PJ_LOG(4, (THIS_FILE, "%s: stAioAttr.u32PtNumPerFrm = %d", __FUNCTION__, stAioAttr.u32PtNumPerFrm));
#endif
    /* config ao resample attr if needed */
    if ((HI_TRUE == gs_bAiReSample) && (enDeviceAudioSampleRate != clock_rate)) {
        unsigned multipl = clock_rate / enDeviceAudioSampleRate;
        // printf("%s: gs_bAioReSample enabled\n\n", __FUNCTION__);
        PJ_LOG(4, (THIS_FILE, "%s: gs_bAiReSample enabled", __FUNCTION__));
        PJ_LOG(4, (THIS_FILE, "%s: multipl x%d enabled", __FUNCTION__, multipl));
        // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_32000;
        // stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
        // stAioAttr.enSamplerate   = clock_rate;
        // stAioAttr.u32PtNumPerFrm = HISI_AUDIO_PTNUMPERFRM * 4;
        stAioAttr.u32PtNumPerFrm = stAioAttr.u32PtNumPerFrm * multipl;

        // stAioAttr.u32PtNumPerFrm = clock_rate * ((int)stAioAttr.enSoundmode + 1) * HISI_MSECOND_PER_BUFFER / 1000;

        /* ai 32k -> 8k */
        enOutSampleRate = enDeviceAudioSampleRate;

        /* ao 8k -> 32k */
        // enInSampleRate  = AUDIO_SAMPLE_RATE_8000;
        // enInSampleRate  = AUDIO_SAMPLE_RATE_8000;
        enInSampleRate  = enDeviceAudioSampleRate;
    } else {
        // printf("%s: gs_bAioReSample disabled\n\n", __FUNCTION__);
        PJ_LOG(4, (THIS_FILE, "%s: gs_bAioReSample disabled", __FUNCTION__));
        // enInSampleRate  = AUDIO_SAMPLE_RATE_16000;
        enInSampleRate  = clock_rate;
        // enOutSampleRate = AUDIO_SAMPLE_RATE_16000;
        enOutSampleRate = clock_rate;
    }
    PJ_LOG(4, (THIS_FILE, "%s: stAioAttr.enSamplerate: %d", __FUNCTION__, stAioAttr.enSamplerate));
    PJ_LOG(4, (THIS_FILE, "%s: enOutSampleRate:        %d", __FUNCTION__, enOutSampleRate));
    PJ_LOG(4, (THIS_FILE, "%s: enInSampleRate:         %d", __FUNCTION__, enInSampleRate));
    return stAioAttr;
}

static pj_status_t hisi_audio_open_playback (struct hisi_audio_stream *stream,
    // struct hisi_audio_factory *wf, const pjmedia_aud_param *param) {
    struct hisi_audio_factory *wf) {

    HI_S32 s32Ret;
    PJ_LOG(4, (THIS_FILE, "hisi_audio_open_playback"));
    PJ_LOG(4, (THIS_FILE, "HISI_COMM_GET_AIO_ATTR"));
    PJ_LOG(4, (THIS_FILE, "%s: stream->stAioAttr.u32ChnCnt       %d!", __FUNCTION__, stream->stAioAttr.u32ChnCnt));

    /* enable AO channle */
    AIO_ATTR_S stAioAttr = {0};
    AUDIO_RESAMPLE_ATTR_S stAudioResampleAttr = {0};
    pj_memcpy(&stAioAttr, &stream->stAioAttr, sizeof(stream->stAioAttr));
    
//    AUDIO_SAMPLE_RATE_E enSamplerate;   /* sample rate*/
//    
//    AUDIO_BIT_WIDTH_E   enBitwidth;     /* bitwidth*/
//    AIO_MODE_E          enWorkmode;     /* master or slave mode*/
//    AUDIO_SOUND_MODE_E  enSoundmode;    /* momo or steror*/
//    HI_U32              u32EXFlag;      /* expand 8bit to 16bit,use AI_EXPAND(only valid for AI 8bit) */
//    HI_U32              u32FrmNum;      /* frame num in buf[2,MAX_AUDIO_FRAME_NUM]*/
//    HI_U32              u32PtNumPerFrm; /* point num per frame (80/160/240/320/480/1024/2048)
//                                            (ADPCM IMA should add 1 point, AMR only support 160)*/
//    HI_U32              u32ChnCnt;      /* channle number on FS, valid value:2/4/8/16 */
//    HI_U32              u32ClkSel;      /* clock select, meaning is diffrent when master and slave mode,
//                                                if sio slave mode:
//                                                    0: AD and DA clock of codec is separate 
//                                                    1: AD and DA clock of codec is inseparate 
//                                                else sio master mode:
//                                                    0:ACKOUT clock is from sio 0
//                                                    1:ACKOUT clock is from sio 1/2
//                                                    */
    
        /* init stAio. all of cases will use it */
//    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
//    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
//    stAioAttr.u32EXFlag = 1;
//    stAioAttr.u32FrmNum = 30;
////    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
//    stAioAttr.u32PtNumPerFrm = 640;
////    stAioAttr.u32PtNumPerFrm = 320;
//    stAioAttr.u32ChnCnt = 2;
//    stAioAttr.u32ClkSel = 1;
    
    
    
//    stAioAttr.u32PtNumPerFrm = 320;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.u32EXFlag = 0;
    
//    stAioAttr.u32PtNumPerFrm = 320;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.u32EXFlag = 0;
    
//    s32Ret = HISI_COMM_AUDIO_StartAo(stream->AoDev, stAioAttr.u32ChnCnt, &stAioAttr, enInSampleRate, gs_bAoReSample, NULL, 0);
    s32Ret = HISI_COMM_AUDIO_StartAo(stream->AoDev, stream->AoChn, &stAioAttr, NULL); // <================================
//    s32Ret = HISI_COMM_AUDIO_StartAo(stream->AoDev, stAioAttr.u32ChnCnt, &stAioAttr, NULL);
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAo failed with %d!", __FUNCTION__, s32Ret));
        HISI_COMM_AUDIO_StopAi(stream->AiDev, stAioAttr.u32ChnCnt, gs_bAiReSample, HI_FALSE);
        return s32Ret;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAo success with %d!", __FUNCTION__, s32Ret));
    }

#if !defined(PJMEDIA_HAS_PASSTHROUGH_CODECS) && PJMEDIA_HAS_PASSTHROUGH_CODECS!=0

    u32AencPtNumPerFrm = stAioAttr.u32PtNumPerFrm;

    HI_S32 s32AdecChn = 0;
    s32Ret = HISI_COMM_AUDIO_StartAdec(s32AdecChn, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAdec failed with %d!", __FUNCTION__, s32Ret));
        return s32Ret;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAdec success with %d!", __FUNCTION__, s32Ret));
    }

    s32Ret = HISI_COMM_AUDIO_AoBindAdec(stream->AoDev, stream->AoChn, stream->AdChn);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_AoBindAdec failed with %d!", __FUNCTION__, s32Ret));
        HISI_COMM_AUDIO_AoUnbindAdec(stream->AoDev, stream->AoChn, stream->AdChn);
    }

#endif  /* PJMEDIA_HAS_PASSTHROUGH_CODECS */

    // unsigned int rate;
    unsigned long tmp_buf_size;

    PJ_LOG(4, (THIS_FILE, "============================================================================================"));

    // rate = stream->param.clock_rate;
    // PJ_LOG(4, (THIS_FILE, "%s: param->output_latency_ms: %d", __FUNCTION__, param->output_latency_ms));
    /* Set the sound device buffer size and latency */
    if (stream->param.flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY) {
        // PJ_LOG(4, (THIS_FILE, "%s: param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY", __FUNCTION__));
        tmp_buf_size = stream->param.clock_rate * stream->param.output_latency_ms / 1000;
    } else {
        // PJ_LOG(4, (THIS_FILE, "%s: param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_LATENCY NO", __FUNCTION__));
        tmp_buf_size = stream->param.clock_rate * PJMEDIA_SND_DEFAULT_PLAY_LATENCY / 1000;
    }
    /* Set period size to samples_per_frame frames. */

    PJ_LOG(4, (THIS_FILE, "%s: param->samples_per_frame %d!", __FUNCTION__, stream->param.samples_per_frame));

    // stream->pb_frames = param->samples_per_frame / param->channel_count;
    // stream->pb_frames = 80 / param->channel_count;
    stream->param.output_latency_ms = tmp_buf_size * 1000 / stream->param.clock_rate;
    // stream->param.output_latency_ms = 100;

    /* Set our buffer */ // OLD
    // stream->play_buf_count = stream->pb_frames * param->channel_count *
    //                       param->bits_per_sample / 8;
    unsigned ptime;
    ptime = stream->param.samples_per_frame * 1000 / (stream->param.clock_rate * stream->param.channel_count);
    PJ_LOG(4, (THIS_FILE, "%s: ptime %d!", __FUNCTION__, ptime));
    stream->play_buf_count = (stream->param.clock_rate *
        ((stream->param.channel_count * stream->param.bits_per_sample) / 8)) * ptime / 1000;

    // PJMEDIA_SPF(param->clock_rate, ptime, param->channel_count)

    // (16000 *
    //     ((1 * 16) / 8)) * 320
    // (param->clock_rate *
    //    ((param->channel_count * param->bits_per_sample) / 8)) * param->samples_per_frame

        // stream->pb_frames * param->channel_count *
        //                   param->bits_per_sample / 8;

                          //     // ptime = prm->samples_per_frame *
//     //     1000 /
//     //     (prm->clock_rate * prm->channel_count);
//     // strm->bytes_per_frame = (prm->clock_rate *
//     //             ((prm->channel_count * prm->bits_per_sample) / 8)) *
//     //              ptime /
//     //              1000;
    // stream->play_buf = (char *) pj_pool_alloc(stream->pool, stream->play_buf_count);
    // stream->play_buf = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->play_buf_count);
    // stream->play_buf = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->bytes_per_frame);
    stream->play_buf = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->play_buf_count);
    if (!stream->play_buf) {
        PJ_LOG(4, (THIS_FILE, "%s: pj_pool_alloc failed!", __FUNCTION__));
        // pj_pool_release(stream->pool);
        return PJ_ENOMEM;
    }
    // stream->play_buf_count = 0;

    // stream->rec_buf = (pj_int16_t*)pj_pool_alloc(pool,
    //         stream->bytes_per_frame);
    // if (!stream->rec_buf) {
    //     pj_pool_release(pool);
    //     return PJ_ENOMEM;
    // }
    // stream->rec_buf_count = 0;

    // stream->play_buf = (pj_int16_t*)pj_pool_alloc(pool, stream->bytes_per_frame);
    // if (!stream->play_buf) {
    //     pj_pool_release(pool);
    //     return PJ_ENOMEM;
    // }


    PJ_LOG(4, (THIS_FILE, "%s: rate               %d!", __FUNCTION__, stream->param.clock_rate));
    PJ_LOG(4, (THIS_FILE, "%s: output_vol         %d!", __FUNCTION__, stream->param.output_vol));
    PJ_LOG(4, (THIS_FILE, "%s: samples_per_frame  %d!", __FUNCTION__, stream->param.samples_per_frame));
    PJ_LOG(4, (THIS_FILE, "%s: channel_count      %d!", __FUNCTION__, stream->param.channel_count));
    PJ_LOG(4, (THIS_FILE, "%s: input_latency_ms   %d!", __FUNCTION__, stream->param.input_latency_ms));
    PJ_LOG(4, (THIS_FILE, "%s: pb_frames          %d!", __FUNCTION__, stream->pb_frames));
    PJ_LOG(4, (THIS_FILE, "%s: tmp_buf_size       %d!", __FUNCTION__, tmp_buf_size));
    PJ_LOG(4, (THIS_FILE, "%s: output_latency_ms  %d!", __FUNCTION__, stream->param.output_latency_ms));
    PJ_LOG(4, (THIS_FILE, "%s: bits_per_sample    %d!", __FUNCTION__, stream->param.bits_per_sample));
    PJ_LOG(4, (THIS_FILE, "%s: play_buf_count     %d!", __FUNCTION__, stream->play_buf_count));
    PJ_LOG(4, (THIS_FILE, "%s: play_buf           %i!", __FUNCTION__, stream->play_buf));

    PJ_LOG(4, (THIS_FILE, "============================================================================================"));
    PJ_LOG(4, (THIS_FILE, "hisi_audio_open_playback: pb_frames = %d clock = %d",
                       stream->pb_frames, stream->param.clock_rate));

    return PJ_SUCCESS;
}

static pj_status_t hisi_audio_open_capture (struct hisi_audio_stream *stream,
    // struct hisi_audio_factory *wf, const pjmedia_aud_param *param) {
    struct hisi_audio_factory *wf) {

    HI_S32 s32Ret;
    // HI_S32 s32AiChnCnt;
    // int captureDeviceCount = 1;
    // AUDIO_DEV   AiDev = HISI_AUDIO_AI_DEV;
    // AI_CHN      AiChn = 0;
    // HI_S32 s32AiChnCnt;
    // struct stAioAttr *pstAioAttr = (struct stAioAttr*)stream.stAioAttr;
    // AIO_ATTR_S stAioAttr = HISI_COMM_GET_AIO_ATTR();
    PJ_LOG(4, (THIS_FILE, "hisi_audio_open_capture"));
    PJ_ASSERT_RETURN(stream->param.rec_id < (int)wf->dev_count, PJMEDIA_EAUD_INVDEV);

    // PJ_ASSERT_RETURN(param->bits_per_sample == 16, PJMEDIA_EAUD_SAMPFORMAT);
    /* enable AI channle */
    // s32AiChnCnt = stream->stAioAttr.u32ChnCnt;
    AIO_ATTR_S stAioAttr;
    PJ_LOG(4, (THIS_FILE, "%s: stream->stAioAttr.u32ChnCnt       %d!", __FUNCTION__, stream->stAioAttr.u32ChnCnt));
    // AIO_ATTR_S stAioAttr = (AIO_ATTR_S)stream->stAioAttr;
    
//    stAioAttr.enSamplerate = AUDIO_SAMPLE_RATE_8000;
//    stAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;
//    stAioAttr.enWorkmode = AIO_MODE_I2S_MASTER;
//    stAioAttr.enSoundmode = AUDIO_SOUND_MODE_MONO;
//    stAioAttr.u32EXFlag = 0;
//    stAioAttr.u32FrmNum = 10;
////    stAioAttr.u32FrmNum = 30;
//    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
//    stAioAttr.u32ChnCnt = 2;
//    stAioAttr.u32ClkSel = 1;
    pj_memcpy(&stAioAttr, &stream->stAioAttr, sizeof(stream->stAioAttr));
    // s32AiChnCnt = stAioAttr.u32ChnCnt;
    PJ_LOG(4, (THIS_FILE, "%s: stream->stAioAttr.u32ChnCnt       %d!", __FUNCTION__, stream->stAioAttr.u32ChnCnt));

    PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi s32AiChnCnt %d!", __FUNCTION__, stAioAttr.u32ChnCnt));
    PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi gs_bAiReSample %d!", __FUNCTION__, gs_bAiReSample));
    PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi gs_bAoReSample %d!", __FUNCTION__, gs_bAoReSample));
    PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi enInSampleRate %d!", __FUNCTION__, enInSampleRate));
    PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi stream->AiDev %d!", __FUNCTION__, stream->AiDev));


    // AI_VQE_CONFIG_S stAiVqeAttr;
    // HI_VOID     *pAiVqeAttr = NULL;
    // static HI_U32 u32AiVqeType = 1;

    // if (1 == u32AiVqeType)
    // {
    //     stAiVqeAttr.s32WorkSampleRate    = AUDIO_SAMPLE_RATE_8000;
    //     stAiVqeAttr.s32FrameSample       = HISI_AUDIO_PTNUMPERFRM;
    //     stAiVqeAttr.enWorkstate          = VQE_WORKSTATE_COMMON;
    //     stAiVqeAttr.bAecOpen             = HI_TRUE;
    //     stAiVqeAttr.stAecCfg.bUsrMode    = HI_FALSE;
    //     stAiVqeAttr.stAecCfg.s8CngMode   = 0;
    //     stAiVqeAttr.bAgcOpen             = HI_TRUE;
    //     stAiVqeAttr.stAgcCfg.bUsrMode    = HI_FALSE;
    //     stAiVqeAttr.bAnrOpen             = HI_TRUE;
    //     stAiVqeAttr.stAnrCfg.bUsrMode    = HI_FALSE;
    //     stAiVqeAttr.bHpfOpen             = HI_TRUE;
    //     stAiVqeAttr.stHpfCfg.bUsrMode    = HI_TRUE;
    //     stAiVqeAttr.stHpfCfg.enHpfFreq   = AUDIO_HPF_FREQ_150;
    //     stAiVqeAttr.bRnrOpen             = HI_FALSE;
    //     stAiVqeAttr.bEqOpen              = HI_FALSE;
    //     stAiVqeAttr.bHdrOpen             = HI_FALSE;

    //     pAiVqeAttr = (HI_VOID *)&stAiVqeAttr;
    // }
    // else
    // {
    //     pAiVqeAttr = HI_NULL;
    // }
    
//    HI_S32 ChnCnt = stAioAttr.u32ChnCnt-1;
//    stAioAttr.u32ChnCnt = 1;

    s32Ret = HISI_COMM_AUDIO_StartAi(stream->AiDev, stAioAttr.u32ChnCnt, &stAioAttr, HI_FALSE, NULL);
//    s32Ret = HISI_COMM_AUDIO_StartAi(stream->AiDev, stAioAttr.u32ChnCnt, &stAioAttr, enOutSampleRate, gs_bAiReSample, NULL, 0);
    // s32Ret = HISI_COMM_AUDIO_StartAi(stream->AiDev, stAioAttr.u32ChnCnt, &stAioAttr, enOutSampleRate, gs_bAiReSample, pAiVqeAttr, u32AiVqeType);
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi(%d) failed with %d!", __FUNCTION__, stAioAttr.u32ChnCnt, s32Ret));
        return s32Ret;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAi(%d) success with %d!", __FUNCTION__, stAioAttr.u32ChnCnt, s32Ret));
    }

#if defined(PJMEDIA_HAS_PASSTHROUGH_CODECS) && PJMEDIA_HAS_PASSTHROUGH_CODECS!=0

    u32AencPtNumPerFrm = stAioAttr.u32PtNumPerFrm;

    HI_S32 s32AencChnCnt = 1;
    s32Ret = HISI_COMM_AUDIO_StartAenc(s32AencChnCnt, u32AencPtNumPerFrm, gs_enPayloadType);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAenc failed with %d!", __FUNCTION__, s32Ret));
        return s32Ret;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StartAenc success with %d!", __FUNCTION__, s32Ret));
    }

    s32Ret = HISI_COMM_AUDIO_AencBindAi(stream->AiDev, stream->AiChn, stream->AeChn);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_AencBindAi failed with %d!", __FUNCTION__, s32Ret));
        HISI_COMM_AUDIO_AencUnbindAi(stream->AiDev, stream->AiChn, stream->AeChn);
    }

#endif  /* PJMEDIA_HAS_PASSTHROUGH_CODECS */

    // unsigned int rate;
    unsigned long tmp_buf_size;

    // /* Set clock rate */
    // rate = param->clock_rate;
    // stream->ca_frames = (unsigned long) param->samples_per_frame /
    //         param->channel_count;

    // /* Set the sound device buffer size and latency */
    // if (param->flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY) {
    //     tmp_buf_size = rate * param->input_latency_ms / 1000;
    // } else {
    //     tmp_buf_size = rate * PJMEDIA_SND_DEFAULT_REC_LATENCY / 1000;
    // }

    // stream->param.input_latency_ms = tmp_buf_size * 1000 / rate;

    // /* Set our buffer */
    // stream->ca_buf_size = stream->ca_frames * param->channel_count *
    //           param->bits_per_sample / 8;
    // stream->rec_buf = (char *)pj_pool_alloc (stream->pool, stream->ca_buf_size);

    // TRACE_((THIS_FILE, "bb10_open_capture: ca_frames = %d clock = %d",
    //                    stream->ca_frames, param->clock_rate));

    PJ_LOG(4, (THIS_FILE, "============================================================================================"));

    /* Set clock rate */
    // rate = stream->param.clock_rate;

    PJ_LOG(4, (THIS_FILE, "%s: param->input_latency_ms: %d!", __FUNCTION__, stream->param.input_latency_ms));
    PJ_LOG(4, (THIS_FILE, "%s: param->input_vol:        %d!", __FUNCTION__, stream->param.input_vol));
    PJ_LOG(4, (THIS_FILE, "%s: rate                     %d!", __FUNCTION__, stream->param.clock_rate));
    PJ_LOG(4, (THIS_FILE, "%s: samples_per_frame        %d!", __FUNCTION__, stream->param.samples_per_frame));
    PJ_LOG(4, (THIS_FILE, "%s: channel_count            %d!", __FUNCTION__, stream->param.channel_count));
    PJ_LOG(4, (THIS_FILE, "%s: input_latency_ms         %d!", __FUNCTION__, stream->param.input_latency_ms));

    // stream->ca_frames = (unsigned long) param->samples_per_frame /
    //         param->channel_count;

    PJ_LOG(4, (THIS_FILE, "%s: ca_frames          %d!", __FUNCTION__, stream->ca_frames));

    /* Set the sound device buffer size and latency */
    if (stream->param.flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY) {
        PJ_LOG(4, (THIS_FILE, "%s: param->flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY", __FUNCTION__));
        tmp_buf_size = stream->param.clock_rate * stream->param.input_latency_ms / 1000;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: param->flags & PJMEDIA_AUD_DEV_CAP_INPUT_LATENCY NO", __FUNCTION__));
        tmp_buf_size = stream->param.clock_rate * PJMEDIA_SND_DEFAULT_REC_LATENCY / 1000;
    }

    PJ_LOG(4, (THIS_FILE, "%s: tmp_buf_size       %d!", __FUNCTION__, tmp_buf_size));

    PJ_LOG(4, (THIS_FILE, "%s: p.input_latency_ms %d!", __FUNCTION__, stream->param.input_latency_ms));
    stream->param.input_latency_ms = tmp_buf_size * 1000 / stream->param.clock_rate;
    PJ_LOG(4, (THIS_FILE, "%s: input_latency_ms   %d!", __FUNCTION__, stream->param.input_latency_ms));
    PJ_LOG(4, (THIS_FILE, "%s: bits_per_sample    %d!", __FUNCTION__, stream->param.bits_per_sample));

    /* Set period size to samples_per_frame frames. */
    // stream->pb_frames = param->samples_per_frame / param->channel_count;
    // stream->param.output_latency_ms = tmp_buf_size * 1000 / rate;

    /* Set our buffer */ // OLD
    // stream->rec_buf_count = stream->ca_frames * param->channel_count *
    //                       param->bits_per_sample / 8;
    unsigned ptime;
    ptime = stream->param.samples_per_frame * 1000 / (stream->param.clock_rate * stream->param.channel_count);
    stream->rec_buf_count = (stream->param.clock_rate *
        ((stream->param.channel_count * stream->param.bits_per_sample) / 8)) * ptime / 1000;

    // pj_size_t capacity = pj_pool_get_capacity(stream->pool);
    // pj_size_t used_size = pj_pool_get_used_size(stream->pool);
    // PJ_LOG(4, (THIS_FILE, "%s: capacity      %d!", __FUNCTION__, capacity));
    // PJ_LOG(4, (THIS_FILE, "%s: used_size     %d!", __FUNCTION__, used_size));

    // pj_int16_t      *rec_buf1;
    // // rec_buf1 = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->rec_buf_count);
    PJ_LOG(4, (THIS_FILE, "%s: rec_buf_count      %d!", __FUNCTION__, stream->rec_buf_count));
    // // stream->play_buf = (char *) pj_pool_alloc(stream->pool, stream->play_buf_count);
    // // stream->play_buf = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->play_buf_count);
    stream->rec_buf = (pj_int16_t *) pj_pool_alloc(stream->pool, stream->rec_buf_count);

    if (!stream->rec_buf) {
        PJ_LOG(4, (THIS_FILE, "%s: pj_pool_alloc failed!", __FUNCTION__));
        // pj_pool_release(stream->pool);
        return PJ_ENOMEM;
    }
    // stream->rec_buf_count = 0;

    // PJ_LOG(4, (THIS_FILE, "%s: rec_buf_count      %d!", __FUNCTION__, stream->rec_buf_count));
    PJ_LOG(4, (THIS_FILE, "%s: rec_buf            %d!", __FUNCTION__, stream->rec_buf));

    PJ_LOG(4, (THIS_FILE, "============================================================================================"));
    // PJ_LOG(4, (THIS_FILE, "hisi_audio_open_playback: ca_frames = %d clock = %d",
    //                    stream->ca_frames, param->clock_rate));

    return PJ_SUCCESS;
}

/* API: create stream */
static pj_status_t null_factory_create_stream(pjmedia_aud_dev_factory *f,
					      const pjmedia_aud_param *param,
					      pjmedia_aud_rec_cb rec_cb,
					      pjmedia_aud_play_cb play_cb,
					      void *user_data,
					      pjmedia_aud_stream **p_aud_strm)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_factory_create_stream", __FUNCTION__, __LINE__));
    struct hisi_audio_factory *nf = (struct hisi_audio_factory*)f;
    pj_pool_t *pool;
    pj_status_t status;
    struct hisi_audio_stream *strm;

    /* Create and Initialize stream descriptor */
//    pool = pj_pool_create(nf->pf, "null_audio-dev", 1000, 1000, NULL);
    pool = pj_pool_create (nf->pf, "HISI_AUDIO_STREAM%p", 1024, 1024, NULL);
    PJ_ASSERT_RETURN(pool != NULL, PJ_ENOMEM);

    strm = PJ_POOL_ZALLOC_T(pool, struct hisi_audio_stream);
    pj_memcpy(&strm->param, param, sizeof(*param));
    strm->pool = pool;
    strm->rec_cb = rec_cb;
    strm->play_cb = play_cb;
    strm->user_data = user_data;
    // ================================================================================================== 
    
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: param->clock_rate: %d", __FUNCTION__, __LINE__, param->clock_rate));
//    AIO_ATTR_S stAioAttr = HISI_COMM_GET_AIO_ATTR(48000);
    AIO_ATTR_S stAioAttr = HISI_COMM_GET_AIO_ATTR(param->clock_rate);
    if (stAioAttr.enSamplerate == 0) {
        return PJMEDIA_AUDIODEV_ERRNO_START;
    }
    pj_memcpy(&strm->stAioAttr, &stAioAttr, sizeof(stAioAttr));
    
    PJ_ASSERT_RETURN(param->play_id < (int)nf->dev_count, PJ_EINVAL);
    PJ_ASSERT_RETURN(param->rec_id < (int)nf->dev_count, PJ_EINVAL);
    
    HI_S32 s32Ret;
    HI_S32 s32AiChnCnt;
    HI_S32 s32AoChnCnt;
    
    /* config audio codec */
    s32Ret = HISI_COMM_AUDIO_CfgAcodec(&strm->stAioAttr, HI_TRUE);
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_CfgAcodec failed with %d!", __FUNCTION__, s32Ret));
        return s32Ret;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_CfgAcodec success with %d!", __FUNCTION__, s32Ret));
    }
    
    

    // ==================================================================================================

    /* Create player stream here */
    if (param->dir & PJMEDIA_DIR_PLAYBACK) {
        status = hisi_audio_open_playback(strm, nf);
        if (status != PJ_SUCCESS) {
            PJ_LOG(4, (THIS_FILE, "hisi_audio_open_playback status != PJ_SUCCESS"));
            pj_pool_release (pool);
            return status;
        }
    }

    /* Create capture stream here */
    if (param->dir & PJMEDIA_DIR_CAPTURE) {
        status = hisi_audio_open_capture(strm, nf);
        if (status != PJ_SUCCESS) {
            PJ_LOG(4, (THIS_FILE, "hisi_audio_open_capture != PJ_SUCCESS"));
            if (strm->param.dir & PJMEDIA_DIR_PLAYBACK) {
                // close_play_pcm(stream);
            }
            pj_pool_release (pool);
            return status;
        }
    }

    /* Apply the remaining settings */
    /* Below is an example if you want to set the output volume */
    if (param->flags & PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING) {
	null_stream_set_cap(&strm->base,
		            PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
		            &param->output_vol);
    }
    if (status != PJ_SUCCESS)
        return status;

    /* Done */
    strm->base.op = &stream_op;
    *p_aud_strm = &strm->base;

    return PJ_SUCCESS;
}

/* API: Get stream info. */
static pj_status_t null_stream_get_param(pjmedia_aud_stream *s,
					 pjmedia_aud_param *pi)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_get_param", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *strm = (struct hisi_audio_stream*)s;

    PJ_ASSERT_RETURN(strm && pi, PJ_EINVAL);

    pj_memcpy(pi, &strm->param, sizeof(*pi));

    /* Example: Update the volume setting */
    if (null_stream_get_cap(s, PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING,
			    &pi->output_vol) == PJ_SUCCESS)
    {
        pi->flags |= PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING;
    }

    return PJ_SUCCESS;
}

/* API: get capability */
static pj_status_t null_stream_get_cap(pjmedia_aud_stream *s,
				       pjmedia_aud_dev_cap cap,
				       void *pval)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_get_cap", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *strm = (struct hisi_audio_stream*)s;

    PJ_UNUSED_ARG(strm);

    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);

    /* Example: Get the output's volume setting */
    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING)
    {
	/* Output volume setting */
	*(unsigned*)pval = 0; // retrieve output device's volume here
	return PJ_SUCCESS;
    } else {
	return PJMEDIA_EAUD_INVCAP;
    }
}

/* API: set capability */
static pj_status_t null_stream_set_cap(pjmedia_aud_stream *s,
				       pjmedia_aud_dev_cap cap,
				       const void *pval)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_set_cap", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *strm = (struct hisi_audio_stream*)s;

    PJ_UNUSED_ARG(strm);

    PJ_ASSERT_RETURN(s && pval, PJ_EINVAL);

    /* Example */
    if (cap==PJMEDIA_AUD_DEV_CAP_OUTPUT_VOLUME_SETTING)
    {
	/* Output volume setting */
	// set output's volume level here
	return PJ_SUCCESS;
    }

    return PJMEDIA_EAUD_INVCAP;
}

static int ca_thread_func (void *arg) {
    struct hisi_audio_stream* stream = (struct hisi_audio_stream *) arg;
    int size                   = stream->rec_buf_count;
    unsigned long nframes      = stream->ca_frames;
    void *user_data            = stream->user_data;

//    stream->AiChn = stream->AiChn + 1;
    
    /* Buffer to fill for PJMEDIA */
    pj_int16_t *buf              = stream->rec_buf;
    pj_timestamp tstamp;
    int result = 0;
    int policy;
    struct sched_param param;
    struct timeval TimeoutVal;

    fd_set read_fds;
    HI_S32 s32Ret;

#if defined(PJMEDIA_HAS_PASSTHROUGH_CODECS) && PJMEDIA_HAS_PASSTHROUGH_CODECS!=0

    AUDIO_STREAM_S stStream;

    FD_ZERO(&read_fds);
    HI_S32 AencFd = HI_MPI_AENC_GetFd(stream->AeChn);
    FD_SET(AencFd, &read_fds);

#else

    HI_S32 AiFd;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S   stAecFrm;
    // fd_set read_fds;
    AI_CHN_PARAM_S stAiChnPara;
    s32Ret = HI_MPI_AI_GetChnParam(stream->AiDev, stream->AiChn, &stAiChnPara);
    if (HI_SUCCESS != s32Ret) {
        PJ_LOG(4, (THIS_FILE, "%s: Get ai chn param failed", __FUNCTION__));
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: Get ai chn param success", __FUNCTION__));
    }

//    stAiChnPara.u32UsrFrmDepth = 10;
    stAiChnPara.u32UsrFrmDepth = 30;

    s32Ret = HI_MPI_AI_SetChnParam(stream->AiDev, stream->AiChn, &stAiChnPara);
    if (HI_SUCCESS != s32Ret) {
        // printf("%s: set ai chn param failed\n", __FUNCTION__);
        PJ_LOG(4, (THIS_FILE, "%s: set ai chn param failed", __FUNCTION__));
        // return NULL;
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: set ai chn param success", __FUNCTION__));
    }

    FD_ZERO(&read_fds);
    AiFd = HI_MPI_AI_GetFd(stream->AiDev, stream->AiChn);
    FD_SET(AiFd, &read_fds);

#endif  /* PJMEDIA_HAS_PASSTHROUGH_CODECS */

    PJ_LOG(4, (THIS_FILE, "ca_thread_func: size = %d ", size));

    if (pthread_getschedparam(pthread_self(), &policy, &param) == 0) {
        param.sched_priority = 18;
        pthread_setschedparam (pthread_self(), policy, &param);
    }

    pj_bzero (buf, size);
    tstamp.u64 = 0;

    // unsigned aps_g711_frame_len = stream->play_buf_count;
    // pj_uint8_t aps_g711_frame_len = stream->play_buf_count;
    // pj_uint8_t aps_g711_frame_len = 0;
    unsigned aps_g711_frame_len = 0;

    PJ_LOG(4, (THIS_FILE, "stream->play_buf_count = %u", stream->play_buf_count));
    PJ_LOG(4, (THIS_FILE, "aps_g711_frame_len = %u", aps_g711_frame_len));
    PJ_LOG(4, (THIS_FILE, "aps_g711_frame_len = %d", aps_g711_frame_len));
    PJ_LOG(4, (THIS_FILE, "ca_thread_func: start loop"));
    while (!stream->quit) {

        TimeoutVal.tv_sec = 1;
        TimeoutVal.tv_usec = 0;

#if defined(PJMEDIA_HAS_PASSTHROUGH_CODECS) && PJMEDIA_HAS_PASSTHROUGH_CODECS!=0

        FD_ZERO(&read_fds);
        FD_SET(AencFd, &read_fds);

        s32Ret = select(AencFd+1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0)  {
            break;
        } else if (0 == s32Ret) {
            PJ_LOG(4, (THIS_FILE, "%s: get aenc stream select time out", __FUNCTION__));
            break;
        }
        if (FD_ISSET(AencFd, &read_fds)) {
            /* get stream from aenc chn */
            s32Ret = HI_MPI_AENC_GetStream(stream->AeChn, &stStream, HI_FALSE);
            if (HI_SUCCESS != s32Ret ) {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AENC_GetStream(%d), failed with %#x!", __FUNCTION__, stream->AeChn, s32Ret));
                stream->quit = 1;
                // return NULL;
            }

            pjmedia_frame_ext frame_ext;
            pjmedia_frame frame;
            // pjmedia_frame frame;
            memset(&frame_ext, 0, sizeof(pjmedia_frame_ext));
            memset(&frame, 0, sizeof(pjmedia_frame));

            HI_U32 u32Len = stStream.u32Len;
            // unsigned char *tmp_frm = (unsigned char *)calloc(stream->play_buf_count, sizeof(unsigned char));
            unsigned char *tmp_frm = (unsigned char *)calloc(u32Len, sizeof(unsigned char));
            if(!tmp_frm) {
                PJ_LOG(4, (THIS_FILE, "%s: Allocation tmp_frm failure.", __FUNCTION__));
                exit (1);
            }

            frame.buf = (unsigned char *)calloc(u32Len, sizeof(unsigned char));
            if(!frame.buf) {
                PJ_LOG(4, (THIS_FILE, "%s: Allocation frame.buf failure.", __FUNCTION__));
                exit (1);
            }
            memcpy(tmp_frm, stStream.pStream+4, u32Len-4);

            PJ_LOG(4, (THIS_FILE, "%s: stStream.u32Len = %d", __FUNCTION__, u32Len));

            for (int i = 0; i < u32Len; i++) {
                printf("%02X ", stStream.pStream[i] & 0xff);
            }
            printf("\n");
            for (int i = 0; i < u32Len; i++) {
                printf("%02X ", tmp_frm[i] & 0xff);
            }
            printf("\n");
            // =========================================================
            unsigned samples_processed = 0;

            /* Make sure it is normal frame. */
            pj_assert(stStream.pStream[0] == 0 && stStream.pStream[1] == 1);

            /* Detect the recorder G.711 frame size, player frame size will
             * follow this recorder frame size.
             */
            if (aps_g711_frame_len == 0) {
            // if (aps_g711_frame_len == stream->play_buf_count) {
                // aps_g711_frame_len = u32Len;
                aps_g711_frame_len = stStream.u32Len-4 < 160? 80 : 160;
                // aps_g711_frame_len = (stStream.u32Len-4) < 160? 80 : 160;
                // aps_g711_frame_len = stStream.u32Len < 320? stStream.u32Len : 320;
                // TRACE_((THIS_FILE, "Detected APS G.711 frame size = %u samples",
                //     aps_g711_frame_len));
                PJ_LOG(4, (THIS_FILE, "%s: Detected APS G.711 frame size = %u samples", __FUNCTION__, aps_g711_frame_len));

            }

            /* Convert APS buffer format into pjmedia_frame_ext. Whenever
             * samples count in the frame is equal to stream's samples per
             * frame, call parent stream callback.
             */
            PJ_LOG(4, (THIS_FILE, "%s: stStream.u32Len < %u = %u", __FUNCTION__, stream->play_buf_count, aps_g711_frame_len));
            // while (samples_processed < aps_g711_frame_len) {
            //     unsigned tmp;
            //     const pj_uint8_t *pb = (const pj_uint8_t*)tmp_frm + samples_processed;
            //     // const pj_uint8_t *pb = (const pj_uint8_t*)stStream.pStream + 4 + samples_processed;

            //     tmp = PJ_MIN(stream->param.samples_per_frame - frame.samples_cnt,
            //              aps_g711_frame_len - samples_processed);
            //     PJ_LOG(4, (THIS_FILE, "%s: stream->param.samples_per_frame - frame.samples_cnt = %u", __FUNCTION__, stream->param.samples_per_frame - frame.samples_cnt));
            //     PJ_LOG(4, (THIS_FILE, "%s: aps_g711_frame_len - samples_processed = %u", __FUNCTION__, aps_g711_frame_len - samples_processed));
            //     PJ_LOG(4, (THIS_FILE, "%s: tmp = %u, tmp << 3 = %u", __FUNCTION__, tmp, tmp << 3));

            //     pjmedia_frame_ext_append_subframe(&frame, pb, tmp << 3, tmp);
            //     samples_processed += tmp;
            //     PJ_LOG(4, (THIS_FILE, "%s: samples_processed = %u", __FUNCTION__, samples_processed));
            //     PJ_LOG(4, (THIS_FILE, "%s: frame.samples_cnt = %u, %u, %u", __FUNCTION__,
            //         frame.samples_cnt, stream->param.samples_per_frame, stream->param.samples_per_frame << 1));

            //     if (frame.samples_cnt == stream->param.samples_per_frame) {
            //         frame.base.type = PJMEDIA_FRAME_TYPE_EXTENDED;
            //         frame.base.size = stream->param.samples_per_frame << 1;
            //         // frame.base.size = stStream.u32Len-4;
            //         stream->rec_cb(stream->user_data, (pjmedia_frame*)&frame);
            //         frame.samples_cnt = 0;
            //         frame.subframe_cnt = 0;
            //     }
            // }

            // while (samples_processed < aps_g711_frame_len) {
            //     unsigned tmp;
            //     const pj_uint8_t *pb = (const pj_uint8_t*)tmp_frm + samples_processed;
            //     // const pj_uint8_t *pb = (const pj_uint8_t*)stStream.pStream + 4 + samples_processed;

            //     tmp = PJ_MIN(stream->param.samples_per_frame - frame.samples_cnt,
            //              aps_g711_frame_len - samples_processed);
            //     PJ_LOG(4, (THIS_FILE, "%s: stream->param.samples_per_frame - frame.samples_cnt = %u", __FUNCTION__, stream->param.samples_per_frame - frame.samples_cnt));
            //     PJ_LOG(4, (THIS_FILE, "%s: aps_g711_frame_len - samples_processed = %u", __FUNCTION__, aps_g711_frame_len - samples_processed));
            //     PJ_LOG(4, (THIS_FILE, "%s: tmp = %u, tmp << 3 = %u", __FUNCTION__, tmp, tmp << 3));

            //     pjmedia_frame_ext_append_subframe(&frame, pb, tmp << 3, tmp);
            //     samples_processed += tmp;
            //     PJ_LOG(4, (THIS_FILE, "%s: samples_processed = %u", __FUNCTION__, samples_processed));
            //     PJ_LOG(4, (THIS_FILE, "%s: frame.samples_cnt = %u, %u, %u", __FUNCTION__,
            //         frame.samples_cnt, stream->param.samples_per_frame, stream->param.samples_per_frame << 1));

            //     if (frame.samples_cnt == stream->param.samples_per_frame) {
            //         frame.base.type = PJMEDIA_FRAME_TYPE_EXTENDED;
            //         frame.base.size = stream->param.samples_per_frame << 1;
            //         // frame.base.size = stStream.u32Len-4;
            //         stream->rec_cb(stream->user_data, (pjmedia_frame*)&frame);
            //         frame.samples_cnt = 0;
            //         frame.subframe_cnt = 0;
            //     }
            // }

            frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
            // frame.size = u32Len;
            frame.size = stream->param.samples_per_frame << 1;
            // frame.buf = tmp_frm;
            memcpy(frame.buf, tmp_frm, u32Len);
            frame.timestamp.u64 = stStream.u64TimeStamp;
            frame.bit_info = 0;

            stream->rec_cb(stream->user_data, (pjmedia_frame*)&frame);
            PJ_LOG(4, (THIS_FILE, "%s: done sending", __FUNCTION__));
            // =========================================================

            // frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
            // frame.base.type = PJMEDIA_FRAME_TYPE_EXTENDED;
            // frame.base.size = stStream.u32Len;
            // frame.base.buf = stStream.pStream;
            // frame.base.timestamp.u64 = stStream.u64TimeStamp;
            // frame.base.bit_info = 0;

            free(tmp_frm);
            free(frame.buf);
            PJ_LOG(4, (THIS_FILE, "%s: free(tmp_frm);", __FUNCTION__));
            tmp_frm = NULL;
            frame.buf = NULL;
            PJ_LOG(4, (THIS_FILE, "%s: tmp_frm = NULL;", __FUNCTION__));

            s32Ret = HI_MPI_AENC_ReleaseStream(stream->AeChn, &stStream);
            if (HI_SUCCESS != s32Ret ) {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AENC_ReleaseStream(%d), failed with %#x!", __FUNCTION__, stream->AeChn, s32Ret));
                stream->quit = 1;
            }
            PJ_LOG(4, (THIS_FILE, "%s: done HI_MPI_AENC_ReleaseStream", __FUNCTION__));
            if (stream->quit == 1)
                break;
            // result = stream->rec_cb (user_data, &frame);
            // if (result != PJ_SUCCESS || stream->quit)
                // break;

            tstamp.u64 += nframes;
        }
#else
#if 1
        FD_ZERO(&read_fds);
        FD_SET(AiFd, &read_fds);

        s32Ret = select(AiFd+1, &read_fds, NULL, NULL, &TimeoutVal);
        // PJ_LOG(4, (THIS_FILE, "%s: select: %d", __FUNCTION__, s32Ret));
        if (s32Ret < 0) {
            break;
        } else if (0 == s32Ret) {
            PJ_LOG(4, (THIS_FILE, "%s: get ai frame select time out!", __FUNCTION__));
            break;
        }

        if (FD_ISSET(AiFd, &read_fds)) {
            /* get frame from ai chn */
            memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));
            memset(&stFrame, 0, sizeof(AUDIO_FRAME_S));
            
            // s32Ret = HI_MPI_AI_GetFrame(0, 0, &stFrame, NULL, HI_FALSE);
            // s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, HI_FALSE);
            // s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, HISI_MSECOND_PER_BUFFER);
            // s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, 0);
//            s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, NULL, HI_TRUE);
            s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, NULL, HI_FALSE);
//            PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_GetFrame(%d, %d), size = %d",\
//                       __FUNCTION__, stream->AiDev, stream->AiChn, stFrame.u32Len));
//            s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, HI_FALSE);
//            s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, HI_TRUE);
//            s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, stream->param.input_latency_ms);
            // s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, HISI_MSECOND_PER_BUFFER);
            // s32Ret = HI_MPI_AI_GetFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm, -1);
            if (HI_SUCCESS != s32Ret ) {
                #if 0
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!",\
                       __FUNCTION__, stream->AiDev, stream->AiChn, s32Ret));
                stream->quit = 1;
                break;
                #else
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!",\
                       __FUNCTION__, stream->AiDev, stream->AiChn, s32Ret));
                continue;
                #endif
            }
            
//            unsigned char *tmpbuf = (unsigned char *)calloc(stFrame.u32Len, sizeof(unsigned char));
//            if(!tmpbuf) {
//                PJ_LOG(4, (THIS_FILE, "%s: Allocation frame.buf failure.", __FUNCTION__));
//                exit (1);
//            }
//            memcpy(tmpbuf, stFrame.pVirAddr[0], stFrame.u32Len);

            
//            PJ_LOG(4, (THIS_FILE, "%s: stFrame.u32Len = %d", __FUNCTION__, stFrame.u32Len));
//            log_bytes(stFrame.pVirAddr[0], stFrame.u32Len);
//            log_bytes(stFrame.pVirAddr[0], stFrame.u32Len);
//            log_bytes(tmpbuf, stFrame.u32Len);
            pjmedia_frame frame;
            memset(&frame, 0, sizeof(pjmedia_frame));
            
            
            /* Write the capture audio data to PJMEDIA */
            frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
            // frame.buf = (void *) &stFrame.u32Seq;

//            frame.buf = tmpbuf;
//            frame.buf = stFrame.pVirAddr[0];
            // frame.buf = (void *) stFrame.pVirAddr[0];
//            frame.size = stFrame.u32Len;
//            PJ_LOG(4, (THIS_FILE, "%s: stream->param.samples_per_frame = %d", __FUNCTION__, stream->param.samples_per_frame));
//            PJ_LOG(4, (THIS_FILE, "%s: stream->param.bits_per_sample = %d", __FUNCTION__, stream->param.bits_per_sample));
            frame.size = stream->param.samples_per_frame * stream->param.bits_per_sample / 8;
//            frame.buf = (unsigned char *)calloc(stFrame.u32Len, sizeof(unsigned char));
            frame.buf = (unsigned char *)calloc(frame.size, sizeof(unsigned char));
            if(!frame.buf) {
                PJ_LOG(4, (THIS_FILE, "%s: Allocation frame.buf failure.", __FUNCTION__));
                exit (1);
            }
            memcpy(frame.buf, stFrame.pVirAddr[0], frame.size);
//            memcpy(frame.buf, stFrame.pVirAddr[0], stFrame.u32Len);
//            frame.size = stFrame.u32Len/2;
            frame.timestamp.u64 = tstamp.u64;
            frame.bit_info = 0;

            /* finally you must release the stream */
//            s32Ret = HI_MPI_AI_ReleaseFrame(stream->AiDev, stream->AiChn, &stFrame, &stAecFrm);
            s32Ret = HI_MPI_AI_ReleaseFrame(stream->AiDev, stream->AiChn, &stFrame, NULL);
            if (HI_SUCCESS != s32Ret ) {
                PJ_LOG(4, (THIS_FILE, "%s: HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!\n", __FUNCTION__, stream->AiDev, stream->AiChn, s32Ret));
                stream->quit = 1;
            }
            
//            free(tmpbuf);
//            tmpbuf = NULL;
            
//            usleep(1);
            if (stream->quit)
                break;
            result = stream->rec_cb (user_data, &frame);
            
            free(frame.buf);
            frame.buf = NULL;
            if (result != PJ_SUCCESS || stream->quit)
                break;

            tstamp.u64 += nframes;
        }
#endif

#endif  /* PJMEDIA_HAS_PASSTHROUGH_CODECS */

    }
    stream->quit = 1;
    stream->rec_thread_exited = 1;
    PJ_LOG(4, (THIS_FILE, "ca_thread_func: Stopped"));
    return PJ_SUCCESS;
}

/**
 * Play audio received from PJMEDIA
 */
static int pb_thread_func (void *arg) {

    struct hisi_audio_stream* stream = (struct hisi_audio_stream *) arg;
    int size                    = stream->play_buf_count;
    unsigned long nframes   = stream->pb_frames;
    void *user_data             = stream->user_data;
    // char *buf               = stream->pb_buf;
    pj_int16_t *buf               = stream->play_buf;
    pj_timestamp tstamp;
    int result = 0;
    int policy;
    struct sched_param param;

    PJ_LOG(4, (THIS_FILE, "pb_thread_func: size = %d ", size));
    if (pthread_getschedparam(pthread_self(), &policy, &param) == 0) {
        param.sched_priority = 18;
        pthread_setschedparam (pthread_self(), policy, &param);
    }

    pj_bzero (buf, size);
    tstamp.u64 = 0;

    HI_S32 s32Ret;
    // HI_S32 s32AdecChn;

    // s32AdecChn = 0;
    int i = 0;

    pj_timestamp now;
    pj_timestamp    last_called;
    // pj_math_stat    delay;
    unsigned int    delay;

    pj_get_timestamp(&now);
    pj_get_timestamp(&last_called);
    // pj_math_stat_init(delay);

    PJ_LOG(4, (THIS_FILE, "pb_thread_func: start loop"));
    while (!stream->quit) {
        pjmedia_frame frame;
        memset(&frame, 0, sizeof(pjmedia_frame));

        frame.type = PJMEDIA_FRAME_TYPE_AUDIO;
        /* pointer to buffer filled by PJMEDIA */
//        frame.buf = buf;
//        frame.size = 320;
//        PJ_LOG(4, (THIS_FILE, "%s: stream->param.samples_per_frame = %d", __FUNCTION__, stream->param.samples_per_frame));
//        PJ_LOG(4, (THIS_FILE, "%s: stream->param.bits_per_sample = %d", __FUNCTION__, stream->param.bits_per_sample));
        frame.size = stream->param.samples_per_frame * stream->param.bits_per_sample / 8;
        frame.buf = (unsigned char *)calloc(frame.size, sizeof(unsigned char));
        if(!frame.buf) {
            PJ_LOG(4, (THIS_FILE, "%s: Allocation frame.buf failure.", __FUNCTION__));
            exit (1);
        }
//        frame.size = size;
        frame.timestamp.u64 = tstamp.u64;
        frame.bit_info = 0;

        result = stream->play_cb (user_data, &frame);
        if (result != PJ_SUCCESS || stream->quit)
            break;

        if (frame.type != PJMEDIA_FRAME_TYPE_AUDIO) {
            pj_bzero (frame.buf, frame.size);
            pj_bzero (buf, size);
            // continue;
            break;
        }
        

#if !defined(PJMEDIA_HAS_PASSTHROUGH_CODECS) && PJMEDIA_HAS_PASSTHROUGH_CODECS!=0

        AUDIO_STREAM_S stAudioStream;
        memset(&stAudioStream, 0, sizeof(AUDIO_STREAM_S));
        stAudioStream.pStream      = frame.buf;
        // stAudioStream.u32PhyAddr   = ;
        stAudioStream.u32Len       = frame.size;
        stAudioStream.u64TimeStamp = frame.timestamp.u64;
        stAudioStream.u32Seq       = frame.buf;

        PJ_LOG(4, (THIS_FILE, "frame.size = %u", frame.size));
        // log_bytes(frame.buf, frame.size);
        // typedef struct hiAUDIO_STREAM_S
        // {
        //     HI_U8 *pStream;         /* the virtual address of stream */
        //     HI_U32 u32PhyAddr;      /* the physics address of stream */
        //     HI_U32 u32Len;          /* stream lenth, by bytes */
        //     HI_U64 u64TimeStamp;    /* frame time stamp*/
        //     HI_U32 u32Seq;          /* frame seq,if stream is not a valid frame,u32Seq is 0*/
        // } AUDIO_STREAM_S;

        /* send audio stream to adec chn */
        // s32Ret = HI_MPI_ADEC_SendStream(stream->AdChn, &stAudioStream, HI_TRUE);
        s32Ret = HI_MPI_ADEC_SendStream(stream->AdChn, &stAudioStream, HI_FALSE);
        if (HI_SUCCESS != s32Ret ) {
            PJ_LOG(4,(THIS_FILE,"%s: HI_MPI_ADEC_SendStream(%d), failed with %#x!", __FUNCTION__, stream->AdChn, s32Ret));
            stream->quit = 1;
        }

#else
#if 1
        AUDIO_FRAME_S stFrame;
        memset(&stFrame, 0, sizeof(AUDIO_FRAME_S));

        // stFrame.enBitwidth = AUDIO_BIT_WIDTH_16;
        stFrame.enBitwidth = stream->stAioAttr.enBitwidth;
        // stFrame.enSoundmode = AUDIO_SOUND_MODE_MONO;
        stFrame.enSoundmode = stream->stAioAttr.enSoundmode;
        stFrame.u64TimeStamp = frame.timestamp.u64;
        stFrame.u32Seq = (HI_U32)frame.buf;
        stFrame.u32Len = frame.size;
        stFrame.pVirAddr[0] = frame.buf;

        /* send frame to ao */
        // s32Ret = HI_MPI_AO_SendFrame(0, 0, &stFrame, 1000);
        // s32Ret = HI_MPI_AO_SendFrame(0, 0, &stFrame, HISI_MSECOND_PER_BUFFER);
//        s32Ret = HI_MPI_AO_SendFrame(stream->AoDev, stream->AoChn, &stFrame, HI_FALSE);
//        PJ_LOG(4, (THIS_FILE, "HI_MPI_AO_SendFrame: frame.size = %d ", frame.size));
//        PJ_LOG(4, (THIS_FILE, "HI_MPI_AO_SendFrame: frame.type = %d ", frame.type));
//        s32Ret = HI_MPI_AO_SendFrame(stream->AoDev, stream->AoChn, &stFrame, HI_FALSE);
        s32Ret = HI_MPI_AO_SendFrame(stream->AoDev, stream->AoChn, &stFrame, HI_TRUE);
//        s32Ret = HI_MPI_AO_SendFrame(0, 0, &stFrame, stream->param.output_latency_ms);
        // s32Ret = HI_MPI_AO_SendFrame(0, 0, &stFrame, -1);
        // s32Ret = HI_MPI_AO_SendFrame(0, 0, &stFrame, 0);
        if (HI_SUCCESS != s32Ret ) {
            PJ_LOG(4,(THIS_FILE,"%s: HI_MPI_AO_SendFrame(%d, %d), failed with %#x!", __FUNCTION__, 0, 0, s32Ret));
            stream->quit = 1;
        }
#endif
#endif  /* PJMEDIA_HAS_PASSTHROUGH_CODECS */


        free(frame.buf);
        frame.size = 0;
//        frame.size = NULL;
        tstamp.u64 += nframes;

    }

    stream->quit = 1;
    stream->play_thread_exited = 1;
    PJ_LOG(4, (THIS_FILE, "pb_thread_func: Stopped"));

    return PJ_SUCCESS;
}

/* API: Start stream. */
static pj_status_t null_stream_start(pjmedia_aud_stream *strm)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_start", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *stream = (struct hisi_audio_stream*)strm;

    PJ_ASSERT_RETURN(stream != NULL, PJ_EINVAL);
    
    pj_status_t status = PJ_SUCCESS;
    
    stream->quit = 0;
    if (stream->param.dir & PJMEDIA_DIR_PLAYBACK) {
        PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_PLAYBACK OK"));
        status = pj_thread_create (stream->pool,
                   "hisisound_playback",
                   pb_thread_func,
                   stream,
                   0,
                   0,
                   &stream->play_thread);
        if (status != PJ_SUCCESS) {
            PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_PLAYBACK pj_thread_create status == PJ_ERROR"));
            return status;
        } else {
            PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_PLAYBACK pj_thread_create status == PJ_SUCCESS"));
        }
    }
    
    if (stream->param.dir & PJMEDIA_DIR_CAPTURE) {
        PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_CAPTURE OK"));
        status = pj_thread_create (stream->pool,
                   "hisisound_capture",
                   ca_thread_func,
                   stream,
                   0,
                   0,
                   &stream->rec_thread);
        if (status != PJ_SUCCESS) {
            PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_CAPTURE pj_thread_create status == PJ_ERROR"));
            stream->quit = PJ_TRUE;
            pj_thread_join(stream->play_thread);
            pj_thread_destroy(stream->play_thread);
            stream->play_thread = NULL;
        } else {
            PJ_LOG(4, (THIS_FILE, "PJMEDIA_DIR_CAPTURE pj_thread_create status == PJ_SUCCESS"));
        }
    }
    
//    PJ_UNUSED_ARG(stream);

    PJ_LOG(4, (THIS_FILE, "[%s]-%d: Starting null audio stream", __FUNCTION__, __LINE__));

//    return PJ_SUCCESS;
    return status;
}

/* API: Stop stream. */
static pj_status_t null_stream_stop(pjmedia_aud_stream *strm)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_stop", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *stream = (struct hisi_audio_stream*)strm;

    PJ_ASSERT_RETURN(strm != NULL, PJ_EINVAL);

    pj_status_t status = PJ_SUCCESS;
    int i, err = 0;

    // strm->go = 0;
    stream->quit = 1;

    for (i=0; !stream->rec_thread_exited && i<100; ++i)
        // pj_thread_sleep(10);
        pj_thread_sleep(1);
    for (i=0; !stream->play_thread_exited && i<100; ++i)
        // pj_thread_sleep(10);
        pj_thread_sleep(1);

    pj_thread_sleep(1);

    PJ_LOG(5,(THIS_FILE, "Stopping stream.."));

    stream->play_thread_initialized = 0;
    stream->rec_thread_initialized = 0;

    PJ_LOG(5,(THIS_FILE, "Done, status=%d", err));

    return status;
//    PJ_UNUSED_ARG(stream);

    PJ_LOG(4, (THIS_FILE, "[%s]-%d: Stopping null audio stream", __FUNCTION__, __LINE__));

//    return PJ_SUCCESS;
}

/* API: Destroy stream. */
static pj_status_t null_stream_destroy(pjmedia_aud_stream *strm)
{
    PJ_LOG(4, (THIS_FILE, "[%s]-%d: null_stream_destroy", __FUNCTION__, __LINE__));
    struct hisi_audio_stream *stream = (struct hisi_audio_stream*)strm;

    PJ_ASSERT_RETURN(stream != NULL, PJ_EINVAL);

    HI_S32 s32Ret;
    
    
//    null_stream_stop(strm);
    
    s32Ret = HISI_COMM_AUDIO_StopAo(stream->AoDev, stream->AoChn, HI_FALSE);
//    s32Ret = HISI_COMM_AUDIO_StopAo(strm->AoDev, strm->s32AoChnCnt, gs_bAoReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StopAo failed with %d!", __FUNCTION__, s32Ret));
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StopAo success with %d!", __FUNCTION__, s32Ret));
    }
    
    s32Ret = HISI_COMM_AUDIO_StopAi(stream->AiDev, stream->s32AiChnCnt, gs_bAiReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS) {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StopAi failed with %d!", __FUNCTION__, s32Ret));
    } else {
        PJ_LOG(4, (THIS_FILE, "%s: HISI_COMM_AUDIO_StopAi success with %d!", __FUNCTION__, s32Ret));
    }

    PJ_LOG(5,(THIS_FILE, "Destroying stream.."));

    pj_pool_release(stream->pool);

    return PJ_SUCCESS;
}



#endif	/* PJMEDIA_AUDIO_DEV_HAS_HISILICON_AUDIO */
