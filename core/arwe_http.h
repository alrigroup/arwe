/*
 * Copyright (c) ALRIGROUP and its affiliates.
 *
 * This code is licensed under the ARGLR - ALRI GROUP LICENSE RESERVED
 * found in the LICENSE file in the root directory of this source tree
 * and at: https://github.com/alrigroup/licenses/tree/main
 */

#ifndef ARWE_HTTP_H
#define ARWE_HTTP_H

#include <stddef.h>
#include <stdint.h>

/* Parser HTTP estrito (Fase 2), estilo picohttpparser/nginx:
 * - zero-copy: devolve ponteiros para o buffer cru;
 * - sem strtol/sscanf em números: Content-Length parseado inline com
 *   guarda de overflow (padrão HAProxy);
 * - completo-antes-de-parsear (anti buffer over-read);
 * - stateless: o chamador mantém o buffer; este módulo só lê.
 */

#define ARWE_HTTP_MAX_METHOD   16
#define ARWE_HTTP_MAX_PATH     2048
#define ARWE_HTTP_MAX_VERSION  16
#define ARWE_HTTP_MAX_HEADERS  64
#define ARWE_HTTP_MAX_REQUEST  (64 * 1024)

/* Cabeçalho já conhecido, indexado por enum (não por strcmp). */
typedef enum {
    ARWE_HTTP_H_CONTENT_LENGTH = 0,
    ARWE_HTTP_H_CONNECTION,
    ARWE_HTTP_H_HOST,
    ARWE_HTTP_H_ACCEPT_ENCODING,
    ARWE_HTTP_H_RANGE,
    ARWE_HTTP_H_COOKIE,
    ARWE_HTTP_H_UNKNOWN,
    ARWE_HTTP_H_COUNT = ARWE_HTTP_H_UNKNOWN
} arwe_http_hdr_id_t;

typedef struct {
    const char *name;
    size_t      name_len;
    const char *value;
    size_t      value_len;
    arwe_http_hdr_id_t id;
} arwe_http_header_t;

typedef struct {
    const char *method;
    size_t      method_len;
    const char *path;
    size_t      path_len;
    const char *version;
    size_t      version_len;

    arwe_http_header_t headers[ARWE_HTTP_MAX_HEADERS];
    int  header_count;

    /* Content-Length já validado (unsigned, sem overflow). -1 = ausente. */
    int64_t content_length;
    /* Total de bytes do corpo esperado (0 se sem corpo). */
    size_t  body_len;

    /* Offset (no buffer) do fim dos headers ("\r\n\r\n"). */
    size_t  header_end;

    /* estado de completeza */
    int  complete;      /* 1 = requisição inteira está no buffer */
    int  bad;           /* 1 = malformada (retornar 400) */
} arwe_http_req_t;

/* Resultados do parse. */
#define ARWE_HTTP_NEED_MORE 0
#define ARWE_HTTP_COMPLETE  1
#define ARWE_HTTP_BAD      -1

/* Parseia `buf[0..len)`. Devolve ARWE_HTTP_NEED_MORE se faltam dados,
 * ARWE_HTTP_COMPLETE se a requisição inteira chegou, ARWE_HTTP_BAD em
 * malformada (o chamador deve responder 400 e fechar). */
int arwe_http_parse(arwe_http_req_t *req, const char *buf, size_t len);

/* Pega um header por id (primeiro que aparecer) ou NULL. */
const char *arwe_http_header(const arwe_http_req_t *req, arwe_http_hdr_id_t id,
                             size_t *value_len);

/* Compara método com uma string literal (sem depender de NUL). */
int arwe_http_method_is(const arwe_http_req_t *req, const char *m);

/* Imprime o valor de um header num buffer terminado em NUL (seguro). */
int arwe_http_header_copy(const arwe_http_req_t *req, arwe_http_hdr_id_t id,
                          char *out, size_t cap);

#endif /* ARWE_HTTP_H */