// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010 Synopsys, Inc. and/or its affiliates
 * Synopsys DesignWare HDMI TX controller driver
 */

#include "hdcp/hdcp_reg.h"
#include "hdcp/hdcp.h"
#include "hdcp/hdcp_14.h"
#include "hdcp/hdcp_verify.h"
#include "hdcp/hdcp_params.h"
#include "identification.h"
#include "util/log.h"
#include "util/error.h"
#include "bsp/access.h"


#define KSV_LEN  5 // KSV value size


/* HDCP Interrupt fields */
#define INT_KSV_ACCESS    (A_APIINTSTAT_KSVACCESSINT_MASK)
#define INT_KSV_SHA1      (A_APIINTSTAT_KSVSHA1CALCINT_MASK)
#define INT_KSV_SHA1_DONE (A_APIINTSTAT_KSVSHA1CALCDONEINT_MASK)
#define INT_HDCP_FAIL     (A_APIINTSTAT_HDCP_FAILED_MASK)
#define INT_HDCP_ENGAGED  (A_APIINTSTAT_HDCP_ENGAGED_MASK)

#define HDCP_DEV_WRITE_MASK(dev, addr, mask, value)  \
	do { if (snps_functions && snps_functions->logger) \
		snps_functions->logger(2, "%s: WRITE 0x%08 MASK 0x%02 VALUE 0x%02",__func__, addr, mask, value); \
		dev_write_mask(dev, addr, mask, value); \
	} while (0)

#define HDCP_DEV_WRITE(dev, addr, value)  \
	do { if (snps_functions && snps_functions->logger) \
		snps_functions->logger(2, "%s: WRITE 0x%08 VALUE 0x%02",__func__, addr, mask, value); \
		dev_write(dev, addr, value); \
	} while (0)

#define HDCP_DEV_READ_MASK(dev, addr, mask)  \
	do { if (snps_functions && snps_functions->logger) \
		snps_functions->logger(2, "%s: READ 0x%08 MASK 0x%02 = 0x%02",__func__, addr, mask, dev_read_mask(dev, addr, mask)); \
	} while (0)

#define HDCP_DEV_READ(dev, addr)  \
	do { if (snps_functions && snps_functions->logger) \
		snps_functions->logger(2, "%s: READ 0x%08 = 0x%02",__func__, addr, mask, dev_read(dev, addr)); \
	} while (0)



void _setDeviceMode(hdmi_tx_dev_t *dev, video_mode_t mode)
{
	u8 set_mode = (mode == HDMI ? 1 : 0) ;  // 1 - HDMI : 0 - DVI
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_HDMIDVI_MASK, set_mode);
}

void _EnableFeature11(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_EN11FEATURE_MASK, bit);
}

void _OverrideHDCP2p2Switch(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, HDCP22REG_CTRL, HDCP22REG_CTRL_OVR_EN_MASK,  bit);
}

void hdcp_rxdetect(hdmi_tx_dev_t *dev, u8 enable)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_RXDETECT_MASK, enable);
}

void _EnableAvmute(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_AVMUTE_MASK, bit);
}

void _RiCheck(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_SYNCRICHECK_MASK, bit);
}

void _BypassEncryption(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_BYPENCRYPTION_MASK, bit);
}

void _EnableI2cFastMode(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_I2CFASTMODE_MASK, bit);
}

void _EnhancedLinkVerification(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_ELVENA_MASK, bit);
}

void hdcp_sw_reset(hdmi_tx_dev_t *dev)
{
	//Software reset signal, active by writing a zero and auto cleared to 1 in the following cycle
	dev_write_mask(dev, A_HDCPCFG1, A_HDCPCFG1_SWRESET_MASK, 0);
}

void _DisableEncryption(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG1, A_HDCPCFG1_ENCRYPTIONDISABLE_MASK, bit);
}

void _EncodingPacketHeader(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG1, A_HDCPCFG1_PH2UPSHFTENC_MASK, bit);
}

void _DisableKsvListCheck(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_HDCPCFG1, A_HDCPCFG1_DISSHA1CHECK_MASK, bit);
}

