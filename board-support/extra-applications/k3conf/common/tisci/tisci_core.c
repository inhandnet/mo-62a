// SPDX-License-Identifier: BSD-3-Clause
/*
 * TISCI core library
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <string.h>
#include <sec_proxy.h>
#include <tisci.h>
#include <tisci_protocol.h>
#include <socinfo.h>

static int seq = 0;

void ti_sci_setup_header(struct ti_sci_msg_hdr *hdr, uint16_t type,
			 uint32_t flags)
{
	hdr->type = type;
	hdr->host = soc_info.host_id;
	hdr->seq = seq++;
	hdr->flags = TI_SCI_FLAG_REQ_ACK_ON_PROCESSED | flags;
}

int ti_sci_xfer_msg(struct k3_sec_proxy_msg *msg)
{
	int ret;

	if (!msg->len || !msg->buf)
		return -1;

	ret = k3_sec_proxy_send(msg);
	if (ret)
		return ret;

	memset(msg->buf, 0, msg->len);
	ret = k3_sec_proxy_recv(msg);
	if (ret)
		return ret;

	if (!ti_sci_is_response_ack(msg->buf))
		return -1;

	return 0;
}

int ti_sci_init(void)
{
	struct ti_sci_msg_resp_version *version;
	struct ti_sci_msg_dm_resp_version *dmversion;
	struct ti_sci_version_info *glb_ver;
	struct ti_sci_dm_version_info *glb_dm_ver;
	struct ti_sci_caps_info *glb_caps;
	struct ti_sci_msg_resp_caps *caps;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf, TI_SCI_MSG_VERSION,
			    0);

	msg.len = sizeof(struct ti_sci_msg_hdr);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return ret;

	version = (struct ti_sci_msg_resp_version *)buf;
	glb_ver = &soc_info.sci_info.version;
	glb_ver->abi_major = version->abi_major;
	glb_ver->abi_minor = version->abi_minor;
	glb_ver->firmware_version = version->version;
	strncpy(glb_ver->firmware_description, version->firmware_description,
		sizeof(glb_ver->firmware_description));

	glb_caps = &soc_info.sci_info.version.caps_info;
	glb_caps->valid = 0;
	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf, TISCI_MSG_QUERY_FW_CAPS,
			    0);

	msg.len = sizeof(struct ti_sci_msg_hdr);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	/* if we did not get a caps info.. ignore.. old firmware */
	if (ret)
		return 0;

	caps = (struct ti_sci_msg_resp_caps *)buf;
	glb_caps->valid = 1;
	glb_caps->fw_caps = caps->fw_caps;

	/* DM version is ONLY valid if caps is set for split mode DM */
	glb_dm_ver = &soc_info.sci_info.version.dm_info;
	glb_dm_ver->valid = 0;
	if (!(caps->fw_caps & 0x100))
		return 0;

	/* Add DM Version information as well */
	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf, TI_SCI_MSG_DM_VERSION,
			    0);

	msg.len = sizeof(struct ti_sci_msg_hdr);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	/* if we did not get a DM rev info.. ignore.. old firmware */
	if (ret)
		return 0;

	dmversion = (struct ti_sci_msg_dm_resp_version *)buf;
	glb_dm_ver->valid = 1;
	glb_dm_ver->dm_version = dmversion->dm_version;
	glb_dm_ver->sub_version = dmversion->sub_version;
	glb_dm_ver->patch_version = dmversion->patch_version;
	glb_dm_ver->abi_major = dmversion->abi_major;
	glb_dm_ver->abi_minor = dmversion->abi_minor;
	strncpy(glb_dm_ver->rm_pm_hal_version, dmversion->rm_pm_hal_version,
		sizeof(glb_dm_ver->rm_pm_hal_version));
	strncpy(glb_dm_ver->sci_server_version, dmversion->sci_server_version,
		sizeof(glb_dm_ver->sci_server_version));

	return 0;
}

int ti_sci_cmd_get_range(uint16_t type, uint16_t subtype, uint16_t host_id,
				struct ti_sci_rm_desc *desc)
{
	struct ti_sci_msg_resp_get_resource_range *resp;
	struct ti_sci_msg_req_get_resource_range *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret = 0;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_GET_RESOURCE_RANGE, 0);
	req = (struct ti_sci_msg_req_get_resource_range *)buf;
	req->type = type;
	req->subtype = subtype;
	req->secondary_host = host_id;

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return ret;

	resp = (struct ti_sci_msg_resp_get_resource_range *)buf;
	desc->start = resp->range_start;
	desc->num = resp->range_num;
	desc->start_sec = resp->range_start_sec;
	desc->num_sec = resp->range_num_sec;

	return 0;
}
