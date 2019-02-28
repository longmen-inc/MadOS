#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "MadOS.h"
#include "CfgUser.h"
#include "ff.h"
#include "testFatFs.h"

#define HELLO_MADOS "Hello MadOS\nNow, we are ONE !!!\n"
#define HELLO_LEN   sizeof(HELLO_MADOS)-1
#define BUFF_LEN    HELLO_LEN + 8
#define WRITE_CNT   3000

static void test_fatfs_act(MadVptr exData);

void Init_TestFatFs(void)
{
    madThreadCreate(test_fatfs_act, 0, 2048, THREAD_PRIO_TEST_FATFS);
}

static void test_fatfs_act(MadVptr exData)
{
    int i;

#if 0
    volatile int res;
    UINT cnt;
    FIL *fil;
    MadU8 *buf;

    fil = (FIL*)malloc(sizeof(FIL));
    if(fil == MNULL) {
        MAD_LOG("fil = (FIL*)malloc(sizeof(FIL)) ... failed\n");
        while(1) {
            madTimeDly(UINT32_MAX);
        }
    }

    res = f_open(fil, "/sd/hello.md", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
    if(res == FR_OK) {
        res = f_write(fil, HELLO_MADOS, HELLO_LEN, &cnt);
        if(res != FR_OK) {
            MAD_LOG("f_write ... failed\n");
        }
        f_close(fil);
        MAD_LOG("f_close ... done\n");
    } else {
        MAD_LOG("f_open ... failed\n");
    }

    res = f_open(fil, "/sd/hello.md", FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if(res == FR_OK) {
        buf = (MadU8*)malloc(BUFF_LEN);
        res = f_read(fil, buf, BUFF_LEN, &cnt);
        if(res != FR_OK) {
            MAD_LOG("f_read ... failed\n");
        } else {
            MAD_LOG("f_read [%d][%s]\n", cnt, buf);
        }
        f_close(fil);
        MAD_LOG("f_close ... done\n");
        free(buf);
    } else {
        MAD_LOG("f_open ... failed\n");
    }
#endif

#if 1
    FILE *fil;
    volatile size_t cnt;
    MadU8 *buf;

    fil = fopen("/sd/hello.md", "w");
    if(fil) {
        cnt = fwrite(HELLO_MADOS, HELLO_LEN, 1, fil);
        fclose(fil);
        MAD_LOG("fwrite [%d / %d]\n", cnt, 1);
    } else {
        MAD_LOG("fopen err\n");
    }

    fil = fopen("/sd/hello.md", "r");
    if(fil) {
        buf = (MadU8*)malloc(BUFF_LEN);
        cnt = fread(buf, HELLO_LEN, 1, fil);
        fclose(fil);
        MAD_LOG("fread [%d][%s]\n", cnt, buf);
        free(buf);
    } else {
        MAD_LOG("fopen err\n");
    }
#endif

    i = 0;
    while(1) {
        if(i++ < WRITE_CNT) {
#if 0
            fil = fopen("/sd/hello.md", "r");
            if(fil) {
                // cnt = fwrite(HELLO_MADOS, HELLO_LEN, 1, fil);
                buf = (MadU8*)malloc(BUFF_LEN);
                cnt = fread(buf, HELLO_LEN, 1, fil);
                fclose(fil);
                MAD_LOG("fread [%d][%s]\n", cnt, buf);
                free(buf);
            } else {
                MAD_LOG("fopen err\n");
                i = WRITE_CNT;
                continue;
            }
#endif

#if 1
            fil = fopen("/sd/hello.md", "w");
            if(fil) {
                // MAD_LOG("fopen ok\n");
                cnt = fwrite(HELLO_MADOS, HELLO_LEN, 1, fil);
                // MAD_LOG("fwrite [%d]\n", cnt);
                fclose(fil);
                // MAD_LOG("fclose ok\n");
            } else {
                MAD_LOG("fopen err\n");
                i = WRITE_CNT;
                continue;
            }
#endif
            if(0 == i%100)
                MAD_LOG("ff [%d / %d]\n", i, WRITE_CNT);
            madTimeDly(10);
        } else {
            madTimeDly(UINT32_MAX);
        }
    }
}
