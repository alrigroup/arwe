/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#include "arwe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#include "arwe_config.h"
#include "arwe_builder.h"
#include "arwe_gateway.h"
#include "arwe_pack.h"
#include "arwe_server.h"
#include "arwe_internal.h"

#define ARWE_APP_MAX_NAME 63

static arwe_server_t *g_active_server = NULL;

static void on_sigterm(int sig) {
    (void)sig;
    if (g_active_server) {
        arwe_server_stop(g_active_server);
    }
    _exit(0);
}

static int file_exists(const char *path) {
    if (!path) return 0;
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

arwe_app_t *arwe_app_new(const char *name) {
    if (!name || name[0] == '\0') return NULL;
    arwe_app_t *app = (arwe_app_t *)malloc(sizeof(*app));
    if (!app) return NULL;
    memset(app, 0, sizeof(*app));

    size_t n = strlen(name);
    if (n > ARWE_APP_MAX_NAME) n = ARWE_APP_MAX_NAME;
    memcpy(app->name, name, n);
    app->name[n] = '\0';
    return app;
}

void arwe_app_free(arwe_app_t *app) {
    if (app) free(app);
}

const char *arwe_app_name(const arwe_app_t *app) {
    return app ? app->name : "";
}

const char *arwe_app_root(const arwe_app_t *app) {
    return app ? app->root : "";
}

void arwe_app_set_root(arwe_app_t *app, const char *root) {
    if (!app || !root) return;
    size_t n = strlen(root);
    if (n >= sizeof(app->root)) n = sizeof(app->root) - 1;
    memcpy(app->root, root, n);
    app->root[n] = '\0';
}

const char *arwe_config_last_error(const arwe_app_t *app) {
    return app ? app->cfg.error : "";
}

int arwe_unit_fill(arwe_unit_t *u, arwe_cfg_t *cfg) {
    if (!u || !cfg) return -1;

    char sect[sizeof("unit:") + ARWE_UNIT_MAX_NAME + 1];
    snprintf(sect, sizeof(sect), "unit:%s", u->name);

    const char *src = arwe_cfg_find_def(cfg, sect, "source", "");
    const char *entry = arwe_cfg_find_def(cfg, sect, "entry", "main.arhtml");
    const char *route = arwe_cfg_find_def(cfg, sect, "route", "");
    const char *compile = arwe_cfg_find_def(cfg, sect, "compile", "");
    const char *langs = arwe_cfg_find_def(cfg, sect, "compile.lang", "");
    const char *obf = arwe_cfg_find_def(cfg, sect, "obfuscate", "no");
    const char *cpy = arwe_cfg_find_def(cfg, sect, "copyright", "");
    if (!cpy || cpy[0] == '\0') {
        cpy = arwe_cfg_find_def(cfg, "app", "copyright", "");
    }

    if (src[0] == '\0') {
        snprintf(cfg->error, sizeof(cfg->error), "unit:%s missing 'source'", u->name);
        return -1;
    }

    size_t nl = strlen(src);
    if (nl > ARWE_UNIT_MAX_SOURCE) nl = ARWE_UNIT_MAX_SOURCE;
    memcpy(u->source, src, nl);
    u->source[nl] = '\0';

    nl = strlen(entry);
    if (nl > ARWE_UNIT_MAX_ENTRY) nl = ARWE_UNIT_MAX_ENTRY;
    memcpy(u->entry, entry, nl);
    u->entry[nl] = '\0';

    /* route: path limpo da página (sem extensão). Default: "/<nome>" */
    nl = strlen(route);
    if (nl > ARWE_UNIT_MAX_ENTRY) nl = ARWE_UNIT_MAX_ENTRY;
    memcpy(u->route, route, nl);
    u->route[nl] = '\0';
    if (u->route[0] == '\0') {
        snprintf(u->route, sizeof(u->route), "/%s", u->name);
    } else if (u->route[0] != '/') {
        /* garante "/" à frente (ex: route=regras -> /regras) */
        char tmp[ARWE_UNIT_MAX_ENTRY + 1];
        snprintf(tmp, sizeof(tmp), "/%s", u->route);
        snprintf(u->route, sizeof(u->route), "%s", tmp);
    }

    u->obfuscate = (strcmp(obf, "yes") == 0 || strcmp(obf, "true") == 0);

    if (cpy && cpy[0] != '\0') {
        size_t cl = strlen(cpy);
        if (cl > sizeof(u->copyright) - 1) cl = sizeof(u->copyright) - 1;
        memcpy(u->copyright, cpy, cl);
        u->copyright[cl] = '\0';
    } else {
        u->copyright[0] = '\0';
    }

    u->file_count = 0;
    const char *p = compile;
    while (p && *p && u->file_count < ARWE_UNIT_MAX_FILES) {
        while (*p == ' ' || *p == '\t') p++;
        const char *comma = strchr(p, ',');
        size_t fl = comma ? (size_t)(comma - p) : strlen(p);
        while (fl > 0 && (p[fl - 1] == ' ' || p[fl - 1] == '\t')) fl--;
        if (fl > 0 && fl <= ARWE_UNIT_MAX_ENTRY) {
            memcpy(u->files[u->file_count], p, fl);
            u->files[u->file_count][fl] = '\0';
            u->file_count++;
        }
        p = comma ? comma + 1 : NULL;
    }

    u->lang_count = 0;
    p = langs;
    while (p && *p && u->lang_count < ARWE_UNIT_MAX_LANGS) {
        while (*p == ' ' || *p == '\t') p++;
        const char *comma = strchr(p, ',');
        size_t fl = comma ? (size_t)(comma - p) : strlen(p);
        while (fl > 0 && (p[fl - 1] == ' ' || p[fl - 1] == '\t')) fl--;
        if (fl > 0 && fl < 16) {
            memcpy(u->langs[u->lang_count], p, fl);
            u->langs[u->lang_count][fl] = '\0';
            u->lang_count++;
        }
        p = comma ? comma + 1 : NULL;
    }

    if (u->lang_count == 0) {
        snprintf(cfg->error, sizeof(cfg->error), "unit:%s missing 'compile.lang'", u->name);
        return -1;
    }

    return 0;
}

int arwe_config_parse_buffer(arwe_app_t *app, const char *buf, size_t len) {
    if (!app) return -1;
    int rc = arwe_cfg_parse(&app->cfg, buf, len);
    if (rc != 0) return rc;

    /* deriva as units [unit:<nome>] e preenche cada uma */
    int unit_count = 0;
    for (int i = 0; i < app->cfg.count && unit_count < ARWE_CFG_MAX_SECTIONS; i++) {
        const arwe_cfg_entry_t *e = &app->cfg.entries[i];
        if (strncmp(e->section, "unit:", 5) == 0 && e->section[5] != '\0') {
            /* só entra na primeira chave de cada seção */
            int seen = 0;
            for (int j = 0; j < unit_count; j++) {
                if (strcmp(app->units[j].name, e->section + 5) == 0) { seen = 1; break; }
            }
            if (!seen) {
                arwe_unit_t *u = &app->units[unit_count++];
                memset(u, 0, sizeof(*u));
                size_t nl = strlen(e->section + 5);
                if (nl > ARWE_UNIT_MAX_NAME) nl = ARWE_UNIT_MAX_NAME;
                memcpy(u->name, e->section + 5, nl);
                u->name[nl] = '\0';
            }
        }
    }
    app->unit_count = unit_count;

    /* preenche source/entry/files/langs de cada unit */
    for (int i = 0; i < app->unit_count; i++) {
        if (arwe_unit_fill(&app->units[i], &app->cfg) != 0) return -1;
    }
    return 0;
}

int arwe_config_load(arwe_app_t *app, const char *path) {
    if (!app || !path) return -1;

    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(app->cfg.error, sizeof(app->cfg.error),
                 "cannot open config: %s", path);
        return -1;
    }
    char buf[ARWE_CFG_MAX_FILE];
    size_t got = fread(buf, 1, sizeof(buf), f);
    int closed = fclose(f);
    if (closed != 0) return -1;
    if (got >= sizeof(buf)) {
        snprintf(app->cfg.error, sizeof(app->cfg.error),
                 "config too large (max %d bytes)", ARWE_CFG_MAX_FILE);
        return -1;
    }

    /* root = diretório do config (para resolver web/ e assets/ relativos) */
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *last = slash && bslash ? (slash > bslash ? slash : bslash)
                     : (slash ? slash : bslash);
    if (last) {
        size_t rlen = (size_t)(last - path);
        if (rlen > 0 && rlen < sizeof(app->root)) {
            memcpy(app->root, path, rlen);
            app->root[rlen] = '\0';
        } else if (rlen == 0) {
            strncpy(app->root, "/", sizeof(app->root) - 1);
        }
    } else {
        strncpy(app->root, ".", sizeof(app->root) - 1);
    }

    int rc = arwe_config_parse_buffer(app, buf, got);
    if (rc == 0) {
        const char *app_name = arwe_config_get(app, "app", "name", "");
        if (app_name && app_name[0] != '\0') {
            size_t n = strlen(app_name);
            if (n > ARWE_APP_MAX_NAME) n = ARWE_APP_MAX_NAME;
            memcpy(app->name, app_name, n);
            app->name[n] = '\0';
        }
    }
    return rc;
}

