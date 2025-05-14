#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

typedef struct sas_label {
    char name[10];
    svm_unit address;
} sas_label;

typedef struct sas_state {
    svm_unit code[100];
    sas_label labels[10];
    svm_unit here;
    int label_index;
} sas_state;

const char* sas_get_token(FILE* fh);
char sas_comment(FILE* fh);
int sas_token_is_number(const char* token);
svm_unit sas_make_number(const char* token);
int sas_token_is_label(sas_state* sas, const char* token, int labelindex);
svm_unit sas_get_label_address(sas_state* sas, const char* name, int labelindex);
int sas_token_is_label_definition(const char* token);
sas_label sas_make_label(const char* name, svm_unit here);

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
        else if (sas_token_is_label_definition(token)) {
            sas.labels[sas.label_index] = sas_make_label(token, sas.here);
            sas.label_index++;
        }
        else {
            sas.here++;
        }
    }
    fseek(fh, 0, SEEK_SET);
    sas.here = 0;
    while (1) {
        token = sas_get_token(fh);
        if (!token)
            break;
        else if (sas_token_is_number(token)) {
            sas.code[sas.here] = sas_make_number(token);
            sas.here++;
        }
        else if (sas_token_is_label_definition(token))
            (void)0;
        else if (sas_token_is_label(&sas, token, sas.label_index)) {
            sas.code[sas.here] = sas_get_label_address(&sas, token, sas.label_index);
            sas.here++;
        }
        else if (strcasecmp(token, "NOP") == 0) {
            sas.code[sas.here] = SVM_NOP;
            sas.here++;
        }
        else if (strcasecmp(token, "LIT") == 0) {
            sas.code[sas.here] = SVM_LIT;
            sas.here++;
        }
        else if (strcasecmp(token, "CALL") == 0) {
            sas.code[sas.here] = SVM_CALL;
            sas.here++;
        }
        else if (strcasecmp(token, "EXIT") == 0) {
            sas.code[sas.here] = SVM_EXIT;
            sas.here++;
        }
        else if (strcasecmp(token, "READ") == 0) {
            sas.code[sas.here] = SVM_READ;
            sas.here++;
        }
        else if (strcasecmp(token, "WRITE") == 0) {
            sas.code[sas.here] = SVM_WRITE;
            sas.here++;
        }
        else if (strcasecmp(token, "DUP") == 0) {
            sas.code[sas.here] = SVM_DUP;
            sas.here++;
        }
        else if (strcasecmp(token, "DROP") == 0) {
            sas.code[sas.here] = SVM_DROP;
            sas.here++;
        }
        else if (strcasecmp(token, "SUB") == 0) {
            sas.code[sas.here] = SVM_SUB;
            sas.here++;
        }
        else if (strcasecmp(token, "JNZ") == 0) {
            sas.code[sas.here] = SVM_JNZ;
            sas.here++;
        }
        else
            printf("Unrecognised: %s\n", token);
    }
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
            printf("token overflow: %.*s\n", i, buf);
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

int sas_token_is_label(sas_state* sas, const char* token, int labelindex) {
    int i = 0;
    for (i = 0; i < labelindex; i++) {
        if (strcasecmp(token, sas->labels[i].name) == 0)
            return 1;
    }
    return 0;
}

svm_unit sas_get_label_address(sas_state* sas, const char* name, int labelindex) {
    int i = 0;
    for (i = 0; i < labelindex; i++) {
        if (strcasecmp(name, sas->labels[i].name) == 0)
            return sas->labels[i].address;
    }
    fprintf(stderr, "Label not found: %s\n", name);
    return -1;
}

int sas_token_is_label_definition(const char* token) {
    int n = strlen(token);
    if (n > 1 && token[n - 1] == ':')
        return 1;
    return 0;
}

sas_label sas_make_label(const char* name, svm_unit here) {
    sas_label l = {0};
    int n = strlen(name) - 1;
    memcpy(l.name, name, n);
    l.name[n] = 0;
    l.address = here;
    return l;
}
