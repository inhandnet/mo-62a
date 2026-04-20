// SPDX-License-Identifier: BSD-3-Clause
/*
 * SCMI Raw Interface driver
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *
 * SCMI Specification: https://developer.arm.com/documentation/den0056/latest/
 */

#include <stdio.h>
#include <stdint.h>
#include <scmi_raw_interface.h>
#include <socinfo.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define TRANS_DEFAULT_BASE_PATH	"/sys/kernel/debug/scmi/0"
#define TRANS_MESSAGE_FILE		"raw/message"
#define TRANS_MAX_RX_TIMEOUT_FILE	"transport/max_rx_timeout_ms"

struct arm_scmi_raw_interface_info raw_info;

int scmi_raw_send(struct scmi_raw_msg *msg)
{
	int scmi_raw_base_fd = -1, fd_message = -1, ret = 0;
	struct pollfd pfd;

	scmi_raw_base_fd = open(TRANS_DEFAULT_BASE_PATH, O_RDONLY | O_NONBLOCK);
	if (scmi_raw_base_fd < 0) {
		fprintf(stderr, "SEND: open(%s) failed, no access to raw interface! %d\n",
			TRANS_DEFAULT_BASE_PATH, scmi_raw_base_fd);
		ret = -1;
		goto exit;
	}

	fd_message = openat(scmi_raw_base_fd, TRANS_MESSAGE_FILE, O_RDWR | O_SYNC);
	if (fd_message < 0) {
		fprintf(stderr, "SEND: openat(%s/%s) failed! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MESSAGE_FILE, fd_message);
		ret = -1;
		goto exit;
	}

	pfd.fd = fd_message;
	pfd.events = POLLIN;

	ret = write(fd_message, msg->buf, msg->len);
	if (ret < 0) {
		fprintf(stderr, "SEND: write to %s/%s failed! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MESSAGE_FILE, ret);
		ret = -1;
		goto exit;
	}

	ret = poll(&pfd, 1, raw_info.max_rx_timeout_ms);
	if (ret <= 0) {
		fprintf(stderr, "SEND: fd poll timeout or error! %d\n", ret);
		ret = -1;
		goto exit;
	}
	ret = 0;

exit:
	if (fd_message >= 0)
		close(fd_message);
	if (scmi_raw_base_fd >= 0)
		close(scmi_raw_base_fd);

	return ret;
}

int scmi_raw_recv(struct scmi_raw_msg *msg)
{
	int scmi_raw_base_fd = -1, fd_message = -1, ret = 0, num_bytes;

	scmi_raw_base_fd = open(TRANS_DEFAULT_BASE_PATH, O_RDONLY | O_NONBLOCK);
	if (scmi_raw_base_fd < 0) {
		fprintf(stderr, "RECV: open(%s) failed, no access to raw interface! %d\n",
			TRANS_DEFAULT_BASE_PATH, scmi_raw_base_fd);
		ret = -1;
		goto exit;
	}

	fd_message = openat(scmi_raw_base_fd, TRANS_MESSAGE_FILE, O_RDONLY | O_NONBLOCK);
	if (fd_message < 0) {
		fprintf(stderr, "RECV: openat(%s/%s) failed! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MESSAGE_FILE, fd_message);
		ret = -1;
		goto exit;
	}

	num_bytes = read(fd_message, msg->buf, SCMI_RAW_MAX_MSG_SIZE);
	if (num_bytes == 0) {
		fprintf(stderr, "RECV: read at %s/%s returned empty! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MESSAGE_FILE, ret);
		ret = -1;
		goto exit;
	} else if (num_bytes < 0) {
		fprintf(stderr, "RECV: read at %s/%s error! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MESSAGE_FILE, ret);
		ret = -1;
		goto exit;
	}
	msg->len = num_bytes;
	ret = 0;

exit:
	if (fd_message >= 0)
		close(fd_message);
	if (scmi_raw_base_fd >= 0)
		close(scmi_raw_base_fd);

	return ret;
}

int scmi_raw_init(void)
{
	int scmi_raw_base_fd = -1, max_rx_timeout_fd = -1, ret = 0;

	scmi_raw_base_fd = open(TRANS_DEFAULT_BASE_PATH, O_RDONLY | O_NONBLOCK);
	if (scmi_raw_base_fd < 0) {
		fprintf(stderr, "INIT: open(%s) failed, no access to raw interface! %d\n",
			TRANS_DEFAULT_BASE_PATH, scmi_raw_base_fd);
		ret = -1;
		goto exit;
	}

	max_rx_timeout_fd = openat(scmi_raw_base_fd, TRANS_MAX_RX_TIMEOUT_FILE,
			O_RDONLY | O_NONBLOCK);
	if (max_rx_timeout_fd < 0) {
		fprintf(stderr, "INIT: openat(%s/%s) failed! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MAX_RX_TIMEOUT_FILE, max_rx_timeout_fd);
		ret = -1;
		goto exit;
	}

	ret = read(max_rx_timeout_fd, &raw_info.max_rx_timeout_ms,
			sizeof(raw_info.max_rx_timeout_ms));
	if (ret < 0) {
		fprintf(stderr, "INIT: read at %s/%s failed! %d\n",
			TRANS_DEFAULT_BASE_PATH, TRANS_MAX_RX_TIMEOUT_FILE, ret);
		ret = -1;
		goto exit;
	}
	ret = 0;

exit:
	if (max_rx_timeout_fd >= 0)
		close(max_rx_timeout_fd);
	if (scmi_raw_base_fd >= 0)
		close(scmi_raw_base_fd);

	return ret;
}
