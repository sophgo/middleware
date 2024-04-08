/*-----------------------------------------------------------------------
//
// Copyright 2014 - 2018 Synopsys, Inc.
//
// This Synopsys software and all associated documentation
// are proprietary to Synopsys, Inc. and may only be used pursuant to the
// terms and conditions of a written license agreement with Synopsys,
// Inc. All other use, reproduction, modification, or distribution of the
// Synopsys software or the associated documentation
// is strictly prohibited.
//
//-----------------------------------------------------------------------
// Project:
//
// Host Library.
//
// Description:
//
// Sample application for HDCP Transmitter.
//
//-----------------------------------------------------------------------*/

/**
 * \defgroup TxSample Tx Sample Application
 * \ingroup SampleApps
 * \brief Sample Application for the HDCP Transmitter.
 *
 * @{
 *
 * \code
 */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>

#include "HostLib.h"
#include "HostLibDriver.h"
#include "HostLibHdcp.h"
#include "HostLibHdcpTx.h"
#ifdef TROOT_POWER_UPDATE
#include "HostLibPowerUpdate.h"
#endif
#include "ipc_msg.h"

//
// Define the firmware initialization parameters.
//
// In this example we point to the address where the firmware is located on a Zynq FPGA board.
// For the IRQ in this example we are not using one so set to 0 for polling mode.
//

//
// Hardware addresses.
//

// Firmware address.
#define HPI_ADDRESS_ESM0     0xD0072000
#define HDCP_FW_ADDRESS      0xD0100000
#define HDCP_FW_SIZE         0x4B000
#define HDCP_DATA_ADDRESS    0xD0150000
#define HDCP_DATA_SIZE       0x20000

#define FIRMWARE_PATH		"/root/firmware.le"

#define VARNAME(a) #a

char* get_error_name(int error) {
   switch(error) {
      case HL_HDCP_SUCCESS:
         return VARNAME(HL_HDCP_SUCCESS);
      case HL_HDCP_ERROR_HDCP_NOT_CAPABLE:
         return VARNAME(HL_HDCP_ERROR_HDCP_NOT_CAPABLE);
      case HL_HDCP_ERROR_RECV_NACK:
         return VARNAME(HL_HDCP_ERROR_RECV_NACK);
      case HL_HDCP_COMM_ERROR_TIMEOUT:
         return VARNAME(HL_HDCP_COMM_ERROR_TIMEOUT);
      case HL_HDCP_COMM_ERROR:
         return VARNAME(HL_HDCP_COMM_ERROR);
      case HL_HDCP_AKE_MSG_FAILED:
         return VARNAME(HL_HDCP_AKE_MSG_FAILED);
      case HL_HDCP_AKE_ERROR:
         return VARNAME(HL_HDCP_AKE_ERROR);
      case HL_HDCP_ERROR_AKE_SEND_AKEINIT_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_SEND_AKEINIT_FAILED);
      case HL_HDCP_ERROR_AKE_SEND_CERT_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_SEND_CERT_FAILED);
      case HL_HDCP_ERROR_AKE_GET_KM_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_GET_KM_FAILED);
      case HL_HDCP_ERROR_AKE_RECV_SEND_H_PRIME_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_RECV_SEND_H_PRIME_FAILED);
      case HL_HDCP_ERROR_AKE_RECV_SEND_PAIRING_INFO_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_RECV_SEND_PAIRING_INFO_FAILED);
      case HL_HDCP_ERROR_AKE_LC_INIT_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_LC_INIT_FAILED);
      case HL_HDCP_ERROR_AKE_LC_RECV_L_PRIME_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_LC_RECV_L_PRIME_FAILED);
      case HL_HDCP_ERROR_AKE_SEND_EKS_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_SEND_EKS_FAILED);
      case HL_HDCP_ERROR_AKE_SEND_RECEIVER_ID_LIST_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_SEND_RECEIVER_ID_LIST_FAILED);
      case HL_HDCP_ERROR_AKE_SEND_CSM_INFO_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_SEND_CSM_INFO_FAILED);
      case HL_HDCP_ERROR_AKE_RECV_CSM_INFO_FAILED:
         return VARNAME(HL_HDCP_ERROR_AKE_RECV_CSM_INFO_FAILED);
      case HL_HDCP_ERROR_HDCP_AKE_DONE_FAILED:
         return VARNAME(HL_HDCP_ERROR_HDCP_AKE_DONE_FAILED);
      case HL_HDCP_TX_CAPABILITY_TIMER_EXPIRED:
         return VARNAME(HL_HDCP_TX_CAPABILITY_TIMER_EXPIRED);
      case HL_HDCP_TX_LVC_EXPIRED:
         return VARNAME(HL_HDCP_TX_LVC_EXPIRED);
      case HL_HDCP_LINK_INTEGRITY_DETECTED:
         return VARNAME(HL_HDCP_LINK_INTEGRITY_DETECTED);
      case HL_HDCP_NOTIFY_COMPLETED_W_ERRORS:
         return VARNAME(HL_HDCP_NOTIFY_COMPLETED_W_ERRORS);
      case HL_TROOT_HDCP_SYSEXCEPT_HPD_LOW:
         return VARNAME(HL_TROOT_HDCP_SYSEXCEPT_HPD_LOW);
      case HL_TROOT_HDCP_SYSEXCEPT_AKE_INIT:
         return VARNAME(HL_TROOT_HDCP_SYSEXCEPT_AKE_INIT);
      case HL_HDCP_ABORTED:
         return VARNAME(HL_HDCP_ABORTED);
      case HL_HDCP_TEST_CP_IRQ_TIMER_EXPIRED:
         return VARNAME(HL_HDCP_TEST_CP_IRQ_TIMER_EXPIRED);
      case HL_HDCP_DP_QSES_SSS_COMPUTATION_FAILED:
         return VARNAME(HL_HDCP_DP_QSES_SSS_COMPUTATION_FAILED);
      case HL_HDCP_DP_QSES_SSS_VERIFICATION_MISMATCH:
         return VARNAME(HL_HDCP_DP_QSES_SSS_VERIFICATION_MISMATCH);
      case HL_HDCP_NOT_AUTHENTICATED:
         return VARNAME(HL_HDCP_NOT_AUTHENTICATED);
      case HL_FAILED:
         return VARNAME(HL_FAILED);
      case HL_INVALID_PARAMETERS:
         return VARNAME(HL_INVALID_PARAMETERS);
      case HL_MB_FAILED:
         return VARNAME(HL_MB_FAILED);
      case HL_REQ_RESP_MISMATCH:
         return VARNAME(HL_REQ_RESP_MISMATCH);
      case HL_COMMAND_TIMEOUT:
         return VARNAME(HL_COMMAND_TIMEOUT);
      case HL_FAILED_TO_LOAD_CODE:
         return VARNAME(HL_FAILED_TO_LOAD_CODE);
      case HL_FAILED_TO_SET_CODE_ADDR:
         return VARNAME(HL_FAILED_TO_SET_CODE_ADDR);
      case HL_FAILED_TO_SET_DATA_ADDR:
         return VARNAME(HL_FAILED_TO_SET_DATA_ADDR);
      case HL_FAILED_TO_SET_VLD_BIT:
         return VARNAME(HL_FAILED_TO_SET_VLD_BIT);
      case HL_FAILED_TO_RD_CONFIG_DATA:
         return VARNAME(HL_FAILED_TO_RD_CONFIG_DATA);
      case HL_FAILED_TO_PARSE_CONFIG_DATA:
         return VARNAME(HL_FAILED_TO_PARSE_CONFIG_DATA);
      case HL_BUFFER_TOO_SMALL:
         return VARNAME(HL_BUFFER_TOO_SMALL);
      case HL_INVALID_COMMAND:
         return VARNAME(HL_INVALID_COMMAND);
      case HL_NO_INSTANCE:
         return VARNAME(HL_NO_INSTANCE);
      case HL_DRIVER_DATA_WRITE_FAILED:
         return VARNAME(HL_DRIVER_DATA_WRITE_FAILED);
      case HL_DRIVER_HPI_WRITE_FAILED:
         return VARNAME(HL_DRIVER_HPI_WRITE_FAILED);
      case HL_DRIVER_HPI_READ_FAILED:
         return VARNAME(HL_DRIVER_HPI_READ_FAILED);
      case HL_INVALID_NUMBER_OF_CFG_ENTRIES:
         return VARNAME(HL_INVALID_NUMBER_OF_CFG_ENTRIES);
      case HL_BAD_AUID:
         return VARNAME(HL_BAD_AUID);
      case HL_COMMAND_REQ_IN_PROGRESS:
         return VARNAME(HL_COMMAND_REQ_IN_PROGRESS);
      case HL_GET_CONFIGURATION_FAILED:
         return VARNAME(HL_GET_CONFIGURATION_FAILED);
      case HL_SET_EXT_BASE_ADDRESS_FAILED:
         return VARNAME(HL_SET_EXT_BASE_ADDRESS_FAILED);
      case HL_EXCEPTION_TYPE_FAILED:
         return VARNAME(HL_EXCEPTION_TYPE_FAILED);
      case HL_FAILED_RD_STATUS_REGISTER:
         return VARNAME(HL_FAILED_RD_STATUS_REGISTER);
      case HL_FAILED_INVALID_EXCEPTION:
         return VARNAME(HL_FAILED_INVALID_EXCEPTION);
      case HL_FAILED_INVALID_EXCEPTION_VAL:
         return VARNAME(HL_FAILED_INVALID_EXCEPTION_VAL);
      case HL_INVALID_MEM_DATA:
         return VARNAME(HL_INVALID_MEM_DATA);
      case HL_INVALID_BASE_CONFIGURATION:
         return VARNAME(HL_INVALID_BASE_CONFIGURATION);
      case HL_INVALID_BASE_CONFIG_START_OFFSET:
         return VARNAME(HL_INVALID_BASE_CONFIG_START_OFFSET);
      default:
         return "UNKNOWN";
   }
}

