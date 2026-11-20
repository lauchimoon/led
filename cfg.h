#ifndef CFG_H
#define CFG_H

#include <stdbool.h>

#define CFG_FILEPATH ".ledrc"

typedef enum {
    CFG_PARAM_FULLSCREEN = 0,
    CFG_PARAM_FONTSIZE,
    CFG_PARAM_NUMBER,
    CFG_PARAM_THEME,
} LedCfgParam;

typedef struct LedCfg {
    bool fullscreen;
    int fontsize;
    bool number;
    int theme;
} LedCfg;

int cfg_read(LedCfg *cfg);
int cfg_write(LedCfg *cfg);

#endif // CFG_H
