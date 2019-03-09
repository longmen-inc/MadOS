/****************************************
 * Interface  : SPI
 * File System: Fatfs R0.13c
 ****************************************/
#include <stdarg.h>
#include <stdlib.h>
#include "MadDev.h"
#include "spi_low.h"
#include "MadDrvSdhc.h"
#include "mstd_crc.h"
#include "../library/FatFs/diskio.h"

#define CMD0    0
#define CMD1    1
#define CMD8    8
#define CMD12  12
#define CMD16  16
#define CMD17  17
#define CMD18  18
#define CMD24  24
#define CMD25  25
#define CMD55  55
#define CMD58  58
#define ACMD41 41

static void mSpiSd_SetCmd(MadU8 *buf, MadU8 cmd, MadU32 arg)
{
    MadU8 i;
    MadU8 *parg = (MadU8*)(&arg);
    buf[0] = 0x40 | cmd;
    for(i=1; i<5; i++) {
        buf[i] = parg[4-i];
    }
    switch (cmd)
    {
        case CMD0:
        case CMD8:
            buf[5] = (CRC7(buf, 5) << 1) | 0x01;
            break;
        default:
            buf[5] = 0xFF;
            break;
    }
}

static MadU8 mSpiSd_BootCmd(mSpi_t *spi, MadU8 *buf, const MadU8 r1, MadU32 *rep)
{
    int i;
    MadU8 *prep;
    MadU8 r, res;
    res = mSpi_INVALID_DATA;
    prep = (MadU8*)rep;
    if(MTRUE == mSpiWriteBytes(spi, buf, 6, CMD_TIME_OUT)) {
        for(i=0; i<CMD_RETRY_NUM; i++) {
            mSpiRead8Bit(spi, &r);
            if(r & 0x80) {
                continue;
            }
            if(r == r1) {
                if(rep) {
                    if(MFALSE == mSpiRead8Bit(spi, &prep[3])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[2])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[1])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[0])) break;
                }
                res = r;
            }
            break;
        }
    }
    return res;
}

static MadU8 mSpiSd_BootCmd2(mSpi_t *spi, MadU8 *buf, const MadU8 r1, MadU32 *rep)
{ // For CMD12
    int i;
    MadU8 *prep;
    MadU8 r, res;
    res = mSpi_INVALID_DATA;
    prep = (MadU8*)rep;
    if(MTRUE == mSpiWriteBytes(spi, buf, 6, CMD_TIME_OUT)) {
        mSpiRead8Bit(spi, &r);
        for(i=0; i<CMD_RETRY_NUM; i++) {
            mSpiRead8Bit(spi, &r);
            if(r & 0x80) {
                continue;
            }
            if(r == r1) {
                if(rep) {
                    if(MFALSE == mSpiRead8Bit(spi, &prep[3])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[2])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[1])) break;
                    if(MFALSE == mSpiRead8Bit(spi, &prep[0])) break;
                }
                res = r;
            }
            break;
        }
    }
    return res;
}

inline
static MadU8 mSpiSd_SendCmd(mSpi_t *spi, MadU8 cmd, MadU32 arg, const MadU8 r1, MadU32 *rep)
{
    MadU8 res;
    MadU8 buf[6];
    mSpiSd_SetCmd(buf, cmd, arg);
    mSpi_NSS_ENABLE(spi);
    res = mSpiSd_BootCmd(spi, buf, r1, rep);
    mSpi_NSS_DISABLE(spi);
    mSpiSend8BitInvalid(spi);
    return res;
}

static int mSpiSd_WriteCmd(mSpi_t *spi, MadU8 cmd, MadU32 arg, const MadU8 r1, MadU32 *rep)
{
    int i, res;
    MadU8 r;
    i = STARTUP_RETRY_NUM;
    res = 1;
    do {
        r = mSpiSd_SendCmd(spi, cmd, arg, r1, rep);
    } while ((r == mSpi_INVALID_DATA) && (--i));
    if(r == mSpi_INVALID_DATA) {
        res = -1;
    }
    return res;
}

inline
static int mSpiSd_Reset(mSpi_t *spi)
{
    MAD_LOG("[SD] mSpiSd_Reset\n");
    mSpiSetClkPrescaler(spi, SPI_BaudRatePrescaler_256); // 36MHz / x
    madTimeDly(100);
    mSpiMulEmpty(spi, 10, DAT_TIME_OUT);
    return mSpiSd_WriteCmd(spi, CMD0, 0, 0x01, 0);
}

inline
static int mSpiSd_OCR(mSpi_t *spi, MadU32 *rep)
{
    return mSpiSd_WriteCmd(spi, CMD58, 0, 0x00, rep);
}

