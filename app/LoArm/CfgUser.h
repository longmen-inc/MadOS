#ifndef __CFG_USER_H__
#define __CFG_USER_H__

enum {
    THREAD_PRIO_SYS_RUNNING     = 1,
    THREAD_PRIO_LOARM           = 10,
    THREAD_PRIO_DRIVER_ETH
};

enum {
    ISR_PRIO_SYSTICK    = 1,
    ISR_PRIO_TIMER,
    ISR_PRIO_ARCH_MEM,
	ISR_PRIO_IP101A,
    ISR_PRIO_TTY_USART,
    ISR_PRIO_PENDSV     = 15
};

#define SYS_RUNNING_INTERVAL_MSECS (500)
#define MAD_OS_STACK_SIZE          (56 * 1024)

#endif