const char *arwe_config_get(const arwe_app_t *app, const char *section, const char *key,
                            const char *def) {
    if (!app) return def ? def : "";
    return arwe_cfg_find_def(&app->cfg, section, key, def);
}

int arwe_config_get_int(const arwe_app_t *app, const char *section, const char *key,
                        int def) {
    if (!app) return def;
    return arwe_cfg_find_int(&app->cfg, section, key, def);
}

int arwe_config_get_bool(const arwe_app_t *app, const char *section, const char *key,
                         int def) {
    if (!app) return def;
    return arwe_cfg_find_bool(&app->cfg, section, key, def);
}

int arwe_config_unit_count(const arwe_app_t *app) {
    return app ? app->unit_count : 0;
}

const arwe_unit_t *arwe_config_unit(const arwe_app_t *app, int idx) {
    if (!app || idx < 0 || idx >= app->unit_count) return NULL;
    return &app->units[idx];
}

int arwe_mount(arwe_app_t *app) {
    if (!app) return -1;

    char build_dir[1300];
    snprintf(build_dir, sizeof(build_dir), "%s/%s", arwe_app_root(app), "build");

    /* Verifica se todos os .arweb já estão presentes */
    int all_exist = 1;
    for (int i = 0; i < arwe_config_unit_count(app); i++) {
        const arwe_unit_t *u = arwe_config_unit(app, i);
        char p1[1300], p2[1300];
        arwe_path_join_suffix(p1, sizeof(p1), build_dir, u->name, ".arweb");
        arwe_path_join_suffix(p2, sizeof(p2), arwe_app_root(app), u->name, ".arweb");
        if (!file_exists(p1) && !file_exists(p2)) {
            all_exist = 0;
            break;
        }
    }

    /* 1. build no start somente se faltar algum .arweb */
    if (!all_exist) {
        if (arwe_builder_execute(app) != 0) {
            fprintf(stderr, "[arwe] mount: build failed\n");
            return -1;
        }
    }

    /* 2. carrega cada unit .arweb em memória (validado) e monta o server */
    arwe_server_t *server = arwe_server_create();
    if (!server) return -1;

    int loaded = 0;
    for (int i = 0; i < arwe_config_unit_count(app); i++) {
        const arwe_unit_t *u = arwe_config_unit(app, i);
        char path[1300];
        if (arwe_path_join_suffix(path, (int)sizeof(path), build_dir, u->name,
                                  ".arweb") != 0) {
            fprintf(stderr, "[arwe] mount: path too long (%s.arweb)\n", u->name);
            continue;
        }

        FILE *f = fopen(path, "rb");
        if (!f) {
            /* Fallback: procura direto na raiz do app (caso descompactado sem subpasta build/) */
            char root_path[1300];
            if (arwe_path_join_suffix(root_path, (int)sizeof(root_path), arwe_app_root(app), u->name, ".arweb") == 0) {
                f = fopen(root_path, "rb");
            }
        }
        if (!f) continue;
        if (fseek(f, 0, SEEK_END) != 0) { fclose(f); continue; }
        long sz = ftell(f);
        if (sz <= 0) { fclose(f); continue; }
        fseek(f, 0, SEEK_SET);
        unsigned char *data = (unsigned char *)malloc((size_t)sz);
        if (!data) { fclose(f); continue; }
        size_t got = fread(data, 1, (size_t)sz, f);
        fclose(f);

        if (arwe_pack_validate(data, got) == 0) {
            arwe_server_load_arweb(server, u, data, got);
            printf("[arwe] mounted %s.arweb (%zu bytes)\n", u->name, got);
            loaded++;
        } else {
            fprintf(stderr, "[arwe] mount: %s.arweb invalid (CRC/format)\n",
                    u->name);
            free(data);
        }
    }
    if (loaded == 0) {
        fprintf(stderr, "[arwe] mount: no .arweb loaded\n");
        arwe_server_free(server);
        return -1;
    }

    /* 3. serve + registra rotas no arws (gateway em thread separada) */
    const char *bind = arwe_config_get(app, "app", "bind", "127.0.0.1");
    int port = arwe_config_get_int(app, "app", "port", 3001);

    g_active_server = server;
#ifndef _WIN32
    signal(SIGTERM, on_sigterm);
    signal(SIGINT, on_sigterm);
#endif

    arwe_gateway_start(app, server);

    int rc = arwe_server_run(server, bind, (uint16_t)port);
    g_active_server = NULL;
    arwe_server_free(server);
    return rc;
}