#ifndef __CFG_USER_H__
#define __CFG_USER_H__

enum {
    THREAD_PRIO_SYS_RUNNING = 1,
    //
    THREAD_PRIO_LWIP_TCPIP,
    THREAD_PRIO_DRIVER_ETH,
};

enum {
    ISR_PRIO_SYSTICK    = 1,
    ISR_PRIO_ARCH_MEM,
    ISR_PRIO_DISK,
	ISR_PRIO_ETH,
    ISR_PRIO_DEV_USART,
    ISR_PRIO_TTY_USART,
    ISR_PRIO_PENDSV     = 15
};

#define SYS_RUNNING_INTERVAL_MSECS (500)
#define MAD_OS_STACK_SIZE          (52 * 1024)
#define MAD_OS_LWIP_DHCP           0

#endif
