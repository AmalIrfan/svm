#ifndef _SVM_H_
#define _SVM_H_
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

typedef int8_t svm_word;
typedef uint16_t svm_dword;
typedef uint32_t svm_vword;

#define SVM_VWORD (sizeof(svm_vword) / sizeof(svm_word))

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
    svm_word memory[MEMORY_SIZE];
    svm_dword pc;
    svm_dword dp;
    svm_dword rp;
    svm_vword cache;
    bool debug;
    bool halt;
    bool jumped;
} svm_state;

void svm_init(svm_state* svm);
void svm_load(svm_state* svm, const svm_word* code, const svm_dword size, bool debug);
void svm_execute(svm_state* svm);

svm_word svm_word_here(svm_state* svm);
svm_dword svm_dword_here(svm_state* svm);
svm_word svm_word_there(svm_state* svm, svm_dword there);
svm_dword svm_dword_there(svm_state* svm, svm_dword there);
void svm_store_there(svm_state* svm, svm_dword there, svm_word value);
void     svm_advance(svm_state* svm);
void     svm_advance_dword(svm_state* svm);
void     svm_dstack_push(svm_state* svm, svm_word value);
svm_word svm_dstack_pop(svm_state* svm);
svm_word svm_dstack_view(svm_state* svm);
void     svm_dstack_set_top(svm_state* svm, svm_word value);
void     svm_rstack_push(svm_state* svm, svm_dword value);
svm_dword svm_rstack_pop(svm_state* svm);
void     svm_jump(svm_state* svm, svm_dword there);
void     svm_check_if_port(svm_state* svm, svm_dword there);

typedef svm_word svm_code;


