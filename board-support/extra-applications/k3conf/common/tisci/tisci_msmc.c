// SPDX-License-Identifier: BSD-3-Clause
/*
 * TISCI MSMC query APIs
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <string.h>
#include <tisci_protocol.h>
#include <tisci.h>

int ti_sci_cmd_get_msmc(uint32_t *s_l, uint32_t *s_h,
			uint32_t *e_l, uint32_t *e_h)
{
	struct ti_sci_msg_resp_query_msmc *resp;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TISCI_MSG_QUERY_MSMC, 0);

	msg.len = sizeof(struct ti_sci_msg_hdr);
	msg.buf = buf;

	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return ret;

	resp = (struct ti_sci_msg_resp_query_msmc *)buf;
	*s_l = resp->msmc_start_low;
	*s_h = resp->msmc_start_high;
	*e_l = resp->msmc_end_low;
	*e_h = resp->msmc_end_high;

	return 0;
}
