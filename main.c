#include <stdio.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

#define SVM_CODE_MAX 200

int main(int argc, char *argv[]) {
    svm_state svm;
    svm_unit code[SVM_CODE_MAX] = {0};
    int n = 0;
    FILE* fh = 0;

    svm_init(&svm);

    if (argc != 2) {
        fprintf(stderr, "Usage: %s input\n", argv[0]);
        return 1;
    }

    fh = fopen(argv[1], "r");
    if (!fh) {
        perror(argv[1]);
        return 1;
    }

    n = fread(code, sizeof(code[0]), SVM_CODE_MAX, fh);
    fclose(fh);
    svm_load(&svm, code, n);

    svm_execute(&svm);

    return 0;
}
