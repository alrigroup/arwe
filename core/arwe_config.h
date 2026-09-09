/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#ifndef ARWE_CONFIG_H
#define ARWE_CONFIG_H

#include <stddef.h>
#include "arwe.h"

typedef struct {
    char section[ARWE_CFG_SECT_MAX + 1];
    char key[ARWE_CFG_KEY_MAX + 1];
    char value[ARWE_CFG_VAL_MAX + 1];
} arwe_cfg_entry_t;

typedef struct {
    arwe_cfg_entry_t entries[ARWE_CFG_MAX_KEYS];
    int count;
    char error[256];
} arwe_cfg_t;

/* Parse de um buffer config.arwe (formato proprietário estilo arws.cfg).
   Estrito: sem alocação, sem strtol, com limites rígidos. */
int arwe_cfg_parse(arwe_cfg_t *cfg, const char *buf, size_t len);

const char *arwe_cfg_find(const arwe_cfg_t *cfg, const char *section, const char *key);
const char *arwe_cfg_find_def(const arwe_cfg_t *cfg, const char *section,
                              const char *key, const char *def);
int arwe_cfg_find_int(const arwe_cfg_t *cfg, const char *section, const char *key,
                      int def);
int arwe_cfg_find_bool(const arwe_cfg_t *cfg, const char *section, const char *key,
                       int def);

#endif /* ARWE_CONFIG_H */