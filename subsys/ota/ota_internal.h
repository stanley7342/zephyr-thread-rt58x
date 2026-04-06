/* Internal API shared between ota_coap.c and ota_flash.c */
#ifndef OTA_INTERNAL_H
#define OTA_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>
#include "ota.h"

/* ota_flash.c exports */
void        ota_state_set(ota_state_t state);
uint32_t    ota_staging_written(void);
bool        ota_version_newer(const uint8_t ver[3]);
int         ota_staging_erase(void);   /* erases and leaves partition open  */
int         ota_staging_open(void);    /* open without erasing (for resume) */
void        ota_staging_close(void);   /* close after download completes    */
int         ota_staging_write(uint32_t offset, const uint8_t *buf, uint16_t len);

#endif /* OTA_INTERNAL_H */