#ifdef SVM_IMPLEMENTATION
void svm_exec_NOP(svm_state* svm) {
    svm_advance(svm);
}
void svm_exec_HLT(svm_state* svm) {
    svm->halt = true;
}
void svm_exec_CAL(svm_state* svm) {
    svm_advance(svm);
    {
        svm_dword there = (svm_dword)((svm->cache >> ((svm->pc % SVM_VWORD) * sizeof(svm_word) * 8)) & 0xFFFF);
        svm_advance_dword(svm);
        svm_rstack_push(svm, svm->pc);
        svm_jump(svm, there);
    }
}
void svm_exec_RET(svm_state* svm) {
    svm_advance(svm);
    svm_jump(svm, svm_rstack_pop(svm));
}
void svm_exec_BNZ(svm_state* svm) {
    svm_advance(svm);
    {
        svm_word offset = svm_word_here(svm);
        svm_advance(svm);
        if (svm_dstack_pop(svm)) {
                svm_jump(svm, svm->pc + offset);
        }
    }
}
void svm_exec_BNG(svm_state* svm) {
    svm_advance(svm);
    {
        svm_word offset = svm_word_here(svm);
        svm_advance(svm);
        if ((int8_t)svm_dstack_pop(svm) < 0) {
                svm_jump(svm, svm->pc + offset);
        }
    }
}
void svm_exec_ADD(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_SUB(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_AND(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_LIT(svm_state* svm) {
    svm_advance(svm);
    svm_dstack_push(svm, svm_word_here(svm));
    svm_advance(svm);
}
void svm_exec_LAD(svm_state* svm) {
    svm_advance(svm);
    {
        svm_dword dword = svm_dword_here(svm);
        svm_dstack_push(svm, dword & 0xFF);
        svm_dstack_push(svm, dword >> 8);
    }
    svm_advance_dword(svm);
	
}
void svm_exec_FCH(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_PUT(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_POP(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_PSH(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_DUP(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_DRP(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_OVR(svm_state* svm) {
	(void)svm;
    exit(2);
}
void svm_exec_ROT(svm_state* svm) {
	(void)svm;
    exit(2);
}

#define SVM_CODE_STR_DEF
#define SVM_CODE_FUNC_DEF
#include "ops.h"

#include <stdio.h>
#include <string.h>

void svm_init(svm_state* svm) {
    printf("MEMORY_SIZE    %5X\n", MEMORY_SIZE);
    printf("DSTACK_OFFSET  %5X DSTACK_SIZE  %5X\n", DSTACK_OFFSET, DSTACK_SIZE);
    printf("RSTACK_OFFSET  %5X RSTACK_SIZE  %5X\n", RSTACK_OFFSET, RSTACK_SIZE);
    printf("GENERAL_OFFSET %5X GENERAL_SIZE %5X\n", GENERAL_OFFSET, GENERAL_SIZE);
    memset(svm, 0, sizeof(*svm));
}

void svm_load(svm_state* svm, const svm_word* code, const svm_dword size, bool debug) {
    if (size > GENERAL_SIZE) {
        printf("CODE OVERFLOW: %d UNITS\n", size - GENERAL_SIZE);
        return;
    }
    memcpy(&svm->memory[GENERAL_OFFSET], code, size * sizeof(*code));
    svm->debug = debug;
}

void svm_execute(svm_state* svm) {
    svm_code code = SVM_NOP;
    while (!svm->halt) {
        if (svm->pc % SVM_VWORD == 0 || svm->jumped) {
            svm->cache = *(svm_vword*)(svm->memory + GENERAL_OFFSET + svm->pc - (svm->pc % SVM_VWORD));
            if (svm->jumped)
                svm->jumped = false;
        }
        code = (svm_code)((svm->cache >> ((svm->pc % SVM_VWORD) * sizeof(svm_word) * 8)) & 0xFF);
        if (svm->debug) {
            int i = 0;
            FILE *fh = 0;
            FILE *fi = fopen("/dev/tty", "r");
            fgetc(fi);
            fclose(fi);
            fprintf(stderr, "%3d %5s", svm->pc, ((int)code > 0 ? svm_code_str[code] : ""));
            if (code == SVM_LIT || code == SVM_BNZ) {
                fprintf(stderr, " %3hhd  ", svm_word_there(svm, svm->pc + 1));
            } else if (code == SVM_CAL || code == SVM_LAD) {
                fprintf(stderr, " %4hu ", svm_dword_there(svm, svm->pc + 1));
            } else {
                fputs("      ", stderr);
            }
            putc('[', stderr);
            for (i = 0; i < svm->dp; i++) {
                fprintf(stderr, "%hhd%s", svm->memory[DSTACK_OFFSET + i],
                        (i + 1 >= svm->dp ? "" : ", "));
            }
            fputs("] r[", stderr);
            for (i = 0; i < svm->rp; i+=sizeof(svm_dword)/sizeof(svm_word)) {
                fprintf(stderr, "%hu%s", *(svm_dword*)(svm->memory + RSTACK_OFFSET + i),
                        (i + sizeof(svm_dword)/sizeof(svm_word) >= svm->rp ? "" : ", "));
            }
            fputs("]\n", stderr);
            fh = fopen("memdump", "w");
            fwrite(svm->memory, 1, MEMORY_SIZE, fh);
            fclose(fh);
        }
        if (code >= SVM_MIN_OP && code <= SVM_MAX_OP) {
            (*svm_code_func[code])(svm);
        }
        else {
            printf("Unrecognised op %d\n", code);
            return;
        }
#if 0
        switch (code) {
        case SVM_NOP:
             svm_advance(svm);
             break;
        case SVM_LIT:
            svm_advance(svm);
            svm_dstack_push(svm, svm_word_here(svm));
            svm_advance(svm);
            break;
        case SVM_LAD:
            svm_advance(svm);
            {
                svm_dword dword = svm_dword_here(svm);
                svm_dstack_push(svm, dword & 0xFF);
                svm_dstack_push(svm, dword >> 8);
            }
            svm_advance_dword(svm);
            break;
        case SVM_CAL:
            svm_advance(svm);
            {
                svm_dword there = svm_dword_here(svm);
                svm_advance_dword(svm);
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
                svm_word top = svm_dstack_pop(svm);
                svm_word over = svm_dstack_view(svm);
                svm_dstack_set_top(svm, top);
                svm_dstack_push(svm, over);
            }
            break;
#endif
        case SVM_OVR:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_word over = svm_dstack_view(svm);
                svm_dstack_push(svm, top);
                svm_dstack_push(svm, over);
            }
            break;
        case SVM_ROT:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_word over = svm_dstack_pop(svm);
                svm_word back = svm_dstack_pop(svm);
                svm_dstack_push(svm, over);
                svm_dstack_push(svm, top);
                svm_dstack_push(svm, back);
            }
            break;
        case SVM_PSH:
            svm_advance(svm);
            {
                svm_dword dword_h = svm_dstack_pop(svm);
                svm_dword dword_l = svm_dstack_pop(svm);
                svm_dword dword = (dword_h & 0xFF) << 8 | (dword_l & 0xFF);
                svm_rstack_push(svm, dword);
            }
            break;
        case SVM_POP:
            svm_advance(svm);
            {
                svm_dword dword = svm_rstack_pop(svm);
                svm_dstack_push(svm, dword & 0xFF);
                svm_dstack_push(svm, dword >> 8);
            }
            break;
        case SVM_SUB:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) - top);
            }
            break;
        case SVM_ADD:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) + top);
            }
            break;
        case SVM_AND:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) & top);
            }
            break;
#if 0
        case SVM_MUL:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) * top);
            }
            break;
        case SVM_DIV:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_word over = svm_dstack_view(svm);
                svm_dstack_set_top(svm, over / top);
                svm_dstack_push(svm, over % top);
            }
            break;
        case SVM_LT:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) < top);
            }
            break;
        case SVM_EQ:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) == top);
            }
            break;
        case SVM_GT:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) > top);
            }
            break;
        case SVM_LTE:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) <= top);
            }
            break;
        case SVM_NEQ:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) != top);
            }
            break;
        case SVM_GTE:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
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
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) | top);
            }
            break;
        case SVM_XOR:
            svm_advance(svm);
            {
                svm_word top = svm_dstack_pop(svm);
                svm_dstack_set_top(svm, svm_dstack_view(svm) ^ top);
            }
            break;
