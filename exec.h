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
        fprintf(stderr, "%04X\n", there);
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
