#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static FILE* outfh = 0;
static void cprintf(const char* fmt, ...);

static int load_opnames(char* buf, int bufsize, const char* res[], int rescount);

int main() {
    static char opnamesbuf[1024] = {0};
    static const char* opnames[32] = {0};
    int opcount = load_opnames(opnamesbuf, 1024, opnames, 32);
    int opcode = 0;
    int i = 0;

    if (opcount < 0) {
        if (opcount == -1) {
            fprintf(stderr, "load_opnames: buffer overflow\n");
        }
        if (opcount == -2) {
            perror("svm.h");
        }
        return 1;
    }

    outfh = fopen("ops.h", "w");
    if (!outfh) {
        perror("ops.h");
        outfh = stdout;
    }

    cprintf("#ifndef _OPS_H_\n"
            "#define _OPS_H_\n"
            "\n");

    opcode=0;
    i=0;
    while (i < opcount) {
        cprintf("#define SVM_%s %2d\n", opnames[i], opcode);
        opcode++;
        i++;
    }

    cprintf("\n"
            "#define SVM_MIN_OP SVM_%s\n"
            "#define SVM_MAX_OP SVM_%s\n",
            opnames[0],
            opnames[opcount - 1]);

    cprintf("\n"
            "#ifdef SVM_CODE_STR\n"
            "const char** svm_code_str;\n"
            "#elif defined(SVM_CODE_STR_DEF)\n"
            "const char* svm_code_str[] = {\n");
    opcode=0;
    i=0;
    while (i < opcount) {
        cprintf("    \"%s\",  /* %2d */\n", opnames[i], opcode);
        opcode++;
        i++;
    }
    cprintf("};\n"
            "#endif /* SVM_CODE_STR */\n");

    cprintf("\n"
            "#ifdef SVM_CODE_FUNC\n"
            "void*const(*const  svm_code_func)(svm_state*);\n"
            "#elif defined(SVM_CODE_FUNC_DEF)\n"
            "void(*const svm_code_func[])(svm_state*) = {\n");
    opcode=0;
    i=0;
    while (i < opcount) {
        cprintf("    &svm_exec_%s,  /* %2d */\n", opnames[i], opcode);
        opcode++;
        i++;
    }
    cprintf("};\n"
            "#endif /* SVM_CODE_FUNC */\n"
            "\n"
            "#endif /* _OPS_H_ */\n");

    #undef opcount
    return 0;
}


static void cprintf(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(outfh, fmt, va);
    va_end(va);
}

static int load_opnames(char* buf, int bufsize, const char* res[], int rescount) {
    char ch = 0;
    int i = 0;
    int j = 0;
    int k = 0;
    int m = 0;
    int capture = 0;
    static const char*const prefix = "svm_exec_";
    static const char delim = '(';
    FILE* fh = fopen("svm.h", "r");
    if (!fh)
        return -2;
    while (!feof(fh)) {
        ch=fgetc(fh);
        if (capture) {
            if (j >= bufsize) {
                fclose(fh);
                return -1;
            }
            if (ch == delim) {
                capture = 0;
                if (k >= rescount) {
                    fclose(fh);
                    return -1;
                }
                buf[j++] = 0;
                res[k++] = buf + m;
                m = j;
            }
            else {
                buf[j++] = ch;
            }
        }
        else if (prefix[i] == ch) {
            i++;
            if (prefix[i] == 0) {
                capture = 1;
                i = 0;
            }
        } else {
            i = 0;
        }
    }
    fclose(fh);
    return k;
}
