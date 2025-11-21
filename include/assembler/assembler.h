/*
 * Copyright (c) 2025, Ibrahim KAIKAA <ibrahimkaikaa@gmail.com>
 * SPDX-License-Identifier: GPL-3.0
 */

#ifndef _ASSEMBLER_H
#define _ASSEMBLER_H

#include <assembler/reader.h>
#include <hal64.h>
#include <stddef.h>

program_t assemble(reader_t reader);

#endif
