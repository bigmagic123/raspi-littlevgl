#pragma once

#include <stdint.h>

/**
 * Writes a message to the mailbox.
 */
void mailbox_write(uint32_t channel, uint32_t value);

/**
 * Reads a response from the mailbox.
 */
uint32_t mailbox_read(uint32_t channel);