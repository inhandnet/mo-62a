// SPDX-License-Identifier: BSD-3-Clause
/*
 * SCMI device ops library
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SCMI Specification: https://developer.arm.com/documentation/den0056/latest/
 */

#include <string.h>
#include <scmi_protocol.h>
#include <scmi.h>

static int scmi_set_device_state(uint32_t id, uint32_t flags, uint32_t state)
{
	struct scmi_msg_req_set_pwr_domain_state *req;
	uint8_t buf[SCMI_RAW_MAX_MSG_SIZE];
	struct scmi_raw_msg msg;

	memset(buf, 0, sizeof(buf));
	scmi_setup_header((struct scmi_msg_hdr *)buf, SCMI_PWR_DOMAIN_ID,
				SCMI_SET_PWR_DOMAIN_STATE_MSG_ID, SCMI_CMD_MSG_TYPE);

	req = (struct scmi_msg_req_set_pwr_domain_state *)buf;
	req->flags = flags;
	req->id = id;
	req->power_state = state;

	msg.len = sizeof(*req);
	msg.buf = buf;

	return scmi_xfer_msg(&msg);
}

int scmi_cmd_enable_device(uint32_t dev_id)
{
	return scmi_set_device_state(dev_id, 0, SCMI_DEVICE_STATE_ON);
}

int scmi_cmd_disable_device(uint32_t dev_id)
{
	return scmi_set_device_state(dev_id, 0, SCMI_DEVICE_STATE_OFF);
}

const char *scmi_cmd_get_device_status(uint32_t dev_id)
{
	struct scmi_msg_resp_get_device_state *resp;
	struct scmi_msg_req_get_device_state *req;
	uint8_t buf[SCMI_RAW_MAX_MSG_SIZE];
	struct scmi_raw_msg msg;
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	scmi_setup_header((struct scmi_msg_hdr *)buf, SCMI_PWR_DOMAIN_ID,
				SCMI_GET_PWR_DOMAIN_STATE_MSG_ID, SCMI_CMD_MSG_TYPE);

	req = (struct scmi_msg_req_get_device_state *)buf;
	req->id = dev_id;

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = scmi_xfer_msg(&msg);
	if (ret)
		return NULL;

	resp = (struct scmi_msg_resp_get_device_state *)buf;
	switch (resp->power_state) {
		case SCMI_DEVICE_STATE_OFF:
			return "DEVICE_STATE_OFF";
		case SCMI_DEVICE_STATE_ON:
			return "DEVICE_STATE_ON";
		default:
			return "DEVICE_STATE_UNKNOWN";
	}
}