// Authentications states.
enum
{
   STATE_INIT,
   STATE_RELOAD_CFG,
   STATE_HLC_INITIALIZE,
   STATE_UNCONNECTED,
   STATE_CONNECTED,
   STATE_START_AUTH,
   STATE_PRE_AUTH,
   STATE_AUTH,
   STATE_DONE,
   STATE_NOT_CAPABLE,
   STATE_KILL,
   STATE_KILL_DONE,
   STATE_HDCP_REAUTHENTICATE,
};

//
// Define firmware Linux device driver name.
//
static const char *device_name = "/dev/hl_dev";

//
// Platform specific function implementations.
// -------------------------------------------
//
// The following implementations must be provided to run the Host Library:
//    -Host Library driver.
//    -Common Abstration Layer:
//       -ELP_MSLEEP()
//       -ELP_LOG_INIT(), ELP_LOG_END()
//

//
// Implement platform specific polling functions.
// ----------------------------------------------
//
// Sample polling notification handler. This must be implemented in the application and
// passed into the Host Library by calling "blockingData.block_func = <polling function>".
// The ELP_MSLEEP() is a macro defined in "elliptic_types.h". The implementation of this is
// platform specific. This function is used when sending HPI commands.
//
static int g_force_timeout = 0;
static int proc_hdmi_hpd = 0;
static uint16_t fsm, prev_fsm;

ELP_STATUS platform_specific_poll_notification(hl_instance_t *hlInst, uint16_t const overridetimeout)
{
   int32_t _tmo;

   // Verify parameters have valid values.
   if ((hlInst == 0) || (HL_GET_POLL_INSTANCE(hlInst) == 0))
   {
      return HL_NO_INSTANCE;
   }

   // Set a timeout loop counter to be decremented on each poll iteration.
   // "overridetimeout" this value was set by HL_SET_TIMEOUT() [see below]
   _tmo = overridetimeout;

   // Loop until HLC_ProcessNotification indicates command has completed, or
   // the timeout has expired.
   while (_tmo > 0)
   {
      if (g_force_timeout){
         printf(">> Forced timeout (1)\n");
         //GP: 27/02/2019 Changed return value, because it causes the lib to freeze 
         //return HL_COMMAND_TIMEOUT;
         return HL_SUCCESS;
      }
      if (HLC_ProcessNotification(hlInst) == HL_DONE)
      {
         return HL_SUCCESS;
      }
      ELP_MSLEEP(HL_GET_SLEEP(hlInst));
      // Decrement the loop counter, it will eventually reach zero when the
      // timeout has expired, terminating this loop.
      if (overridetimeout != 0)
      {
         _tmo = _tmo - HL_GET_SLEEP(hlInst);
      }
   }

   return HL_COMMAND_TIMEOUT;
}

