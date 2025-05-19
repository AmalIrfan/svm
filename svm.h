#ifndef _SVM_H_
#define _SVM_H_
#include <stdint.h>

typedef int8_t svm_unit;
typedef uint16_t svm_addr;

#define MEMORY_SIZE 0x10000
#define DSTACK_OFFSET  0x0000
#define RSTACK_OFFSET  0x0100
#define GENERAL_OFFSET 0x0200
#define DSTACK_SIZE  (RSTACK_OFFSET  - DSTACK_OFFSET)
#define RSTACK_SIZE  (GENERAL_OFFSET - RSTACK_OFFSET)
#define GENERAL_SIZE (MEMORY_SIZE    - GENERAL_OFFSET)

typedef struct svm_state {
    svm_unit memory[MEMORY_SIZE];
    svm_addr pc;
    svm_addr dp;
    svm_addr rp;
} svm_state;

void svm_init(svm_state* svm);
void svm_load(svm_state* svm, const svm_unit* code, const svm_addr size);
void svm_execute(svm_state* svm);

svm_unit svm_unit_here(svm_state* svm);
svm_addr svm_addr_here(svm_state* svm);
svm_unit svm_unit_there(svm_state* svm, svm_addr there);
svm_addr svm_addr_there(svm_state* svm, svm_addr there);
void svm_store_there(svm_state* svm, svm_addr there, svm_unit value);
void     svm_advance(svm_state* svm);
void     svm_advance_addr(svm_state* svm);
void     svm_dstack_push(svm_state* svm, svm_unit value);
svm_unit svm_dstack_pop(svm_state* svm);
svm_unit svm_dstack_view(svm_state* svm);
void     svm_dstack_set_top(svm_state* svm, svm_unit value);
void     svm_rstack_push(svm_state* svm, svm_addr value);
svm_addr svm_rstack_pop(svm_state* svm);
void     svm_jump(svm_state* svm, svm_addr there);

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
    SVM_ROT,
    SVM_RPUSH,
    SVM_RPOP,
    SVM_SUB,
    SVM_ADD,
    SVM_MUL,
    SVM_DIV,
    SVM_LT,
    SVM_EQ,
    SVM_GT,
    SVM_LTE,
    SVM_NEQ,
    SVM_GTE,
    SVM_NOT,
    SVM_AND,
    SVM_OR,
    SVM_XOR,
    SVM_JNZ,
    SVM_STORE,
    SVM_LOAD,
    _SVM_MAX
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
    "ROT",
    "RPUSH",
    "RPOP",
    "SUB",
    "ADD",
    "MUL",
    "DIV",
    "LT",
    "EQ",
    "GT",
    "LTE",
    "NEQ",
    "GTE",
    "NOT",
    "AND",
    "OR",
    "XOR",
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

void svm_load(svm_state* svm, const svm_unit* code, const svm_addr size) {
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
            fprintf(stderr, "%3d %5s", svm->pc, ((int)code > 0 ? svm_code_str[code] : ""));
            if (code == SVM_LIT || code == SVM_JNZ) {
                fprintf(stderr, " %3hhd  ", svm_unit_there(svm, svm->pc + 1));
            } else if (code == SVM_CALL) {
                fprintf(stderr, " %4hu ", svm_addr_there(svm, svm->pc + 1));
            } else {
                fputs("      ", stderr);
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
            {
                svm_addr there = svm_addr_here(svm);
                svm_advance_addr(svm);
                svm_rstack_push(svm, svm->pc);
                svm_jump(svm, there);
            }
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
        case SVM_ROT:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_unit over = svm_dstack_pop(svm);
                svm_unit back = svm_dstack_pop(svm);
                svm_dstack_push(svm, over);
                svm_dstack_push(svm, top);
                svm_dstack_push(svm, back);
            }
            break;
        case SVM_RPUSH:
            svm_advance(svm);
            svm_rstack_push(svm, svm_dstack_pop(svm));
            break;
        case SVM_RPOP:
            svm_advance(svm);
            svm_dstack_push(svm, svm_rstack_pop(svm));
            break;
        case SVM_SUB:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) - top);
            }
            break;
        case SVM_ADD:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) + top);
            }
            break;
        case SVM_MUL:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) * top);
            }
            break;
        case SVM_DIV:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_unit over = svm_dstack_view(svm);
                svm_dstack_set_top(svm, over / top);
                svm_dstack_push(svm, over % top);
            }
            break;
        case SVM_LT:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) < top);
            }
            break;
        case SVM_EQ:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) == top);
            }
            break;
        case SVM_GT:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) > top);
            }
            break;
        case SVM_LTE:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) <= top);
            }
            break;
        case SVM_NEQ:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) != top);
            }
            break;
        case SVM_GTE:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) >= top);
            }
            break;
        case SVM_NOT:
            svm_advance(svm);
            {
                svm_dstack_set_top(svm, ~svm_dstack_view(svm));
            }
            break;
        case SVM_AND:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) & top);
            }
            break;
        case SVM_OR:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) | top);
            }
            break;
        case SVM_XOR:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) ^ top);
            }
            break;
        case SVM_JNZ:
            svm_advance(svm);
            {
                svm_unit offset = svm_unit_here(svm);
                svm_advance(svm);
                if (svm_dstack_pop(svm)) {
                        svm_jump(svm, svm->pc + offset);
                }
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
    return svm->memory[GENERAL_OFFSET + svm->pc];
}

