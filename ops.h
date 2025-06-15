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
#define SVM_DIV  8
#define SVM_AND  9
#define SVM_OR 10
#define SVM_LIT 11
#define SVM_LAD 12
#define SVM_FCH 13
#define SVM_PUT 14
#define SVM_POP 15
#define SVM_PSH 16
#define SVM_DUP 17
#define SVM_DRP 18
#define SVM_OVR 19
#define SVM_SWP 20
#define SVM_ROT 21

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
    "DIV",  /*  8 */
    "AND",  /*  9 */
    "OR",  /* 10 */
    "LIT",  /* 11 */
    "LAD",  /* 12 */
    "FCH",  /* 13 */
    "PUT",  /* 14 */
    "POP",  /* 15 */
    "PSH",  /* 16 */
    "DUP",  /* 17 */
    "DRP",  /* 18 */
    "OVR",  /* 19 */
    "SWP",  /* 20 */
    "ROT",  /* 21 */
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
    &svm_exec_DIV,  /*  8 */
    &svm_exec_AND,  /*  9 */
    &svm_exec_OR,  /* 10 */
    &svm_exec_LIT,  /* 11 */
    &svm_exec_LAD,  /* 12 */
    &svm_exec_FCH,  /* 13 */
    &svm_exec_PUT,  /* 14 */
    &svm_exec_POP,  /* 15 */
    &svm_exec_PSH,  /* 16 */
    &svm_exec_DUP,  /* 17 */
    &svm_exec_DRP,  /* 18 */
    &svm_exec_OVR,  /* 19 */
    &svm_exec_SWP,  /* 20 */
    &svm_exec_ROT,  /* 21 */
};
#endif /* SVM_CODE_FUNC */

#endif /* _OPS_H_ */
