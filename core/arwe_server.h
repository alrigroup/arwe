/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#ifndef ARWE_SERVER_H
#define ARWE_SERVER_H

#include <stddef.h>
#include <stdint.h>

#include "arwe.h"

/* Servidor HTTP event-loop (Fase 2), modelo nginx:
 * - serve só da tabela em memória (nada de disco por request);
 * - rota por nome exato validado no build (sem path-traversal);
 * - keep-alive, timeouts anti-slowloris, headers de segurança;
 * - Content-Type: application/wasm nos módulos wasm.
 */

#define ARWE_SERVER_MAX_ROUTES  128
#define ARWE_SERVER_MAX_CONN    1024
#define ARWE_SERVER_LISTEN_BACKLOG 4096
#define ARWE_SERVER_HEADER_TIMEOUT_MS 10000
#define ARWE_SERVER_BODY_TIMEOUT_MS   10000
#define ARWE_SERVER_KEEPALIVE_MS      5000
#define ARWE_SERVER_RECV_BUF   ARWE_HTTP_MAX_REQUEST

/* Cache policy por rota. */
typedef enum {
    ARWE_CACHE_NO_STORE,      /* 200 de erro/aviso */
    ARWE_CACHE_NO_CACHE,      /* entrada .arhtml: no-cache + ETag */
    ARWE_CACHE_IMMUTABLE      /* estáticos com hash: immutable 1y */
} arwe_cache_t;

typedef struct {
    const char *path;         /* rota exata (ex.: "/", "/mod/main.wasm") */
    const uint8_t *data;
    size_t        size;
    const char   *content_type;
    arwe_cache_t  cache;
} arwe_route_t;

typedef struct arwe_server arwe_server_t;

/* Cria e prepara o servidor (não abre socket ainda). */
arwe_server_t *arwe_server_create(void);

/* Adiciona uma rota estática à tabela em memória. `path` deve ser uma
   string estável (o servidor não copia). */
int arwe_server_add_route(arwe_server_t *s, const char *path,
                          const uint8_t *data, size_t size,
                          const char *content_type, arwe_cache_t cache);

/* Define uma rota de fallback customizada para 404 (ex: página notfound). */
int arwe_server_set_notfound_route(arwe_server_t *s, const arwe_route_t *route);

/* Carrega um .arweb (validado) e expõe suas seções como rotas:
 *   - <entry> e "/"  -> seção app.html (no-cache)
 *   - /mod/main.wasm -> seção mod/main.wasm (immutable, application/wasm)
 *   - /bundle.js     -> seção bundle.js (immutable, application/javascript)
 * Retorna 0 ou -1. */
int arwe_server_load_arweb(arwe_server_t *s, const arwe_unit_t *unit,
                           const uint8_t *arweb, size_t arweb_len);

/* Abre o socket, bind/listen e roda o event loop no thread atual.
   Retorna 0 em sucesso (loop sai em arwe_server_stop) ou <0 no bind. */
int arwe_server_run(arwe_server_t *s, const char *bind, uint16_t port);

/* Pede ao event loop para parar (chamado de outro thread). */
void arwe_server_stop(arwe_server_t *s);

void arwe_server_free(arwe_server_t *s);

/* Rota do servidor (para logs/query). */
int arwe_server_route_count(const arwe_server_t *s);
const arwe_route_t *arwe_server_route(const arwe_server_t *s, int idx);
int arwe_server_has_notfound_route(const arwe_server_t *s);

#endif /* ARWE_SERVER_H */