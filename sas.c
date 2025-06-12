#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

#define SAS_MAX_SYMBOLS 100
#define SAS_TOKEN 20

typedef struct sas_symbol {
    char name[SAS_TOKEN];
    svm_dword dwordess;
    int dword; /* >0xFF */
} sas_symbol;

typedef struct sas_symbol_array {
    sas_symbol symbols[SAS_MAX_SYMBOLS];
    int index;
} sas_symbol_array;

typedef struct sas_state {
    svm_word code[1000];
    sas_symbol_array symbol_defs;
    sas_symbol_array symbol_uses;
    svm_dword here;
    int dword;
    FILE* fh;
} sas_state;

const char* sas_get_token(FILE* fh);
char sas_comment(FILE* fh);
int sas_token_is_number(const char* token);
int sas_make_number(const char* token);
int sas_token_is_symbol(sas_state* sas, const char* token, int symbolindex);
int sas_token_is_symbol_definition(const char* token);
sas_symbol sas_make_symbol(const char* name, svm_dword here, int suffix, int dword);
int sas_make_symbol_def(sas_state* sas, const char* token);
int sas_use_symbol(sas_state* sas, const char* name);
int sas_resolve_symbol_uses(sas_state* sas);
int sas_try_assemble(sas_state* sas, const char* token);
int sas_assemble(sas_state* sas);
int sas_disassemble(sas_state* sas);
int sas_stricmp(const char* a, const char* b);

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
            if (n <= 255 && !sas->dword) {
                sas->code[sas->here] = n;
                sas->here++;
            } else if (n <= 0xFFFF && sas->dword) {
                *(svm_dword*)(sas->code + sas->here) = n;
                sas->here = sas->here + (sizeof(svm_dword) / sizeof(svm_word));
                sas->dword = 0;
            } else {
                fprintf(stdout, "Error: Out of bounds: %d\n", n);
                return 1;
            }
        }
        else if (sas_token_is_symbol_definition(token)) {
            if (sas_make_symbol_def(sas, token))
                return 1;
        }
        else if (sas_try_assemble(sas, token) == 0) {
            /* successfully assembled */
        }
        else { /* assume it is a symbol */
            if (sas_use_symbol(sas, token))
                return 1;
        }
    }

    if (sas_resolve_symbol_uses(sas))
        return 1;

    if (sas->here % SVM_VWORD)
        sas->here += SVM_VWORD - (sas->here % SVM_VWORD);

    fwrite(sas->code, sizeof(sas->code[0]), sas->here, stdout);
    return 0;
}

int sas_disassemble(sas_state* sas) {
    svm_code ins = 0;
    int val = 0;
    fread(&ins, sizeof(svm_word), 1, sas->fh);
    while (!feof(sas->fh)) {
        if (ins >= SVM_MIN_OP && ins <= SVM_MAX_OP)
            fprintf(stdout, "    %s", svm_code_str[ins]);
        else
            fprintf(stdout, "    error: %hhd\n", ins);
        if (ins == SVM_LIT || ins == SVM_BNZ || ins == SVM_BNG) {
            fread(&val, sizeof(svm_word), 1, sas->fh);
            fprintf(stdout, " %hhd", val);
        }
        if (ins == SVM_CAL || ins == SVM_LAD) {
            fread(&val, sizeof(svm_dword), 1, sas->fh);
            fprintf(stdout, " %hd", val);
        }
        putc('\n', stdout);
        fread(&ins, sizeof(svm_word), 1, sas->fh);
    }
    return 0;
}

int sas_stricmp(const char* a, const char* b) {
    /* TODO: NULL check */
    while (*a && *b && toupper(*a) == toupper(*b)) {
        a++;
        b++;
    }
    return (int)*a - (int)*b;
}

const char* sas_get_token(FILE* fh) {
    static char buf[SAS_TOKEN];
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
    else if (*token == '+')
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
    } else if (*token == '+') {
        neg = 0;
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

int sas_symbol_exists(sas_state* sas, const char* name) {
    int i = 0;
    for (i = 0; i < sas->symbol_defs.index; i++) {
        if (sas_stricmp(name, sas->symbol_defs.symbols[i].name) == 0)
            return 1;
    }
    return 0;
}

int sas_token_is_symbol_definition(const char* token) {
    int n = strlen(token);
    if (n > 1 && token[n - 1] == ':')
        return 1;
    if (n > 1 && token[n - 1] == '=')
        return 1;
    return 0;
}

int sas_make_symbol_def(sas_state* sas, const char* token) {
    sas_symbol symb = {0};
    int n = strlen(token);
    if (sas->symbol_defs.index >= SAS_MAX_SYMBOLS) {
        fprintf(stderr, "Error: too many symbol defs\n");
        return 1;
    }
    if (token[n - 1] == ':') {
        symb = sas_make_symbol(token, sas->here, 1, 0);
        if (sas_symbol_exists(sas, symb.name)) {
            fprintf(stderr, "Error: symbol redefintion `%s`\n", symb.name);
            return 1;
        }
    }
    else if (token[n - 1] == '=') {
        symb = sas_make_symbol(token, 0, 1, 0);
        if (sas_symbol_exists(sas, symb.name)) {
            fprintf(stderr, "Error: symbol redefintion `%s`\n", symb.name);
            return 1;
        }
        token = sas_get_token(sas->fh);
        if (!sas_token_is_number(token)) {
            fprintf(stderr, "Error: symbol must be followed by a number: `%s %s`\n", symb.name, token);
            return 1;
        }
        n = sas_make_number(token);
        if ((unsigned int)n <= 0xFF) {
            symb.dwordess = n;
            symb.dword = 0;
        } else if ((unsigned int)n <= 0xFFFF) {
            symb.dwordess = n;
            symb.dword = 1;
        } else {
            fprintf(stderr, "Error: number out of range: `%x`\n", n);
            return 1;
        }
    }
    sas->symbol_defs.symbols[sas->symbol_defs.index] = symb;
    sas->symbol_defs.index++;
    return 0;
}

sas_symbol sas_make_symbol(const char* name, svm_dword here, int suffix, int dword) {
    sas_symbol l = {0};
    int n = strlen(name) - suffix;
    memcpy(l.name, name, n);
    l.name[n] = 0;
    l.dwordess = here;
    l.dword = dword;
    return l;
}

int sas_use_symbol(sas_state* sas, const char* name) {
    if (sas->symbol_uses.index >= SAS_MAX_SYMBOLS) {
        fprintf(stderr, "Error: too many symbol uses\n");
        return 1;
    }
    sas->symbol_uses.symbols[sas->symbol_uses.index] = sas_make_symbol(name, sas->here, 0, sas->dword);
    sas->symbol_uses.index++;
    if (sas->dword) {
        *(svm_dword*)(sas->code + sas->here) = 0;
        sas->here = sas->here + sizeof(svm_dword) / sizeof(svm_word);
    }
    else {
        sas->code[sas->here] = 0;
        sas->here++;
    }
    sas->dword = 0;
    return 0;
}

int sas_resolve_symbol_uses(sas_state* sas) {
    int i = 0;
    int j = 0;
    const char *name = NULL;
    svm_dword there = 0;
    int dword = 0;
    int matches = 0;
    for (j = 0; j < sas->symbol_uses.index; j++) {
        matches = 0;
        name = sas->symbol_uses.symbols[j].name;
        there = sas->symbol_uses.symbols[j].dwordess;
        dword = sas->symbol_uses.symbols[j].dword;
        /* set uses to defs as the match */
        for (i = 0; i < sas->symbol_defs.index; i++) {
            if (strcmp(name, sas->symbol_defs.symbols[i].name) == 0) {
                if (dword) {
                    *(svm_dword*)(sas->code + there) = sas->symbol_defs.symbols[i].dwordess;
                }
                else
                    sas->code[there] = sas->symbol_defs.symbols[i].dwordess - there - sizeof(svm_word);
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
    static char dir[SAS_TOKEN];
    int i = 0;
    svm_code code = SVM_NOP;
    int n = strlen(token);
    memcpy(dir, token, n + 1);
    for (i = 0; i < n; i++) {
        dir[i] = toupper(dir[i]);
    }
    for (code = SVM_MIN_OP; code <= SVM_MAX_OP; code++) {
        if (strcmp(svm_code_str[code], dir) == 0) {
            if (code == SVM_CAL || code == SVM_LAD) { /* 3word */
                sas->dword = 1;
                if (sas->here % SVM_VWORD != 0 && sas->here % SVM_VWORD >= 3) /* 2 or less left */
                    sas->here += SVM_VWORD - (sas->here % SVM_VWORD);
            }
            if (code == SVM_BNZ || code == SVM_BNG) { /* 2word */
                if (sas->here % SVM_VWORD != 0 && sas->here % SVM_VWORD == 3) /* 1 left */
                    sas->here += SVM_VWORD - (sas->here % SVM_VWORD);
            }
            sas->code[sas->here] = code;
            sas->here++;
            return 0;
        }
    }
    return 1;
}