u8 _HdcpEngaged(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, A_HDCPOBS0, A_HDCPOBS0_HDCPENGAGED_MASK);
}

u8 _AuthenticationState(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, A_HDCPOBS0, A_HDCPOBS0_SUBSTATEA_MASK | A_HDCPOBS0_STATEA_MASK);
}

u8 _CipherState(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, A_HDCPOBS2, A_HDCPOBS2_STATEE_MASK);
}

u8 _RevocationState(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, (A_HDCPOBS1), A_HDCPOBS1_STATER_MASK);
}

u8 _OessState(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, (A_HDCPOBS1), A_HDCPOBS1_STATEOEG_MASK);
}

u8 _EessState(hdmi_tx_dev_t *dev)
{
	return dev_read_mask(dev, (A_HDCPOBS2), A_HDCPOBS2_STATEEEG_MASK);
}

u8 _DebugInfo(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, A_HDCPOBS3);
}

void _InterruptClear(hdmi_tx_dev_t *dev, u8 value)
{
	dev_write(dev, (A_APIINTCLR), value);
}

u8 _InterruptStatus(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, A_APIINTSTAT);
}

void _InterruptMask(hdmi_tx_dev_t *dev, u8 value)
{;
	dev_write(dev, (A_APIINTMSK), value);
}

u8 _InterruptMaskStatus(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, A_APIINTMSK);
}

void _HSyncPolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_VIDPOLCFG, A_VIDPOLCFG_HSYNCPOL_MASK, bit);
}


void _VSyncPolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_VIDPOLCFG, A_VIDPOLCFG_VSYNCPOL_MASK, bit);
}

void _DataEnablePolarity(hdmi_tx_dev_t *dev, u8 bit)
{
	dev_write_mask(dev, A_VIDPOLCFG, A_VIDPOLCFG_DATAENPOL_MASK, bit);
}

void _UnencryptedVideoColor(hdmi_tx_dev_t *dev, u8 value)
{

	dev_write_mask(dev, A_VIDPOLCFG, A_VIDPOLCFG_UNENCRYPTCONF_MASK, value);
}

void _OessWindowSize(hdmi_tx_dev_t *dev, u8 value)
{
	dev_write(dev, (A_OESSWCFG), value);
}

u16 _CoreVersion(hdmi_tx_dev_t *dev)
{
	u16 version = 0;
	version = dev_read(dev, A_COREVERLSB);
	version |= dev_read(dev, A_COREVERMSB) << 8;
	return version;
}

u8 _ControllerVersion(hdmi_tx_dev_t *dev)
{
	return dev_read(dev, A_HDCPCFG0);
}

void _MemoryAccessRequest(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE();
	dev_write_mask(dev, A_KSVMEMCTRL, A_KSVMEMCTRL_KSVMEMREQUEST_MASK, bit);
}

u8 _MemoryAccessGranted(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)((dev_read(dev, A_KSVMEMCTRL) & A_KSVMEMCTRL_KSVMEMACCESS_MASK) >> 1);
}

void _UpdateKsvListState(hdmi_tx_dev_t *dev, u8 bit)
{
	LOG_TRACE1(bit);
	dev_write_mask(dev, A_KSVMEMCTRL, A_KSVMEMCTRL_SHA1FAIL_MASK, bit);
	dev_write_mask(dev, A_KSVMEMCTRL, A_KSVMEMCTRL_KSVCTRLUPD_MASK, 1);
	dev_write_mask(dev, A_KSVMEMCTRL, A_KSVMEMCTRL_KSVCTRLUPD_MASK, 0);
}

u8 _ksv_sha1_status(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	return (u8)((dev_read(dev, A_KSVMEMCTRL) & A_KSVMEMCTRL_KSVSHA1STATUS_MASK) >> 4);
}

u16 _BStatusRead(hdmi_tx_dev_t *dev)
{
	u16 bstatus = 0;

	bstatus	= dev_read(dev, HDCP_BSTATUS) ;
	bstatus	|= dev_read(dev, HDCP_BSTATUS + ADDR_JUMP) << 8;
	return bstatus;
}