// Sample polling monitoring status handler. This must be implemented in the application and
// passed into the Host Library by calling "blockingData.block_monitor_func = <polling function>".
// The implementation of this is platform specific. This function is used when monitoring exceptions
// from the firmware.
//
ELP_STATUS platform_specific_poll_monitor_status(hl_instance_t *hlInst, hl_auid auid,
                                                 uint16_t const overridetimeout)
{
   int32_t _tmo;

   if ((hlInst == 0) || (HL_GET_POLL_INSTANCE(hlInst) == 0))
   {
      return HL_NO_INSTANCE;
   }

   if (overridetimeout == HL_NO_TIMEOUT)
   {
      _tmo = HL_NO_TIMEOUT + 1;            // Do not timeout.
   }
   else if (overridetimeout == HL_DEFAULT_TIMEOUT)
   {
      _tmo = HL_GET_TIMEOUT(hlInst);
   }
   else
   {
      _tmo = overridetimeout;
   }

   while (_tmo > HL_NO_TIMEOUT)
   {
      if (g_force_timeout){
         printf(">> Forced timeout (2)\n");
         return HL_COMMAND_TIMEOUT;
      }
      if (HostLib_CheckAppInterrupt(hlInst, auid) == HL_TRUE)
      {
         printf("Process notification successful APP_ID %d\n", auid);
         return HL_SUCCESS;
      }
      ELP_MSLEEP(HL_GET_SLEEP(hlInst));
      if (overridetimeout != HL_NO_TIMEOUT)
      {
#ifdef LINUX_OS_OFF_TARGET
         _tmo = _tmo - 200;
         usleep(100000);
#else
         _tmo = _tmo - HL_GET_SLEEP(hlInst);
#endif
      }
   }

   printf("Monitor timed out\n");
   return HL_COMMAND_TIMEOUT;
}

//
// Stub function to check status of Connected bit. For this sample application we
// wait for user input, then always return 1. Return non-zero if the Connected signal
// is high. This should be replaced by an actual connection check in your system.
//
uint8_t check_connected(void)
{
   int ret = 0;
   int tmp = 0;

   tmp = read(proc_hdmi_hpd, &ret, sizeof(ret));
   if (tmp != sizeof(ret))
      printf("\n");

   return (ret & 1);
}

uint8_t is_authenticated(hl_instance_t *hl_inst, hl_auid auid, uint16_t timeout,
		hl_blocking_data_t *blockingData)
{
   ELP_STATUS error;

   error = HLC_HDCPTX_Monitor(hl_inst, auid, timeout, blockingData);

   if ((error == HL_HDCP_NOTIFY_COMPLETED) ||
       (error == HL_COMMAND_TIMEOUT) ||
       (error == HL_HDCP_CSM_UPDATED) ||
       (error == HL_TROOT_HDCP_SYSEXCEPT_AKE_INIT) ||
       (error == HL_FAILED_BLOCK_FUNC))
      return 1;

   printf("[TX] Monitor returned: %d [%s]\n", error, get_error_name(error));
   printf("[TX] Authentication dropped\n");
   return 0;
}

int* set_v_address(unsigned int address, int fd, unsigned int value)
{

   unsigned     p_need, p_size;
   off_t	m_address, offset; // Mapped Address
   int		size=4;
   char *	map;
   void *	v_address; // Virtual Address
   int *	temp;

   p_size = sysconf(_SC_PAGE_SIZE);

   // mapped address, for mmap() must be page aligned
   m_address = address & ~(off_t)(p_size - 1);
   // offset in one page
   offset = address & (p_size - 1);
   // pages needed
   p_need = (offset + size) & ~(p_size-1);

   if((( offset + size ) & (p_size-1)) != 0){
      // size needed = number of complete pages + 1
      p_need += p_size;
   }
   /*
   PROT_EXEC  Pages may be executed.
   PROT_READ  Pages may be read.
   PROT_WRITE Pages may be written.
   PROT_NONE  Pages may not be accessed.

   MAP_SHARED  Changes are shared.
   MAP_PRIVATE Changes are private.
   MAP_FIXED Interpret addr exactly.
   */

   map = mmap(NULL, p_need, PROT_READ|PROT_WRITE, MAP_SHARED, fd, m_address);

   if (map == MAP_FAILED)
      return MAP_FAILED;

   v_address = (char* ) map + (unsigned)offset;
   *(int *)(v_address) = value;
   temp = (int*) *(off_t *)(v_address);
   munmap(map, p_need);

   return temp;
}

int reg_write(unsigned int address, unsigned int value){

   int fd = open ("/dev/mem", O_RDWR | O_SYNC);

   if (fd == -1 ){
      // Provides access to the computer's physical memory.
      return fd;
   }

   __attribute__((unused)) int *temp = set_v_address((off_t)address, fd, value);

   return 0;
}

void write_keys()
{
   int i = 0;

   uint32_t keys [12] = {
      // Nonce
      0x123a4712, 0x1234787b, 0x98172347, 0x823473ab,
      // PKf = 0x00112233445566778899aabbccddeeff
      0xccddeeff, 0x8899aabb, 0x44556677, 0x00112233,
      // DUK = 0xffeeddccbbaa99887766554433221100
      0x33221100, 0x77665544, 0xbbaa9988, 0xffeeddcc
   };
   uint32_t key_addr[12] = {
      0xd0070024, 0xd0070028, 0xd007002c, 0xd0070030,
      0xd0070004, 0xd0070008, 0xd007000c, 0xd0070010,
      0xd0070014, 0xd0070018, 0xd007001c, 0xd0070020
   };

   // Write the PKf, DUK and nonce
   // Write to the iskp_nonce registers
   for(i=0; i < 12; i++){
      reg_write(key_addr[i], keys[i]);
   }
}

/**
 * @brief This thread is responsible for monitoring IPC by the HDMI TX
 * app and raise kill requests whenever they appear.
 * On hdmi tx kernel implementation, this thread must be reimplemented
 * to check hdcp driver kill request.
 * 
 * @param vargp hostlib driver instance
 * @return void* 
 */
