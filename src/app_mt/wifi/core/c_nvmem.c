/*****************************************************************************
*
*  nvmem.c  - CC3000 Host Driver Implementation.
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/

//*****************************************************************************
//
//! \addtogroup nvmem_api
//! @{
//
//*****************************************************************************

#include <string.h>
#include "nvmem.h"
#include "hci.h"
#include "socket.h"

//*****************************************************************************
//
// Prototypes for the structures for APIs.
//
//*****************************************************************************

#define NVMEM_READ_PARAMS_LEN   (12)
#define NVMEM_CREATE_PARAMS_LEN (8)
#define NVMEM_WRITE_PARAMS_LEN  (16)


//*****************************************************************************
//
//!  nvmem_read
//!
//!  @param  ulFileId   nvmem file id:\n
//!                     NVMEM_NVS_FILEID, NVMEM_NVS_SHADOW_FILEID,
//!                     NVMEM_WLAN_CONFIG_FILEID, NVMEM_WLAN_CONFIG_SHADOW_FILEID,
//!                     NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!                     NVMEM_MAC_FILEID, NVMEM_FRONTEND_VARS_FILEID,
//!                     NVMEM_IP_CONFIG_FILEID, NVMEM_IP_CONFIG_SHADOW_FILEID,
//!                     NVMEM_BOOTLOADER_SP_FILEID, NVMEM_RM_FILEID,
//!                     and user files 12-15.
//!  @param  ulLength    number of bytes to read
//!  @param  ulOffset    ulOffset in file from where to read
//!  @param  buff        output buffer pointer
//!
//!  @return       number of bytes read, otherwise error.
//!
//!  @brief       Reads data from the file referred by the ulFileId parameter.
//!               Reads data from file ulOffset till length. Err if the file can't
//!               be used, is invalid, or if the read is out of bounds.
//!
//*****************************************************************************
signed long c_nvmem_read(uint32_t ulFileId, uint32_t ulLength,
    uint32_t ulOffset, uint8_t *buff)
{
  uint8_t ucStatus = 0xFF;
  uint8_t *args;
  hci_data_read_params_t params;

  args = hci_get_cmd_buffer();

  // Fill in HCI packet structure
  args = UINT32_TO_STREAM(args, ulFileId);
  args = UINT32_TO_STREAM(args, ulLength);
  args = UINT32_TO_STREAM(args, ulOffset);

  // Initiate a HCI command
  hci_command_send(HCI_CMND_NVMEM_READ, NVMEM_READ_PARAMS_LEN,
      HCI_CMND_NVMEM_READ, &ucStatus);

  // In case there is data - read it - even if an error code is returned
  // Note: It is the user responsibility to ignore the data in case of an error code

  // Wait for the data in a synchronous way. Here we assume that the buffer is
  // big enough to store also parameters of nvmem

  params.fromlen = NULL;
  params.from = NULL;
  params.buf = buff;
  hci_wait_for_data(&params);

  return ucStatus;
}

//*****************************************************************************
//
//!  nvmem_write
//!
//!  @param  ulFileId nvmem file id:\n
//!                   NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!                   NVMEM_MAC_FILEID, NVMEM_BOOTLOADER_SP_FILEID,
//!                   and user files 12-15.
//!  @param  ulLength       number of bytes to write
//!  @param  ulEntryOffset  offset in file to start write operation from
//!  @param  buff           data to write
//!
//!  @return       on success 0, error otherwise.
//!
//!  @brief       Write data to nvmem.
//!               writes data to file referred by the ulFileId parameter.
//!               Writes data to file ulOffset till ulLength.The file id will be
//!               marked invalid till the write is done. The file entry doesn't
//!               need to be valid - only allocated.
//!
//*****************************************************************************
signed long c_nvmem_write(uint32_t ulFileId, uint32_t ulLength, uint32_t ulEntryOffset, uint8_t *buff)
{
  long iRes;
  uint8_t *args;

  iRes = EFAIL;

  args = hci_get_data_cmd_buffer();

  // Fill in HCI packet structure
  args = UINT32_TO_STREAM(args, ulFileId);
  args = UINT32_TO_STREAM(args, 12);
  args = UINT32_TO_STREAM(args, ulLength);
  args = UINT32_TO_STREAM(args, ulEntryOffset);

  memcpy(args, buff, ulLength);

  // Initiate a HCI command but it will come on data channel
  hci_data_command_send(
      HCI_CMND_NVMEM_WRITE, NVMEM_WRITE_PARAMS_LEN, ulLength,
      HCI_EVNT_NVMEM_WRITE, &iRes);

  return(iRes);
}


//*****************************************************************************
//
//!  nvmem_set_mac_address
//!
//!  @param  mac   mac address to be set
//!
//!  @return       on success 0, error otherwise.
//!
//!  @brief       Write MAC address to EEPROM.
//!               mac address as appears over the air (OUI first)
//!
//*****************************************************************************
uint8_t c_nvmem_set_mac_address(uint8_t *mac)
{
  return  c_nvmem_write(NVMEM_MAC_FILEID, MAC_ADDR_LEN, 0, mac);
}

//*****************************************************************************
//
//!  nvmem_get_mac_address
//!
//!  @param[out]  mac   mac address
//!
//!  @return       on success 0, error otherwise.
//!
//!  @brief       Read MAC address from EEPROM.
//!               mac address as appears over the air (OUI first)
//!
//*****************************************************************************
uint8_t c_nvmem_get_mac_address(uint8_t *mac)
{
  return  c_nvmem_read(NVMEM_MAC_FILEID, MAC_ADDR_LEN, 0, mac);
}

//*****************************************************************************
//
//!  nvmem_write_patch
//!
//!  @param  ulFileId   nvmem file id:\n
//!                     NVMEM_WLAN_DRIVER_SP_FILEID, NVMEM_WLAN_FW_SP_FILEID,
//!  @param  spLength   number of bytes to write
//!  @param  spData     SP data to write
//!
//!  @return       on success 0, error otherwise.
//!
//!  @brief      program a patch to a specific file ID.
//!              The SP data is assumed to be organized in 2-dimensional.
//!              Each line is SP_PORTION_SIZE bytes long. Actual programming is
//!              applied in SP_PORTION_SIZE bytes portions.
//!
//*****************************************************************************
uint8_t c_nvmem_write_patch(uint32_t ulFileId, uint32_t spLength,
                                          const uint8_t *spData)
{
  long status = 0;
  uint16_t offset = 0;
  uint8_t* spDataPtr = (uint8_t*)spData;

  while ((status == 0) && (spLength >= SP_PORTION_SIZE)) {
    status = c_nvmem_write(ulFileId, SP_PORTION_SIZE, offset, spDataPtr);
    offset += SP_PORTION_SIZE;
    spLength -= SP_PORTION_SIZE;
    spDataPtr += SP_PORTION_SIZE;
  }

  if (status != 0) {
    // NVMEM error occurred
    return status;
  }

  if (spLength != 0) {
    // if reached here, a reminder is left
    status = c_nvmem_write(ulFileId, spLength, offset, spDataPtr);
  }

  return status;
}

//*****************************************************************************
//
//!  nvmem_read_sp_version
//!
//!  @param[out]  patchVer    first number indicates package ID and the second
//!                           number indicates package build number
//!
//!  @return       on success  0, error otherwise.
//!
//!  @brief      Read patch version. read package version (WiFi FW patch,
//!              driver-supplicant-NS patch, bootloader patch)
//!
//*****************************************************************************
uint8_t c_nvmem_read_sp_version(nvmem_sp_version_t* sp_version)
{
  uint8_t retBuf[5];

  // Initiate a HCI command, no args are required
  hci_command_send(HCI_CMND_READ_SP_VERSION, 0,
      HCI_CMND_READ_SP_VERSION, retBuf);

  sp_version->package_id = retBuf[3];
  sp_version->package_build = retBuf[4];

  return retBuf[0];
}

//*****************************************************************************
//
//!  nvmem_create_entry
//!
//!  @param       ulFileId    nvmem file Id:\n
//!                           * NVMEM_AES128_KEY_FILEID: 12
//!                           * NVMEM_SHARED_MEM_FILEID: 13
//!                           * and fileIDs 14 and 15
//!  @param       ulNewLen    entry ulLength
//!
//!  @return       on success 0, error otherwise.
//!
//!  @brief      Create new file entry and allocate space on the NVMEM.
//!              Applies only to user files.
//!              Modify the size of file.
//!              If the entry is unallocated - allocate it to size
//!              ulNewLen (marked invalid).
//!              If it is allocated then deallocate it first.
//!              To just mark the file as invalid without resizing -
//!              set ulNewLen=0.
//!
//*****************************************************************************
signed long c_nvmem_create_entry(uint32_t ulFileId, uint32_t ulNewLen)
{
  uint8_t *args;
  uint16_t retval;

  args = hci_get_cmd_buffer();

  // Fill in HCI packet structure
  args = UINT32_TO_STREAM(args, ulFileId);
  args = UINT32_TO_STREAM(args, ulNewLen);

  // Initiate a HCI command
  hci_command_send(HCI_CMND_NVMEM_CREATE_ENTRY, NVMEM_CREATE_PARAMS_LEN,
      HCI_CMND_NVMEM_CREATE_ENTRY, &retval);

  return(retval);
}



//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

