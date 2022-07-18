#include "device.h"
#include "logitech_g300.h"
#include "drevo_calibur_v2.h"

Device *known_devices[KNOWN_DEVICES_LEN] = {
    &Logitech_G300,
    &Drevo_Calibur_V2
};