svm_addr svm_addr_here(svm_state* svm) {
    return *(svm_addr*)(svm->memory + GENERAL_OFFSET + svm->pc);
}

svm_unit svm_unit_there(svm_state* svm, svm_addr there) {
    return svm->memory[GENERAL_OFFSET + there];
}

svm_addr svm_addr_there(svm_state* svm, svm_addr there) {
    return *(svm_addr*)(svm->memory + GENERAL_OFFSET + there);
}

void svm_store_there(svm_state* svm, svm_addr there, svm_unit value) {
    there = there % GENERAL_SIZE;
    svm->memory[GENERAL_OFFSET + there] = value;
}

void svm_advance(svm_state* svm) {
    svm->pc = (svm->pc + 1) % GENERAL_SIZE;
}

void svm_advance_addr(svm_state* svm) {
    svm->pc = (svm->pc + sizeof(svm_addr) / sizeof(svm_unit)) % GENERAL_SIZE;
}

void svm_dstack_push(svm_state* svm, svm_unit value) {
    svm->memory[DSTACK_OFFSET + svm->dp] = value;
    svm->dp = (svm->dp + 1) % DSTACK_SIZE;
}

svm_unit svm_dstack_pop(svm_state* svm) {
    if (svm->dp > 0)
        svm->dp = svm->dp - 1;
    else
        svm->dp = DSTACK_SIZE - 1;
    return svm->memory[DSTACK_OFFSET + svm->dp];
}

void svm_dstack_set_top(svm_state* svm, svm_unit value) {
    svm_addr top = svm->dp - 1;
    if (svm->dp == 0)
        top = DSTACK_SIZE - 1;
    svm->memory[DSTACK_OFFSET + top] = value;
}

svm_unit svm_dstack_view(svm_state* svm) {
    svm_addr top = 0;
    if (svm->dp > 0)
        top = svm->dp - 1;
    else
        top = DSTACK_SIZE - 1;
    return svm->memory[DSTACK_OFFSET + top];
}

void svm_rstack_push(svm_state* svm, svm_addr value) {
    *(svm_addr*)(svm->memory + RSTACK_OFFSET + svm->rp) = value;
    svm->rp = (svm->rp + sizeof(svm_addr) / sizeof(svm_unit)) % RSTACK_SIZE;
}

svm_addr svm_rstack_pop(svm_state* svm) {
    if (svm->rp > 0)
        svm->rp = svm->rp - sizeof(svm_addr) / sizeof(svm_unit);
    else
        svm->rp = RSTACK_SIZE - sizeof(svm_addr) / sizeof(svm_unit);
    return *(svm_addr*)(svm->memory + RSTACK_OFFSET + svm->rp);
}

void svm_jump(svm_state* svm, svm_addr there) {
     svm->pc = there;
}

#endif /* SVM_IMPLEMENTATION */
#endif /* _SVM_H_ */
