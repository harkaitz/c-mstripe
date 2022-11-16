#ifndef MSTRIPE_H
#define MSTRIPE_H

#ifdef FLYCHECK
#  define _POSIX_C_SOURCE 201000L
#endif
#ifndef _POSIX_C_SOURCE
#  error Please define _POSIX_C_SOURCE before including mstripe.h
#endif

#include <curl/curl.h>
#include <jansson.h>
#include <str/urlencode.h>

#include <stdbool.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

struct mstripe {
    char const *public_key;
    char const *secret_key;
};

typedef struct {
    CURL *curl;
    FILE *res; char *resm; size_t resmsz;
    FILE *req; char *reqm; size_t reqmsz;
    char  userpwd[256];
    bool  used;
} mstripe_cnx;

/* ---------------------------
 * ---- CONNECTION OBJECT ----
 * --------------------------- */

static inline void
mstripe_cnx_deinit(mstripe_cnx *_cnx) {
    if (_cnx->curl) curl_easy_cleanup(_cnx->curl);
    if (_cnx->res)  fclose(_cnx->res);
    if (_cnx->req)  fclose(_cnx->req);
    free(_cnx->resm);
    free(_cnx->reqm);
}

static inline bool
mstripe_cnx_init(mstripe_cnx *_cnx, const struct mstripe *_opts) {

    memset(_cnx, 0, sizeof(mstripe_cnx));
    sprintf(_cnx->userpwd, "%s:", _opts->secret_key);

    _cnx->curl = curl_easy_init();
    if (!_cnx->curl/*err*/) goto fail_curl_init;
    
    _cnx->res = open_memstream(&_cnx->resm, &_cnx->resmsz);
    if (!_cnx->res/*err*/) goto fail_errno;
    _cnx->req = open_memstream(&_cnx->reqm, &_cnx->reqmsz);
    if (!_cnx->res/*err*/) goto fail_errno;
    
    curl_easy_setopt(_cnx->curl, CURLOPT_SSL_VERIFYPEER , 0L);
    curl_easy_setopt(_cnx->curl, CURLOPT_SSL_VERIFYHOST , 0L);
    curl_easy_setopt(_cnx->curl, CURLOPT_LOW_SPEED_TIME , 30L);
    curl_easy_setopt(_cnx->curl, CURLOPT_LOW_SPEED_LIMIT, 10L);
    curl_easy_setopt(_cnx->curl, CURLOPT_ACCEPT_ENCODING, NULL);
    curl_easy_setopt(_cnx->curl, CURLOPT_VERBOSE        , 0L);
    curl_easy_setopt(_cnx->curl, CURLOPT_USERPWD, _cnx->userpwd);
    curl_easy_setopt(_cnx->curl, CURLOPT_WRITEFUNCTION, fwrite);
    curl_easy_setopt(_cnx->curl, CURLOPT_WRITEDATA, _cnx->res);
    
    return true;
 fail_errno:
    syslog(LOG_ERR, "%s", strerror(errno));
    goto cleanup;
 fail_curl_init:
    syslog(LOG_ERR, "curl_easy_init() failed.");
    goto cleanup;
 cleanup:
    mstripe_cnx_deinit(_cnx);
    return false;    
}

static inline bool
mstripe_cnx_response(mstripe_cnx *_cnx, long *_rcode, json_t **_res) {

    bool r = false;
    int e;
    json_t *j = NULL;
    json_t *p;
    json_error_t  je;
    char const *s;
    
    if (_rcode) {
        e = curl_easy_getinfo(_cnx->curl, CURLINFO_RESPONSE_CODE, _rcode);
        if (e!=CURLE_OK/*err*/) goto fail_curl;
    }
    
    fflush(_cnx->res);
    j = json_loadb(_cnx->resm, _cnx->resmsz, JSON_ALLOW_NUL, &je);
    if (!j/*err*/) goto fail_json;
    if ((p = json_object_get(j, "error")) &&
        (p = json_object_get(p, "message")) &&
        (s = json_string_value(p))) {
        syslog(LOG_ERR, "%s", s);
        goto cleanup;
    }
    
    if (_res) {
        *_res = json_incref(j);
    }

    r = true;
 cleanup:
    json_decref(j);
    return r;
 fail_curl:
    syslog(LOG_ERR, "%s", curl_easy_strerror(e));
    goto cleanup;
 fail_json:
    syslog(LOG_ERR, "Failed parsing the response: %i:%s", je.line, je.text);
    goto cleanup;
}

