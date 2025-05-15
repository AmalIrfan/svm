#ifndef _SVM_H_
#define _SVM_H_

typedef int svm_unit;

#define MEMORY_SIZE 0x10000
#define DSTACK_OFFSET  0x0000
#define RSTACK_OFFSET  0x3000
#define GENERAL_OFFSET 0x5000
#define DSTACK_SIZE  (RSTACK_OFFSET  - DSTACK_OFFSET)
#define RSTACK_SIZE  (GENERAL_OFFSET - RSTACK_OFFSET)
#define GENERAL_SIZE (MEMORY_SIZE    - GENERAL_OFFSET)

typedef struct svm_state {
    svm_unit memory[MEMORY_SIZE];
    svm_unit here;
    svm_unit dp;
    svm_unit rp;
} svm_state;

void svm_init(svm_state* svm);
void svm_load(svm_state* svm, const svm_unit* code, const svm_unit size);
void svm_execute(svm_state* svm);

svm_unit svm_unit_here(svm_state* svm);
svm_unit svm_unit_there(svm_state* svm, svm_unit there);
void svm_store_there(svm_state* svm, svm_unit there, svm_unit value);
void     svm_advance(svm_state* svm);
void     svm_dstack_push(svm_state* svm, svm_unit value);
svm_unit svm_dstack_pop(svm_state* svm);
svm_unit svm_dstack_view(svm_state* svm);
void     svm_dstack_set_top(svm_state* svm, svm_unit value);
void     svm_rstack_push(svm_state* svm, svm_unit value);
svm_unit svm_rstack_pop(svm_state* svm);
void     svm_jump(svm_state* svm, svm_unit there);

typedef enum svm_code {
    SVM_NOP,
    SVM_LIT,
    SVM_CALL,
    SVM_EXIT,
    SVM_READ,
    SVM_WRITE,
    SVM_DUP,
    SVM_DROP,
    SVM_SWAP,
    SVM_OVER,
    SVM_SUB,
    SVM_JNZ,
    SVM_STORE,
    SVM_LOAD
} svm_code;

const char* svm_code_str[] = {
    "NOP",
    "LIT",
    "CALL",
    "EXIT",
    "READ",
    "WRITE",
    "DUP",
    "DROP",
    "SWAP",
    "OVER",
    "SUB",
    "JNZ",
    "STORE",
    "LOAD"
};

#ifdef SVM_IMPLEMENTATION
#include <stdio.h>
#include <string.h>

void svm_init(svm_state* svm) {
    printf("MEMORY_SIZE    %5X\n", MEMORY_SIZE);
    printf("DSTACK_OFFSET  %5X DSTACK_SIZE  %5X\n", DSTACK_OFFSET, DSTACK_SIZE);
    printf("RSTACK_OFFSET  %5X RSTACK_SIZE  %5X\n", RSTACK_OFFSET, RSTACK_SIZE);
    printf("GENERAL_OFFSET %5X GENERAL_SIZE %5X\n", GENERAL_OFFSET, GENERAL_SIZE);
    memset(svm, 0, sizeof(*svm));
}

void svm_load(svm_state* svm, const svm_unit* code, const svm_unit size) {
    if (size > GENERAL_SIZE) {
        printf("CODE OVERFLOW: %d UNITS\n", size - GENERAL_SIZE);
        return;
    }
    memcpy(&svm->memory[GENERAL_OFFSET], code, size * sizeof(*code));
}

