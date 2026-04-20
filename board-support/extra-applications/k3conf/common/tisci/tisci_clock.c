// SPDX-License-Identifier: BSD-3-Clause
/*
 * TISCI clock ops library
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#include <string.h>
#include <sec_proxy.h>
#include <tisci_protocol.h>
#include <tisci.h>

static const char clock_state[MAX_CLOCK_HW_STATES + 1][MAX_CLOCK_STATE_LENGTH] = {
	[MSG_CLOCK_HW_STATE_NOT_READY] = "CLK_STATE_NOT_READY",
	[MSG_CLOCK_HW_STATE_READY] = "CLK_STATE_READY",
	[MAX_CLOCK_HW_STATES] = "CLK_STATE_UNKNOWN"
};

static int ti_sci_set_clock_state(uint32_t dev_id, uint32_t clk_id,
				  uint32_t flags, uint8_t state)
{
	struct ti_sci_msg_req_set_clock_state *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_SET_CLOCK_STATE, flags);
	req = (struct ti_sci_msg_req_set_clock_state *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}
	req->request_state = state;

	msg.len = sizeof(*req);
	msg.buf = buf;
	return ti_sci_xfer_msg(&msg);
}

int ti_sci_cmd_get_clk(uint32_t dev_id, uint32_t clk_id)
{
	return ti_sci_set_clock_state(dev_id, clk_id,
				      MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
				      MSG_CLOCK_SW_STATE_REQ);
}

int ti_sci_cmd_put_clk(uint32_t dev_id, uint32_t clk_id)
{
	return ti_sci_set_clock_state(dev_id, clk_id,
				      MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE,
				      MSG_CLOCK_SW_STATE_UNREQ);
}

const char *ti_sci_cmd_get_clk_state(uint32_t dev_id, uint32_t clk_id)
{
	struct ti_sci_msg_resp_get_clock_state *resp;
	struct ti_sci_msg_req_get_clock_state *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_GET_CLOCK_STATE, 0);
	req = (struct ti_sci_msg_req_get_clock_state *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return NULL;

	resp = (struct ti_sci_msg_resp_get_clock_state *)buf;

	return clock_state[resp->current_state];
}

int ti_sci_cmd_set_clk_freq(uint32_t dev_id, uint32_t clk_id, uint64_t freq)
{
	struct ti_sci_msg_req_set_clock_freq *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_SET_CLOCK_FREQ, 0);
	req = (struct ti_sci_msg_req_set_clock_freq *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}
	/* ToDo: Get best match freq and set that freq */
	req->min_freq_hz = freq;
	req->target_freq_hz = freq;
	req->max_freq_hz = freq;

	msg.len = sizeof(*req);
	msg.buf = buf;
	return ti_sci_xfer_msg(&msg);
}

int ti_sci_cmd_get_clk_freq(uint32_t dev_id, uint32_t clk_id, uint64_t *freq)
{
	struct ti_sci_msg_resp_get_clock_freq *resp;
	struct ti_sci_msg_req_get_clock_freq *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_GET_CLOCK_FREQ, 0);
	req = (struct ti_sci_msg_req_get_clock_freq *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return ret;

	resp = (struct ti_sci_msg_resp_get_clock_freq *)buf;
	*freq = resp->freq_hz;

	return 0;
}

int ti_sci_cmd_get_clk_parent(uint32_t dev_id, uint32_t clk_id, uint32_t *parent_clk_id)
{
	struct ti_sci_msg_get_clock_parent_resp *resp;
	struct ti_sci_msg_get_clock_parent_req *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;
	int ret;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_GET_CLOCK_PARENT, 0);
	req = (struct ti_sci_msg_get_clock_parent_req *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}

	msg.len = sizeof(*req);
	msg.buf = buf;
	ret = ti_sci_xfer_msg(&msg);
	if (ret)
		return ret;

	resp = (struct ti_sci_msg_get_clock_parent_resp *)buf;
	if (resp->parent_clk_id < 255)
		*parent_clk_id = resp->parent_clk_id;
	else
		*parent_clk_id = resp->parent_clk_id_32;

	return 0;
}

int ti_sci_cmd_set_clk_parent(uint32_t dev_id, uint32_t clk_id, uint32_t parent_clk_id)
{
	struct ti_sci_msg_set_clock_parent_req *req;
	uint8_t buf[SEC_PROXY_MAX_MSG_SIZE];
	struct k3_sec_proxy_msg msg;

	memset(buf, 0, sizeof(buf));
	ti_sci_setup_header((struct ti_sci_msg_hdr *)buf,
			    TI_SCI_MSG_SET_CLOCK_PARENT, 0);
	req = (struct ti_sci_msg_set_clock_parent_req *)buf;
	req->dev_id = dev_id;
	if (clk_id < 255) {
		req->clk_id = clk_id;
	} else {
		req->clk_id = 255;
		req->clk_id_32 = clk_id;
	}
	if (parent_clk_id < 255) {
		req->parent_clk_id = parent_clk_id;
	} else {
		req->parent_clk_id = 255;
		req->parent_clk_id_32 = parent_clk_id;
	}

	msg.len = sizeof(*req);
	msg.buf = buf;
	return ti_sci_xfer_msg(&msg);
}
