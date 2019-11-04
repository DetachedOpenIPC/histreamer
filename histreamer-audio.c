/* 
 * File:   main.c
 * Author: ussh
 *
 * Created on 4 августа 2019 г., 15:20
 */

//#include <pjmedia/rtp.h>
#include <stdio.h>
#include <stdlib.h>
#include "pjlib.h"
#include "pjlib-util.h"

#define THIS_FILE "histreamer-audio.c"

static void my_perror(const char *title, pj_status_t status)
{
    PJ_PERROR(1,(THIS_FILE, status, title));
}

int audio_stream_util(int argc, char *argv[]);
int vidstreammain(int argc, char *argv[]);

/*
 * 
 */
int main(int argc, char** argv) {
    pj_status_t status;
    // pj_caching_pool cp;
    
    status = pj_init();
    if (status != PJ_SUCCESS) {
        my_perror("Error initializing PJLIB", status);
        return 1;
    }
    
    // pj_caching_pool_init(&cp, NULL, 1024*1024 );
    pjlib_util_init();

    PJ_LOG(2, (THIS_FILE, "init done, starting app"));
    audio_stream_util(argc, argv);
//    vidstreammain(argc, argv);
    
    // pj_caching_pool_destroy(&cp);
    PJ_LOG(2, (THIS_FILE, "pj_shutdown.."));
    pj_shutdown();
    return (EXIT_SUCCESS);
    // return (EXIT_SUCCESS);
}