void svm_execute(svm_state* svm) {
    svm_code code = SVM_NOP;
    while (1) {
        code = (svm_code)svm_unit_here(svm);
#if 1
        {
            int i = 0;
            fprintf(stderr, "%3d %5s", svm->here, ((int)code > 0 ? svm_code_str[code] : ""));
            if (code == SVM_LIT) {
                fprintf(stderr, " %2d ", svm_unit_there(svm, svm->here + 1));
            } else {
                fputs("    ", stderr);
            }
            putc('[', stderr);
            for (i = 0; i < svm->dp; i++) {
                fprintf(stderr, "%d%s", svm->memory[DSTACK_OFFSET + i],
                        (i + 1 >= svm->dp ? "" : ", "));
            }
            fputs("] r[", stderr);
            for (i = 0; i < svm->rp; i++) {
                fprintf(stderr, "%d%s", svm->memory[RSTACK_OFFSET + i],
                        (i + 1 >= svm->rp ? "" : ", "));
            }
            fputs("]\n", stderr);
        }
#endif
        switch (code) {
        case SVM_NOP:
             svm_advance(svm);
             break;
        case SVM_LIT:
            svm_advance(svm);
            svm_dstack_push(svm, svm_unit_here(svm));
            svm_advance(svm);
            break;
        case SVM_CALL:
            svm_advance(svm);
            svm_rstack_push(svm, svm->here);
            svm_jump(svm, svm_dstack_pop(svm));
            break;
        case SVM_EXIT:
            svm_advance(svm);
            svm_jump(svm, svm_rstack_pop(svm));
            break;
        case SVM_READ:
            svm_advance(svm);
            svm_dstack_push(svm, getc(stdin));
            break;
        case SVM_WRITE:
            svm_advance(svm);
            putc(svm_dstack_pop(svm), stdout);
            break;
        case SVM_DUP:
            svm_advance(svm);
            svm_dstack_push(svm, svm_dstack_view(svm));
            break;
        case SVM_DROP:
            svm_advance(svm);
            svm_dstack_pop(svm);
            break;
        case SVM_SWAP:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_unit over = svm_dstack_view(svm);
                svm_dstack_set_top(svm, top);
                svm_dstack_push(svm, over);
            }
            break;
        case SVM_OVER:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_unit over = svm_dstack_view(svm);
                svm_dstack_push(svm, top);
                svm_dstack_push(svm, over);
            }
            break;
        case SVM_SUB:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) - top);
            }
            break;
        case SVM_JNZ:
            svm_advance(svm);
            {
                svm_unit there = svm_dstack_pop(svm);
                if (svm_dstack_view(svm)) {
                        svm_jump(svm, there);
                }
                svm_dstack_pop(svm);
            }
            break;
        case SVM_STORE:
            svm_advance(svm);
            {
                svm_unit there = svm_dstack_pop(svm);
                svm_store_there(svm, there, svm_dstack_pop(svm));
            }
            break;
        case SVM_LOAD:
            svm_advance(svm);
            {
                svm_unit there = svm_dstack_pop(svm);
                svm_dstack_push(svm, svm_unit_there(svm, there));
            }
            break;
        default:
            printf("Unrecognised op %d\n", code);
            return;
        }
    }
}

svm_unit svm_unit_here(svm_state* svm) {
    return svm->memory[GENERAL_OFFSET + svm->here];
}

svm_unit svm_unit_there(svm_state* svm, svm_unit there) {
    return svm->memory[GENERAL_OFFSET + there];
}

void svm_store_there(svm_state* svm, svm_unit there, svm_unit value) {
    if (there < 0)
        there = GENERAL_SIZE - there;
    else
        there = there % GENERAL_SIZE;
    svm->memory[GENERAL_OFFSET + there] = value;
}

void svm_advance(svm_state* svm) {
    svm->here = (svm->here + 1) % GENERAL_SIZE;
}

void svm_dstack_push(svm_state* svm, svm_unit value) {
    svm->memory[DSTACK_OFFSET + svm->dp] = value;
    svm->dp = (svm->dp + 1) % DSTACK_SIZE;
}

svm_unit svm_dstack_pop(svm_state* svm) {
    svm->dp = svm->dp - 1;
    if (svm->dp < 0)
        svm->dp = DSTACK_SIZE + svm->dp;
    return svm->memory[DSTACK_OFFSET + svm->dp];
}

void svm_dstack_set_top(svm_state* svm, svm_unit value) {
    svm_unit top = svm->dp - 1;
    if (svm->dp == 0)
        top = DSTACK_SIZE - 1;
    svm->memory[DSTACK_OFFSET + top] = value;
}

svm_unit svm_dstack_view(svm_state* svm) {
    svm_unit top = svm->dp - 1;
    if (svm->dp == 0)
        top = DSTACK_SIZE - 1;
    return svm->memory[DSTACK_OFFSET + top];
}

void svm_rstack_push(svm_state* svm, svm_unit value) {
    svm->memory[RSTACK_OFFSET + svm->rp] = value;
    svm->rp = (svm->rp + 1) % RSTACK_SIZE;
}

svm_unit svm_rstack_pop(svm_state* svm) {
    svm->rp = svm->rp - 1;
    if (svm->rp < 0)
        svm->rp = RSTACK_SIZE + svm->rp;
    return svm->memory[RSTACK_OFFSET + svm->rp];
}

void svm_jump(svm_state* svm, svm_unit there) {
     svm->here = there;
}

#endif /* SVM_IMPLEMENTATION */
#endif /* _SVM_H_ */
