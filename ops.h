#ifndef _OPS_H_
#define _OPS_H_

#define SVM_NOP  0
#define SVM_HLT  1
#define SVM_CAL  2
#define SVM_RET  3
#define SVM_BNZ  4
#define SVM_BNG  5
#define SVM_ADD  6
#define SVM_SUB  7
#define SVM_AND  8
#define SVM_LIT  9
#define SVM_LAD 10
#define SVM_FCH 11
#define SVM_PUT 12
#define SVM_POP 13
#define SVM_PSH 14
#define SVM_DUP 15
#define SVM_DRP 16
#define SVM_OVR 17
#define SVM_ROT 18

#define SVM_MIN_OP SVM_NOP
#define SVM_MAX_OP SVM_ROT

#ifdef SVM_CODE_STR
const char** svm_code_str;
#elif defined(SVM_CODE_STR_DEF)
const char* svm_code_str[] = {
    "NOP",  /*  0 */
    "HLT",  /*  1 */
    "CAL",  /*  2 */
    "RET",  /*  3 */
    "BNZ",  /*  4 */
    "BNG",  /*  5 */
    "ADD",  /*  6 */
    "SUB",  /*  7 */
    "AND",  /*  8 */
    "LIT",  /*  9 */
    "LAD",  /* 10 */
    "FCH",  /* 11 */
    "PUT",  /* 12 */
    "POP",  /* 13 */
    "PSH",  /* 14 */
    "DUP",  /* 15 */
    "DRP",  /* 16 */
    "OVR",  /* 17 */
    "ROT",  /* 18 */
};
#endif /* SVM_CODE_STR */

#ifdef SVM_CODE_FUNC
void*const(*const  svm_code_func)(svm_state*);
#elif defined(SVM_CODE_FUNC_DEF)
void(*const svm_code_func[])(svm_state*) = {
    &svm_exec_NOP,  /*  0 */
    &svm_exec_HLT,  /*  1 */
    &svm_exec_CAL,  /*  2 */
    &svm_exec_RET,  /*  3 */
    &svm_exec_BNZ,  /*  4 */
    &svm_exec_BNG,  /*  5 */
    &svm_exec_ADD,  /*  6 */
    &svm_exec_SUB,  /*  7 */
    &svm_exec_AND,  /*  8 */
    &svm_exec_LIT,  /*  9 */
    &svm_exec_LAD,  /* 10 */
    &svm_exec_FCH,  /* 11 */
    &svm_exec_PUT,  /* 12 */
    &svm_exec_POP,  /* 13 */
    &svm_exec_PSH,  /* 14 */
    &svm_exec_DUP,  /* 15 */
    &svm_exec_DRP,  /* 16 */
    &svm_exec_OVR,  /* 17 */
    &svm_exec_ROT,  /* 18 */
};
#endif /* SVM_CODE_FUNC */

#endif /* _OPS_H_ */
