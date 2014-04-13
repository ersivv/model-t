
#include <ch.h>
#include <hal.h>

#include "bootloader.h"
#include "bootloader_api.h"
#include "sxfs.h"
#include "dfuse.h"
#include "app_hdr.h"
#include "crc/crc32.h"

#include <chprintf.h>
#include <string.h>


typedef void (*app_entry_t)(void);

extern uint8_t __app_start__[];

typedef enum {
  BOOT_DEFAULT,
  BOOT_LOAD_RECOVERY_IMG,
  BOOT_LOAD_UPDATE_IMG,
} boot_cmd_t;


static void
jump_to_app(uint32_t address);

static void
boot_app(void);

static void
write_app_img(sxfs_part_id_t part);

static void
process_boot_cmd(void);


__attribute__ ((section("bootloader_api")))
const bootloader_api_t _bootloader_api = {
    .get_version = bootloader_get_version,
    .load_recovery_img = bootloader_load_recovery_img,
    .load_update_img = bootloader_load_update_img
};


void
bootloader_exec()
{
  chprintf(SD_STDIO, "BrewBit Model-T Bootloader v%d.%d.%d\r\n",
      MAJOR_VERSION,
      MINOR_VERSION,
      PATCH_VERSION);

  process_boot_cmd();

  chprintf(SD_STDIO, "Booting app... ");
  boot_app();

  /* Uh oh, we should have jumped to the app... */
  write_app_img(SP_RECOVERY_IMG);

  chprintf(SD_STDIO, "Booting recovery image... ");
  boot_app();

  chprintf(SD_STDIO, "Recovery image failed to boot! Halting.\r\n");
  chSysHalt();
}

static void
jump_to_app(uint32_t address)
{
  uint32_t jump_addr = *(volatile uint32_t*)(address + 0x04); /* reset ptr in vector table */
  app_entry_t app_entry = (app_entry_t)jump_addr;

  /* Reset all interrupts to default */
  chSysDisable();

  /* Clear pending interrupts just to be on the safe side */
  SCB_ICSR = ICSR_PENDSVCLR;

  /* Disable all interrupts */
  int i;
  for(i = 0; i < 8; ++i)
    NVIC->ICER[i] = NVIC->IABR[i];

  /* Set stack pointer as in application's vector table */
  __set_MSP(*(volatile uint32_t*)address);
  app_entry();
}

static void
process_boot_cmd()
{
  boot_cmd_t boot_cmd;
  sxfs_read(SP_BOOT_PARAMS, 0, (uint8_t*)&boot_cmd, sizeof(boot_cmd));

  switch (boot_cmd) {
    case BOOT_LOAD_RECOVERY_IMG:
      chprintf(SD_STDIO, "Application has requested to load recovery image\r\n");
      write_app_img(SP_RECOVERY_IMG);
      break;

    case BOOT_LOAD_UPDATE_IMG:
      chprintf(SD_STDIO, "Application has requested to load update image\r\n");
      write_app_img(SP_UPDATE_IMG);
      break;

    case BOOT_DEFAULT:
    default:
      break;
  }
}

static void
boot_app()
{
  if (memcmp(_app_hdr.magic, "BBMT-APP", 8) == 0) {
    chprintf(SD_STDIO, "OK\r\n");
    chprintf(SD_STDIO, "  Version: %d.%d.%d\r\n",
        _app_hdr.major_version,
        _app_hdr.minor_version,
        _app_hdr.patch_version);
    chprintf(SD_STDIO, "  Image Size: %d bytes\r\n", _app_hdr.img_size);
    chprintf(SD_STDIO, "  CRC: 0x%08x\r\n", _app_hdr.crc);

    chprintf(SD_STDIO, "Verifying CRC... ");
    uint32_t crc_calc = crc32_block(0xffffffff, __app_start__, _app_hdr.img_size) ^ 0xffffffff;

    if (crc_calc == _app_hdr.crc) {
      chprintf(SD_STDIO, "OK\r\n");

      chprintf(SD_STDIO, "Starting app.\r\n");
      chThdSleepMilliseconds(100);
      jump_to_app((uint32_t)__app_start__);
    }
    else {
      chprintf(SD_STDIO, "ERROR\r\n");
      chprintf(SD_STDIO, "  Calculated CRC: 0x%08x\r\n", crc_calc);
    }
  }
  else {
    chprintf(SD_STDIO, "ERROR\r\n");
    chprintf(SD_STDIO, "  No valid app found.\r\n");
  }
}

static void
write_app_img(sxfs_part_id_t part)
{
  chprintf(SD_STDIO, "  Writing app image... ");
  if (dfuse_apply_update(part))
    chprintf(SD_STDIO, "OK!\r\n");
  else
    chprintf(SD_STDIO, "FAILED!\r\n");
}

const char*
bootloader_get_version()
{
  return VERSION_STR;
}

static void
save_boot_cmd(boot_cmd_t boot_cmd)
{
  sxfs_write(SP_BOOT_PARAMS, 0, (uint8_t*)&boot_cmd, sizeof(boot_cmd));

  NVIC_SystemReset();
}

void
bootloader_load_recovery_img()
{
  save_boot_cmd(BOOT_LOAD_RECOVERY_IMG);
}

void
bootloader_load_update_img()
{
  save_boot_cmd(BOOT_LOAD_UPDATE_IMG);
}
