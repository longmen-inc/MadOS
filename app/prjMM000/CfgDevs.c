#include "MadDev.h"

extern MadDev_t Tty;
extern MadDev_t Tty1;
extern MadDev_t Sd0;
extern MadDev_t Eth0;
extern MadDev_t Hx711_0;

MadDev_t *DevsList[] = {
    &Tty,
    &Tty1,
    &Sd0,
    &Eth0,
    &Hx711_0,
    MAD_DEVP_PLACE,
    MAD_DEVP_END
};
