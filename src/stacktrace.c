/*
 * SPDX-FileCopyrightText: 2022 Jarosław Wierzbicki
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#define _GNU_SOURCE
#define UNW_LOCAL_ONLY
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libunwind.h>
#include <dlfcn.h>

#include "stacktrace.h"

static void _print_frame(size_t frame_no, unw_cursor_t *cursor)
{
    uintptr_t obj_offset = 0;
    unw_word_t ip, sym_offset = 0;
    const char *obj_name = NULL, *sym_name = NULL;
    int ret;

    /* get program counter/instruction pointer value */
    ret = unw_get_reg(cursor, UNW_REG_IP, &ip);
    if (ret != 0) {
        printf("#%-2zu  <frame decoding error>\n", frame_no);
        return;
    }

    /* get symbol name */
    char buffer[32];
    ret = unw_get_proc_name(cursor, buffer, sizeof(buffer), &sym_offset);
    if (ret == 0) {
        sym_name = buffer;
    } else if (ret == -UNW_ENOMEM) {
        /* symbol name has been truncated so add "..." at the end */
        memcpy(buffer+sizeof(buffer)-4, "...", 3);
        buffer[sizeof(buffer)-1] = '\0';
        sym_name = buffer;
    }

    /* get library info */
    Dl_info dl_info = {0};
    ret = dladdr((void *) ip, &dl_info);
    if (ret != 0) {
        if (dl_info.dli_fname != NULL)
            obj_name = dl_info.dli_fname;
        if (dl_info.dli_fbase != NULL)
            obj_offset = (uintptr_t) ip - (uintptr_t) dl_info.dli_fbase;
        if (sym_name == NULL && dl_info.dli_sname != NULL)
            sym_name = dl_info.dli_sname;
    }

    if (obj_name == NULL)
        obj_name = "???";

    if (sym_name == NULL)
        sym_name = "??\?()";

    printf("#%-2zu  0x%" PRIxPTR "  %s()+0x%" PRIxPTR " (in %s+0x%" PRIxPTR ")\n",
           frame_no, (uintptr_t) ip, sym_name, (uintptr_t) sym_offset, obj_name,
           obj_offset);
}

void dump_stack(void)
{
    int ret;
    unw_context_t context;

    ret = unw_getcontext(&context);
    if (ret != 0) {
        fprintf(stderr, "unw_getcontext() failed\n");
        return;
    }

    unw_cursor_t cursor;

    ret = unw_init_local(&cursor, &context);
    if (ret != 0) {
        fprintf(stderr, "unw_init_local() failed with error code: %d\n", ret);
        return;
    }

    size_t frame = 0;
    while ((ret = unw_step(&cursor)) > 0)
        _print_frame(frame++, &cursor);

    if (ret != 0) {
        fprintf(stderr, "unw_step() failed with error code: %d\n", ret);
    }
}
