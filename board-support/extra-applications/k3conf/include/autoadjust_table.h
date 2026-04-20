/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Table printing library helpers
 *
 * Copyright (C) 2010-2019 Texas Instruments Incorporated - https://www.ti.com/
 */


#ifndef __AUTOADJUST_TABLE_H
#define __AUTOADJUST_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define TABLE_MAX_ROW		4000
#define TABLE_MAX_COL		20
#define TABLE_MAX_ELT_LEN	100

#define TABLE_HAS_TITLE		1
#define TABLE_HAS_SUBTITLE	2

int autoadjust_table_init(
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN]);

int autoadjust_table_print(
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN],
	unsigned int row_nbr, unsigned int col_nbr);

int autoadjust_table_fprint(FILE *stream,
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN],
	unsigned int row_nbr, unsigned int col_nbr);

int autoadjust_table_generic_fprint(FILE *stream,
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN],
	unsigned int row_nbr, unsigned int col_nbr, unsigned int flags);

int autoadjust_table_strncpy(
	char table[TABLE_MAX_ROW][TABLE_MAX_COL][TABLE_MAX_ELT_LEN],
	unsigned int row, unsigned int col, char s[TABLE_MAX_ELT_LEN]);


#endif