#endif
        case SVM_BNG:
            svm_advance(svm);
            {
                svm_word offset = svm_word_here(svm);
                svm_advance(svm);
                if ((int8_t)svm_dstack_pop(svm) < 0) {
                        svm_jump(svm, svm->pc + offset);
                }
            }
            break;
        case SVM_BNZ:
            svm_advance(svm);
            {
                svm_word offset = svm_word_here(svm);
                svm_advance(svm);
                if (svm_dstack_pop(svm)) {
                        svm_jump(svm, svm->pc + offset);
                }
            }
            break;
        case SVM_PUT:
            svm_advance(svm);
            {
                svm_dword dword_h = svm_dstack_pop(svm);
                svm_dword dword_l = svm_dstack_pop(svm);
                svm_dword there = (dword_h & 0xFF) << 8 | (dword_l & 0xFF);
                svm_store_there(svm, there, svm_dstack_pop(svm));
            }
            break;
        case SVM_FCH:
            svm_advance(svm);
            {
                svm_dword dword_h = svm_dstack_pop(svm);
                svm_dword dword_l = svm_dstack_pop(svm);
                svm_dword there = (dword_h & 0xFF) << 8 | (dword_l & 0xFF);
                svm_check_if_port(svm, there);
                svm_dstack_push(svm, svm_word_there(svm, there));
            }
            break;
        case SVM_HLT:
            printf("Halted\n");
            return;
        default:
            printf("Unrecognised op %d\n", code);
            return;
        }
#endif
    }
}

svm_word svm_word_here(svm_state* svm) {
    return svm->memory[GENERAL_OFFSET + svm->pc];
}

svm_dword svm_dword_here(svm_state* svm) {
    return *(svm_dword*)(svm->memory + GENERAL_OFFSET + svm->pc);
}

svm_word svm_word_there(svm_state* svm, svm_dword there) {
    return svm->memory[GENERAL_OFFSET + there];
}

svm_dword svm_dword_there(svm_state* svm, svm_dword there) {
    return *(svm_dword*)(svm->memory + GENERAL_OFFSET + there);
}

void svm_store_there(svm_state* svm, svm_dword there, svm_word value) {
    there = there % GENERAL_SIZE;
    svm->memory[GENERAL_OFFSET + there] = value;
}

void svm_advance(svm_state* svm) {
    svm->pc = (svm->pc + 1) % GENERAL_SIZE;
}

void svm_advance_dword(svm_state* svm) {
    svm->pc = (svm->pc + sizeof(svm_dword) / sizeof(svm_word)) % GENERAL_SIZE;
}

void svm_dstack_push(svm_state* svm, svm_word value) {
    svm->memory[DSTACK_OFFSET + svm->dp] = value;
    svm->dp = (svm->dp + 1) % DSTACK_SIZE;
}

svm_word svm_dstack_pop(svm_state* svm) {
    if (svm->dp > 0)
        svm->dp = svm->dp - 1;
    else
        svm->dp = DSTACK_SIZE - 1;
    return svm->memory[DSTACK_OFFSET + svm->dp];
}

void svm_dstack_set_top(svm_state* svm, svm_word value) {
    svm_dword top = svm->dp - 1;
    if (svm->dp == 0)
        top = DSTACK_SIZE - 1;
    svm->memory[DSTACK_OFFSET + top] = value;
}

svm_word svm_dstack_view(svm_state* svm) {
    svm_dword top = 0;
    if (svm->dp > 0)
        top = svm->dp - 1;
    else
        top = DSTACK_SIZE - 1;
    return svm->memory[DSTACK_OFFSET + top];
}

void svm_rstack_push(svm_state* svm, svm_dword value) {
    *(svm_dword*)(svm->memory + RSTACK_OFFSET + svm->rp) = value;
    svm->rp = (svm->rp + sizeof(svm_dword) / sizeof(svm_word)) % RSTACK_SIZE;
}

svm_dword svm_rstack_pop(svm_state* svm) {
    if (svm->rp > 0)
        svm->rp = svm->rp - sizeof(svm_dword) / sizeof(svm_word);
    else
        svm->rp = RSTACK_SIZE - sizeof(svm_dword) / sizeof(svm_word);
    return *(svm_dword*)(svm->memory + RSTACK_OFFSET + svm->rp);
}

void svm_jump(svm_state* svm, svm_dword there) {
    svm->pc = there;
    svm->jumped = true;
}

void svm_check_if_port(svm_state* svm, svm_dword there) {
    switch (there + GENERAL_OFFSET) {
        case PORT_IN:
            svm->memory[PORT_DATA] = getc(stdin);
            break;
        case PORT_OUT:
            putc(svm->memory[PORT_DATA], stdout);
            break;
    }
}
#else /* !SVM_IMPLEMENTATION */
#define SVM_CODE_STR
#define SVM_CODE_FUNC
#include "ops.h"
#endif /* SVM_IMPLEMENTATION */
#endif /* _SVM_H_ */
