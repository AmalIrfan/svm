#ifndef _SVM_H_
#define _SVM_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef int8_t svm_unit;
typedef uint16_t svm_addr;

#define MEMORY_SIZE 0x10000
#define DSTACK_OFFSET  0x0000
#define RSTACK_OFFSET  0x0100
#define GENERAL_OFFSET 0x0200
#define DSTACK_SIZE  (RSTACK_OFFSET  - DSTACK_OFFSET)
#define RSTACK_SIZE  (GENERAL_OFFSET - RSTACK_OFFSET)
#define GENERAL_SIZE (MEMORY_SIZE    - GENERAL_OFFSET)
#define PORT_IN   0xFFFD
#define PORT_OUT  0xFFFE
#define PORT_DATA 0xFFFF

typedef struct svm_state {
    svm_unit memory[MEMORY_SIZE];
    svm_addr pc;
    svm_addr dp;
    svm_addr rp;
    bool debug;
} svm_state;

void svm_init(svm_state* svm);
void svm_load(svm_state* svm, const svm_unit* code, const svm_addr size, bool debug);
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
void     svm_check_if_port(svm_state* svm, svm_addr there);

typedef svm_unit svm_code;

#include "ops.h"

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

void svm_load(svm_state* svm, const svm_unit* code, const svm_addr size, bool debug) {
    if (size > GENERAL_SIZE) {
        printf("CODE OVERFLOW: %d UNITS\n", size - GENERAL_SIZE);
        return;
    }
    memcpy(&svm->memory[GENERAL_OFFSET], code, size * sizeof(*code));
    svm->debug = debug;
}

void svm_execute(svm_state* svm) {
    svm_code code = SVM_NOP;
    while (1) {
        code = (svm_code)svm_unit_here(svm);
        if (svm->debug) {
            int i = 0;
            FILE *fh = 0;
            FILE *fi = fopen("/dev/tty", "r");
            fgetc(fi);
            fclose(fi);
            fprintf(stderr, "%3d %5s", svm->pc, ((int)code > 0 ? svm_code_str[code] : ""));
            if (code == SVM_LIT || code == SVM_BNZ) {
                fprintf(stderr, " %3hhd  ", svm_unit_there(svm, svm->pc + 1));
            } else if (code == SVM_CAL || code == SVM_LAD) {
                fprintf(stderr, " %4hu ", svm_addr_there(svm, svm->pc + 1));
            } else {
                fputs("      ", stderr);
            }
            putc('[', stderr);
            for (i = 0; i < svm->dp; i++) {
                fprintf(stderr, "%hhd%s", svm->memory[DSTACK_OFFSET + i],
                        (i + 1 >= svm->dp ? "" : ", "));
            }
            fputs("] r[", stderr);
            for (i = 0; i < svm->rp; i+=sizeof(svm_addr)/sizeof(svm_unit)) {
                fprintf(stderr, "%hu%s", *(svm_addr*)(svm->memory + RSTACK_OFFSET + i),
                        (i + sizeof(svm_addr)/sizeof(svm_unit) >= svm->rp ? "" : ", "));
            }
            fputs("]\n", stderr);
            fh = fopen("memdump", "w");
            fwrite(svm->memory, 1, MEMORY_SIZE, fh);
            fclose(fh);
        }
        switch (code) {
        case SVM_NOP:
             svm_advance(svm);
             break;
        case SVM_LIT:
            svm_advance(svm);
            svm_dstack_push(svm, svm_unit_here(svm));
            svm_advance(svm);
            break;
        case SVM_LAD:
            svm_advance(svm);
            {
                svm_addr addr = svm_addr_here(svm);
                svm_dstack_push(svm, addr & 0xFF);
                svm_dstack_push(svm, addr >> 8);
            }
            svm_advance_addr(svm);
            break;
        case SVM_CAL:
            svm_advance(svm);
            {
                svm_addr there = svm_addr_here(svm);
                svm_advance_addr(svm);
                svm_rstack_push(svm, svm->pc);
                svm_jump(svm, there);
            }
            break;
        case SVM_RET:
            svm_advance(svm);
            svm_jump(svm, svm_rstack_pop(svm));
            break;
        case SVM_DUP:
            svm_advance(svm);
            svm_dstack_push(svm, svm_dstack_view(svm));
            break;
        case SVM_DRP:
            svm_advance(svm);
            svm_dstack_pop(svm);
            break;
#if 0
        case SVM_SWAP:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_unit over = svm_dstack_view(svm);
                svm_dstack_set_top(svm, top);
                svm_dstack_push(svm, over);
            }
            break;
#endif
        case SVM_OVR:
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
        case SVM_PSH:
            svm_advance(svm);
            {
                svm_addr addr_h = svm_dstack_pop(svm);
                svm_addr addr_l = svm_dstack_pop(svm);
                svm_addr addr = (addr_h & 0xFF) << 8 | (addr_l & 0xFF);
                svm_rstack_push(svm, addr);
            }
            break;
        case SVM_POP:
            svm_advance(svm);
            {
                svm_addr addr = svm_rstack_pop(svm);
                svm_dstack_push(svm, addr & 0xFF);
                svm_dstack_push(svm, addr >> 8);
            }
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
        case SVM_AND:
            svm_advance(svm);
            {
                svm_unit top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) & top);
            }
            break;
#if 0
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
#endif
        case SVM_BNG:
            svm_advance(svm);
            {
                svm_unit offset = svm_unit_here(svm);
                svm_advance(svm);
                if ((int8_t)svm_dstack_pop(svm) < 0) {
                        svm_jump(svm, svm->pc + offset);
                }
            }
            break;
        case SVM_BNZ:
            svm_advance(svm);
            {
                svm_unit offset = svm_unit_here(svm);
                svm_advance(svm);
                if (svm_dstack_pop(svm)) {
                        svm_jump(svm, svm->pc + offset);
                }
            }
            break;
        case SVM_PUT:
            svm_advance(svm);
            {
                svm_addr addr_h = svm_dstack_pop(svm);
                svm_addr addr_l = svm_dstack_pop(svm);
                svm_addr there = (addr_h & 0xFF) << 8 | (addr_l & 0xFF);
                svm_store_there(svm, there, svm_dstack_pop(svm));
            }
            break;
        case SVM_FCH:
            svm_advance(svm);
            {
                svm_addr addr_h = svm_dstack_pop(svm);
                svm_addr addr_l = svm_dstack_pop(svm);
                svm_addr there = (addr_h & 0xFF) << 8 | (addr_l & 0xFF);
                svm_check_if_port(svm, there);
                svm_dstack_push(svm, svm_unit_there(svm, there));
            }
            break;
        case SVM_HLT:
            printf("Halted\n");
            return;
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

void svm_check_if_port(svm_state* svm, svm_addr there) {
    switch (there + GENERAL_OFFSET) {
        case PORT_IN:
            svm->memory[PORT_DATA] = getc(stdin);
            break;
        case PORT_OUT:
            putc(svm->memory[PORT_DATA], stdout);
            break;
    }
}

#endif /* SVM_IMPLEMENTATION */
#endif /* _SVM_H_ */
