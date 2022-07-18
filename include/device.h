#ifndef DEVICE_H
#define DEVICE_H


#include <stdio.h>
#include <stdint.h>
#include <curses.h>
#include <libusb-1.0/libusb.h>


#define DEV_NAME_LEN 32
#define KNOWN_DEVICES_LEN 2
#define TYPE_STR(t) (t==MOUSE ? "(mouse)" : "(keyboard)")

typedef void (*DeviceCallback)(libusb_device_handle *dev_handle, WINDOW *dev_win);

typedef enum {
    MOUSE,
    KEYBOARD,
} DeviceType;

typedef struct _Device {
    char name[DEV_NAME_LEN];
    DeviceType type; 
    uint16_t vid;
    uint16_t pid;
    DeviceCallback cb;
} Device;

#endif