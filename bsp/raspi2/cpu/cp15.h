#ifndef __CP15_H__
#define __CP15_H__
#include "bcm283x.h"

#ifndef   __STATIC_FORCEINLINE
#define __STATIC_FORCEINLINE     __attribute__((always_inline)) static inline
#endif

static inline void enable_cpu_timer_intr(int cpu)
{
	CORETIMER_INTCTL(cpu) = 0x8;
}

/**
  \brief   Data Synchronization Barrier
  \details Acts as a special kind of Data Memory Barrier.
           It completes when all explicit memory accesses before this instruction complete.
 */
__STATIC_FORCEINLINE  void __DSB(void)
{
    __asm__ volatile ("dsb 0xF":::"memory");
}

static inline void enable_cntv(void)
{
    rt_uint32_t cntv_ctl;
    cntv_ctl = 1;
    asm volatile ("mcr p15, 0, %0, c14, c3, 1" :: "r"(cntv_ctl) ); // write CNTV_CTL
}

static inline void disable_cntv(void)
{
    rt_uint32_t cntv_ctl;
    cntv_ctl = 0;
    asm volatile ("mcr p15, 0, %0, c14, c3, 1" :: "r"(cntv_ctl) ); // write CNTV_CTL
}

static inline  void mask_cntv(void)
{
    rt_uint32_t cntv_ctl;
    cntv_ctl = 2;
    asm volatile ("mcr p15, 0, %0, c14, c3, 1" :: "r"(cntv_ctl) ); // write CNTV_CTL
}

static inline void unmask_cntv(void)
{
    rt_uint32_t cntv_ctl;
    cntv_ctl = 1;
    asm volatile ("mcr p15, 0, %0, c14, c3, 1" :: "r"(cntv_ctl) ); // write CNTV_CTL
}

static inline rt_uint64_t read_cntvct(void)
{
    rt_uint32_t val,val1;
    asm volatile("mrrc p15, 1, %0, %1, c14" : "=r" (val),"=r" (val1));
    return (val);
}

static inline rt_uint64_t read_cntvoff(void)
{

    rt_uint64_t val;
    asm volatile("mrrc p15, 4, %Q0, %R0, c14" : "=r" (val));
    return (val);
}

static inline rt_uint32_t read_cntv_tval(void)
{
    rt_uint32_t val;
    asm volatile ("mrc p15, 0, %0, c14, c3, 0" : "=r"(val) );
    return val;
}


static inline  void write_cntv_tval(rt_uint32_t val)
{
    asm volatile ("mcr p15, 0, %0, c14, c3, 0" :: "r"(val) );
    return;
}

static inline rt_uint32_t read_cntfrq(void)
{
    rt_uint32_t val;
    asm volatile ("mrc p15, 0, %0, c14, c0, 0" : "=r"(val) );
    return val;
}


static inline  rt_uint32_t read_cntctrl(void)
{
    rt_uint32_t val;
    asm volatile ("mrc p15, 0, %0, c14, c1, 0" : "=r"(val) );
    return val;
}

static inline uint32_t write_cntctrl(uint32_t val)
{

    asm volatile ("mcr p15, 0, %0, c14, c1, 0" : :"r"(val) );
    return val;
}

unsigned long rt_cpu_get_smp_id(void);

int rt_hw_cpu_id(void);

void rt_cpu_mmu_disable(void);
void rt_cpu_mmu_enable(void);
void rt_cpu_tlb_set(volatile unsigned long*);

void rt_cpu_vector_set_base(unsigned int addr);

#endif
