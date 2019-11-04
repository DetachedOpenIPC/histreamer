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
#include "sensor.h"

static pthread_t gs_IspPid;
    
    
/******************************************************************************
* funciton : Sensor init
* note : different sensor corresponding different lib. So,  you can change
		  SENSOR_TYPE in Makefile.para, instead of modify program code.
******************************************************************************/
HI_S32 HISI_COMM_ISP_SensorInit(HISI_SENSOR_S sensor)
{
    HI_S32 s32Ret;
#if 0
#ifdef ISP_V1
    /* 1. sensor init */
    sensor_init();

    /* 0: linear mode, 1: WDR mode */
    sensor_mode_set(0);
#endif
#endif
    
#if 0
    if (LoadSensorLibrary(sensor.lib_name) == 1) {
        printf("Error loading %s\n", sensor.lib_name);
        return EXIT_FAILURE;
    } else {
        printf("Sensor lib %s loaded\n", sensor.lib_name);
    }
#endif
    /* 2. sensor register callback */
    s32Ret = sensor_register_callback();
    if (s32Ret != HI_SUCCESS)
    {
        printf("%s: sensor_register_callback failed with %#x!\n", \
               __FUNCTION__, s32Ret);
        return s32Ret;
    }
//    sensor_read_all_regs();
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : ISP init
******************************************************************************/
HI_S32 HISI_COMM_ISP_Init(HISI_SENSOR_S sensor)
{
    HI_S32 s32Ret;
    ISP_IMAGE_ATTR_S stImageAttr;
    ISP_INPUT_TIMING_S stInputTiming;
#ifdef ISP_V2
    ALG_LIB_S stLib;

    /* 1. register ae lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    s32Ret = HI_MPI_AE_Register(&stLib);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_AE_Register failed!\n", __FUNCTION__);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_AE_Register Success!\n", __FUNCTION__);
    }

    /* 2. register awb lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    s32Ret = HI_MPI_AWB_Register(&stLib);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_AWB_Register failed!\n", __FUNCTION__);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_AWB_Register Success!\n", __FUNCTION__);
    }

    /* 3. register af lib */
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    s32Ret = HI_MPI_AF_Register(&stLib);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_AF_Register failed!\n", __FUNCTION__);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_AF_Register Success!\n", __FUNCTION__);
    }
#endif
    /* 4. isp init */
    s32Ret = HI_MPI_ISP_Init();
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_ISP_Init failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_ISP_Init Success!\n", __FUNCTION__);
    }

    /* 5. isp set image attributes */
    /* note : different sensor, different ISP_IMAGE_ATTR_S define.
              if the sensor you used is different, you can change
              ISP_IMAGE_ATTR_S definition */


    stImageAttr.enBayer      = sensor.image_attrs.enBayer;
    stImageAttr.u16FrameRate = sensor.image_attrs.u16FrameRate;
    stImageAttr.u16Width     = sensor.image_attrs.u16Width;
    stImageAttr.u16Height    = sensor.image_attrs.u16Height;


    s32Ret = HI_MPI_ISP_SetImageAttr(&stImageAttr);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_ISP_SetImageAttr failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_ISP_SetImageAttr Success!\n", __FUNCTION__);
    }

    /* 6. isp set timing */

    stInputTiming.enWndMode = ISP_WIND_ALL;
    stInputTiming.u16HorWndStart = 72;
    stInputTiming.u16HorWndLength = 1280;
    stInputTiming.u16VerWndStart = 20;
    stInputTiming.u16VerWndLength = 720;
            
    s32Ret = HI_MPI_ISP_SetInputTiming(&stInputTiming);
    if (s32Ret != HI_SUCCESS) {
        HISI_PRT("%s: HI_MPI_ISP_SetInputTiming failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    } else {
        HISI_PRT("%s: HI_MPI_ISP_SetInputTiming Success!\n", __FUNCTION__);
    }

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : ISP Run
******************************************************************************/
HI_S32 HISI_COMM_ISP_Run(HISI_SENSOR_S sensor)
{
    HISI_PRT("%s: HISI_COMM_ISP_Run!\n", __FUNCTION__);
    HI_S32 s32Ret;

    s32Ret = HISI_COMM_ISP_Init(sensor);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: ISP init failed with %#x!\n", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

#if 1
    if (0 != pthread_create(&gs_IspPid, 0, (void* (*)(void*))HI_MPI_ISP_Run, NULL)) {
        printf("%s: create isp running thread failed!\n", __FUNCTION__);
        return HI_FAILURE;
    } else {
        printf("%s: create isp running thread Success!\n", __FUNCTION__);
    }
#else
	/* configure thread priority */
	if (1)
	{
		#include <sched.h>

		pthread_attr_t attr;
		struct sched_param param;
		int newprio = 50;

		pthread_attr_init(&attr);

		if (1)
		{
			int policy = 0;
			int min, max;

			pthread_attr_getschedpolicy(&attr, &policy);
			printf("-->default thread use policy is %d --<\n", policy);

			pthread_attr_setschedpolicy(&attr, SCHED_RR);
			pthread_attr_getschedpolicy(&attr, &policy);
			printf("-->current thread use policy is %d --<\n", policy);

			switch (policy)
			{
				case SCHED_FIFO:
					printf("-->current thread use policy is SCHED_FIFO --<\n");
					break;

				case SCHED_RR:
					printf("-->current thread use policy is SCHED_RR --<\n");
					break;

				case SCHED_OTHER:
					printf("-->current thread use policy is SCHED_OTHER --<\n");
					break;

				default:
					printf("-->current thread use policy is UNKNOW --<\n");
					break;
			}

			min = sched_get_priority_min(policy);
			max = sched_get_priority_max(policy);

			printf("-->current thread policy priority range (%d ~ %d) --<\n", min, max);
		}

		pthread_attr_getschedparam(&attr, &param);

		printf("-->default isp thread priority is %d , next be %d --<\n", param.sched_priority, newprio);
		param.sched_priority = newprio;
		pthread_attr_setschedparam(&attr, &param);

		if (0 != pthread_create(&gs_IspPid, &attr, (void* (*)(void*))HI_MPI_ISP_Run, NULL))
		{
			printf("%s: create isp running thread failed!\n", __FUNCTION__);
			return HI_FAILURE;
		}

		pthread_attr_destroy(&attr);
	}
#endif

#ifdef ISP_V1
    /* load configure file if there is */
#ifdef SAMPLE_LOAD_ISPREGFILE
    s32Ret = SAMPLE_COMM_ISP_LoadRegFile(CFG_OPT_LOAD, SAMPLE_ISPCFG_FILE);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: load isp cfg file failed![%s]\n", __FUNCTION__, SAMPLE_ISPCFG_FILE);
        return HI_FAILURE;
    }
#endif
#endif

    return HI_SUCCESS;
}

/******************************************************************************
* funciton : stop ISP, and stop isp thread
******************************************************************************/
HI_VOID HISI_COMM_ISP_Stop()
{
    HI_MPI_ISP_Exit();
#ifdef ISP_V2

    ALG_LIB_S stLib;
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AE_LIB_NAME);
    HI_MPI_AE_UnRegister(&stLib);
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AWB_LIB_NAME);
    HI_MPI_AWB_UnRegister(&stLib);
    stLib.s32Id = 0;
    strcpy(stLib.acLibName, HI_AF_LIB_NAME);
    HI_MPI_AF_UnRegister(&stLib);
    sensor_unregister_callback();

#endif
    pthread_join(gs_IspPid, 0);
    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif