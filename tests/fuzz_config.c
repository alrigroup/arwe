/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

/* Fuzz target do parser config.arwe (libFuzzer). */

#include <stddef.h>
#include <stdint.h>

#include "arwe_config.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    arwe_cfg_t cfg;
    if (size > ARWE_CFG_MAX_FILE) size = ARWE_CFG_MAX_FILE;
    arwe_cfg_parse(&cfg, (const char *)data, size);
    return 0;
}