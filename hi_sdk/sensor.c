/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "sensor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>

int (*sensor_register_callback_fn)(void);
int (*sensor_unregister_callback_fn)(void);
void *libsns_so = NULL;

int tryLoadLibrary(const char *path) {
    printf("try to load: %s\n", path);
    
    void * libsns_so = dlopen(path, RTLD_LAZY);
    if (libsns_so == NULL)
    {
        printf("dlopen \"%s\" error: %s\n", path, dlerror());
        
        return 1;
    }
    
    
//    libsns_so = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
    printf("libsns_so 0x%08" PRIXPTR "\n", (uintptr_t) libsns_so);
//    if (libsns_so == NULL) {
//        printf("dlopen \"%s\" error: %s\n", path, dlerror());
//        return 0;
//    }
    sensor_register_callback_fn = dlsym(libsns_so, "sensor_register_callback");
    sensor_unregister_callback_fn = dlsym(libsns_so, "sensor_unregister_callback");
    return 0;
}
int LoadSensorLibrary(const char *libsns_name) {
    char path[250];
    
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }
//    sprintf(path, "%s", libsns_name);
//    snprintf(path, 250, "%s", libsns_name);
//    if (tryLoadLibrary(path) == 0) return 1;
    snprintf(path, 250, "%s/%s", cwd, libsns_name);
    if (tryLoadLibrary(path) == 0) return 0;
    snprintf(path, 250, "./%s", libsns_name);
    if (tryLoadLibrary(path) == 0) return 0;
    snprintf(path, 250, "/usr/lib/%s", libsns_name);
    if (tryLoadLibrary(path) == 0) return 0;
    return 1;
}

void UnloadSensorLibrary() {
    dlclose(libsns_so);
    libsns_so = NULL;
}
#if 0
int sensor_register_callback(void) {
    return sensor_register_callback_fn();
}
int sensor_unregister_callback(void) {
    return sensor_unregister_callback_fn();
}
#endif