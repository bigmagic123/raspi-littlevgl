/*
 * File      : board.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2017, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 */

#include <rthw.h>
#include <rtthread.h>

#include "board.h"
#include "drv_uart.h"
#include "cp15.h"

#define CORE0_TIMER_IRQ_CTRL    HWREG32(0x40000040)

unsigned int cntfrq;

void rt_hw_timer_isr(int vector, void *parameter)
{
    rt_tick_increase();
    mask_cntv();
    __DSB();
    write_cntv_tval(cntfrq);
    __DSB();
    unmask_cntv();
    __DSB();
}

void rt_hw_timer_init(void)
{
    rt_hw_interrupt_install(3, rt_hw_timer_isr, RT_NULL, "tick");
    rt_hw_interrupt_umask(3);
    __DSB();
    //cntfrq = 35000;
    cntfrq = read_cntfrq();
    cntfrq = cntfrq/(RT_TICK_PER_SECOND);
    write_cntv_tval(cntfrq);
    enable_cntv();
    __DSB();
    enable_cpu_timer_intr(rt_hw_cpu_id());
}

void vector_copy(void)
{
    rt_memcpy((void*)0x0, (void*)0x8000, 64);
}

void rt_hw_board_init(void)
{
    /* initialize hardware interrupt */
    rt_hw_interrupt_init();
    vector_copy();

    /* initialize uart */
    rt_hw_uart_init();

#ifdef RT_USING_CONSOLE
    /* set console device */
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif /* RT_USING_CONSOLE */
#ifdef RT_USING_HEAP
    /* initialize memory system */
    rt_kprintf("heap: 0x%08x - 0x%08x\n", RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
    rt_system_heap_init(RT_HW_HEAP_BEGIN, RT_HW_HEAP_END);
#endif

    /* initialize timer for os tick */
    rt_hw_timer_init();
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