/* ------------------
 * ---- REQUESTS ----
 * ------------------ */

static inline bool
mstripe_post(const struct mstripe *_cfg, json_t **_res, char const *_url, char const *_kv[]) {

    bool        r = false;
    mstripe_cnx c;
    int         e,i;
    json_t     *j = NULL;

    e = mstripe_cnx_init(&c, _cfg);
    if (!e/*err*/) return false;
    
    for (i=0; _kv[i]; i+=2) {
        fprintf(c.req, "%s%s=", (i!=0)?"&":"", _kv[i]);
        urlencode_f(c.req, _kv[i+1]);
    }
    fputc('\0', c.req);
    fflush(c.req);

    curl_easy_setopt(c.curl, CURLOPT_URL, _url);
    curl_easy_setopt(c.curl, CURLOPT_POSTFIELDS, c.reqm);

    e = curl_easy_perform(c.curl);
    if (e==-1/*err*/) goto fail_curl;

    e = mstripe_cnx_response(&c, NULL, &j);
    if (!e/*err*/) goto cleanup;

    if (_res) {
        *_res = json_incref(j);
    }
    
    r = true;
 cleanup:
    json_decref(j);
    mstripe_cnx_deinit(&c);
    return r;
 fail_curl:
    syslog(LOG_ERR, "%s", curl_easy_strerror(e));
    goto cleanup;
}

static inline bool
mstripe_get(const struct mstripe *_cfg, json_t **_res, char const *_url, char const *_kv[]) {
    bool        r = false;
    mstripe_cnx c;
    int         e,i;
    json_t     *j = NULL;

    e = mstripe_cnx_init(&c, _cfg);
    if (!e/*err*/) return false;

    fputs(_url, c.req);
    for (i=0; _kv[i]; i+=2) {
        fprintf(c.req, "%s%s=", (i==0)?"?":"&", _kv[i]);
        urlencode_f(c.req, _kv[i+1]);
    }
    fputc('\0', c.req);
    fflush(c.req);

    curl_easy_setopt(c.curl, CURLOPT_URL, c.reqm);

    e = curl_easy_perform(c.curl);
    if (e==-1/*err*/) goto fail_curl;

    e = mstripe_cnx_response(&c, NULL, &j);
    if (!e/*err*/) goto cleanup;
    
    if (_res) {
        *_res = json_incref(j);
    }
    
    r = true;
 cleanup:
    json_decref(j);
    mstripe_cnx_deinit(&c);
    return r;
 fail_curl:
    syslog(LOG_ERR, "%s", curl_easy_strerror(e));
    goto cleanup;
}

/* -------------------------------
 * ---- CONFIGURATION UTILITY ----
 * ------------------------------- */
static inline bool
mstripe_init(struct mstripe *_cfg) {

    bool        rel  = (getenv("RELEASE_MODE"))?true:false;
    char const *e_pk = (rel)?"STRIPE_PUBLIC_KEY":"STRIPE_TEST_PUBLIC_KEY";
    char const *e_sk = (rel)?"STRIPE_SECRET_KEY":"STRIPE_TEST_SECRET_KEY";
    char       *s_pk = NULL;
    char       *s_sk = NULL;
    char       *m_pk = NULL;
    char       *m_sk = NULL;
    
    if (!m_pk && (s_pk = getenv(e_pk))) m_pk = strdup(s_pk);
    if (!m_sk && (s_sk = getenv(e_sk))) m_sk = strdup(s_sk);

    if (!s_pk) { syslog(LOG_ERR, "Please set %s", e_pk); return false; }
    if (!s_sk) { syslog(LOG_ERR, "Please set %s", e_sk); return false; }
    if (!m_pk) { syslog(LOG_ERR, "Not enough memory");   return false; }
    if (!m_sk) { syslog(LOG_ERR, "Not enough memory");   return false; }
    
    _cfg->public_key = m_pk;
    _cfg->secret_key = m_sk;
    return true;
}

#endif
