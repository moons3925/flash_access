/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FlashAccessor.h"

#include "hardware/flash.h"
#include "pico/flash.h"

// Flashの定数を読み出す時のポインター
// XIPという技術を使っているため、読み出す時にはXIP_BASE上にオフセットをのせる必要がある

const uint8_t *flash_target_contents =
    (const uint8_t *)(XIP_BASE + FLASH_TARGET_OFFSET);

void FlashAccessor::erase_last_sector() {
    int rc = flash_safe_execute(call_flash_range_erase,
                                (void *)FLASH_TARGET_OFFSET, UINT32_MAX);
    hard_assert(rc == PICO_OK);
}

void FlashAccessor::write_last_sector(uint8_t *ptr) {
    erase_last_sector();
    uintptr_t params[] = {FLASH_TARGET_OFFSET, (uintptr_t)ptr};
    int rc = flash_safe_execute(call_flash_range_program, params, UINT32_MAX);
    hard_assert(rc == PICO_OK);
}

void FlashAccessor::read_last_sector(uint8_t *dst) {
    for (uint i = 0; i < FLASH_SECTOR_SIZE; ++i) {
        dst[i] = flash_target_contents[i];
    }
}

void FlashAccessor::call_flash_range_erase(void *param) {
    uint32_t offset = (uint32_t)param;
    flash_range_erase(offset, FLASH_SECTOR_SIZE);
}

void FlashAccessor::call_flash_range_program(void *param) {
    uint32_t offset = ((uintptr_t *)param)[0];
    const uint8_t *data = (const uint8_t *)((uintptr_t *)param)[1];
    flash_range_program(offset, data, FLASH_SECTOR_SIZE);
}
