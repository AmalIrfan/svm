#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

#define SAS_MAX_LABELS 20

typedef struct sas_label {
    char name[10];
    svm_unit address;
} sas_label;

typedef struct sas_label_array {
    sas_label labels[SAS_MAX_LABELS];
    int index;
} sas_label_array;

typedef struct sas_state {
    svm_unit code[200];
    sas_label_array label_defs;
    sas_label_array label_uses;
    svm_unit here;
} sas_state;

const char* sas_get_token(FILE* fh);
char sas_comment(FILE* fh);
int sas_token_is_number(const char* token);
svm_unit sas_make_number(const char* token);
int sas_token_is_label(sas_state* sas, const char* token, int labelindex);
int sas_token_is_label_definition(const char* token);
sas_label sas_make_label(const char* name, svm_unit here, int suffix);
int sas_make_label_def(sas_state* sas, const char* token);
int sas_use_label(sas_state* sas, const char* name);
int sas_resolve_label_uses(sas_state* sas);
int sas_try_assemble(sas_state* sas, const char* token);

int main(int argc, const char* argv[]) {
    const char* token;
    sas_state sas = {0};
    FILE* fh = 0;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input > output\n", argv[0]);
        return 1;
    }
    fh = fopen(argv[1], "r");
    if (!fh) {
        perror(argv[1]);
        return 1;
    }
    while (1) {
        token = sas_get_token(fh);
        if (!token)
            break;
        else if (sas_token_is_number(token)) {
            sas.code[sas.here] = sas_make_number(token);
            sas.here++;
        }
        else if (sas_token_is_label_definition(token)) {
            if (sas_make_label_def(&sas, token))
                return 1;
        }
        else if (sas_try_assemble(&sas, token) == 0) {
            /* successfully assembled */
        }
        else { /* assume it is a label */
            if (sas_use_label(&sas, token))
                return 1;
        }
    }

    if (sas_resolve_label_uses(&sas))
        return 1;

    fwrite(sas.code, sizeof(sas.code[0]), sas.here, stdout);
    return 0;
}

const char* sas_get_token(FILE* fh) {
    static char buf[10];
    int i = 0;
    char ch = getc(fh);
    while (isspace(ch)) {
        ch = getc(fh);
    }
    while (ch == ';') {
        ch = sas_comment(fh);
    }
    if (ch < 0)
        return NULL;
    while (!isspace(ch) && ch > 0) {
        buf[i++] = ch;
        if (i + 1 >= (int)sizeof(buf)) {
            fprintf(stderr, "token overflow: %.*s\n", i, buf);
            break;
        }
        ch = getc(fh);
    }
    buf[i++] = 0;
    return buf;
}

char sas_comment(FILE* fh) {
    char ch = getc(fh);
    while (ch != '\n')
        ch = getc(fh);
    while (isspace(ch))
        ch = getc(fh);
    return ch;
}

int sas_token_is_number(const char* token) {
    if (*token == '-')
        token++;
    if (!*token)
        return 0;
    while (*token) {
        if (!isdigit(*token))
            return 0;
        token++;
    }
    return 1;
}

svm_unit sas_make_number(const char* token) {
    svm_unit n = 0;
    int neg = 0;
    if (*token == '-') {
        neg = 1;
        token++;
    }
    while (*token) {
        n = n * 10 + (*token) - '0';
        token++;
    }
    if (neg)
        n = -n;
    return n;
}

int sas_label_exists(sas_state* sas, const char* name) {
    int i = 0;
    for (i = 0; i < sas->label_defs.index; i++) {
        if (strcasecmp(name, sas->label_defs.labels[i].name) == 0)
            return 1;
    }
    return 0;
}

int sas_token_is_label_definition(const char* token) {
    int n = strlen(token);
    if (n > 1 && token[n - 1] == ':')
        return 1;
    return 0;
}

int sas_make_label_def(sas_state* sas, const char* token) {
    sas_label l = sas_make_label(token, sas->here, 1);
    if (sas->label_defs.index >= SAS_MAX_LABELS) {
        fprintf(stderr, "Error: too many label defs\n");
        return 1;
    }
    if (sas_label_exists(sas, l.name)) {
        fprintf(stderr, "Error: label redefintion `%s`\n", l.name);
        return 1;
    }
    sas->label_defs.labels[sas->label_defs.index] = l;
    sas->label_defs.index++;
    return 0;
}

sas_label sas_make_label(const char* name, svm_unit here, int suffix) {
    sas_label l = {0};
    int n = strlen(name) - suffix;
    memcpy(l.name, name, n);
    l.name[n] = 0;
    l.address = here;
    return l;
}

int sas_use_label(sas_state* sas, const char* name) {
    if (sas->label_uses.index >= SAS_MAX_LABELS) {
        fprintf(stderr, "Error: too many label uses\n");
        return 1;
    }
    sas->label_uses.labels[sas->label_uses.index] = sas_make_label(name, sas->here, 0);
    sas->label_uses.index++;
    sas->code[sas->here] = 0;
    sas->here++;
    return 0;
}

int sas_resolve_label_uses(sas_state* sas) {
    int i = 0;
    int j = 0;
    const char *name = NULL;
    svm_unit there = 0;
    int matches = 0;
    for (j = 0; j < sas->label_uses.index; j++) {
        matches = 0;
        name = sas->label_uses.labels[j].name;
        there = sas->label_uses.labels[j].address;
        /* set uses to defs as the match */
        for (i = 0; i < sas->label_defs.index; i++) {
            if (strcmp(name, sas->label_defs.labels[i].name) == 0) {
                sas->code[there] = sas->label_defs.labels[i].address;
                matches++;
            }
        }
        if (!matches) {
            fprintf(stderr, "Unrecognised: %s\n", name);
            return 1;
        }
    }
    return 0;
}

int sas_try_assemble(sas_state* sas, const char* token) {
    static char dir[SAS_MAX_LABELS];
    int i = 0;
    svm_code code = SVM_NOP;
    strcpy(dir, token);
    for (i = 0; i < (int)strlen(dir); i++) {
        dir[i] = toupper(dir[i]);
    }
    for (code = SVM_NOP; code < _SVM_MAX; code++) {
        if (strcmp(svm_code_str[code], dir) == 0) {
            sas->code[sas->here] = code;
            sas->here++;
            return 0;
        }
    }
    return 1;
}
