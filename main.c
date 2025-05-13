#define SVM_IMPLEMENTATION
#include "svm.h"

const svm_unit code[] = {
    0
};

int main() {
    svm_state svm;
    svm_init(&svm);
    svm_load(code, sizeof code / sizeof code[0]);
    svm_execute(&svm);
    return 0;
}
