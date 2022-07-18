#include "drevo_calibur_v2.h"

#define NAME "Drevo Calibur V2"
#define VID 0x05AC
#define PID 0x0250

void calibur_v2_cb(libusb_device_handle *dev_handle, WINDOW *dev_win);


Device Drevo_Calibur_V2 = {
    NAME,
    KEYBOARD,
    VID,
    PID,
    calibur_v2_cb
};

void calibur_v2_cb(libusb_device_handle *dev_handle, WINDOW *dev_win) {

}