void *reset_thread_fun(void *vargp)
{
   int ret;
   ELP_STATUS error;
   enum ipc_status status;
   hl_driver_t *hldrv_inst = (hl_driver_t*)vargp;
   uint32_t kill_req;

   printf("[RESET THR] kill_req_monitor started.\n");
   while (1) {
      // Get the hdmi status
      ret = get_hdmi_status();
      if(ret<0){
         if (ret == -ENOMSG){
            continue;
         }
         printf("[RESET THR] Failed getting hdmitx status: err=%d\n", ret);
         continue;
      }

      status = ret;
      if (status == STATUS_HDMI_HDCP_REAUTH_REQUEST) {
         printf("[RESET THR] HDCP check HPD request!\n");
         fsm = STATE_HDCP_REAUTHENTICATE;
         g_force_timeout = 1;
         continue;
      }

      if (status == STATUS_HDMI_KILL_REQUEST){
         printf("[RESET THR] HDCP Firmware kill request!\n");
         g_force_timeout = 1;
         hldrv_inst->set_kill_req(hldrv_inst->instance, 1);
      }
      else {
         printf("[RESET THR] FAIL: Received invalid status %d\n", status);
         continue;
      }

      // wait for kill done
      kill_req = 1;
      while(kill_req){
         if ((error = hldrv_inst->get_kill_status(hldrv_inst->instance, &kill_req)) != HL_DRIVER_SUCCESS){
            fprintf(stderr, "[RESET THR] Failed checking if kill_req: %d [%s]\n", error, get_error_name(error));
         }
      }

      printf("[RESET THR] Informing hdmi that the firmware was killed...\n");
      ret = send_hdcp_status(STATUS_HDCP_FIRMWARE_KILLED);
      if (ret<0){
         printf("[RESET THR] Failed sending status with error: %s\n", strerror(-ret));
      }

      printf("[RESET THR] Getting hdmi status...\n");
      ret = get_hdmi_status();
      if (ret<0){
         printf("[RESET THR] Failed getting status with error: %s\n", strerror(-ret));
      }
      status = ret;
      switch (status){
         case STATUS_HDMI_LOAD_REQUEST:
            printf("[RESET THR] Hostlib can be reinitialized.\n");
            hldrv_inst->set_load_req(hldrv_inst->instance);
            break;
         default:
            printf("[RESET THR] FAIL: Received invalid status %d\n", status);
            break;
      }
   }
   printf("[RESET THR] kill_req_monitor done\n");
   return NULL;
}

