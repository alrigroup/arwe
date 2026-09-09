/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#ifndef ARWE_H
#define ARWE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ARWE_VERSION "0.2.0"

/* Limites rígidos de segurança do parser/contêiner (anti-bomba). */
#define ARWE_CFG_MAX_FILE        (64 * 1024)
#define ARWE_CFG_MAX_KEYS        256
#define ARWE_CFG_MAX_SECTIONS    16
#define ARWE_CFG_KEY_MAX         63
#define ARWE_CFG_VAL_MAX         4095
#define ARWE_CFG_SECT_MAX        63

#define ARWE_UNIT_MAX_NAME       63
#define ARWE_UNIT_MAX_SOURCE     1023
#define ARWE_UNIT_MAX_ENTRY      255
#define ARWE_UNIT_MAX_FILES      16
#define ARWE_UNIT_MAX_LANGS      8

#define ARWE_ARWEB_MAGIC         "ALRIGROUP@ARWEB"
#define ARWE_ARWEB_VERSION       1
#define ARWE_ARWEB_MAX_SECTIONS  64
#define ARWE_ARWEB_NAME_MAX      31

typedef struct arwe_app arwe_app_t;

/* Unit lógica declarada em config.arwe ([unit:<nome>]). */
typedef struct {
    char name[ARWE_UNIT_MAX_NAME + 1];
    char source[ARWE_UNIT_MAX_SOURCE + 1];
    char entry[ARWE_UNIT_MAX_ENTRY + 1];
    char route[ARWE_UNIT_MAX_ENTRY + 1];
    char files[ARWE_UNIT_MAX_FILES][ARWE_UNIT_MAX_ENTRY + 1];
    int  file_count;
    char langs[ARWE_UNIT_MAX_LANGS][16];
    int  lang_count;
    int  obfuscate;
    char copyright[4096];
} arwe_unit_t;

/* App: detém config parseada + resultados do builder. */
arwe_app_t *arwe_app_new(const char *name);
void arwe_app_free(arwe_app_t *app);
const char *arwe_app_name(const arwe_app_t *app);
const char *arwe_app_root(const arwe_app_t *app);

/* config.arwe */
int arwe_config_load(arwe_app_t *app, const char *path);
int arwe_config_parse_buffer(arwe_app_t *app, const char *buf, size_t len);
const char *arwe_config_get(const arwe_app_t *app, const char *section, const char *key,
                            const char *def);
int arwe_config_get_int(const arwe_app_t *app, const char *section, const char *key,
                        int def);
int arwe_config_get_bool(const arwe_app_t *app, const char *section, const char *key,
                         int def);
int arwe_config_unit_count(const arwe_app_t *app);
const arwe_unit_t *arwe_config_unit(const arwe_app_t *app, int idx);
const char *arwe_config_last_error(const arwe_app_t *app);

/* Builder (Fase 1) */
int arwe_builder_execute(arwe_app_t *app);
int arwe_builder_execute_out(arwe_app_t *app, const char *out_dir);

/* Mount (Fase 2): build no start -> carrega .arweb em memória -> serve
   (event loop) + registra rotas no arws (gateway em thread separada).
   Bloqueia rodando o event loop do servidor até erro/parada. */
int arwe_mount(arwe_app_t *app);

#ifdef __cplusplus
}
#endif

#endif /* ARWE_H */