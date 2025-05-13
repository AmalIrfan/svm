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
} svm_state;

void svm_init(svm_state* svm);
void svm_load(svm_state* svm, const svm_unit* code, const svm_unit size);
void svm_execute(svm_state* svm);

svm_unit svm_unit_here(svm_state* svm);
void svm_advance(svm_state* svm);

typedef enum svm_code {
    SVM_CALL,
    SVM_EXIT
} svm_code;

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
    svm_code code = 0;
    while (1) {
        code = svm_unit_here(svm);
        switch (code) {
        case SVM_CALL:
        case SVM_EXIT:
            svm_advance(svm);
            break;
        default:
            printf("Unrecognised op %d\n", code);
            return;
        }
    }
}

svm_unit svm_unit_here(svm_state* svm) {
    return svm->memory[svm->here];
}

void svm_advance(svm_state* svm) {
    svm->here = (svm->here + 1) % MEMORY_SIZE;
}

#endif /* SVM_IMPLEMENTATION */
#endif /* _SVM_H_ */
