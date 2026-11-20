#include "cfg.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CFG_LINE_SIZE 1024

static char *trim(char *s);
static int parse_param(char *line, LedCfgParam *param, int *val);
static bool is_numeric(const char *s, int *result);
static void read_param_to_cfg(LedCfgParam param, LedCfg *cfg, int val);

int cfg_read(LedCfg *cfg)
{
    cfg->fullscreen = false;
    cfg->fontsize = 24;
    cfg->number = false;
    cfg->theme = 1;

    char *home = getenv("HOME");
    if (!home) // weird...
        return 1;

    char filepath[4096] = { 0 };
    snprintf(filepath, 4096, "%s/%s", home, CFG_FILEPATH);

    FILE *f = fopen(filepath, "r");

    // Create .ledrc file with default settings
    if (!f) {
        f = fopen(filepath, "w");
        if (!f) {
            fprintf(stderr, "error: could not create configuration file");
            return 1;
        }
        fprintf(f, "fullscreen=false\nfontsize=24\nnumber=false\ntheme=1\n");
        fclose(f);
        return 0;
    }

    char line[CFG_LINE_SIZE] = { 0 };
    while (fgets(line, CFG_LINE_SIZE, f) != NULL) {
        char *content = trim(line);
        if (*content == '\0' || *content == '#' || *content == ';')
            continue;

        LedCfgParam param;
        int val;
        if (parse_param(content, &param, &val) != 0) {
            fclose(f);
            return 1;
        }

        read_param_to_cfg(param, cfg, val);
    }

    fclose(f);
    return 0;
}

int cfg_write(LedCfg *cfg)
{
    char *home = getenv("HOME");
    if (!home)
        return 1;

    char filepath[4096] = { 0 };
    snprintf(filepath, 4096, "%s/%s", home, CFG_FILEPATH);

    FILE *f = fopen(filepath, "w");
    if (!f)
        return 2;

    fprintf(f, "fullscreen=%s\nfontsize=%d\nnumber=%s\ntheme=%d\n",
                cfg->fullscreen? "true" : "false",
                cfg->fontsize,
                cfg->number? "true" : "false",
                cfg->theme);

    fclose(f);
    return 0;
}

static char *trim(char *s)
{
    while (*s == ' ' || *s == '\t')
        ++s;

    char *end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' ||
                       end[-1] == '\r' || end[-1] == '\n'))
        --end;
    *end = '\0';
    return s;
}

static int parse_param(char *line, LedCfgParam *param, int *val)
{
    char *eq = strchr(line, '=');
    if (!eq) {
        fprintf(stderr, "error: missing '=' in '%s'\n", line);
        return 1;
    }

    *eq = '\0';
    char *key = trim(line);
    char *value = trim(eq + 1);

    if (*key == '\0' || *value == '\0') {
        fprintf(stderr, "error: malformed parameter=value line\n");
        return 1;
    }

    bool bool_key;
    if (!strcmp(key, "fullscreen")) {
        *param = CFG_PARAM_FULLSCREEN;
        bool_key = true;
    } else if (!strcmp(key, "fontsize")) {
        *param = CFG_PARAM_FONTSIZE;
        bool_key = false;
    } else if (!strcmp(key, "number")) {
        *param = CFG_PARAM_NUMBER;
        bool_key = true;
    } else if (!strcmp(key, "theme")) {
        *param = CFG_PARAM_THEME;
        bool_key = false;
    } else {
        fprintf(stderr, "error: unknown parameter '%s'\n", key);
        return 1;
    }

    if (bool_key) {
        if (!strcmp(value, "true")) *val = 1;
        else if (!strcmp(value, "false")) *val = 0;
        else {
            fprintf(stderr, "error: parameter '%s' expects true/false, got '%s'\n",
                    key, value);
            return 1;
        }
    } else if (!is_numeric(value, val)) {
        fprintf(stderr, "error: parameter '%s' expects an integer, got '%s'\n",
                key, value);
        return 1;
    }

    return 0;
}

static bool is_numeric(const char *s, int *result)
{
    if (!s || !result || *s == '\0')
        return false;

    int i = 0;
    int sign = 1;

    // Optional sign
    if (s[0] == '-') {
        sign = -1;
        ++i;
    } else if (s[0] == '+')
        ++i;

    // String must have digits
    if (s[i] == '\0')
        return false;

    int res = 0;
    while (s[i] != '\0') {
        if (s[i] < '0' || s[i] > '9')
            return false;

        int digit = s[i] - '0';
        if (sign == 1) {
            if (res > (INT_MAX - digit) / 10)
                return false; // Overflow
            res = res * 10 + digit;
        } else {
            if (res < (INT_MIN + digit) / 10)
                return false; // Underflow
            res = res * 10 - digit;
        }
        ++i;
    }

    *result = res;
    return true;
}

static void read_param_to_cfg(LedCfgParam param, LedCfg *cfg, int val)
{
    switch (param) {
        case CFG_PARAM_FULLSCREEN: cfg->fullscreen = val; break;
        case CFG_PARAM_FONTSIZE: cfg->fontsize = val; break;
        case CFG_PARAM_NUMBER: cfg->number = val; break;
        case CFG_PARAM_THEME: cfg->theme = val; break;
        default: break;
    }
}
