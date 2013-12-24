
#include <ch.h>
#include <hal.h>

#include "ota_update.h"
#include "message.h"
#include "bbmt.pb.h"
#include "web_api.h"
#include "sxfs.h"

#include <stdio.h>


static msg_t
ota_update_thread(void* arg);

static void
ota_update_dispatch(msg_id_t id, void* msg_data, void* user_data);

static void
dispatch_ota_update_start(void);

static void
dispatch_ota_update_chunk(UpdateChunk* update_chunk);


static sxfs_part_t part;


void
ota_update_init()
{
  Thread* update_thd = chThdCreateFromHeap(NULL, 1024, NORMALPRIO, ota_update_thread, NULL);

  msg_subscribe(MSG_OTAU_START, update_thd, ota_update_dispatch, NULL);
  msg_subscribe(MSG_OTAU_CHUNK, update_thd, ota_update_dispatch, NULL);
}

void
ota_update_start()
{
  msg_post(MSG_OTAU_START, NULL);
}

static msg_t
ota_update_thread(void* arg)
{
  (void)arg;

  chRegSetThreadName("ota_update");

  while (1) {
    thread_msg_t* msg = msg_get();

    ota_update_dispatch(msg->id, msg->msg_data, msg->user_data);

    msg_release(msg);
  }

  return 0;
}

static void
ota_update_dispatch(msg_id_t id, void* msg_data, void* user_data)
{
  (void)user_data;

  switch (id) {
  case MSG_OTAU_CHUNK:
    dispatch_ota_update_chunk(msg_data);
    break;

  case MSG_OTAU_START:
    dispatch_ota_update_start();
    break;

  default:
    break;
  }
}

static void
dispatch_ota_update_start()
{
//  msg_send(MSG_OTAU_STATUS, NULL);

  printf("part clear\r\n");

  if (!sxfs_part_clear(SP_OTA_UPDATE_IMG))
    printf("part clear failed\r\n");

  if (!sxfs_part_open(&part, SP_OTA_UPDATE_IMG))
    printf("part open failed\r\n");

  web_api_request_update();
}

static void
dispatch_ota_update_chunk(UpdateChunk* update_chunk)
{
  sxfs_part_write(&part, update_chunk->data.bytes, update_chunk->data.size);

  // if last chunk...
  if (update_chunk->data.size < sizeof(update_chunk->data.bytes)) {
    if (sxfs_part_verify(SP_OTA_UPDATE_IMG)) {
      printf("image verified resetting to apply update...\r\n");

      msg_send(MSG_SHUTDOWN, NULL);

      chThdSleepSeconds(1);

      NVIC_SystemReset();
    }
    else {
      printf("sxfs verify failed\r\n");
    }
  }
}