void _M0Read(hdmi_tx_dev_t *dev, u8 * data)
{
	u8 i = 0;
	for (i = 0 ; i < HDCP_M0_SIZE; i++ ) {
		data[i] = dev_read(dev, HDCP_M0 + (i * ADDR_JUMP));
	}
}
#if 0
int _KsvListRead(hdmi_tx_dev_t *dev, u16 size, u8 * data)
{
	u8 i = 0;

	if(size > HDCP_KSV_SIZE) {
		LOGGER(SNPS_ERROR,"Invalid number of devices");
		return -1;
	}

	for (i = 0 ; i < size; i++ ){
		data[i] = dev_read(dev, HDCP_KSV + (i * ADDR_JUMP));
	}
	return 0;
}
#endif
void _SHA1VHRead(hdmi_tx_dev_t *dev, u8 * data)
{
	u8 i = 0;
	for (i = 0 ; i < HDCP_VH_SIZE; i++ ) {
		data[i] = dev_read(dev, HDCP_VH + (i * ADDR_JUMP));
	}
}

void _RevocListWrite(hdmi_tx_dev_t *dev, u16 addr, u8 data)
{
	LOG_TRACE2(addr, data);
	dev_write(dev, HDCP_REVOC_LIST + addr, data);
}

void _AnWrite(hdmi_tx_dev_t *dev, u8 * data)
{
	short i = 0;
	LOG_TRACE();
	if (data != 0) {
		LOG_TRACE1(data[0]);
		for (i = 0; i <= (HDCPREG_AN7 - HDCPREG_AN0); i++) {
			dev_write(dev, (HDCPREG_AN0 + (i * ADDR_JUMP)), data[i]);
		}
		dev_write_mask(dev, HDCPREG_ANCONF, HDCPREG_ANCONF_OANBYPASS_MASK, 1);
	} else {
		dev_write_mask(dev, HDCPREG_ANCONF, HDCPREG_ANCONF_OANBYPASS_MASK, 0);
	}
}

u8 _BksvRead(hdmi_tx_dev_t *dev, u8 * bksv)
{
	short i = 0;
	if (bksv != 0) {
		LOG_TRACE1(bksv[0]);
		for (i = 0; i <= (HDCPREG_BKSV4 - HDCPREG_BKSV0); i++) {
			bksv[i] =  dev_read(dev, HDCPREG_BKSV0 + (i * 4));
		}
		return i;
	} else {
		return 0;
	}
}

u8 _hdcp_2p2_version(hdmi_tx_dev_t *dev)
{
	LOGGER(SNPS_WARN , "%s:TBI", __func__);
	//1 - Configure the RX FIFO to read HDCP 2.2 version:
	//	a. Write 8'h50 to h22s_rxmsg_addr.rxmsg_addr.
	//	b. Write 8'h01 to h22s_rxmsg_nbyteslow.rxmsg_nbytes_lo.
	//	c. Write 2'b00 to h22s_rxmsg_nbyteshigh.rxmsg_nbytes_hi.
	//	d. Write 1'b0 to h22s_rxmsg_cfg.rxmsg_rcv_autostart.
	//	e. Write 1'b1 to h22s_rxmsg_ctrl.rxmsg_rcv_start.

	//2 - Wait for the interrupt hdcp2version_chg to be asserted.

	//3 - The HDCP 2.2 version can be read by the software using two different procedures:
	//	a. Read h22s_hdcp2version_sts.hdcp2version_sts.
	//	Or
	//	b. Read from h22s_rxmsg_byte.rxmsg_byte.
	//	Write 1'b1 to h22s_rxmsg_ff_ctrl.rxmsgfifo_pop
	return 0x0;
}

u8 _hdcp_2p2_reset_engine(hdmi_tx_dev_t *dev)
{
	LOGGER(SNPS_WARN , "%s:TBI", __func__);
	//Reset the HDCP 2.2 engine, write “1” in the h22s_ctrl.swrstreq bit field register.
	return TRUE;
}

u8 _hdcp_2p2_authentication(hdmi_tx_dev_t *dev)
{
	LOGGER(SNPS_WARN , "%s:TBI", __func__);
	//Perform the HDCP 2.2 authentication as described in the “HDCP 2.2 Authentication” on page 228.
	//Skip the remaining steps as they are related to HDMI 1.4.
	return TRUE;
}

#ifdef ROMLESS
void _WriteAksv(hdmi_tx_dev_t *dev, u8 aksv[7])
{
	access_CoreWrite(dev, aksv[0], (A_HDCPREG_DPK6), 0, 8);
	access_CoreWrite(dev, aksv[1], (A_HDCPREG_DPK5), 0, 8);
	access_CoreWrite(dev, aksv[2], (A_HDCPREG_DPK4), 0, 8);
	access_CoreWrite(dev, aksv[3], (A_HDCPREG_DPK3), 0, 8);
	access_CoreWrite(dev, aksv[4], (A_HDCPREG_DPK2), 0, 8);
	access_CoreWrite(dev, aksv[5], (A_HDCPREG_DPK1), 0, 8);
	access_CoreWrite(dev, aksv[6], (A_HDCPREG_DPK0), 0, 8);
}

void _WaitMemAccess(hdmi_tx_dev_t *dev)
{
	while (!access_CoreRead(dev, (A_HDCPREG_RMLSTS), 6, 1)) ;
}

void _WriteSeed(hdmi_tx_dev_t *dev, u8 encKey[2])
{
	access_CoreWrite(dev, encKey[0], (A_HDCPREG_SEED1), 0, 8);
	access_CoreWrite(dev, encKey[1], (A_HDCPREG_SEED0), 0, 8);
}

void _EnableEncrypt(hdmi_tx_dev_t *dev, u8 enable)
{
	access_CoreWrite(dev, enable, (A_HDCPREG_RMLCTL), 0, 1);
}

void _StoreEncryptKeys(hdmi_tx_dev_t *dev,u8 keys[560])
{
	int key_nr = 0;
	for (key_nr = 0; key_nr < 280; key_nr = key_nr + 7) {
		access_CoreWrite(dev, keys[key_nr + 0], (A_HDCPREG_DPK6), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 1], (A_HDCPREG_DPK5), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 2], (A_HDCPREG_DPK4), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 3], (A_HDCPREG_DPK3), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 4], (A_HDCPREG_DPK2), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 5], (A_HDCPREG_DPK1), 0, 8);
		access_CoreWrite(dev, keys[key_nr + 6], (A_HDCPREG_DPK0), 0, 8);
		_WaitMemAccess(dev);
	}
}
#endif


int hdcp_initialize(hdmi_tx_dev_t *dev)
{
	LOG_TRACE();
	hdcp_rxdetect(dev, 0);
	_DataEnablePolarity(dev, dev->snps_hdmi_ctrl.data_enable_polarity);
	_DisableEncryption(dev, 1);
	return TRUE;
}

void hdcp_1p4_configure(hdmi_tx_dev_t *dev, hdcpParams_t * hdcp)
{
	//_OverrideHDCP2p2Switch(dev, TRUE);

	/* HDCP only */
	_EnableFeature11(dev,(hdcp->mEnable11Feature > 0) ? 1 : 0);
	_RiCheck(dev,(hdcp->mRiCheck > 0) ? 1 : 0);
	_EnableI2cFastMode(dev,	(hdcp->mI2cFastMode > 0) ? 1 : 0);
	_EnhancedLinkVerification(dev,(hdcp->mEnhancedLinkVerification > 0) ? 1 : 0);

	/* fixed */
	_EnableAvmute(dev, FALSE);
	_UnencryptedVideoColor(dev, 0x00);
	_EncodingPacketHeader(dev, TRUE);

	//9 - Set encryption
	_OessWindowSize(dev, 64);
	_BypassEncryption(dev, FALSE);
	_DisableEncryption(dev, FALSE);

	//10 - Reset the HDCP 1.4 engine
	hdcp_sw_reset(dev);

	//11 - Configure Device Private Keys (required when DWC_HDMI_HDCP_DPK_ROMLESS configuration
	//	parameter is set to True [1]), which is illustrated in Figure 3-7 on page 82. For required memory
	//	contents, refer to the “HDCP DPK 56-bit Memory Mapping” table in Chapter 2 of the DesignWare
	//	HDMI Transmitter Controller Databook.
#ifdef ROMLESS
	/* check if controller is version 1.4a or higher to support DPK keys external storage */
	if (_ControllerVersion(dev) >= 0x14) {
		hdcp_WriteDpkKeys(dev, hdcp);
	}
#endif


	//12 - Enable encryption
	hdcp_rxdetect(dev, 1);

	LOGGER(SNPS_DEBUG, "HDCP enable interrupts");
	_InterruptClear(dev, A_APIINTCLR_HDCP_FAILED_MASK |
						 A_APIINTCLR_HDCP_ENGAGED_MASK |
						 A_APIINTSTAT_KSVSHA1CALCDONEINT_MASK);
	/* enable KSV list SHA1 verification interrupt */
	_InterruptMask(dev, (~(A_APIINTMSK_HDCP_FAILED_MASK |
					     A_APIINTMSK_HDCP_ENGAGED_MASK |
					     A_APIINTSTAT_KSVSHA1CALCDONEINT_MASK)) & _InterruptMaskStatus(dev));
}

int hdcp_configure(hdmi_tx_dev_t *dev, hdcpParams_t * hdcp, videoParams_t *video)
{
	video_mode_t mode = dev->snps_hdmi_ctrl.hdmi_on;
	u8 hsPol = video->mDtd.mHSyncPolarity;
	u8 vsPol = video->mDtd.mVSyncPolarity;
	static int hdcp_2p2 = 0;

	if(dev->snps_hdmi_ctrl.hdcp_on == 0){
		LOGGER(SNPS_WARN, "HDCP is not active");
		return TRUE;
	}

	// Before configure HDCP we should configure the internal parameters
	hdcp->maxDevices = 128;
	hdcp->mI2cFastMode = 0;
	if(hdcp->mKsvListBuffer == NULL)
		hdcp->mKsvListBuffer = malloc(sizeof(u8) * 670);
	memcpy(&dev->hdcp, hdcp, sizeof(hdcpParams_t));

	//1 - To determine if the controller supports HDCP
	if(id_product_type(dev) != 0xC1){
		LOGGER(SNPS_ERROR, "Controller does not supports HDCP");
		return FALSE;
	}

	//2 - To determine the HDCP version of the transmitter
	if(id_hdcp22_support(dev) == HDCP_22_SNPS){
		LOGGER(SNPS_DEBUG, "HDCP 2.2 SNPS is present (both HDCP 1.4 and HDCP 2.2 versions supported)");
		hdcp_2p2 = 1;
	}
	else if(id_hdcp22_support(dev) == HDCP_22_EXT){
		LOGGER(SNPS_DEBUG, "HDCP 2.2 External is present (both HDCP 1.4 and HDCP 2.2 versions supported)");
		hdcp_2p2 = 2;
	}
	else{
		LOGGER(SNPS_DEBUG, "HDCP 2.2 is not present (HDCP 1.4 support only)");
		hdcp_2p2 = 0;
	}

	//3 - Select DVI or HDMI mode
	LOGGER(SNPS_DEBUG,"Set HDCP %s", mode == HDMI ? "HDMI" : "DVI");
	dev_write_mask(dev, A_HDCPCFG0, A_HDCPCFG0_HDMIDVI_MASK, (mode == HDMI) ? 1 : 0);

	//4 - Set the Data enable, Hsync, and VSync polarity
	_HSyncPolarity(dev, (hsPol > 0) ? 1 : 0);
	_VSyncPolarity(dev, (vsPol > 0) ? 1 : 0);
	_DataEnablePolarity(dev, (dev->snps_hdmi_ctrl.data_enable_polarity > 0) ? 1 : 0);

	//5 - If hdcp22_snps read in Step 2 is 0 (Synopsys HDCP 2.2 not supported), skip to Step 9.
	if((hdcp_2p2 == 0x00) || (hdcp_2p2 == 0x02) ){
		LOGGER(SNPS_DEBUG, "Configuring HDCP 1.4");
		hdcp_1p4_configure(dev, hdcp);
		return TRUE;
	}
	else{
		LOGGER(SNPS_DEBUG, "Configuring HDCP 2.2 SNPS");
		//6 - If hdcp22_snps read in Step 2 is 1 (Synopsys HDCP 2.2 is supported):
		//Read HDCP2Version through the HDCP 2.2 Synopsys RX Message FIFO and select HDCP version in
		//the Transmitter (refer to “Reading HDCP 2.2 Version (HDCP Port 0x50)” on page 83).
		
			//b. If HDCP2Version is 0x04, write “1” to mc_opctrl.h22s_ovr_val (this selects HDCP 2.2).
			dev_write_mask(dev, 0x1000C, 1 << 5, 0x1);
			//c. Write “1” to mc_opctrl.h22s_switch_lck.
			dev_write_mask(dev, 0x1000C, 1 << 4, 0x1);
		
			//7 - Reset the HDCP 2.2 engine
			_hdcp_2p2_reset_engine(dev);

			//8 - Perform the HDCP 2.2 authentication
			_hdcp_2p2_authentication(dev);
	}
	return TRUE;
}

// SHA-1 calculation by Software
u8 _read_ksv_list(hdmi_tx_dev_t *dev, int *param)
{
	int timeout = 1000;
	u16 bstatus = 0;
	u16 deviceCount = 0;
	int valid = HDCP_IDLE;
	int size = 0;
	int i = 0;

	u8 *hdcp_ksv_list_buffer = dev->hdcp.mKsvListBuffer;

	// 1 - Wait for an interrupt to be triggered (a_apiintstat.KSVSha1calcint)
	// This is called from the INT_KSV_SHA1 irq so nothing is required for this step

	// 2 - Request access to KSV memory through setting a_ksvmemctrl.KSVMEMrequest to 1'b1 and
	// pool a_ksvmemctrl.KSVMEMaccess until this value is 1'b1 (access granted).
	_MemoryAccessRequest(dev,TRUE);
	while(_MemoryAccessGranted(dev) == 0 && timeout--){
		asm volatile ("nop");
	}

	if (_MemoryAccessGranted(dev) == 0){
		_MemoryAccessRequest(dev,FALSE);
		LOGGER(SNPS_ERROR, "KSV List memory access denied");
		*param = 0;
		return HDCP_KSV_LIST_ERR_MEM_ACCESS;
	}

	// 3 - Read VH', M0, Bstatus, and the KSV FIFO. The data is stored in the revocation memory, as
	// provided in the "Address Mapping for Maximum Memory Allocation" table in the databook.
	bstatus = _BStatusRead(dev);
	deviceCount = bstatus & BSTATUS_DEVICE_COUNT_MASK;

	if(deviceCount > dev->hdcp.maxDevices) {
		*param = 0;
		LOGGER(SNPS_ERROR,"depth exceeds KSV List memory");
		return HDCP_KSV_LIST_ERR_DEPTH_EXCEEDED;
	}

	size = deviceCount * KSV_LEN + HEADER + SHAMAX;

	for (i = 0; i < size; i++){
		if (i < HEADER) { /* BSTATUS & M0 */
			hdcp_ksv_list_buffer[(deviceCount * KSV_LEN) + i] = (u8)dev_read(dev, HDCP_BSTATUS + (i * ADDR_JUMP));
		}
		else if (i < (HEADER + (deviceCount * KSV_LEN))) { /* KSV list */
			hdcp_ksv_list_buffer[i - HEADER] = (u8)dev_read(dev, HDCP_BSTATUS + (i * ADDR_JUMP));
		}
		else { /* SHA */
			hdcp_ksv_list_buffer[i] = (u8)dev_read(dev, HDCP_BSTATUS + (i * ADDR_JUMP));
		}
	}

	// 4 - Calculate the SHA-1 checksum (VH) over M0, Bstatus, and the KSV FIFO.
	if(hdcp_verify_ksv(dev, hdcp_ksv_list_buffer, size) == TRUE){
		valid = HDCP_KSV_LIST_READY;
		LOGGER(SNPS_DEBUG, "HDCP_KSV_LIST_READY");
	}
	else{
		valid = HDCP_ERR_KSV_LIST_NOT_VALID;
		LOGGER(SNPS_DEBUG, "HDCP_ERR_KSV_LIST_NOT_VALID");
	}

	// 5 - If the calculated VH equals the VH', set a_ksvmemctrl.SHA1fail to 0 and set
	// a_ksvmemctrl.KSVCTRLupd to 1. If the calculated VH is different from VH' then set
	// a_ksvmemctrl.SHA1fail to 1 and set a_ksvmemctrl.KSVCTRLupd to 1, forcing the controller
	// to re-authenticate from the beginning.
	_MemoryAccessRequest(dev,0);
	_UpdateKsvListState(dev,(valid == HDCP_KSV_LIST_READY) ? 0 : 1);

	return valid;
}

