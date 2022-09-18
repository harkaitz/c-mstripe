#ifndef _MSTRIPE_CUSTOMER_H_
#define _MSTRIPE_CUSTOMER_H_

#include "mstripe.h"

static inline bool
mstripe_customer_dashboard(const struct mstripe *_cfg,
                           char const            _customer[],
                           FILE                 *_fp_url,
                           char const            _opt_return_url[])
{
    char const *u     = "https://api.stripe.com/v1/billing_portal/sessions";
    char const *k[20] = { NULL };
    size_t      ksz   = 0;
    json_t     *j     = NULL;
    bool        r     = false;
    char const *s;
    int         e;

    k[ksz++] = "customer";
    k[ksz++] = _customer;
    if (_opt_return_url) {
        k[ksz++] = "return_url";
        k[ksz++] = _opt_return_url;
    }
    k[ksz++] = NULL;
    
    
    e = mstripe_post(_cfg, &j, u, k);
    if (!e/*err*/) return false;

    s = json_string_value(json_object_get(j, "url"));
    if (!s/*err*/) goto fail_response;

    if (_fp_url) {
        fputs(s, _fp_url);
    }

    r = true;
 cleanup:
    json_decref(j);
    return r;
 fail_response:
    syslog(LOG_ERR, "Stripe returned an invalid response.");
    goto cleanup;
}

static inline bool
mstripe_customer_fetch(const struct mstripe *_cfg,
                       char const            _customer_id[],
                       json_t              **_customer)
{
    static char const url_f[] = "https://api.stripe.com/v1/customers/%s";
    char const       *kv[]    = {"expand[]", "subscriptions", NULL};
    char              url[sizeof(url_f)+strlen(_customer_id)+2];
    int               e;
    sprintf(url, url_f, _customer_id);
    e = mstripe_get(_cfg, _customer, url, kv);
    if (!e/*err*/) return false;
    return true;
}

static inline bool
mstripe_customer_deleted(json_t *_customer)
{
    if (!_customer) return false;
    if (json_boolean_value(json_object_get(_customer, "deleted"))) return true;
    return false;
}

static inline bool
mstripe_customer_list(const struct mstripe *_cfg, json_t **_customers, bool *_has_more)
{
    json_t        *j           = NULL;
    size_t         kvsz        = 0;
    char const    *kv[20];
    int            e;

    kv[kvsz++] = "limit";
    kv[kvsz++] = "100";
    if (_customers && (*_customers) && _has_more && (*_has_more)) {
        size_t n = json_array_size(*_customers);
        if (n && (j = json_array_get(*_customers, n-1))) {
            kv[kvsz++] = "starting_after";
            kv[kvsz++] = json_string_value(json_object_get(j, "id"));
            j = NULL;
        }
    }
    kv[kvsz++] = NULL;

    e = mstripe_get(_cfg, &j, "https://api.stripe.com/v1/customers", kv);
    if (!e/*err*/) return false;

    if (_customers) {
        if (*_customers) json_decref(*_customers);
        *_customers = json_incref(json_object_get(j, "data"));
    }
    if (_has_more) {
        *_has_more = json_boolean_value(json_object_get(j, "has_more"));
    }
    
    json_decref(j);
    return true;
}


#endif
