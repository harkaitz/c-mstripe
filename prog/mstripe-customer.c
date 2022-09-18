#define _POSIX_C_SOURCE 201000L
#include "../mstripe-customer.h"
#include <getopt.h>

static char const _help[] =
    "Usage: mstripe-customer [-i CUSTOMER-ID][...]" "\n"
    "-u : Print dashboard URL (requires -i)."       "\n"
    "-j : Print customer JSON (requires -i)."       "\n"
    "-l : Print customer list with emails."         "\n";

int main (int _argc, char *_argv[]) {

    struct mstripe mstripe     = {0};
    char const    *customer_id = NULL;
    json_t        *customer    = NULL;
    json_t        *customers   = NULL;
    json_t        *j;
    bool           opt_u       = false;
    bool           opt_j       = false;
    bool           opt_l       = false;
    int            ret         = 1;
    int            opt,e,i;

    if (_argc == 1) {
        fputs(_help, stdout);
        return 0;
    }
    
    openlog("mstripe", LOG_PERROR, LOG_USER);

    if (!mstripe_init(&mstripe)) return 1;

    while((opt = getopt (_argc, _argv, "hi:ujl")) != -1) {
        switch (opt) {
        case 'h': fputs(_help, stdout); return 0;
        case 'i': customer_id = optarg; break;
        case 'u': opt_u = true; break;
        case 'j': opt_j = true; break;
        case 'l': opt_l = true; break;
        case '?':
        default:
            return 1;
        }
    }

    if (opt_u)
        {
            if (!customer_id/*err*/) goto fail_missing_id;
            e = mstripe_customer_dashboard(&mstripe, customer_id, stdout, NULL);
            if (!e/*err*/) goto cleanup;
        }
    else if (opt_j)
        {
            if (!customer_id/*err*/) goto fail_missing_id;
            e = mstripe_customer_fetch(&mstripe, customer_id, &customer);
            if (!e/*err*/) goto cleanup;
            if (mstripe_customer_deleted(customer)/*err*/) goto fail_deleted;
            json_dumpf(customer, stdout, JSON_INDENT(4));
            fputc('\n', stdout);
        }
    else if (opt_l)
        {
            bool has_more = 0;
            do {
                e = mstripe_customer_list(&mstripe, &customers, &has_more);
                if (!e/*err*/) goto cleanup;
                json_array_foreach(customers, i, j) {
                    char const *id    = json_string_value(json_object_get(j, "id"));
                    char const *email = json_string_value(json_object_get(j, "email"));
                    fprintf(stdout, "%s %s\n", id, (email)?email:"");
                }
            } while (has_more);   
        }

    ret = 0;
 cleanup:
    if (customer)  json_decref(customer);
    if (customers) json_decref(customers);
    return ret;
 fail_missing_id:
    syslog(LOG_ERR, "Please specify a customer id with -i");
    goto cleanup;
 fail_deleted:
    syslog(LOG_ERR, "The customer was deleted.");
    goto cleanup;
}