ELP_STATUS tx_application(hl_driver_t *hldrv_inst, void *mylog, uint8_t *image, uint32_t image_size, hl_auid auid)
{
   ELP_STATUS     error;
   uint32_t       content_type;
   uint8_t        stream_id;
   hl_ex_status_t exStatus;
   hl_csm_info_t  csm;
   uint32_t       app_hdcp_state;
   uint16_t       monitor_timeout;
   uint8_t        connected = 0;
   uint8_t        protocol = HL_HDCP_PROTOCOL_HDMI;

   uint32_t code_load_req;
   uint32_t kill_req;

   hl_instance_t hl_inst;
   hl_polling_info_t hlpoll_inst;
   hl_blocking_data_t blockingData;

   // Initialize all variables.
   error = HL_FAILED;
   fsm = STATE_INIT;
   prev_fsm = fsm;
   content_type = 0;
   stream_id = 0;
   memset(&exStatus, 0, sizeof(hl_ex_status_t));
   memset(&csm, 0, sizeof(hl_csm_info_t));
   app_hdcp_state = HL_HDCP_STATE_RESET;
   monitor_timeout = (65 * 1000);     // Time to wait for TxMonitor to detect notification.
   memset(&hl_inst, 0, sizeof(hl_instance_t));
   memset(&hlpoll_inst, 0, sizeof(hl_polling_info_t));
   memset(&blockingData, 0, sizeof(hl_blocking_data_t));
   HL_SET_TIMEOUT(&hlpoll_inst, 1000); // Use 1 second.

   //
   // Load SRM (if required)
   // -----------------------
   //
   // Prior to running the authentication, the configuration might require an SRM to be loaded.
   // The configuration parameter is [SRM_VERSION] and is located in troot_hdcpmgr.cfg.
   //
   // ELP_STATUS HLC_HDCPTX_LoadSRM(hl_instance_t *hlInst, uint8_t *buf, uint32_t size,
   //                               hl_blocking_data_t *blockingData)
   //

   //
   // Load Pairing (if required)
   // --------------------------
   //
   // Loading the Pairing information (from a HLC_HDCPTX_SavePairing() called before) must be done
   // before starting. This data was stored off-line somewhere and reloaded for this authentication.
   // Please make sure Pairing is enabled in the configuration file. The configuration parameter
   // is [PAIRING_ENABLED] and is located in troot_hdcptx.cfg.
   //
   // ELP_STATUS HLC_HDCPTX_LoadPairing(hl_instance_t *hlInst, uint8_t *buf, uint32_t size)
   //
   // If pairing is not enabled in the configuration, you can enable during runtime by calling
   //
   // ELP_STATUS HLC_HDCPTX_EnablePairing(hl_instance_t *hlInst, uint8_t OnOff, uint32_t ctrl_flag,
   //                                     hl_blocking_data_t *blockingData)
   //

   while (1)
   {
      usleep(100000);
      //
      // Default timeouts.
      //
      HL_SET_TIMEOUT(&hlpoll_inst, 1000); // Use 1 second.
      monitor_timeout = (65 * 1000);     // Time to wait for TxMonitor to detect notification.

      //
      // Check connected bit.
      // --------------------
      //
      // Check and make sure the connected bit is asserted. This indicates that a RX downstream
      // device is connected and ready to communicate.
      //

      connected = check_connected();

      if ((error = hldrv_inst->get_kill_status(hldrv_inst->instance, &kill_req)) != HL_DRIVER_SUCCESS){
            fprintf(stderr, "Failed checking if kill_req: %d [%s]\n", error, get_error_name(error));
            break;
      }

      switch (fsm)
      {
         case STATE_UNCONNECTED:
         case STATE_CONNECTED:
         case STATE_PRE_AUTH:
         case STATE_AUTH:
         case STATE_DONE:
         case STATE_NOT_CAPABLE:
            if (kill_req){
               printf("[TX] Kill request received...\n");
               fsm = STATE_KILL;
               break;
            }
            break;
         default:
            break;
      }

      switch (fsm)
      {
         case STATE_INIT:
            printf("[TX] STATE_INIT\n");
            prev_fsm = fsm;
            // Application is asking the driver if the firmware is already loaded
            if ((error = hldrv_inst->get_load_status(hldrv_inst->instance, &code_load_req)) != HL_DRIVER_SUCCESS){
               fprintf(stderr, "Failed checking if code is loaded: %d [%s]\n", error, get_error_name(error));
               break;
            }
            if (code_load_req){
               fsm = STATE_HLC_INITIALIZE;
            }
            else {
               fsm = STATE_RELOAD_CFG;
            }
            break;

         case STATE_HLC_INITIALIZE:
            printf("[TX] STATE_HLC_INITIALIZE\n");
            prev_fsm = fsm;
            // Initialize the keys
            write_keys();
            //
            // Initialize the polling instances.
            // ---------------------------------
            // Set up the timing for polling events (default setup).
            //
            HL_SET_SLEEP(&hlpoll_inst, 10); // Use 10 ms.
            HL_SET_TIMEOUT(&hlpoll_inst, 10000); // Use 10s.

            //
            // Load and start the firmware. When completed the firmware will have booted and is ready to
            // accept commands. Function details are provided in the API documentation.
            //
            if ((error = HLC_Initialize(&hl_inst, mylog, image, image_size, hldrv_inst, &hlpoll_inst)) != HL_SUCCESS)
            {
               fprintf(stderr, "Failed HLC_Initialize, error %d [%s]\n", error, get_error_name(error));
               // Close Host Library session, close Host Library driver and release allocated memory.
               break;
            }

            // Set the interrupts that the firmware accept/acknowledge.
            // Function details are provided in the API documentation.
            if ((error = HLC_EnableDisableInterrupts(&hl_inst, 0, HL_IRQ_ENABLE_ALL)) != HL_SUCCESS)
            {
               fprintf(stderr, "Failed HLC_EnableDisableInterrupts, error %d [%s]\n", error, get_error_name(error));
               break;
            }

            // Initialize the blocking data type with your platform specific polling functions.
            blockingData.block_func = platform_specific_poll_notification;
            blockingData.block_monitor_func = platform_specific_poll_monitor_status;

            //
            // Enable logging in the firmware (if needed).
            //
            if ((error = HLC_LogControl(&hl_inst, 1, 0, &blockingData)) != HL_SUCCESS)
            {
               fprintf(stderr, "Failed HLC_LogControl, error %d [%s]\n", error, get_error_name(error));
               break;
            }

            printf("Firmware HDCP TX application initialized.\n");
            printf("TX APP_ID %d\n", auid);

#ifdef TROOT_POWER_UPDATE
            // Note:  The power update feature is a purchased feature.
            //		If the feature is not available, the system will power up automatically.
            // Power Up the Firmware.
            if ((error = HLC_Power_Update(&hl_inst, HL_BASE_SYSTEM_AUID, HL_PWR_UP, &blockingData)) != HL_SUCCESS)
            {
               fprintf(stderr, "Failed HLC_LogControl, error %d [%s]\n", error, get_error_name(error));
               break;
            }
#endif

            //
            // Set up the Communication I2C Settings
            // ------------------------------------
            //
            // The DDC bit rate may optionally be configured for i2c. Only applies to HDMI interface.
            //
            // ELP_STATUS HLC_HDCPTX_SetI2CFrequency(hl_instance_t *hlInst, hl_auid app_auid, uint32_t freq,
            //													hl_blocking_data_t *blockingData)
            //
            // Enable Short Reads (used for RXSTATUS reads for downstream devices)
            //
            // ELP_STATUS HLC_HDCPTX_SetI2CShortRead(hl_instance_t *hlInst, hl_auid app_auid, uint32_t on_off,
            //													hl_blocking_data_t *blockingData);
            //
            fsm = STATE_UNCONNECTED;
            break;
         case STATE_RELOAD_CFG:
            printf("[TX] STATE_RELOAD_CFG\n");
            prev_fsm = fsm;

            if ((error = HLC_ReloadConfiguration(&hl_inst, mylog, hldrv_inst, &hlpoll_inst)) != HL_SUCCESS)
            {
               fprintf(stderr, "Failed HLC_ReloadConfiguration, error %d [%s]\n", error, get_error_name(error));
               break;
            }

            fsm=STATE_UNCONNECTED;
            break;
         case STATE_KILL:
            printf("[TX] STATE_KILL\n");
            prev_fsm = fsm;
            g_force_timeout = 0;
            HLC_Kill(&hl_inst, HL_END_SESSION);
            usleep(10000);

            if ((error = hldrv_inst->set_kill_req(hldrv_inst->instance, 0)) != HL_DRIVER_SUCCESS){
               fprintf(stderr, "Setting kill done: %d [%s]\n", error, get_error_name(error));
               break;
            }

            fsm=STATE_KILL_DONE;
            break;
         case STATE_KILL_DONE:
            printf("[TX] STATE_KILL_DONE\n");
            prev_fsm = fsm;

            if ((error = hldrv_inst->get_load_status(hldrv_inst->instance, &code_load_req)) != HL_DRIVER_SUCCESS){
               fprintf(stderr, "Failed checking if code is loaded: %d [%s]\n", error, get_error_name(error));
               break;
            }
            if (code_load_req){
               fsm = STATE_HLC_INITIALIZE;
            }
            break;
         case STATE_UNCONNECTED:
            if (fsm != prev_fsm) printf("[TX] STATE_UNCONNECTED\n");
            prev_fsm = fsm;

            if (connected)
            {
               printf("[TX] Connection detected\n");
               fsm = STATE_CONNECTED;
            }
            break;

         case STATE_CONNECTED:
            printf("[TX] STATE_CONNECTED\n");
            prev_fsm = fsm;

            //
            // Resetting the firmware.
            // -----------------------
            //
            // You must call HLC_Reset() before starting. This will reset the internal software states.
            // This will clear all internal states of the system which include: Capability states and
            // Authentication states.
            // Function details are provided in the API documentation.
            // Type: [Mandatory]
            //
            error = HLC_Reset(&hl_inst, auid, &blockingData);
            if (error != HL_SUCCESS)
            {
               printf("[TX] HLC_Reset: %d [%s]\n", error, get_error_name(error));
               break;
            }

            //
            // Confirm state by getting the HDCP TX state. You do not need to call this every time. This
            // is here to demonstrate what each state represents.
            // Type: [Optional]
            //
            error = HLC_HDCP_GetState(&hl_inst, auid, &app_hdcp_state, &blockingData);
            if (error != HL_SUCCESS)
            {
               printf("[TX] HLC_HDCP_GetState: %d [%s]\n", error, get_error_name(error));
               fsm = STATE_CONNECTED;
               break;
            }

            if (app_hdcp_state != HL_HDCP_STATE_RESET && app_hdcp_state != HL_HDCP_STATE_DISCONNECTED)
            {
               printf("[TX] HDCP App is in invalid state [%d]\n", app_hdcp_state);
               fsm = STATE_CONNECTED;
               break;
            }

            //
            // Set up the HDCP protocol.
            // -------------------------
            //
            // Establish which protocol you need:
            //    -Protocol HDMI = HL_HDCP_PROTOCOL_HDMI
            //    -Protocol DP   = HL_HDCP_PROTOCOL_DP
            // Must be in the RESET state.
            //
            error = HLC_HDCP_SetProtocol(&hl_inst, auid, protocol, &blockingData);
            if (error != HL_SUCCESS)
            {
               printf("[TX] HLC_HDCP_SetProtocol: %d [%s]\n", error, get_error_name(error));
               fsm = STATE_CONNECTED;
               break;
            }

            //
            // Start Low Value Content early (if required).
            // --------------------------------------------
            //
            // The following function will allow to pass the low value content earlier, prior to checking
            // capability. Note, that there are restrictions to using this function. Please refer to the
            // user guide for more information.
            // Procedure: [Optional]
            //
            // ELP_STATUS HLC_HDCPTX_EnableLowValueContent(hl_instance_t *hlInst, hl_auid app_auid,
            //                                             hl_blocking_data_t *blockingData);
            //

            //
            // Get downstream capability.
            // --------------------------
            //
            // You must call HLC_HDCPTX_SetCapability() to get determine if downstream device is
            // HDCP2.2 capable. Function details are provided in the API documentation.
            // Procedure: [Mandatory]
            //
            // At this point, you will want to adjust the timeouts to ensure that to at least match the
            // COMM timeout. For capability, the longest timeout will be a COMM TIMEOUT. The configuration
            // parameter is [COMM_TIMEOUT] and is defined in troot_hdcptx.cfg.
            //

            HL_SET_TIMEOUT(&hlpoll_inst, 8000); // Use 8 seconds.

            error = HLC_HDCPTX_SetCapability(&hl_inst, auid, &blockingData);
            if (error == HL_SUCCESS)
            {
               printf("[TX] Downstream receiver is HDCP 2.2 capable\n");

               //
               // Confirm state by getting the HDCP TX state. You do not need to call this every time.
               // This is here to demonstrate what each state represents.
               // Procedure: [Optional]
               //
               error = HLC_HDCP_GetState(&hl_inst, auid, &app_hdcp_state, &blockingData);
               if (error != HL_SUCCESS)
               {
                  printf("[TX] HLC_HDCP_GetState: %d [%s]\n", error, get_error_name(error));
                  fsm = STATE_CONNECTED;
                  break;
               }

               if (app_hdcp_state != HL_HDCP_STATE_CAPABLE_NOT_AUTHENTICATED)
               {
                  printf("[TX] HDCP App is in invalid state [%d]\n", app_hdcp_state);
                  fsm = STATE_CONNECTED;
                  break;
               }

               fsm = STATE_START_AUTH;
            }
            else if (HL_GET_EXDATA(&(blockingData.exStatus)) == HL_HDCP_ERROR_HDCP_NOT_CAPABLE)
            {
               printf("[TX] Downstream receiver sent that it is not HDCP 2.2 capable.\n");

               //
               // Confirm state by getting the HDCP TX state. You do not need to call this every time.
               // This is here to demonstrate what each state represents.
               // Procedure: [Optional]
               //
               error = HLC_HDCP_GetState(&hl_inst, auid, &app_hdcp_state, &blockingData);
               if (error != HL_SUCCESS)
               {
                  printf("[TX] HLC_HDCP_GetState: %d [%s]\n", error, get_error_name(error));
                  fsm = STATE_CONNECTED;
                  break;
               }

               if (app_hdcp_state != HL_HDCP_STATE_NOT_CAPABLE)
               {
                  printf("[TX] HDCP App is invalid state [%d]\n", app_hdcp_state);
                  fsm = STATE_CONNECTED;
                  break;
               }

               // The RX downstream device is not capable.
               fsm = STATE_NOT_CAPABLE;
            }
            else
            {
               //
               // There are several reasons why a capability check might fail,
               // including (temporary) communication issues. This application
               // will simply retry until there is a valid response either way.
               //

               printf("[TX] Capability request failed with error code: %d [%s]. Retry ...\n", error, get_error_name(error));

               switch(HL_GET_EXDATA(&exStatus))
               {
                  case HL_HDCP_ERROR_RECV_NACK:
                      /*
                      The reference sample application has a distinct check for a NACK to demonstrate how to handle this condition returned from the ESM
                      but no handling as it is dependent on the application code to decide what to do. Returning a NACK is not necessarily a failure as the application
                      may want to return the capability check several more times before "failing", or it may decide to give up after one time and try 1.4 capability instead.
                      However, we need to add under sample application that in case that NACK received, the negotiation failed as an example.
                      It's up to customers to decide their course of action if NACK received
                       */
                     printf("[TX %d] Received a NACK from the downstream device.\n", auid);
                     break;

                  default: // Unknown error.
                     printf("[TX] Unknown Error Code = %d\n", HL_GET_EXDATA(&exStatus));
                     break;
               }

               printf("[TX] Exception Vector = %x\n", HL_GET_EXFLAG(&(blockingData.exStatus)));
               printf("[TX] AUID             = %x\n", HL_GET_EXAUID(&(blockingData.exStatus)));
               printf("[TX] Exception Type   = %x\n", HL_GET_EXTYPE(&(blockingData.exStatus)));
               printf("[TX] Error Code       = %d\n", HL_GET_EXDATA(&(blockingData.exStatus)));

               //
               // Confirm state by getting the HDCP TX state. You do not need to call this every time.
               // This is here to demonstrate what each state represents. By calling HLC_HDCP_GetState()
               // here, this will return the previous state.
               // Procedure: [Optional]
               //
               // error = HLC_HDCP_GetState(&hl_inst, auid, &app_hdcp_state, &blockingData);

            }
            break;

         case STATE_START_AUTH:
            printf("[TX] STATE_START_AUTH\n");
            prev_fsm = fsm;

            //
            // Authenticate with RX downstream device.
            // ---------------------------------------
            //
            // If you have not issued a reset, you must call authenticate STOP
            // before starting another authentication. This ensures the HDCP TX is in the correct state.
            HLC_HDCP_Authenticate(&hl_inst, auid, HL_HDCP_STOP_AUTHENTICATION, &blockingData);

            // Function details are provided in the API documentation.
            // Procedure: [Mandatory]
            // You must call HLC_HDCP_Authenticate(..., HL_HDCP_START_AUTHENTICATION,...) with a
            // START authentication parameter to begin authentication with the RX downstream device.
            // Calling this function will block (because of the polling mode) until the system has
            // completed authentication.
            // Function details are provided in the API documentation.
            // Procedure: [Mandatory]
            //
            // At this point, you will want to adjust the timeouts to ensure that a timeout exceeds a
            // normal authentication time.
            // The line below is just an example timeout. This timeout represents the longest blocking
            // time for this function.
            //

            HL_SET_TIMEOUT(&hlpoll_inst, 30000);  // Use 30 seconds.

            //
            // Load CSM data.
            // --------------
            // The CSM data can be sent to the TX at any time while the TX authentication is not in
            // progress or when the TX sends the CSM_NOTIFY exception indicating it needs the CSM data.
            // The hl_csm_info_t structure is populated and passed to the LoadCSM API.

            // Stream Id known... Send to the firmware before authentication starts.
            csm.csmData[0].stream_id = stream_id;
            csm.csmData[0].content_type = content_type;
            error = HLC_HDCPTX_LoadCSM(&hl_inst, auid, &csm, 1, &blockingData);
            if (error != HL_SUCCESS)
            {
               printf("[TX] Failed to load CSM data: %d [%s]\n", error, get_error_name(error));
               fsm = STATE_CONNECTED;
               break;
            }

            error = HLC_HDCP_Authenticate(&hl_inst,
                                          auid,
                                          HL_HDCP_START_AUTHENTICATION,
                                          &blockingData);
            // The only error returned from this API is TROOT_HDCP_INVALID_STATE
            // which indicates that a START authentication command was issued when
            // the TX not in a state to accept the command.
            if (error != HL_SUCCESS)
            {
               printf("[TX] Failed to start authentication: %d [%s]\n", error, get_error_name(error));

               fsm = STATE_UNCONNECTED;

               //
               // Save this information because calling HLC_HDCP_GetState() will wipe it.
               //
               exStatus = blockingData.exStatus;

               //
               // Confirm state by getting the HDCP TX state. You do not need to call this every time.
               // This is here to demonstrate what each state represents.
               // Procedure: [Optional]
               //
               error = HLC_HDCP_GetState(&hl_inst, auid, &app_hdcp_state, &blockingData);
               if (error != HL_SUCCESS)
               {
                  printf("[TX] HLC_HDCP_GetState: %d [%s]\n", error, get_error_name(error));
                  fsm = STATE_CONNECTED;
                  break;
               }
               // You can only be in the STOPPED or CAPABLE state to start authentication.
               if ((app_hdcp_state != TROOT_HDCP_HL_AUTHENTICATION_STOPPED) &&
                   (app_hdcp_state != TROOT_HDCP_HL_CAPABLE_NOT_AUTHENTICATED))
               {
                  printf("[TX] HDCP App is in invalid state [%d]\n", app_hdcp_state);

                  //
                  // call HLC_Reset() and try again ("STATE_UNCONNECTED").
                  //

                  fsm = STATE_UNCONNECTED;
               break;
               }
            }
            else
            {
               //
               // Monitor the authentication process.
               // Procedure: [Optional]
               //
               fsm = STATE_AUTH;
            }
            break;

         case STATE_AUTH:
            printf("[TX] STATE_AUTH\n");
            prev_fsm = fsm;

            //
            // Monitor authentication.
            // -----------------------
            //
            // Call HLC_HDCPTX_Monitor() to monitor the results of the authentication request. If
            // authentication is successful, it will return an exception vector of
            // "HL_HDCP_EXCEPT_AKE_COMPLETE_NOTIFY", with an error code of "HL_HDCP_SUCCESS".
            // Function details are provided in the API documentation.
            // Procedure: [Mandatory]
            //
            error = HLC_HDCPTX_Monitor(&hl_inst, auid, monitor_timeout, &blockingData);
            if (error == HL_HDCP_WAITING_FOR_CSM_COMMAND)
            {
               printf("[TX %d] HL_HDCP_WAITING_FOR_CSM_COMMAND\n", auid);
               // This exception vector indicates that the TX has completed authentication in SKE, is
               // authenticating with a downstream RX repeater and is ready to accept the CSM data from
               // the Host Controller via the LoadCSM API.
               // Note:  This notification will occur if the LoadCSM was not called prior to HLC_HDCP_Authenticate command.
               error = HLC_HDCPTX_LoadCSM(&hl_inst, auid, &csm, 1, &blockingData);
               if (error != HL_SUCCESS)
               {
                  printf("[TX] Failed to load CSM data: %d [%s]\n", error, get_error_name(error));
                  fsm = STATE_CONNECTED;
                  break;
               }

               // Call the monitor again to capture the next exception.
               error = HLC_HDCPTX_Monitor(&hl_inst, auid, monitor_timeout, &blockingData);
            }

            // At this point any other error other than HL_HDCP_NOTIFY_COMPLETED indicates an error
            // occurred.
            // Error exception vectors:
            //    -HL_HDCP_LINK_INTEGRITY_DETECTED
            //    -HL_TROOT_HDCP_SYSEXCEPT_HPD_LOW
            //    -HL_HDCP_NOTIFY_COMPLETED_W_ERRORS
            //    -HL_HDCP_TX_LVC_EXPIRED
            //    -HL_HDCP_TX_CAPABILITY_TIMER_EXPIRED
            if (error != HL_HDCP_NOTIFY_COMPLETED)
            {
               printf("[TX] Exception Type = %x [%s]\n", error, get_error_name(error));
               printf("[TX] Error Code     = %d [%s]\n", HL_GET_EXDATA(&(blockingData.exStatus)), get_error_name(HL_GET_EXDATA(&(blockingData.exStatus))));

               // This error indicates the firmware has died and must be rebooted.
               if (error == HL_HDCP_ABORTED)
               {
                  printf("[TX %d] Aborted\n", auid);
                  fsm = STATE_CONNECTED;
                  break;
               }
               //
               // Firmware code stopped. Print all the information here.
               // Examples of some possible error codes:
               //    -HL_HDCP_ERROR_AKE_SEND_AKEINIT_FAILED
               //    -HL_HDCP_ERROR_AKE_GET_KM_FAILED
               //    -HL_HDCP_ERROR_AKE_RECV_SEND_H_PRIME_FAILED
               //    -HL_HDCP_ERROR_AKE_RECV_SEND_PAIRING_INFO_FAILED
               //    -HL_HDCP_ERROR_AKE_LC_INIT_FAILED
               //    -HL_HDCP_ERROR_AKE_LC_RECV_L_PRIME_FAILED
               //    -HL_HDCP_ERROR_AKE_SEND_EKS_FAILED
               //    -HL_HDCP_ERROR_AKE_SEND_CSM_INFO_FAILED
               //    -HL_HDCP_ERROR_HDCP_AKE_DONE_FAILED
               fsm = STATE_UNCONNECTED;
            }
            else
            {
               printf("[TX] Authenticated\n");

               fsm = STATE_DONE;
            }
            break;

         case STATE_NOT_CAPABLE:
            if (prev_fsm != fsm) printf("[TX] STATE_NOT_CAPABLE\n");
            prev_fsm = fsm;
            // Here there is no RX downstream HDCP 2.2 device.
            break;

         case STATE_DONE:
            if (prev_fsm != fsm) printf("[TX] STATE_DONE\n");
            prev_fsm = fsm;
            if (is_authenticated(&hl_inst, auid, monitor_timeout, &blockingData))
               usleep(1000);
            else {
               printf("[TX] Retrying Authentication\n");
               fsm = STATE_UNCONNECTED;
            }
            break;

         case STATE_HDCP_REAUTHENTICATE:
            g_force_timeout = 0;
            prev_fsm = fsm;
            fsm = STATE_UNCONNECTED;
            printf("[TX] Reauthentication request from HDMI APP\n");
            printf("[TX] Retrying Authentication\n");
            break;

         default:
            break;
      }
   }

   //
   // Save Pairing
   // -------------
   //
   // Saving the Pairing information must be done after everything is done. This data can be stored
   // off-line somewhere and reloaded at a later time.
   //
   // ELP_STATUS HLC_HDCPTX_SavePairing(hl_instance_t *hlInst, uint8_t *PairData, uint32_t *BufferSize,
   //                                   hl_auid auid);
   //
  return 0;
}

