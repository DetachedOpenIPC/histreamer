/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   sensor.h
 * Author: ussh
 *
 * Created on 14 июня 2019 г., 19:10
 */

#ifndef SENSOR_H
#define SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

extern int (*sensor_register_callback_fn)(void);
extern int (*sensor_unregister_callback_fn)(void);
extern void *libsns_so;

int tryLoadLibrary(const char *path);
int LoadSensorLibrary(const char *libsns_name);
void UnloadSensorLibrary();

int sensor_register_callback(void);
int sensor_unregister_callback(void);


#ifdef __cplusplus
}
#endif

#endif /* SENSOR_H */

