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
    
static pthread_t gs_VencPid;
static HISI_VENC_GETSTREAM_PARA_S gs_stPara;
    
/******************************************************************************
* funciton : Start venc stream mode (h264, mjpeg)
* note      : rate control parameter need adjust, according your case.
******************************************************************************/
HI_S32 HISI_COMM_VENC_Start(VENC_GRP VencGrp,VENC_CHN VencChn, PAYLOAD_TYPE_E enType, VIDEO_NORM_E enNorm, PIC_SIZE_E enSize, HISI_RC_E enRcMode)
{
    HI_S32 s32Ret;
    VENC_CHN_ATTR_S stVencChnAttr;
    VENC_ATTR_H264_S stH264Attr;
    VENC_ATTR_H264_CBR_S    stH264Cbr;
    VENC_ATTR_H264_VBR_S    stH264Vbr;
    VENC_ATTR_H264_FIXQP_S  stH264FixQp;
    VENC_ATTR_MJPEG_S stMjpegAttr;
    VENC_ATTR_MJPEG_FIXQP_S stMjpegeFixQp;
    VENC_ATTR_JPEG_S stJpegAttr;
    SIZE_S stPicSize;

    s32Ret = HISI_COMM_SYS_GetPicSize(enNorm, enSize, &stPicSize);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("Get picture size failed!\n");
        return HI_FAILURE;
    } else {
        HISI_PRT("HISI_COMM_SYS_GetPicSize SUCCESS with %#x!\n", s32Ret);
    }
    /******************************************
     step 1: Greate Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_CreateGroup(VencGrp);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_CreateGroup[%d] failed with %#x!\n",\
                 VencGrp, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_CreateGroup[%d] SUCCESS with %#x!\n",\
                 VencGrp, s32Ret);
    }

    /******************************************
     step 2:  Create Venc Channel
    ******************************************/
    stVencChnAttr.stVeAttr.enType = enType;
    switch(enType) {
        case PT_H264:
        {
            stH264Attr.u32MaxPicWidth = stPicSize.u32Width;
            stH264Attr.u32MaxPicHeight = stPicSize.u32Height;
            stH264Attr.u32PicWidth = stPicSize.u32Width;/*the picture width*/
            stH264Attr.u32PicHeight = stPicSize.u32Height;/*the picture height*/
            stH264Attr.u32BufSize  = stPicSize.u32Width * stPicSize.u32Height * 2;/*stream buffer size*/
            stH264Attr.u32Profile  = 1;/*0: baseline; 1:MP; 2:HP   ? */
            stH264Attr.bByFrame = HI_TRUE;/*get stream mode is slice mode or frame mode?*/
            stH264Attr.bField = HI_FALSE;  /* surpport frame code only for hi3516, bfield = HI_FALSE */
            stH264Attr.bMainStream = HI_TRUE; /* surpport main stream only for hi3516, bMainStream = HI_TRUE */
            stH264Attr.u32Priority = 0; /*channels precedence level. invalidate for hi3516*/
            stH264Attr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame. Invalidate for hi3516*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrH264e, &stH264Attr, sizeof(VENC_ATTR_H264_S));

            if(HISI_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBRv2;
                stH264Cbr.u32Gop            = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Cbr.u32StatTime       = 1; /* stream rate statics time(s) */
                stH264Cbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* input (vi) frame rate */
                stH264Cbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;/* target frame rate */
                switch (enSize)
                {
                    case PIC_QCIF:
                       stH264Cbr.u32BitRate = 128; /* average bit rate */
                       break;                       
                    case PIC_QVGA:    /* 320 * 240 */
                    case PIC_CIF:	
                       stH264Cbr.u32BitRate = 256;
                       break;
                    case PIC_D1:
                    case PIC_VGA:	   /* 640 * 480 */
                       stH264Cbr.u32BitRate = 1024;
                       break;
                    case PIC_HD720:   /* 1280 * 720 */
                       stH264Cbr.u32BitRate = 1024*2;
                       break;
                    case PIC_HD1080:  /* 1920 * 1080 */
                    	   stH264Cbr.u32BitRate = 1024*4;
                       break;
                    default :
                       stH264Cbr.u32BitRate = 1024*8;
                       break;
                }
                
                stH264Cbr.u32FluctuateLevel = 0; /* average bit rate */
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Cbr, &stH264Cbr, sizeof(VENC_ATTR_H264_CBR_S));
            }
            else if (HISI_RC_FIXQP == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264FIXQP;
                stH264FixQp.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264FixQp.u32IQp = 20;
                stH264FixQp.u32PQp = 23;
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264FixQp, &stH264FixQp,sizeof(VENC_ATTR_H264_FIXQP_S));
            }
            else if (HISI_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264VBRv2;
                stH264Vbr.u32Gop = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32StatTime = 1;
                stH264Vbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stH264Vbr.u32MinQp = 24;
                stH264Vbr.u32MaxQp = 45;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stH264Vbr.u32MaxBitRate= 128; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stH264Vbr.u32MaxBitRate = 256;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stH264Vbr.u32MaxBitRate = 1024;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stH264Vbr.u32MaxBitRate = 1024*2;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stH264Vbr.u32MaxBitRate = 1024*4;
                	   break;
                  default :
                       stH264Vbr.u32MaxBitRate = 1024*8;
                       break;
                }
                memcpy(&stVencChnAttr.stRcAttr.stAttrH264Vbr, &stH264Vbr, sizeof(VENC_ATTR_H264_VBR_S));
            }
            else
            {
                return HI_FAILURE;
            }
        }
        break;
        
        case PT_MJPEG:
        {
            stMjpegAttr.u32MaxPicWidth = stPicSize.u32Width;
            stMjpegAttr.u32MaxPicHeight = stPicSize.u32Height;
            stMjpegAttr.u32PicWidth = stPicSize.u32Width;
            stMjpegAttr.u32PicHeight = stPicSize.u32Height;
            stMjpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stMjpegAttr.bByFrame = HI_TRUE;  /*get stream mode is field mode  or frame mode*/
            stMjpegAttr.bMainStream = HI_TRUE;  /*main stream or minor stream types?*/
            stMjpegAttr.bVIField = HI_FALSE;  /*the sign of the VI picture is field or frame?*/
            stMjpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));

            if(HISI_RC_FIXQP == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGFIXQP;
                stMjpegeFixQp.u32Qfactor        = 90;
                stMjpegeFixQp.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stMjpegeFixQp.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                memcpy(&stVencChnAttr.stRcAttr.stAttrMjpegeFixQp, &stMjpegeFixQp,
                       sizeof(VENC_ATTR_MJPEG_FIXQP_S));
            }
            else if (HISI_RC_CBR == enRcMode)
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32StatTime       = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32ViFrmRate      = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.fr32TargetFrmRate = (VIDEO_ENCODING_MODE_PAL== enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32FluctuateLevel = 0;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 384*3; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 768*3;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*3*3;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*5*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*10*3;
                	   break;
                  default :
                       stVencChnAttr.stRcAttr.stAttrMjpegeCbr.u32BitRate = 1024*7*3;
                       break;
                }
            }
            else if (HISI_RC_VBR == enRcMode) 
            {
                stVencChnAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGVBR;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32StatTime = 1;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32ViFrmRate = (VIDEO_ENCODING_MODE_PAL == enNorm)?25:30;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.fr32TargetFrmRate = 5;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MinQfactor = 50;
                stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxQfactor = 95;
                switch (enSize)
                {
                  case PIC_QCIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate= 256*3; /* average bit rate */
                	   break;
                  case PIC_QVGA:    /* 320 * 240 */
                  case PIC_CIF:
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 512*3;
                       break;
                  case PIC_D1:
                  case PIC_VGA:	   /* 640 * 480 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*2*3;
                       break;
                  case PIC_HD720:   /* 1280 * 720 */
                	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*3*3;
                	   break;
                  case PIC_HD1080:  /* 1920 * 1080 */
                  	   stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*6*3;
                	   break;
                  default :
                       stVencChnAttr.stRcAttr.stAttrMjpegeVbr.u32MaxBitRate = 1024*4*3;
                       break;
                }
            } else {
                HISI_PRT("cann't support other mode in this version!\n");

                return HI_FAILURE;
            }
        }
        break;
            
        case PT_JPEG:
            stJpegAttr.u32PicWidth  = stPicSize.u32Width;
            stJpegAttr.u32PicHeight = stPicSize.u32Height;
            stJpegAttr.u32BufSize = stPicSize.u32Width * stPicSize.u32Height * 2;
            stJpegAttr.bByFrame = HI_TRUE;/*get stream mode is field mode  or frame mode*/
            stJpegAttr.bVIField = HI_FALSE;/*the sign of the VI picture is field or frame?*/
            stJpegAttr.u32Priority = 0;/*channels precedence level*/
            memcpy(&stVencChnAttr.stVeAttr.stAttrMjpeg, &stMjpegAttr, sizeof(VENC_ATTR_MJPEG_S));
            break;
        default:
            return HI_ERR_VENC_NOT_SUPPORT;
    }

    s32Ret = HI_MPI_VENC_CreateChn(VencChn, &stVencChnAttr);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_CreateChn [%d] faild with %#x!\n",\
                VencChn, s32Ret);
        return s32Ret;
    } else {
        HISI_PRT("HI_MPI_VENC_CreateChn [%d] SUCCESS with %#x!\n",\
                VencChn, s32Ret);
    }

    /******************************************
     step 3:  Regist Venc Channel to VencGrp
    ******************************************/
    s32Ret = HI_MPI_VENC_RegisterChn(VencGrp, VencChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_RegisterChn faild with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_RegisterChn SUCCESS with %#x!\n", s32Ret);
    }

    /******************************************
     step 4:  Start Recv Venc Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StartRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_StartRecvPic faild with%#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_StartRecvPic SUCCESS with %#x!\n", s32Ret);
    }

    return HI_SUCCESS;

}

/******************************************************************************
* function : venc bind vpss           
******************************************************************************/
HI_S32 HISI_COMM_VENC_BindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = GrpChn;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_SYS_Bind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_SYS_Bind SUCCESS with %#x!\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************
* funciton : get file postfix according palyload_type.
******************************************************************************/
HI_S32 HISI_COMM_VENC_GetFilePostfix(PAYLOAD_TYPE_E enPayload, char *szFilePostfix)
{
    if (PT_H264 == enPayload) {
        strcpy(szFilePostfix, ".h264");
    } else if (PT_JPEG == enPayload) {
        strcpy(szFilePostfix, ".jpg");
    } else if (PT_MJPEG == enPayload) {
        strcpy(szFilePostfix, ".mjp");
    } else if (PT_MP4VIDEO == enPayload) {
        strcpy(szFilePostfix, ".mp4");
    } else {
        HISI_PRT("payload type err!\n");
        return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : save H264 stream
******************************************************************************/
HI_S32 HISI_COMM_VENC_SaveH264(FILE* fpH264File, VENC_STREAM_S *pstStream)
{
    HI_S32 i;

    
    for (i = 0; i < pstStream->u32PackCount; i++) {
        fwrite(pstStream->pstPack[i].pu8Addr[0],
               pstStream->pstPack[i].u32Len[0], 1, fpH264File);

        fflush(fpH264File);

        if (pstStream->pstPack[i].u32Len[1] > 0) {
            fwrite(pstStream->pstPack[i].pu8Addr[1],
                   pstStream->pstPack[i].u32Len[1], 1, fpH264File);

            fflush(fpH264File);
        }
    }
    

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : save mjpeg stream. 
* WARNING: in Hi3531, user needn't write SOI & EOI.
******************************************************************************/
HI_S32 HISI_COMM_VENC_SaveMJpeg(FILE* fpJpegFile, VENC_STREAM_S *pstStream)
{
    VENC_PACK_S*  pstData;
    HI_U32 i;

    //fwrite(g_SOI, 1, sizeof(g_SOI), fpJpegFile); //in Hi3531, user needn't write SOI!

    for (i = 0; i < pstStream->u32PackCount; i++) {
        pstData = &pstStream->pstPack[i];
        fwrite(pstData->pu8Addr[0], pstData->u32Len[0], 1, fpJpegFile);
        fwrite(pstData->pu8Addr[1], pstData->u32Len[1], 1, fpJpegFile);
    }

    //fwrite(g_EOI, 1, sizeof(g_EOI), fpJpegFile);//in Hi3531, user needn't write SOI!

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : save stream
******************************************************************************/
HI_S32 HISI_COMM_VENC_SaveStream(PAYLOAD_TYPE_E enType,FILE *pFd, VENC_STREAM_S *pstStream)
{
    HI_S32 s32Ret;

    if (PT_H264 == enType) {
        s32Ret = HISI_COMM_VENC_SaveH264(pFd, pstStream);
    } else if (PT_MJPEG == enType) {
        s32Ret = HISI_COMM_VENC_SaveMJpeg(pFd, pstStream);
    } else {
        return HI_FAILURE;
    }
    return s32Ret;
}

/******************************************************************************
* funciton : get stream from each channels and save them
******************************************************************************/
HI_VOID* HISI_COMM_VENC_GetVencStreamProc(HI_VOID *p)
{
    HI_S32 i;
    HI_S32 s32ChnTotal;
    VENC_CHN_ATTR_S stVencChnAttr;
    HISI_VENC_GETSTREAM_PARA_S *pstPara;
    HI_S32 maxfd = 0;
    struct timeval TimeoutVal;
    fd_set read_fds;
    HI_S32 VencFd[VENC_MAX_CHN_NUM];
    HI_CHAR aszFileName[VENC_MAX_CHN_NUM][64];
    FILE *pFile[VENC_MAX_CHN_NUM];
    char szFilePostfix[10];
    VENC_CHN_STAT_S stStat;
    VENC_STREAM_S stStream;
    HI_S32 s32Ret;
    VENC_CHN VencChn;
    PAYLOAD_TYPE_E enPayLoadType[VENC_MAX_CHN_NUM];
//    
    pstPara = (HISI_VENC_GETSTREAM_PARA_S*)p;
    s32ChnTotal = pstPara->s32Cnt;
//
    /******************************************
     step 1:  check & prepare save-file & venc-fd
    ******************************************/
    if (s32ChnTotal >= VENC_MAX_CHN_NUM) {
        HISI_PRT("input count invaild\n");
        return NULL;
    }
    for (i = 0; i < s32ChnTotal; i++) {
        /* decide the stream file name, and open file to save stream */
        VencChn = i;
        s32Ret = HI_MPI_VENC_GetChnAttr(VencChn, &stVencChnAttr);
        if(s32Ret != HI_SUCCESS) {
            HISI_PRT("HI_MPI_VENC_GetChnAttr chn[%d] failed with %#x!\n", \
                   VencChn, s32Ret);
            return NULL;
        } else {
            HISI_PRT("HI_MPI_VENC_GetChnAttr chn[%d] SUCCESS with %#x!\n", \
                   VencChn, s32Ret);
        }
        enPayLoadType[i] = stVencChnAttr.stVeAttr.enType;

        s32Ret = HISI_COMM_VENC_GetFilePostfix(enPayLoadType[i], szFilePostfix);
        if(s32Ret != HI_SUCCESS) {
            HISI_PRT("HISI_COMM_VENC_GetFilePostfix [%d] failed with %#x!\n", \
                   stVencChnAttr.stVeAttr.enType, s32Ret);
            return NULL;
        } else {
            HISI_PRT("HISI_COMM_VENC_GetFilePostfix [%d] SUCCESS with %#x!\n", \
                   stVencChnAttr.stVeAttr.enType, s32Ret);
        }
        sprintf(aszFileName[i], "stream_chn%d%s", i, szFilePostfix);
        pFile[i] = fopen(aszFileName[i], "wb");
        if (!pFile[i]) {
            HISI_PRT("open file[%s] failed!\n", 
                   aszFileName[i]);
            return NULL;
        }

        /* Set Venc Fd. */
        VencFd[i] = HI_MPI_VENC_GetFd(i);
        if (VencFd[i] < 0) {
            HISI_PRT("HI_MPI_VENC_GetFd failed with %#x!\n", 
                   VencFd[i]);
            return NULL;
        } else {
            HISI_PRT("HI_MPI_VENC_GetFd SUCCESS with %#x!\n", 
                   VencFd[i]);
        }
        if (maxfd <= VencFd[i]) {
            maxfd = VencFd[i];
        }
    }

    /******************************************
     step 2:  Start to get streams of each channel.
    ******************************************/
    while (HI_TRUE == pstPara->bThreadStart)
    {
        FD_ZERO(&read_fds);
        for (i = 0; i < s32ChnTotal; i++) {
            FD_SET(VencFd[i], &read_fds);
        }

        TimeoutVal.tv_sec  = 3;
        TimeoutVal.tv_usec = 0;
//        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, NULL);
        s32Ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32Ret < 0) {
            HISI_PRT("select failed!\n");
            break;
        } else if (s32Ret == 0) {
            HISI_PRT("get venc stream time out, exit thread\n");
            continue;
        } else {
            HISI_PRT("get venc stream ok\n");
            for (i = 0; i < s32ChnTotal; i++) {
                if (FD_ISSET(VencFd[i], &read_fds)) {
                    /*******************************************************
                     step 2.1 : query how many packs in one-frame stream.
                    *******************************************************/
                    memset(&stStream, 0, sizeof(stStream));
                    s32Ret = HI_MPI_VENC_Query(i, &stStat);
                    if (HI_SUCCESS != s32Ret) {
                        HISI_PRT("HI_MPI_VENC_Query chn[%d] failed with %#x!\n", i, s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.2 : malloc corresponding number of pack nodes.
                    *******************************************************/
                    stStream.pstPack = (VENC_PACK_S*)malloc(sizeof(VENC_PACK_S) * stStat.u32CurPacks);
                    if (NULL == stStream.pstPack) {
                        HISI_PRT("malloc stream pack failed!\n");
                        break;
                    }
                    
                    /*******************************************************
                     step 2.3 : call mpi to get one-frame stream
                    *******************************************************/
                    stStream.u32PackCount = stStat.u32CurPacks;
                    s32Ret = HI_MPI_VENC_GetStream(i, &stStream, HI_TRUE);
                    if (HI_SUCCESS != s32Ret) {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        HISI_PRT("HI_MPI_VENC_GetStream failed with %#x!\n", \
                               s32Ret);
                        break;
                    }

                    /*******************************************************
                     step 2.4 : save frame to file
                    *******************************************************/
                    s32Ret = HISI_COMM_VENC_SaveStream(enPayLoadType[i], pFile[i], &stStream);
                    if (HI_SUCCESS != s32Ret) {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        HISI_PRT("save stream failed!\n");
                        break;
                    }
                    /*******************************************************
                     step 2.5 : release stream
                    *******************************************************/
                    s32Ret = HI_MPI_VENC_ReleaseStream(i, &stStream);
                    if (HI_SUCCESS != s32Ret) {
                        free(stStream.pstPack);
                        stStream.pstPack = NULL;
                        break;
                    }
                    /*******************************************************
                     step 2.6 : free pack nodes
                    *******************************************************/
                    free(stStream.pstPack);
                    stStream.pstPack = NULL;
                }
            }
        }
    }

    /*******************************************************
    * step 3 : close save-file
    *******************************************************/
    for (i = 0; i < s32ChnTotal; i++)
    {
        fclose(pFile[i]);
    }

    return NULL;
}

/******************************************************************************
* function : venc unbind vpss           
******************************************************************************/
HI_S32 HISI_COMM_VENC_UnBindVpss(VENC_GRP GrpChn,VPSS_GRP VpssGrp,VPSS_CHN VpssChn)
{
    HI_S32 s32Ret = HI_SUCCESS;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;

    stSrcChn.enModId = HI_ID_VPSS;
    stSrcChn.s32DevId = VpssGrp;
    stSrcChn.s32ChnId = VpssChn;

    stDestChn.enModId = HI_ID_GROUP;
    stDestChn.s32DevId = GrpChn;
    stDestChn.s32ChnId = 0;

    s32Ret = HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("HI_MPI_SYS_UnBind failed with %#x!\n", s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_SYS_UnBind SUCCESS with %#x!\n", s32Ret);
    }

    return s32Ret;
}

/******************************************************************************
* funciton : Stop venc ( stream mode -- H264, MJPEG )
******************************************************************************/
HI_S32 HISI_COMM_VENC_Stop(VENC_GRP VencGrp,VENC_CHN VencChn)
{
    HI_S32 s32Ret;

    /******************************************
     step 1:  Stop Recv Pictures
    ******************************************/
    s32Ret = HI_MPI_VENC_StopRecvPic(VencChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_StopRecvPic vechn[%d] SUCCESS with %#x!\n",\
               VencChn, s32Ret);
    }

    /******************************************
     step 2:  UnRegist Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_UnRegisterChn(VencChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_UnRegisterChn vechn[%d] SUCCESS with %#x!\n",\
               VencChn, s32Ret);
    }

    /******************************************
     step 3:  Distroy Venc Channel
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyChn(VencChn);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_DestroyChn vechn[%d] failed with %#x!\n",\
               VencChn, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_DestroyChn vechn[%d] SUCCESS with %#x!\n",\
               VencChn, s32Ret);
    }

    /******************************************
     step 4:  Distroy Venc Group
    ******************************************/
    s32Ret = HI_MPI_VENC_DestroyGroup(VencGrp);
    if (HI_SUCCESS != s32Ret) {
        HISI_PRT("HI_MPI_VENC_DestroyGroup group[%d] failed with %#x!\n",\
               VencGrp, s32Ret);
        return HI_FAILURE;
    } else {
        HISI_PRT("HI_MPI_VENC_DestroyGroup vechn[%d] SUCCESS with %#x!\n",\
               VencChn, s32Ret);
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : start get venc stream process thread
******************************************************************************/
HI_S32 HISI_COMM_VENC_StartGetStream(HI_S32 s32Cnt)
{
    gs_stPara.bThreadStart = HI_TRUE;
    gs_stPara.s32Cnt = s32Cnt;

    return pthread_create(&gs_VencPid, 0, HISI_COMM_VENC_GetVencStreamProc, (HI_VOID*)&gs_stPara);
}

/******************************************************************************
* funciton : stop get venc stream process.
******************************************************************************/
HI_S32 HISI_COMM_VENC_StopGetStream()
{
    if (HI_TRUE == gs_stPara.bThreadStart)
    {
        gs_stPara.bThreadStart = HI_FALSE;
        pthread_join(gs_VencPid, 0);
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif