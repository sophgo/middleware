/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: mipi_tx.h
 * Description:
 */

#ifndef __CVI_MIPI_TX_H__
#define __CVI_MIPI_TX_H__

#include <cvi_comm_mipi_tx.h>

/* mipi_tx_cfg: Configure MIPI TX device
 *
 * @param fd: Device file descriptor
 * @param dev_cfg: Device configuration info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_cfg(int fd, struct combo_dev_cfg_s *dev_cfg);

/* mipi_tx_send_cmd: Send command to MIPI TX
 *
 * @param fd: Device file descriptor
 * @param cmd_info: Command info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_send_cmd(int fd, struct cmd_info_s *cmd_info);

/* mipi_tx_recv_cmd: Receive command from MIPI TX
 *
 * @param fd: Device file descriptor
 * @param cmd_info: Received command info
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_recv_cmd(int fd, struct get_cmd_info_s *cmd_info);

/* mipi_tx_enable: Enable MIPI TX
 *
 * @param fd: Device file descriptor
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_enable(int fd);

/* mipi_tx_disable: Disable MIPI TX
 *
 * @param fd: Device file descriptor
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_disable(int fd);

/* mipi_tx_set_hs_settle: Set HS settle config
 *
 * @param fd: Device file descriptor
 * @param hs_cfg: HS settle config
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_set_hs_settle(int fd, const struct hs_settle_s *hs_cfg);

/* mipi_tx_get_hs_settle: Get HS settle config
 *
 * @param fd: Device file descriptor
 * @param hs_cfg: HS settle config
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_get_hs_settle(int fd, struct hs_settle_s *hs_cfg);

/* mipi_tx_suspend: Suspend MIPI TX
 *
 * @param fd: Device file descriptor
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_suspend(int fd);

/* mipi_tx_resume: Resume MIPI TX
 *
 * @param fd: Device file descriptor
 * @return: status of operation. CVI_SUCCESS if OK.
 */
int mipi_tx_resume(int fd);

#endif // __MIPI_TX_H__

