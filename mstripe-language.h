#ifndef MSTRIPE_LANGUAGE_H
#define MSTRIPE_LANGUAGE_H

#include <locale.h>
#include <string.h>

char const *mstripe_language(void) {
    
    struct {
        char const *fr;
        char const *to;
    } table[] = {
        {"en_GB", "en-GB"},
        {"fr_CA", "fr-CA"},
        {"pt_BR", "pt-BR"},
        {"zh_HK", "zh-HK"},
        {"zh_TW", "zh-TW"},
        /* Spain. */
        {"eu_ES", "es"},
        {"ca_ES", "es"},
        {"gl_ES", "es"},
        /* Others. */
        {"bg"   , NULL},
        {"cs"   , NULL}, {"da"   , NULL}, {"de"   , NULL},
        {"el"   , NULL}, {"en"   , NULL}, {"es"   , NULL},
        {"et"   , NULL}, {"fi"   , NULL}, {"fr"   , NULL},
        {"hr"   , NULL}, {"hu"   , NULL}, {"id"   , NULL},
        {"it"   , NULL}, {"ja"   , NULL}, {"ko"   , NULL},
        {"lt"   , NULL}, {"lv"   , NULL}, {"ms"   , NULL},
        {"mt"   , NULL}, {"nb"   , NULL}, {"nl"   , NULL},
        {"pl"   , NULL}, {"pt"   , NULL}, {"ro"   , NULL},
        {"ru"   , NULL}, {"sk"   , NULL}, {"sl"   , NULL},
        {"sv"   , NULL}, {"th"   , NULL}, {"tr"   , NULL},
        {"vi"   , NULL}, {"zh"   , NULL}, {"or"   , NULL},
        {NULL   , NULL}
    };
    char *l; int i;
    if ((l = setlocale(LC_MESSAGES, NULL))) {
        for (i=0; table[i].fr; i++) {
            if (!strncmp(l, table[i].fr, strlen(table[i].fr))) {
                return (table[i].to)?table[i].to:table[i].fr;
            }
        }
    }
    return "en";
}


#endif
