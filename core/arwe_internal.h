/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#ifndef ARWE_INTERNAL_H
#define ARWE_INTERNAL_H

#include "arwe.h"
#include "arwe_config.h"

#define ARWE_APP_MAX_NAME 63

struct arwe_app {
    char name[ARWE_APP_MAX_NAME + 1];
    char root[1024];
    arwe_cfg_t cfg;
    arwe_unit_t units[ARWE_CFG_MAX_SECTIONS];
    int unit_count;
};

/* Preenche uma unit (source/entry/files/langs) a partir do config.
   Compartilhado entre core e builder. Retorna 0 ou -1 (erro no cfg). */
int arwe_unit_fill(arwe_unit_t *u, arwe_cfg_t *cfg);

#endif /* ARWE_INTERNAL_H */