inline
static int mSpiSd_Init(mSpi_t *spi)
{
    int i, res;
    MadU32 rep;
    rep = 0;
    res = mSpiSd_WriteCmd(spi, CMD8, 0x000001AA, 0x01, &rep);
    if(0 < res) {
        MAD_LOG("[SD] SDC v2+ 0x%08X\n", rep);
        rep &= 0x00000FFF;
        if(rep == 0x1AA){
            i = CMD_RETRY_NUM;
            do {
                res = mSpiSd_WriteCmd(spi, CMD55, 0, 0x01, 0);
                if(res > 0) {
                    res = mSpiSd_WriteCmd(spi, ACMD41, 0x40000000, 0x00, 0);
                }
            } while((res < 0) && (--i));
        }
    } else {
        MAD_LOG("[SD] SDC v1\n");
        res = mSpiSd_WriteCmd(spi, CMD55, 0, 0x01, 0);
        if(0 < res) {
            res = mSpiSd_WriteCmd(spi, ACMD41, 0x00000000, 0x00, 0);
            if(0 > res) {
                res = mSpiSd_WriteCmd(spi, CMD1, 0, 0x00, 0);
            }
        }
    }
    return res;
}

inline
static int mSpiSd_SetBlkSize(mSpi_t *spi)
{
    MAD_LOG("[SD] mSpiSd_SetBlkSize\n");
    return mSpiSd_WriteCmd(spi, CMD16, SECTOR_SIZE, 0x00, 0);
}

inline
static int mSpiSd_WaitIdle(mSpi_t *spi)
{
    int count;
    MadU8 tmp;
    count = IDLE_RETRY_NUM;
    do {
        mSpiMulRead(spi, &tmp, 8, DAT_TIME_OUT);
    } while ((tmp != 0xFF) && (--count));
    if(tmp != 0xFF) {
        return -1;
    } else {
        return 1;
    }
}

static int mSpiSd_ReadOneSector(mSpi_t *spi, MadU8 *buff)
{
    int i;
    MadU8 res, tmp;
    res = -1;
    i = CMD_RETRY_NUM;
    do {
        mSpiRead8Bit(spi, &tmp);
    } while((tmp != 0xFE) && (--i));
    if(tmp == 0xFE) {
        if(MTRUE == mSpiReadBytes(spi, buff, SECTOR_SIZE, DAT_TIME_OUT)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            res = 1;
        }
    }
    return res;
}

static int mSpiSd_WriteOneSector(mSpi_t *spi, const MadU8 *buff, MadU8 head)
{
    int i;
    MadU8 res, tmp;
    res = -1;
    if(MTRUE == mSpiSend8Bit(spi, head)) {
        if(MTRUE == mSpiWriteBytes(spi, buff, SECTOR_SIZE, DAT_TIME_OUT)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            i = CMD_RETRY_NUM;
            do {
                mSpiRead8Bit(spi, &tmp);
            } while((tmp == 0xFF) && (--i));
            switch (tmp & 0x1F)
            {
                case 0x05:
                    res = 1;
                    break;
                case 0x0B:
                case 0x0D:
                default:
                    break;
            }
            mSpiSd_WaitIdle(spi);
        }
    }
    return res;
}

static int mSpiSd_read(mSpi_t *spi, MadU8 *data, MadU32 sector, MadU32 count, SdType_t type)
{
    MadU8 res;
    MadU8 buf[6];
    MadU32 addr;
    res = -1;
    if(count == 0) return res;
    if(type == SdType_SC) {
        addr = sector << SECTOR_ROLL;
    } else {
        addr = sector;
    }
    if(count == 1) {
        mSpiSd_SetCmd(buf, CMD17, addr);
        mSpi_NSS_ENABLE(spi);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0x00, 0)) {
            res = mSpiSd_ReadOneSector(spi, data);
        }
        mSpi_NSS_DISABLE(spi);
    } else {
        mSpiSd_SetCmd(buf, CMD18, addr);
        mSpi_NSS_ENABLE(spi);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0x00, 0)) {
            do {
                res   = mSpiSd_ReadOneSector(spi, data);
                data += SECTOR_SIZE;
            } while ((res == 1) && (--count));
            mSpiSd_SetCmd(buf, CMD12, 0);
            mSpiSd_BootCmd2(spi, buf, 0x00, 0);
            mSpiSd_WaitIdle(spi);
        }
        mSpi_NSS_DISABLE(spi);
    }
    mSpiSend8BitInvalid(spi);
    return res;
}

static int mSpiSd_write(mSpi_t *spi, const MadU8 *data, MadU32 sector, MadU32 count, SdType_t type)
{
    MadU8 res;
    MadU8 buf[6];
    MadU32 addr;
    res = -1;
    if(count == 0) return res;
    if(type == SdType_SC) {
        addr = sector << SECTOR_ROLL;
    } else {
        addr = sector;
    }
    if(count == 1) {
        mSpiSd_SetCmd(buf, CMD24, addr);
        mSpi_NSS_ENABLE(spi);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0x00, 0)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            res = mSpiSd_WriteOneSector(spi, data, 0xFE);
        }
        mSpi_NSS_DISABLE(spi);
    } else {
        mSpiSd_SetCmd(buf, CMD25, addr);
        mSpi_NSS_ENABLE(spi);
        if(0x00 == mSpiSd_BootCmd(spi, buf, 0x00, 0)) {
            mSpiMulEmpty(spi, 2, DAT_TIME_OUT);
            do {
                res   = mSpiSd_WriteOneSector(spi, data, 0xFC);
                data += SECTOR_SIZE;
            } while((res == 1) && (--count));
            mSpiSend8Bit(spi, 0xFD);
            mSpiSd_WaitIdle(spi);
        }
        mSpi_NSS_DISABLE(spi);
    }
    mSpiSend8BitInvalid(spi);
    return res;
}

