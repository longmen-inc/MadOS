#ifndef __MOD_RFID_TY__H__
#define __MOD_RFID_TY__H__

#include "ModLoraCfg.h"
#include "MadDrvRfid.h"

#define RFID_FLAG_PORT      GPIOE
#define RFID_FLAG_PIN       GPIO_Pin_0

#define RFID_RX_INTERVAL    (3)
#define RFID_TX_INTERVAL    (1000 * 15)

#define RFID_TOP_SIZE       (4 + 2)
#define RFID_HEAD_SIZE      (RFID_TOP_SIZE + 4 + 1)
#define RFID_ID_MAX_NUM     ((LORA_RX_BUFF_SIZE - RFID_HEAD_SIZE) / RFID_ID_LEN)
#define RFID_RX_BUFF_SIZE   (RFID_ID_ORGLEN * RFID_RX_MAX_NUM)
#define RFID_ID_BUFF_SIZE   (RFID_ID_LEN * RFID_ID_MAX_NUM)
#define RFID_TX_BUFF_SIZE   (RFID_HEAD_SIZE + RFID_ID_BUFF_SIZE)

#define RFID_HW_VERSION     (15)
#define RFID_SW_VERSION     (15)
#define RFID_CMD_UP         (0x20)
#define RFID_CMD_UP_BURST   (RFID_CMD_UP | 0x01)
#define RFID_CMD_UP_PERIOD  (RFID_CMD_UP | 0x02)
#define RFID_DEV_TYPE       (17)
#define RFID_STAMP          (0)

extern char    rfid_tx_buff[RFID_TX_BUFF_SIZE];

extern MadBool rfid_id_buff_lock    (void);
extern MadBool rfid_id_buff_trylock (void);
extern void    rfid_id_buff_unlock  (void);
extern void    rfid_clear_id_buff   (size_t n);

#endif
