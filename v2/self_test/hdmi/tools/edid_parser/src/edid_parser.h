#ifndef SRC_EDID_PARSER_H_
#define SRC_EDID_PARSER_H_

#include "includes.h"
#include "edid_type.h"
#include "edid.h"

int edid_parser(hdmi_tx_dev_t *dev, u8 * buffer, edidCeaExt_t *edidExt, u16 edid_size);

#endif /* SRC_EDID_PARSER_H_ */
