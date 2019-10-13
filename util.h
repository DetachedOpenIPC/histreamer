/* $Id$ */
/* 
 * Copyright (C) 2008-2011 Teluu Inc. (http://www.teluu.com)
 * Copyright (C) 2003-2008 Benny Prijono <benny@prijono.org>
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
#include <stdlib.h>	/* strtol() */

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include "pj/types.h"
#include "pj/errno.h"
#include "pj/log.h"

/* Util to display the error message for the specified error code  */
static int app_perror( const char *sender, const char *title, 
		       pj_status_t status)
{
    char errmsg[PJ_ERR_MSG_SIZE];

    pj_strerror(status, errmsg, sizeof(errmsg));

    PJ_LOG(3,(sender, "%s: %s [code=%d]", title, errmsg, status));
    return 1;
}



/* Constants */
#define CLOCK_RATE	44100
#define NSAMPLES	(CLOCK_RATE * 20 / 1000)
#define NCHANNELS	1
#define NBITS		16

/*
 * Common sound options.
 */
#define SND_USAGE   \
"  -d, --dev=NUM        Sound device use device id NUM (default=-1)	 \n"\
"  -r, --rate=HZ        Set clock rate in samples per sec (default=44100)\n"\
"  -c, --channel=NUM    Set # of channels (default=1 for mono).		 \n"\
"  -f, --frame=NUM      Set # of samples per frame (default equival 20ms)\n"\
"  -b, --bit=NUM        Set # of bits per sample (default=16)		 \n"

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H */



