/******************************************************************************

  Copyright (C), 2001-2011, Hisilicon Tech. Co., Ltd.

 ******************************************************************************
  File Name     : sony138_sensor_ctl.c
  Version       : Initial Draft
  Author        : Hisilicon multimedia software group
  Created       : 2011/09/09
  Description   : Sony IMX138 sensor driver
  History       :
  1.Date        : 2011/09/09
    Author      : MPP
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

//#include "hi_ssp.h"
#include "hi_i2c.h"


int sony_sensor_write_packet(I2C_DATA_S i2c_data)
{
	int fd = -1;
	int ret;
//	unsigned int value;
        
//        I2C_DATA_S i2c_data ;

	fd = open("/dev/hi_i2c", 0);
	if(fd < 0)
	{
		printf("Open /dev/hi_i2c error!\n");
		return -1;
	}

        

        ret = ioctl(fd, CMD_I2C_WRITE, &i2c_data);
//	printf("sony_sensor_write_packet value = %u, ret = %d!\n", i2c_data, ret);

	close(fd);
	return 0;
}


int sensor_write_register(int reg_addr, int reg_value)
{
    I2C_DATA_S i2c_data ;
    unsigned int reg_width = 2;
    unsigned int data_width = 1;
    unsigned int device_addr = 0x34;
    
//	unsigned int value = (unsigned int)(((addr&0xffff)<<8) | (data & 0xff));
        
//    printf("dev_addr:0x%2x; reg_addr:0x%2x; reg_value:0x%2x; reg_width: %d; data_width: %d.\n", 
//        device_addr, reg_addr, reg_value, reg_width, data_width);
    i2c_data.dev_addr = device_addr ; 
    i2c_data.reg_addr = reg_addr    ; 
    i2c_data.addr_byte_num = reg_width  ; 
    i2c_data.data     = reg_value         ; 
    i2c_data.data_byte_num = data_width ;
        
        
    return sony_sensor_write_packet(i2c_data);
}

void sensor_init()
{
    sensor_write_register(0x3000, 0x01); //Standby
    usleep(200000);
    
    sensor_write_register(0x3005, 0x1);
    sensor_write_register(0x3006, 0x0);
    sensor_write_register(0x3007, 0x10);
    sensor_write_register(0x3009, 0x2);
    sensor_write_register(0x300a, 0xf0);
    
    sensor_write_register(0x300c, 0x0);
    sensor_write_register(0x300f, 0x1);
    sensor_write_register(0x3010, 0x39);
    sensor_write_register(0x3012, 0x50);
    sensor_write_register(0x3017, 0x1);
    sensor_write_register(0x3018, 0xee);
    sensor_write_register(0x3019, 0x2);

    sensor_write_register(0x301a, 0x0);
    sensor_write_register(0x301b, 0xc0);
    sensor_write_register(0x301c, 0x19);
    sensor_write_register(0x301d, 0xff);
    sensor_write_register(0x301e, 0x1);
    sensor_write_register(0x3036, 0x14);

    sensor_write_register(0x3038, 0x0);
    sensor_write_register(0x3039, 0x0);
    sensor_write_register(0x303a, 0x19);
    sensor_write_register(0x303b, 0x4);
    sensor_write_register(0x303c, 0x0);
    sensor_write_register(0x303d, 0x0);
    sensor_write_register(0x303e, 0x1c);
    sensor_write_register(0x303f, 0x5);
    sensor_write_register(0x3044, 0x1);
    sensor_write_register(0x3054, 0x63);


    sensor_write_register(0x305b, 0x0);
    sensor_write_register(0x305d, 0x0);
    sensor_write_register(0x305f, 0x10);
    sensor_write_register(0x30bf, 0x1f);
    sensor_write_register(0x3065, 0x20);
    sensor_write_register(0x3086, 0x1);
    sensor_write_register(0x30cf, 0xd1);
    sensor_write_register(0x30d0, 0x1b);
    sensor_write_register(0x30d2, 0x5f);
    sensor_write_register(0x30d3, 0x0);

    sensor_write_register(0x3112, 0x0);
    sensor_write_register(0x311d, 0x7);
    sensor_write_register(0x3123, 0x7);
    sensor_write_register(0x3126, 0xdf);
    sensor_write_register(0x3147, 0x87);
    sensor_write_register(0x3203, 0xcd);
    sensor_write_register(0x3207, 0x4b);
    sensor_write_register(0x3209, 0xe9);
    sensor_write_register(0x3213, 0x1b);
    sensor_write_register(0x3215, 0xed);
    sensor_write_register(0x3216, 0x1);
    sensor_write_register(0x3218, 0x9);
    sensor_write_register(0x321a, 0x19);
    sensor_write_register(0x321b, 0xa1);
    sensor_write_register(0x321c, 0x11);
    sensor_write_register(0x3227, 0x0);
    sensor_write_register(0x3228, 0x5);
    sensor_write_register(0x3229, 0xec);
    sensor_write_register(0x322a, 0x40);
    sensor_write_register(0x322b, 0x11);

    sensor_write_register(0x322d, 0x22);
    sensor_write_register(0x322e, 0x0);
    sensor_write_register(0x322f, 0x5);
    sensor_write_register(0x3231, 0xec);
    sensor_write_register(0x3232, 0x40);
    sensor_write_register(0x3233, 0x11);
    sensor_write_register(0x3235, 0x23);
    sensor_write_register(0x3236, 0xb0);
    sensor_write_register(0x3237, 0x4);
    sensor_write_register(0x3239, 0x24);
    sensor_write_register(0x323a, 0x30);
    sensor_write_register(0x323b, 0x4);
    sensor_write_register(0x323c, 0xed);
    sensor_write_register(0x323d, 0xc0);
    sensor_write_register(0x323e, 0x10);
    sensor_write_register(0x3240, 0x44);
    sensor_write_register(0x3241, 0xa0);
    sensor_write_register(0x3242, 0x4);
    sensor_write_register(0x3243, 0xd);
    sensor_write_register(0x3244, 0x31);
    sensor_write_register(0x3245, 0x11);
    sensor_write_register(0x3247, 0xec);
    sensor_write_register(0x3248, 0xd0);
    sensor_write_register(0x3249, 0x1d);
    
    sensor_write_register(0x3255, 0x3);
    sensor_write_register(0x3256, 0x54);
    sensor_write_register(0x3257, 0x60);
    sensor_write_register(0x3258, 0x1f);
    sensor_write_register(0x325a, 0xa9);
    sensor_write_register(0x325b, 0x50);
    sensor_write_register(0x325c, 0xa);
    sensor_write_register(0x325d, 0x25);
    sensor_write_register(0x325e, 0x11);
    sensor_write_register(0x325f, 0x12);
    sensor_write_register(0x3261, 0x9b);
    sensor_write_register(0x3266, 0xd0);
    sensor_write_register(0x3267, 0x8);
    sensor_write_register(0x326a, 0x20);
    sensor_write_register(0x326b, 0xa);
    sensor_write_register(0x326e, 0x20);
    sensor_write_register(0x326f, 0xa);
    sensor_write_register(0x3272, 0x20);
    sensor_write_register(0x3273, 0xa);
    sensor_write_register(0x3275, 0xec);
    sensor_write_register(0x327d, 0xa5);
    sensor_write_register(0x327e, 0x20);
    sensor_write_register(0x327f, 0xa);
    sensor_write_register(0x3281, 0xef);
    sensor_write_register(0x3282, 0xc0);
    sensor_write_register(0x3283, 0xe);
    sensor_write_register(0x3285, 0xf6);
    sensor_write_register(0x328a, 0x60);
    sensor_write_register(0x328b, 0x1f);
    sensor_write_register(0x328d, 0xbb);
    sensor_write_register(0x328e, 0x90);
    sensor_write_register(0x328f, 0xd);
    sensor_write_register(0x3290, 0x39);
    sensor_write_register(0x3291, 0xc1);
    sensor_write_register(0x3292, 0x1d);
    sensor_write_register(0x3294, 0xc9);
    sensor_write_register(0x3295, 0x70);
    sensor_write_register(0x3296, 0xe);
    sensor_write_register(0x3297, 0x47);
    sensor_write_register(0x3298, 0xa1);
    sensor_write_register(0x3299, 0x1e);
    sensor_write_register(0x329b, 0xc5);
    sensor_write_register(0x329c, 0xb0);
    sensor_write_register(0x329d, 0xe);
    sensor_write_register(0x329e, 0x43);
    sensor_write_register(0x329f, 0xe1);
    sensor_write_register(0x32a0, 0x1e);
    sensor_write_register(0x32a2, 0xbb);
    sensor_write_register(0x32a3, 0x10);
    sensor_write_register(0x32a4, 0xc);
    sensor_write_register(0x32a6, 0xb3);
    sensor_write_register(0x32a7, 0x30);
    sensor_write_register(0x32a8, 0xa);
    sensor_write_register(0x32a9, 0x29);
    sensor_write_register(0x32aa, 0x91);
    sensor_write_register(0x32ab, 0x11);
    sensor_write_register(0x32ad, 0xb4);
    sensor_write_register(0x32ae, 0x40);
    sensor_write_register(0x32af, 0xa);
    sensor_write_register(0x32b0, 0x2a);
    sensor_write_register(0x32b1, 0xa1);
    sensor_write_register(0x32b2, 0x11);
    sensor_write_register(0x32b4, 0xab);
    sensor_write_register(0x32b5, 0xb0);
    sensor_write_register(0x32b6, 0xb);
    sensor_write_register(0x32b7, 0x21);
    sensor_write_register(0x32b8, 0x11);
    sensor_write_register(0x32b9, 0x13);
    sensor_write_register(0x32bb, 0xac);
    sensor_write_register(0x32bc, 0xc0);
    sensor_write_register(0x32bd, 0xb);
    sensor_write_register(0x32be, 0x22);
    sensor_write_register(0x32bf, 0x21);
    sensor_write_register(0x32c0, 0x13);
    sensor_write_register(0x32c2, 0xad);
    sensor_write_register(0x32c3, 0x10);
    sensor_write_register(0x32c4, 0xb);
    sensor_write_register(0x32c5, 0x23);
    sensor_write_register(0x32c6, 0x71);
    sensor_write_register(0x32c7, 0x12);
    sensor_write_register(0x32c9, 0xb5);
    sensor_write_register(0x32ca, 0x90);
    sensor_write_register(0x32cb, 0xb);
    sensor_write_register(0x32cc, 0x2b);
    sensor_write_register(0x32cd, 0xf1);
    sensor_write_register(0x32ce, 0x12);
    sensor_write_register(0x32d0, 0xbb);
    sensor_write_register(0x32d1, 0x10);
    sensor_write_register(0x32d2, 0xc);
    sensor_write_register(0x32d4, 0xe7);
    sensor_write_register(0x32d5, 0x90);
    sensor_write_register(0x32d6, 0xe);
    sensor_write_register(0x32d8, 0x45);
    sensor_write_register(0x32d9, 0x11);
    sensor_write_register(0x32da, 0x1f);
    sensor_write_register(0x32eb, 0xa4);
    sensor_write_register(0x32ec, 0x60);
    sensor_write_register(0x32ed, 0x1f);
    sensor_write_register(0x3261, 0x9b);
    sensor_write_register(0x3266, 0xd0);
    sensor_write_register(0x3267, 0x8);
    
    usleep(200000);
    sensor_write_register(0x3000, 0x0); //release standy
    usleep(200000);
    
    sensor_write_register(0x3002, 0x0); //Master mose start
    usleep(200000);
    
//    usleep(200000);
//    sensor_write_register(0x200, 0x00); //release standy
//    usleep(200000);
//    sensor_write_register(0x202, 0x00); //Master mose start
//    usleep(200000);
//    sensor_write_register(0x249, 0x0A); //XVS & XHS output
//    usleep(200000);
    sensor_write_register(0x3049, 0xa);
 

    printf("-------Sony IMX138 Sensor Initial OK!-------\n");
}

void sensor_init225() // 225
{
    sensor_write_register(0x3000, 0x1);
    usleep(200000);

    sensor_write_register(0x3005, 0x1);
    sensor_write_register(0x3007, 0x0);
    sensor_write_register(0x3009, 0x1);
    sensor_write_register(0x300f, 0x0);
    sensor_write_register(0x3012, 0x2c);
    sensor_write_register(0x3013, 0x1);
    sensor_write_register(0x3016, 0x9);
    sensor_write_register(0x3018, 0x4c);
    sensor_write_register(0x3019, 0x4);
    sensor_write_register(0x301b, 0x90);
    sensor_write_register(0x301c, 0x11);
    sensor_write_register(0x301d, 0xc2);
    sensor_write_register(0x3046, 0x3);
    sensor_write_register(0x3047, 0x6);
    sensor_write_register(0x3048, 0xc0);
    sensor_write_register(0x3049, 0xa);
    sensor_write_register(0x305c, 0x20);
    sensor_write_register(0x305d, 0x0);
    sensor_write_register(0x305e, 0x20);
    sensor_write_register(0x305f, 0x0);
    sensor_write_register(0x3070, 0x2);
    sensor_write_register(0x3071, 0x1);
    sensor_write_register(0x309e, 0x22);
    sensor_write_register(0x30a5, 0xfb);
    sensor_write_register(0x30a6, 0x2);
    sensor_write_register(0x30b3, 0xff);
    sensor_write_register(0x30b4, 0x1);
    sensor_write_register(0x30b5, 0x42);
    sensor_write_register(0x30b8, 0x10);
    sensor_write_register(0x30c2, 0x1);
    sensor_write_register(0x310f, 0xf);
    sensor_write_register(0x3110, 0xe);
    sensor_write_register(0x3111, 0xe7);
    sensor_write_register(0x3112, 0x9c);
    sensor_write_register(0x3113, 0x83);
    sensor_write_register(0x3114, 0x10);
    sensor_write_register(0x3115, 0x42);
    sensor_write_register(0x3128, 0x1e);
    sensor_write_register(0x31ed, 0x38);
    sensor_write_register(0x320c, 0xcf);
    sensor_write_register(0x324c, 0x40);
    sensor_write_register(0x324d, 0x3);
    sensor_write_register(0x3261, 0xe0);
    sensor_write_register(0x3262, 0x2);
    sensor_write_register(0x326e, 0x2f);
    sensor_write_register(0x326f, 0x30);
    sensor_write_register(0x3270, 0x3);
    sensor_write_register(0x3298, 0x0);
    sensor_write_register(0x329a, 0x12);
    sensor_write_register(0x329b, 0xf1);
    sensor_write_register(0x329c, 0xc);

    usleep(200000);

    sensor_write_register(0x3000, 0x0);

    usleep(200000);

    sensor_write_register(0x3002, 0x0);
    usleep(200000);
    sensor_write_register(0x3049, 0x80);
 

    printf("-------Sony IMX225 Sensor Initial OK!-------\n");
}

//int sony_sensor_write_packet(unsigned int data)
//{
//	int fd = -1;
//	int ret;
//	unsigned int value;
//
//	fd = open("/dev/ssp", 0);
//	if(fd < 0)
//	{
//		printf("Open /dev/ssp error!\n");
//		return -1;
//	}
//
//	value = data;
//
//	ret = ioctl(fd, SSP_WRITE_ALT, &value);
//	printf("sony_sensor_write_packet value = %u, ret = %d!\n", value, ret);
//
//	close(fd);
//	return 0;
//}
//
//int sony_sensor_read_packet(unsigned int data)
//{
//	unsigned int value;
//	int fd = -1;
//	int ret;
//
//	fd = open("/dev/ssp", 0);
//	if(fd < 0)
//	{
//		printf("Open /dev/ssp error!\n");
//		return -1;
//	}
//
//	value = data;
//
//	ret = ioctl(fd, SSP_READ_ALT, &value);
//	printf("sony_sensor_read_packet value = %#x, ret = %d!\n", value, ret);
//
//	close(fd);
//	return (value&0xff);
//}
//
//int sensor_write_register(int addr, int data)
//{
//	unsigned int value = (unsigned int)(((addr&0xffff)<<8) | (data & 0xff));
//    return sony_sensor_write_packet(value);
//}
//
//int sensor_read_register(int addr)
//{
////	unsigned int data = (unsigned int)(((addr&0xffff)<<8));
//    unsigned int data = (unsigned int)(addr); 
//    return sony_sensor_read_packet(data);
//}
//
//void sensor_prog(int* rom)
//{
//}
//
//void setup_sensor(int isp_mode)
//{
//}
//
//void sensor_read_all_regs()
//{
//    
//    printf("0x200: %#x!\n\n\n\n\n", sensor_read_register(131073)); //Standby
//    usleep(200000);
//    printf("0x200: %#x!\n", sensor_read_register(0x200)); //Standby
//    usleep(200000);
//    printf("0x203: %#x!\n", sensor_read_register(0x203)); 
//    usleep(200000);
//    printf("0x205: %#x!\n", sensor_read_register(0x205)); 
//    usleep(200000);
//    printf("0x206: %#x!\n", sensor_read_register(0x206)); 
//    usleep(200000);
//    printf("0x207: %#x!\n", sensor_read_register(0x207));
//    usleep(200000);
//    printf("0x209: %#x!\n", sensor_read_register(0x209));
//    usleep(200000);
//    printf("0x20A: %#x!\n", sensor_read_register(0x20A));
//    usleep(200000);
//    printf("0x20C: %#x!\n", sensor_read_register(0x20C));
//    usleep(200000);
//    printf("0x20F: %#x!\n", sensor_read_register(0x20F));
//    usleep(200000);
//    printf("0x210: %#x!\n", sensor_read_register(0x210));
//    usleep(200000);
//    printf("0x212: %#x!\n", sensor_read_register(0x212));
//    usleep(200000);
//    printf("0x217: %#x!\n", sensor_read_register(0x217));
//    usleep(200000);
//    printf("0x218: %#x!\n", sensor_read_register(0x218));
//    usleep(200000);
//    printf("0x219: %#x!\n", sensor_read_register(0x219));
//    usleep(200000);
//    printf("0x21A: %#x!\n", sensor_read_register(0x21A));
//    usleep(200000);
//    printf("0x21B: %#x!\n", sensor_read_register(0x21B));
//    usleep(200000);
//    printf("0x21C: %#x!\n", sensor_read_register(0x21C));
//    usleep(200000);
//    printf("0x21D: %#x!\n", sensor_read_register(0x21D));
//    usleep(200000);
//    printf("0x21E: %#x!\n", sensor_read_register(0x21E));
//    usleep(200000);
//    printf("0x236: %#x!\n", sensor_read_register(0x236));
//    usleep(200000);
//    printf("0x238: %#x!\n", sensor_read_register(0x238));
//    usleep(200000);
//    printf("0x239: %#x!\n", sensor_read_register(0x239));
//    usleep(200000);
//    printf("0x23A: %#x!\n", sensor_read_register(0x23A));
//    usleep(200000);
//    printf("0x23B: %#x!\n", sensor_read_register(0x23B));
//    usleep(200000);
//    printf("0x23C: %#x!\n", sensor_read_register(0x23C));
//    usleep(200000);
//    printf("0x23D: %#x!\n", sensor_read_register(0x23D));
//    usleep(200000);
//    printf("0x23E: %#x!\n", sensor_read_register(0x23E));
//    usleep(200000);
//    printf("0x23F: %#x!\n", sensor_read_register(0x23F));
//    usleep(200000);
//    printf("0x244: %#x!\n", sensor_read_register(0x244));
//    usleep(200000);
//    printf("0x254: %#x!\n", sensor_read_register(0x254));
//    usleep(200000);
//    printf("0x25B: %#x!\n", sensor_read_register(0x25B));
//    usleep(200000);
//    printf("0x25D: %#x!\n", sensor_read_register(0x25D));
//    usleep(200000);
//    printf("0x25F: %#x!\n", sensor_read_register(0x25F));
//    usleep(200000);
//    printf("0x2BF: %#x!\n", sensor_read_register(0x2BF));
//    usleep(200000);
//    printf("0x265: %#x!\n", sensor_read_register(0x265));
//    usleep(200000);
//    printf("0x286: %#x!\n", sensor_read_register(0x286));
//    usleep(200000);
//    printf("0x2CF: %#x!\n", sensor_read_register(0x2CF));
//    usleep(200000);
//    printf("0x2D0: %#x!\n", sensor_read_register(0x2D0));
//    usleep(200000);
//    printf("0x2D2: %#x!\n", sensor_read_register(0x2D2));
//    usleep(200000);
//    printf("0x2D3: %#x!\n", sensor_read_register(0x2D3));
//    usleep(200000);
//    printf("0x249: %#x!\n", sensor_read_register(0x249));
//    usleep(200000);
//}
//
/*
 
 */