int main(void)
{
   void               *mylog;       // Logging instance.
   hl_driver_t        hldrv_inst;   // Host Library driver instance.
   ELP_STATUS         error;
   FILE               *fptr;
   struct stat        st;
   uint8_t            *fw;
   pthread_t          reset_thread;
   hl_auid            auid;

   // Initialize all variables.
   mylog = 0;
   error = 0;
   fptr = 0;
   fw = 0;

   memset(&hldrv_inst, 0, sizeof(hl_driver_t));
   memset(&st, 0, sizeof(struct stat));

   auid = 0;

   // Read file of firmware image.
   if ((fptr = fopen(FIRMWARE_PATH, "rb")) == NULL)
   {
      fprintf(stderr, "Failed to open firmware file [%s]\n", FIRMWARE_PATH);
      return EXIT_FAILURE;
   }

   if (stat(FIRMWARE_PATH, &st) < 0)
   {
      fprintf(stderr, "Failed to get information of firmware file [%s].\n", FIRMWARE_PATH);
      fclose(fptr);
      return EXIT_FAILURE;
   }

   if (!(st.st_size > 0))
   {
      fprintf(stderr, "Failed to read firmware information size [%jd]\n", (intmax_t)st.st_size);
      fclose(fptr);
      return EXIT_FAILURE;
   }

   if ((fw = malloc(st.st_size)) == NULL)
   {
      fprintf(stderr, "Failed to allocate %jd for reading the firmware\n", (intmax_t)st.st_size);
      fclose(fptr);
      return EXIT_FAILURE;
   }

   if (fread(fw, 1, st.st_size, fptr) != (intmax_t)st.st_size)
   {
      fprintf(stderr, "Failed to read firmware file [%s]\n", FIRMWARE_PATH);
      fclose(fptr);
      error = EXIT_FAILURE;
      // Release allocated memory.
      goto RELEASE_MEMORY;
   }
   fclose(fptr);

   printf("Firmware: [%s]\n", FIRMWARE_PATH);
   printf("          %jd bytes\n", (intmax_t)st.st_size);

   //
   // Target platform.
   // ----------------
   //
   // The target platform for this sample application is a Linux platform and is targetted to run in
   // USER SPACE. Therefore, this package contains the following drivers:
   //    - Logging: Linux base logging method located in
   //               [cal/system/linux_os/elliptic_log.c] (ELP_LOG_*)
   //    - Host Library driver: Linux kernel driver that allows communication between the sample and the
   //                           firmware located in [host_lib_driver/linux]
   //

   //
   // Enable logging.
   // ---------------
   //
   // The logging feature is part of the platform definition. The ELP_LOG_INIT() macro points to the
   // elp_log_init(...) function in the Common Abstraction Layer (CAL). Depending on your platform, you
   // might need to update the implementation of this file.
   //
   proc_hdmi_hpd = open("/proc/hdmi_tx/hpd", O_RDWR);
   if(proc_hdmi_hpd == 0){
      fprintf(stderr, "[TX] Failed to open /proc/hdmi_tx/hpd: %d\n", proc_hdmi_hpd);
   }

   ELP_LOG_INIT(&mylog, (LOG_CRITICAL_ERR | LOG_WARNING_ERR | LOG_INFO | LOG_DUMP), "HOSTLIB", NULL);

   printf("Host Library version V%04x\n", HLC_Version());

   //
   // Open the Host Library driver.
   // -----------------------------
   //
   // The host_lib_driver_open() function will open the platform driver that will allow communication
   // between the sample and the firmware. The implementation of this driver is platform specific.
   //
   if ((error = host_lib_driver_open(&hldrv_inst, device_name, HPI_ADDRESS_ESM0,
                                     (uint32_t)HDCP_FW_ADDRESS, HDCP_FW_SIZE,
                                     (uint32_t)HDCP_DATA_ADDRESS, HDCP_DATA_SIZE)) != HL_SUCCESS)
   {
      fprintf(stderr, "Failed to open firmware driver %s, error %d\n", device_name, error);
      error = EXIT_FAILURE;
      // Close Host Library driver and release allocated memory.
      goto CLOSE_DRIVER;
   }

   // Run thread that monitors kill requests
   pthread_create(&reset_thread, NULL, reset_thread_fun, &hldrv_inst); 

   // Run the hdcp_tx application
   error = tx_application(&hldrv_inst, mylog, fw, st.st_size, auid);

   pthread_join(reset_thread, NULL);

CLOSE_DRIVER:
   ELP_LOG_END(mylog);
   host_lib_driver_close(&hldrv_inst);

RELEASE_MEMORY:
   free(fw);

   if (error == 0)
   {
      printf("Exiting successfully\n");
   }
   else
   {
      printf("Exiting with err=%d\n", error);
   }
   return error;
}

/**
 * \endcode
 * @}
 */
