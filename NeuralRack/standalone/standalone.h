/*
 * standalone.h
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2025 brummer <brummer@web.de>
 */


#pragma once

#ifndef STANDALONE_H_
#define STANDALONE_H_

#ifdef __cplusplus
extern "C" {
#endif


struct Interface {
    uint32_t standalone;
};

#include "widgets.h"

int 
ends_with(const char* name, const char* extension) {
    const char* ldot = strrchr(name, '.');
    if (ldot != NULL) {
        size_t length = strlen(extension);
        return strncmp(ldot + 1, extension, length) == 0;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif //STANDALONE_H_
