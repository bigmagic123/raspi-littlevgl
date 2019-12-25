#include "mailbox.h"

#include "bcm283x.h"

#define MAILBOX_BASE    (PER_BASE + 0x0000B880)

#define MAILBOX0_READ   ((volatile uint32_t*)(MAILBOX_BASE + 0x00))
#define MAILBOX0_PEEK   ((volatile uint32_t*)(MAILBOX_BASE + 0x10))
#define MAILBOX0_SENDER ((volatile uint32_t*)(MAILBOX_BASE + 0x14))
#define MAILBOX0_STATUS ((volatile uint32_t*)(MAILBOX_BASE + 0x18))
#define MAILBOX0_CONFIG ((volatile uint32_t*)(MAILBOX_BASE + 0x1C))

#define MAILBOX1_WRITE  ((volatile uint32_t*)(MAILBOX_BASE + 0x20))

#define MAILBOX_FULL    0x80000000
#define MAILBOX_EMPTY   0x40000000

void mailbox_write(uint32_t channel, uint32_t value)
{
    while (*MAILBOX0_STATUS & MAILBOX_FULL);
    *MAILBOX1_WRITE = (value & ~0xF) | (channel & 0xF);
}

uint32_t mailbox_read(uint32_t channel)
{
    while (1)
    {
        while (*MAILBOX0_STATUS & MAILBOX_EMPTY);
        uint32_t read = *MAILBOX0_READ;

        if ((read & 0xF) == channel)
        {
            return read & ~0xF;
        }
    }
}
