#define _POSIX_C_SOURCE 201000L
#include "mstripe.h"

int main (int _argc, char *_argv[]) {

    bool           rel        = (getenv("RELEASE_MODE"))?true:false;
    char const    *return_url = getenv("STRIPE_RETURN_URL");
    struct mstripe mstripe    = {0};
    char const    *k;
    int            e;

    openlog("mstripe", LOG_PERROR, LOG_USER);

    /* Read keys. */
    mstripe.public_key = getenv(k = (rel)?"STRIPE_PUBLIC_KEY":"STRIPE_TEST_PUBLIC_KEY");
    if (!mstripe.public_key) {
        syslog(LOG_ERR, "Please set %s", k);
        return 1;
    }
    mstripe.secret_key = getenv(k = (rel)?"STRIPE_SECRET_KEY":"STRIPE_TEST_SECRET_KEY");
    if (!mstripe.secret_key) {
        syslog(LOG_ERR, "Please set %s", k);
        return 1;
    }

    /* Print help. */
    if (_argc == 1 || !strcmp(_argv[1], "--help") || !strcmp(_argv[1], "-h")) {
        fputs("c-dashboard-url CUSTOMER... : Print the URL for the clients.\n",
              stdout);
        return 0;
    }

    /* Perform operations. */
    if (!strcmp(_argv[1], "c-dashboard-url")) {
        for (int i=2; i<_argc; i++) {
            e = mstripe_customer_dashboard(&mstripe, _argv[i], stdout, return_url);
            if (!e/*err*/) return 1;
            fputc('\n', stdout);
        }
    }
    
    return 0;
}
