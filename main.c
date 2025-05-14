#include <stdio.h>

#define SVM_IMPLEMENTATION
#include "svm.h"

#define SVM_CODE_MAX 100

int main() {
    svm_state svm;
    svm_unit code[SVM_CODE_MAX] = {0};
    int n = 0;
    svm_init(&svm);

    n = fread(code, sizeof(code[0]), SVM_CODE_MAX, stdin);
    svm_load(&svm, code, n);

    if (!freopen("/dev/tty", "r", stdin)) {
        fflush(stdout);
        perror("fopen: /dev/tty");
        return 1;
    }

    svm_execute(&svm);

    return 0;
}
