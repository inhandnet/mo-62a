// SPDX-License-Identifier: BSD-3-Clause
/*
 * SCMI core library
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SCMI Specification: https://developer.arm.com/documentation/den0056/latest/
 */

#include <string.h>
#include <scmi_raw_interface.h>
#include <scmi.h>
#include <scmi_protocol.h>
#include <socinfo.h>

#define SCMI_TOKEN_MASK		0x3FF
#define SCMI_TOKEN_SHIFT		18
#define SCMI_PROTOCOL_ID_SHIFT		10
#define SCMI_MSG_TYPE_SHIFT		8

static uint32_t monotonic_token;

void scmi_setup_header(struct scmi_msg_hdr *msg_hdr, uint32_t protocol_id, uint32_t msg_id,
			 uint32_t msg_type)
{
	msg_hdr->hdr |= (monotonic_token++ & SCMI_TOKEN_MASK) << SCMI_TOKEN_SHIFT;
	msg_hdr->hdr |= protocol_id << SCMI_PROTOCOL_ID_SHIFT;
	msg_hdr->hdr |= msg_type << SCMI_MSG_TYPE_SHIFT;
	msg_hdr->hdr |= msg_id;
}

int scmi_xfer_msg(struct scmi_raw_msg *msg)
{
	int ret, status;

	ret = scmi_raw_send(msg);
	if (ret)
		return ret;

	memset(msg->buf, 0, msg->len);
	ret = scmi_raw_recv(msg);
	if (ret)
		return ret;

	/* Return status is always second 32 bit value */
	if (msg->len < 2 * sizeof(int32_t))
		return -1;
	status = ((int32_t *)msg->buf)[1];
	if (status)
		return status;

	return 0;
}

int scmi_init(void)
{
	int ret;
	struct scmi_raw_msg msg;
	uint8_t buf[SCMI_RAW_MAX_MSG_SIZE];
	struct scmi_msg_resp_impl_version *resp;

	memset(buf, 0, sizeof(buf));
	scmi_setup_header((struct scmi_msg_hdr *)buf, SCMI_BASE_ID, SCMI_GET_IMPL_VERS_MSG_ID,
				SCMI_CMD_MSG_TYPE);
	msg.len = sizeof(struct scmi_msg_hdr);
	msg.buf = buf;
	ret = scmi_xfer_msg(&msg);
	if (ret)
		return ret;

	resp = (struct scmi_msg_resp_impl_version *)msg.buf;
	soc_info.scmi_info.version.impl_version = resp->impl_version;

	return 0;
}
