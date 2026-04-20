// SPDX-License-Identifier: BSD-3-Clause
/*
 * TISCI device ops library
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <string.h>
#include <tisci_protocol.h>
#include <tisci.h>

static const char device_state[MAX_DEVICE_HW_STATES + 1][MAX_DEVICE_STATE_LENGTH] = {
	[MSG_DEVICE_HW_STATE_OFF] = "DEVICE_STATE_OFF",
	[MSG_DEVICE_HW_STATE_ON] = "DEVICE_STATE_ON",
	[MSG_DEVICE_HW_STATE_TRANS] = "DEVICE_STATE_TRANS",
	[MAX_DEVICE_HW_STATES] = "DEVICE_STATE_UNKNOWN"
};

static int ti_sci_set_device_state(uint32_t id, uint32_t flags, uint8_t state)
{
	struct ti_sci_msg_req_set_device_state *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_SET_DEVICE_STATE, flags);
	req = (struct ti_sci_msg_req_set_device_state *)buf;
	req->id = id;
	req->state = state;

	msg.len = sizeof(*req);
	msg.buf = buf;
	return ti_sci_xfer_msg(&msg);
}

int ti_sci_cmd_enable_device(uint32_t dev_id)
{
	return ti_sci_set_device_state(dev_id, 0, MSG_DEVICE_SW_STATE_ON);
}

int ti_sci_cmd_disable_device(uint32_t dev_id)
{
	return ti_sci_set_device_state(dev_id, 0, MSG_DEVICE_SW_STATE_AUTO_OFF);
}

const char *ti_sci_cmd_get_device_status(uint32_t dev_id)
{
	struct ti_sci_msg_resp_get_device_state *resp;
	struct ti_sci_msg_req_get_device_state *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_GET_DEVICE_STATE, 0);
	req = (struct ti_sci_msg_req_get_device_state *)buf;
	req->id = dev_id;

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return NULL;

	resp = (struct ti_sci_msg_resp_get_device_state *)buf;

	return device_state[resp->current_state];
}
