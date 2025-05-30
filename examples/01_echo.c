#define SVM_IMPLEMENTATION
#include "svm.h"

const svm_word code[] = {
    SVM_LIT, 4, SVM_CALL, -1,
    SVM_READ, SVM_DUP, SVM_WRITE, SVM_LIT, 10, SVM_SUB, SVM_LIT, 4, SVM_JNZ,
    SVM_EXIT
};

int main() {
    svm_state svm;
    svm_init(&svm);
    svm_load(&svm, code, sizeof code / sizeof code[0]);
    svm_execute(&svm);
    return 0;
}