u8 hdcp_event_handler(hdmi_tx_dev_t *dev, int *param)
{
	u8 interrupt_status = 0;
	int valid = HDCP_IDLE;

	LOG_TRACE();
	interrupt_status = hdcp_interrupt_status(dev);
	LOGGER(SNPS_TRACE, "hdcp_interrupt_status %d", interrupt_status);

	if (interrupt_status == 0){
		LOGGER(SNPS_TRACE, "HDCP_IDLE");
		return HDCP_IDLE;
	}

	hdcp_interrupt_clear(dev, interrupt_status);


	if(interrupt_status & INT_KSV_SHA1){
		LOGGER(SNPS_WARN, "INT_KSV_SHA1");
		return _read_ksv_list(dev, param);
	}

	if ((interrupt_status & INT_HDCP_FAIL) != 0) {
		*param = 0;
		LOGGER(SNPS_TRACE, "HDCP_FAILED");
		return HDCP_FAILED;
	}

	if ((interrupt_status & INT_HDCP_ENGAGED) != 0) {
		*param = 1;
		LOGGER(SNPS_TRACE, "HDCP_ENGAGED");
		return HDCP_ENGAGED;
	}

#if 0
	if ((state & A_APIINTSTAT_KEEPOUTERRORINT_MASK) != 0)
		LOGGER(SNPS_ERROR,"keep out error interrupt");

	if ((state & A_APIINTSTAT_LOSTARBITRATION_MASK) != 0)
		LOGGER(SNPS_ERROR,"lost arbitration error interrupt");
		return HDCP_IDLE;

	if ((state & A_APIINTSTAT_I2CNACK_MASK) != 0)
		LOGGER(SNPS_ERROR,"i2c nack error interrupt");
		return HDCP_IDLE;

#endif

	return valid;
}

void hdcp_av_mute(hdmi_tx_dev_t *dev, int enable)
{
	LOG_TRACE1(enable);
	_EnableAvmute(dev,
			(enable == TRUE) ? 1 : 0);
}

void hdcp_disable_encryption(hdmi_tx_dev_t *dev, int disable)
{
	LOG_TRACE1(disable);
	_DisableEncryption(dev,	(disable == TRUE) ? 1 : 0);
}

u8 hdcp_interrupt_status(hdmi_tx_dev_t *dev)
{
	return _InterruptStatus(dev);
}

int hdcp_interrupt_clear(hdmi_tx_dev_t *dev, u8 value)
{
	_InterruptClear(dev, value);
	return TRUE;
}

#ifdef ROMLESS
void hdcp_WriteDpkKeys(hdmi_tx_dev_t *dev, hdcpParams_t * params)
{
	_WaitMemAccess(dev );
	_WriteAksv(dev, params->mAksv);
	_WaitMemAccess(dev);
	_EnableEncrypt(dev, 1);
	_WriteSeed(dev, params->mSwEncKey);
	_StoreEncryptKeys(dev, params->mKeys);
}
#endif