static int Drv_open   (const char *, int, va_list);
static int Drv_creat  (const char *, mode_t);
static int Drv_fcntl  (int fd, int cmd, va_list);
static int Drv_write  (int fd, const void *buf, size_t len);
static int Drv_read   (int fd, void *buf, size_t len);
static int Drv_close  (int fd);
static int Drv_isatty (int fd);

const MadDrv_t MadDrvSdhc = {
    Drv_open,
    Drv_creat,
    Drv_fcntl,
    Drv_write,
    Drv_read,
    Drv_close,
    Drv_isatty,
    0
};

static int Drv_open(const char * file, int flag, va_list args)
{
    int      i;
    SdInfo_t *sd_info;
    int      fd   = (int)file;
    MadDev_t *dev = DevsList[fd];
    mSpi_t   *spi = (mSpi_t *)(dev->dev);
    mSpi_InitData_t *initData = (mSpi_InitData_t*)(dev->args);

    (void)args;

    dev->ptr      = malloc(sizeof(SdInfo_t));
    dev->txBuff   = 0;
    dev->rxBuff   = 0;
    dev->txLocker = 0;
    dev->rxLocker = 0;
    if(0 == dev->ptr) return -1;
    sd_info = (SdInfo_t*)(dev->ptr);
    if(MFALSE == mSpiInit(spi, initData)) {
        free(dev->ptr);
        return -1;
    }

    i = 1;
    do {
        madTimeDly(3000);
        do {
            MAD_LOG("[SD] Startup ... [%d]\n", i);
            if(0 > mSpiSd_Reset(spi)) break;
            if(0 > mSpiSd_Init(spi)) break;
            if(0 > mSpiSd_OCR(spi, &sd_info->OCR)) break;
            MAD_LOG("[SD] OCR = 0x%08X\n", sd_info->OCR);
            if(sd_info->OCR & 0x40000000) {
                sd_info->type = SdType_HC;
            } else {
                sd_info->type = SdType_SC;
            }
            if(0 > mSpiSd_SetBlkSize(spi)) break;
            mSpiSetClkPrescaler(spi, SPI_BaudRatePrescaler_2); // 36MHz / x
            MAD_LOG("[SD] Ready (%s)\n", (sd_info->type == SdType_SC) ? "SDSC" : "SDHC");
            return 1;
        } while(0);
    } while(i++ < STARTUP_RETRY_NUM);

    free(dev->ptr);
    mSpiDeInit(spi);
    MAD_LOG("[SD] Error\n");
    return -1;
}

static int Drv_creat(const char * file, mode_t mode)
{
    (void)file;
    (void)mode;
    return -1;
}

static int Drv_fcntl(int fd, int cmd, va_list args)
{
    int res;
    MadDev_t *dev     = DevsList[fd];
    mSpi_t   *spi     = (mSpi_t*)(dev->dev);
    SdInfo_t *sd_info = (SdInfo_t*)(dev->ptr);

    res = -1;
    switch (cmd)
    {
        case F_DISK_STATUS: {
            res = mSpiSd_OCR(spi, &sd_info->OCR);
            break;
        }

        case F_DISK_READ: {
            MadU8* buff;
            MadU32 sector;
            MadU32 count;
            buff   = va_arg(args, MadU8*);
            sector = va_arg(args, MadU32);
            count  = va_arg(args, MadU32);
            res = mSpiSd_read(spi, buff, sector, count, sd_info->type);
            break;
        }

        case F_DISK_WRITE: {
            const MadU8* buff;
            MadU32 sector;
            MadU32 count;
            buff   = va_arg(args, MadU8*);
            sector = va_arg(args, MadU32);
            count  = va_arg(args, MadU32);
            res = mSpiSd_write(spi, buff, sector, count, sd_info->type);
            break;
        }

        case CTRL_SYNC: {
            res = 1;
            break;
        }

        default:
            MAD_LOG("[Error] SDHC ... Unknown CMD");
            break;
    }
    return res;
}

static int Drv_write(int fd, const void *buf, size_t len)
{
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

static int Drv_read(int fd, void *buf, size_t len)
{
    (void)fd;
    (void)buf;
    (void)len;
    return -1;
}

static int Drv_close(int fd)
{
    MadDev_t *dev = DevsList[fd];
    mSpi_t   *spi = (mSpi_t *)(dev->dev);
    free(dev->ptr);
    mSpiDeInit(spi);
    MAD_LOG("[SD]...Closed\n");
    return 0;
}

static int Drv_isatty(int fd)
{
    (void)fd;
    return 0;
}