//void sensor_init()
//{
//    sensor_write_register(0x200, 0x01); //Standby
//    usleep(200000);
//    printf("Standby!\n");
////    sensor_write_register(0x203, 0x01); //Soft reset
//    sensor_write_register(0x203, 0x00); //Soft reset
//    usleep(200000);
//    printf("Soft reset!\n");
//
//    // chip_id = 0x2
//    sensor_write_register(0x205, 0x01); //12bit
//    sensor_write_register(0x206, 0x00); //Drive mode:All-pix scan mode(720p mode)
////    sensor_write_register(0x207, 0x10); //Window mode:720p mode
//    sensor_write_register(0x209, 0x02); //30fps mode
//    sensor_write_register(0x20A, 0xF0); //black level
//
//    /*linear & WDR mode is different*/
//    sensor_write_register(0x20C, 0x00);
//    sensor_write_register(0x20F, 0x01);
//    sensor_write_register(0x210, 0x39);
//    sensor_write_register(0x212, 0x50);
//
//    sensor_write_register(0x217, 0x01);
//    sensor_write_register(0x218, 0xF0);
//    sensor_write_register(0x219, 0x02);
//    sensor_write_register(0x21A, 0x00);
//
//    sensor_write_register(0x21B, 0xC0);//different from imx104
//    sensor_write_register(0x21C, 0x19);
//
//    sensor_write_register(0x21D, 0xFF);
//    sensor_write_register(0x21E, 0x01);
//    sensor_write_register(0x236, 0x14); //VB size
//
//    sensor_write_register(0x238, 0x00); //cropping postion(Vertical position)
//    sensor_write_register(0x239, 0x00);
//
//    sensor_write_register(0x23A, 0x19); //cropping size(Vertical direction)
//    sensor_write_register(0x23B, 0x04);
//
//    sensor_write_register(0x23C, 0x00); //cropping postion(horizontal position)
//    sensor_write_register(0x23D, 0x00);
//    sensor_write_register(0x23E, 0x1C); //cropping size(horizontal direction)
//    sensor_write_register(0x23F, 0x05);
//    sensor_write_register(0x244, 0x00); //Parallel CMOS SDR output
////    sensor_write_register(0x244, 0x01); //Parallel CMOS SDR output
//    sensor_write_register(0x254, 0x63);
//    sensor_write_register(0x25B, 0x00); //CLK 37.125MHz
//    sensor_write_register(0x25D, 0x00); //CLK 37.125MHz
//    sensor_write_register(0x25F, 0x10); //invalid
//    sensor_write_register(0x2BF, 0x1F);
//
//    /*linear & WDR mode is different*/
//    sensor_write_register(0x265, 0x20);
//    sensor_write_register(0x286, 0x01);
//    sensor_write_register(0x2CF, 0xD1);
//    sensor_write_register(0x2D0, 0x1B);
//    sensor_write_register(0x2D2, 0x5F);
//    sensor_write_register(0x2D3, 0x00);
//
////    // chip_id = 0x3
////    sensor_write_register(0x312, 0x00);
////    sensor_write_register(0x31D, 0x07);
////    sensor_write_register(0x323, 0x07);
////    sensor_write_register(0x326, 0xDF);
////    sensor_write_register(0x347, 0x87);
////
////    // chip_id = 0x4
////    sensor_write_register(0x403, 0xCD);
////    sensor_write_register(0x407, 0x4B);
////    sensor_write_register(0x409, 0xE9);
////    sensor_write_register(0x413, 0x1B);
////    sensor_write_register(0x415, 0xED);
////    sensor_write_register(0x416, 0x01);
////    sensor_write_register(0x418, 0x09);
////    sensor_write_register(0x41A, 0x19);
////    sensor_write_register(0x41B, 0xA1);
////    sensor_write_register(0x41C, 0x11);
////    sensor_write_register(0x427, 0x00);
////    sensor_write_register(0x428, 0x05);
////    sensor_write_register(0x429, 0xEC);
////    sensor_write_register(0x42A, 0x40);
////    sensor_write_register(0x42B, 0x11);
////    sensor_write_register(0x42D, 0x22);
////    sensor_write_register(0x42E, 0x00);
////    sensor_write_register(0x42F, 0x05);
////    sensor_write_register(0x431, 0xEC);
////    sensor_write_register(0x432, 0x40);
////    sensor_write_register(0x433, 0x11);
////    sensor_write_register(0x435, 0x23);
////    sensor_write_register(0x436, 0xB0);
////    sensor_write_register(0x437, 0x04);
////    sensor_write_register(0x439, 0x24);
////    sensor_write_register(0x43A, 0x30);
////    sensor_write_register(0x43B, 0x04);
////    sensor_write_register(0x43C, 0xED);
////    sensor_write_register(0x43D, 0xC0);
////    sensor_write_register(0x43E, 0x10);
////    sensor_write_register(0x440, 0x44);
////    sensor_write_register(0x441, 0xA0);
////    sensor_write_register(0x442, 0x04);
////    sensor_write_register(0x443, 0x0D);
////    sensor_write_register(0x444, 0x31);
////    sensor_write_register(0x445, 0x11);
////    sensor_write_register(0x447, 0xEC);
////    sensor_write_register(0x448, 0xD0);
////    sensor_write_register(0x449, 0x1D);
////    sensor_write_register(0x455, 0x03);
////    sensor_write_register(0x456, 0x54);
////    sensor_write_register(0x457, 0x60);
////    sensor_write_register(0x458, 0x1F);
////    sensor_write_register(0x45A, 0xA9);
////    sensor_write_register(0x45B, 0x50);
////    sensor_write_register(0x45C, 0x0A);
////    sensor_write_register(0x45D, 0x25);
////    sensor_write_register(0x45E, 0x11);
////    sensor_write_register(0x45F, 0x12);
////    sensor_write_register(0x461, 0x9B);
////    sensor_write_register(0x466, 0xD0);
////    sensor_write_register(0x467, 0x08);
////    sensor_write_register(0x46A, 0x20);
////    sensor_write_register(0x46B, 0x0A);
////    sensor_write_register(0x46E, 0x20);
////    sensor_write_register(0x46F, 0x0A);
////    sensor_write_register(0x472, 0x20);
////    sensor_write_register(0x473, 0x0A);
////    sensor_write_register(0x475, 0xEC);
////    sensor_write_register(0x47D, 0xA5);
////    sensor_write_register(0x47E, 0x20);
////    sensor_write_register(0x47F, 0x0A);
////    sensor_write_register(0x481, 0xEF);
////    sensor_write_register(0x482, 0xC0);
////    sensor_write_register(0x483, 0x0E);
////    sensor_write_register(0x485, 0xF6);
////    sensor_write_register(0x48A, 0x60);
////    sensor_write_register(0x48B, 0x1F);
////    sensor_write_register(0x48D, 0xBB);
////    sensor_write_register(0x48E, 0x90);
////    sensor_write_register(0x48F, 0x0D);
////    sensor_write_register(0x490, 0x39);
////    sensor_write_register(0x491, 0xC1);
////    sensor_write_register(0x492, 0x1D);
////    sensor_write_register(0x494, 0xC9);
////    sensor_write_register(0x495, 0x70);
////    sensor_write_register(0x496, 0x0E);
////    sensor_write_register(0x497, 0x47);
////    sensor_write_register(0x498, 0xA1);
////    sensor_write_register(0x499, 0x1E);
////    sensor_write_register(0x49B, 0xC5);
////    sensor_write_register(0x49C, 0xB0);
////    sensor_write_register(0x49D, 0x0E);
////    sensor_write_register(0x49E, 0x43);
////    sensor_write_register(0x49F, 0xE1);
////    sensor_write_register(0x4A0, 0x1E);
////    sensor_write_register(0x4A2, 0xBB);
////    sensor_write_register(0x4A3, 0x10);
////    sensor_write_register(0x4A4, 0x0C);
////    sensor_write_register(0x4A6, 0xB3);
////    sensor_write_register(0x4A7, 0x30);
////    sensor_write_register(0x4A8, 0x0A);
////    sensor_write_register(0x4A9, 0x29);
////    sensor_write_register(0x4AA, 0x91);
////    sensor_write_register(0x4AB, 0x11);
////    sensor_write_register(0x4AD, 0xB4);
////    sensor_write_register(0x4AE, 0x40);
////    sensor_write_register(0x4AF, 0x0A);
////    sensor_write_register(0x4B0, 0x2A);
////    sensor_write_register(0x4B1, 0xA1);
////    sensor_write_register(0x4B2, 0x11);
////    sensor_write_register(0x4B4, 0xAB);
////    sensor_write_register(0x4B5, 0xB0);
////    sensor_write_register(0x4B6, 0x0B);
////    sensor_write_register(0x4B7, 0x21);
////    sensor_write_register(0x4B8, 0x11);
////    sensor_write_register(0x4B9, 0x13);
////    sensor_write_register(0x4BB, 0xAC);
////    sensor_write_register(0x4BC, 0xC0);
////    sensor_write_register(0x4BD, 0x0B);
////    sensor_write_register(0x4BE, 0x22);
////    sensor_write_register(0x4BF, 0x21);
////    sensor_write_register(0x4C0, 0x13);
////    sensor_write_register(0x4C2, 0xAD);
////    sensor_write_register(0x4C3, 0x10);
////    sensor_write_register(0x4C4, 0x0B);
////    sensor_write_register(0x4C5, 0x23);
////    sensor_write_register(0x4C6, 0x71);
////    sensor_write_register(0x4C7, 0x12);
////    sensor_write_register(0x4C9, 0xB5);
////    sensor_write_register(0x4CA, 0x90);
////    sensor_write_register(0x4CB, 0x0B);
////    sensor_write_register(0x4CC, 0x2B);
////    sensor_write_register(0x4CD, 0xF1);
////    sensor_write_register(0x4CE, 0x12);
////    sensor_write_register(0x4D0, 0xBB);
////    sensor_write_register(0x4D1, 0x10);
////    sensor_write_register(0x4D2, 0x0C);
////    sensor_write_register(0x4D4, 0xE7);
////    sensor_write_register(0x4D5, 0x90);
////    sensor_write_register(0x4D6, 0x0E);
////    sensor_write_register(0x4D8, 0x45);
////    sensor_write_register(0x4D9, 0x11);
////    sensor_write_register(0x4DA, 0x1F);
////    sensor_write_register(0x4EB, 0xA4);
////    sensor_write_register(0x4EC, 0x60);
////    sensor_write_register(0x4ED, 0x1F);
////
////    /*linear & WDR mode is different*/
////    sensor_write_register(0x461, 0x9B);
////    sensor_write_register(0x466, 0xD0);
////    sensor_write_register(0x467, 0x08);
//
//    usleep(200000);
//    sensor_write_register(0x200, 0x00); //release standy
//    usleep(200000);
//    sensor_write_register(0x202, 0x00); //Master mose start
//    usleep(200000);
//    sensor_write_register(0x249, 0x0A); //XVS & XHS output
//    usleep(200000);
//
//    printf("-------Sony IMX138 Sensor Initial OK!-------\n");
//}

