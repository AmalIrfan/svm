#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

#define SAS_MAX_LABELS 20

typedef struct sas_label {
    char name[20];
    svm_addr address;
    int addr;
} sas_label;

typedef struct sas_label_array {
    sas_label labels[SAS_MAX_LABELS];
    int index;
} sas_label_array;

typedef struct sas_state {
    svm_unit code[200];
    sas_label_array label_defs;
    sas_label_array label_uses;
    svm_addr here;
    int addr;
    FILE* fh;
} sas_state;

const char* sas_get_token(FILE* fh);
char sas_comment(FILE* fh);
int sas_token_is_number(const char* token);
int sas_make_number(const char* token);
int sas_token_is_label(sas_state* sas, const char* token, int labelindex);
int sas_token_is_label_definition(const char* token);
sas_label sas_make_label(const char* name, svm_addr here, int suffix, int addr);
int sas_make_label_def(sas_state* sas, const char* token);
int sas_use_label(sas_state* sas, const char* name);
int sas_resolve_label_uses(sas_state* sas);
int sas_try_assemble(sas_state* sas, const char* token);
int sas_assemble(sas_state* sas);
int sas_disassemble(sas_state* sas);

int main(int argc, const char* argv[]) {
    sas_state sas = {0};
    int fl = 1;
    int dis = 0;
    if (argc == 3) {
        fl++;
        if (strcmp(argv[1],"d") == 0)
            dis = 1;
    }
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s [d] input > output\n", argv[0]);
        return 1;
    }
    sas.fh = fopen(argv[fl], "r");
    if (!sas.fh) {
        perror(argv[fl]);
        return 1;
    }
    if (dis == 0)
        sas_assemble(&sas);
    else
        sas_disassemble(&sas);
    fclose(sas.fh);
    return 0;
}

int sas_assemble(sas_state* sas) {
    const char* token;
    while (1) {
        token = sas_get_token(sas->fh);
        if (!token)
            break;
        else if (sas_token_is_number(token)) {
            int n = sas_make_number(token);
            if (n <= 255 && !sas->addr) {
                sas->code[sas->here] = n;
                sas->here++;
            } else if (n <= 0xFFFF && sas->addr) {
                *(svm_addr*)(sas->code + sas->here) = n;
                sas->here = sas->here + (sizeof(svm_addr) / sizeof(svm_unit));
                sas->addr = 0;
            } else {
                fprintf(stdout, "Error: Out of bounds: %d\n", n);
                return 1;
            }
        }
        else if (sas_token_is_label_definition(token)) {
            if (sas_make_label_def(sas, token))
                return 1;
        }
        else if (sas_try_assemble(sas, token) == 0) {
            /* successfully assembled */
        }
        else { /* assume it is a label */
            if (sas_use_label(sas, token))
                return 1;
        }
    }

    if (sas_resolve_label_uses(sas))
        return 1;

    fwrite(sas->code, sizeof(sas->code[0]), sas->here, stdout);
    return 0;
}

int sas_disassemble(sas_state* sas) {
    svm_code ins = 0;
    int val = 0;
    fread(&ins, sizeof(svm_unit), 1, sas->fh);
    while (!feof(sas->fh)) {
        if (ins >= SVM_NOP && ins < _SVM_MAX)
            fprintf(stdout, "    %s", svm_code_str[ins]);
        else
            fprintf(stdout, "    error: %hhd\n", ins);
        if (ins == SVM_LIT || ins == SVM_JNZ) {
            fread(&val, sizeof(svm_unit), 1, sas->fh);
            fprintf(stdout, " %hhd", val);
        }
        if (ins == SVM_CALL || ins == SVM_LITA) {
            fread(&val, sizeof(svm_addr), 1, sas->fh);
            fprintf(stdout, " %hd", val);
        }
        putc('\n', stdout);
        fread(&ins, sizeof(svm_unit), 1, sas->fh);
    }
    return 0;
}

const char* sas_get_token(FILE* fh) {
    static char buf[20];
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

int sas_make_number(const char* token) {
    int n = 0;
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
    sas_label l = sas_make_label(token, sas->here, 1, 0);
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

sas_label sas_make_label(const char* name, svm_addr here, int suffix, int addr) {
    sas_label l = {0};
    int n = strlen(name) - suffix;
    memcpy(l.name, name, n);
    l.name[n] = 0;
    l.address = here;
    l.addr = addr;
    return l;
}

int sas_use_label(sas_state* sas, const char* name) {
    if (sas->label_uses.index >= SAS_MAX_LABELS) {
        fprintf(stderr, "Error: too many label uses\n");
        return 1;
    }
    sas->label_uses.labels[sas->label_uses.index] = sas_make_label(name, sas->here, 0, sas->addr);
    sas->label_uses.index++;
    if (sas->addr) {
        *(svm_addr*)(sas->code + sas->here) = 0;
        sas->here = sas->here + sizeof(svm_addr) / sizeof(svm_unit);
    }
    else {
        sas->code[sas->here] = 0;
        sas->here++;
    }
    sas->addr = 0;
    return 0;
}

int sas_resolve_label_uses(sas_state* sas) {
    int i = 0;
    int j = 0;
    const char *name = NULL;
    svm_addr there = 0;
    int addr = 0;
    int matches = 0;
    for (j = 0; j < sas->label_uses.index; j++) {
        matches = 0;
        name = sas->label_uses.labels[j].name;
        there = sas->label_uses.labels[j].address;
        addr = sas->label_uses.labels[j].addr;
        /* set uses to defs as the match */
        for (i = 0; i < sas->label_defs.index; i++) {
            if (strcmp(name, sas->label_defs.labels[i].name) == 0) {
                if (addr) {
                    *(svm_addr*)(sas->code + there) = sas->label_defs.labels[i].address;
                }
                else
                    sas->code[there] = sas->label_defs.labels[i].address - there - sizeof(svm_unit);
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
            if (code == SVM_CALL || code == SVM_LITA)
                sas->addr = 1;
            sas->code[sas->here] = code;
            sas->here++;
            return 0;
        }
    }
    return 1;
}
