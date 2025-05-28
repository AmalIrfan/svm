#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static FILE* outfh = 0;
void cprintf(const char* fmt, ...);

int main() {
    static const char* opnames[] = {
        "NOP",
        "HLT",
        "CAL",
        "RET",
        "BNZ",
        "BNG",
        "ADD",
        "SUB",
        "AND",
        "LIT",
        "LAD",
        "FCH",
        "PUT",
        "POP",
        "PSH",
        "DUP",
        "DRP",
        "OVR",
        "ROT",
        0
    };
    const char** opname = (opnames+0);
    int opcode = 0;
    #define opcount (sizeof(opnames) / sizeof(opnames[0]) - 1)

    outfh = fopen("ops.h", "w");
    if (!outfh) {
        perror("ops.h");
        outfh = stdout;
    }

    cprintf("#ifndef _OPS_H_\n"
            "#define _OPS_H_\n"
            "\n");

    opname=(opnames+0);
    opcode=0;
    while (*opname) {
        cprintf("#define SVM_%s %2d\n", *opname, opcode);
        opname++;
        opcode++;
    }

    cprintf("\n"
            "#define SVM_MIN_OP SVM_%s\n"
            "#define SVM_MAX_OP SVM_%s\n",
            opnames[0],
            opnames[opcount - 1]);

    cprintf("\n"
            "const char* svm_code_str[] = {\n");
    opname=(opnames+0);
    opcode=0;
    while (*opname) {
        cprintf("    \"%s\",  /* %2d */\n", *opname, opcode);
        opname++;
        opcode++;
    }
    cprintf("0,\n"
            "};\n");
    cprintf("\n"
            "#endif /* _OPS_H_ */\n");

    #undef opcount
    return 0;
}


void cprintf(const char* fmt, ...) {
    va_list va;
    va_start(va, fmt);
    vfprintf(outfh, fmt, va);
    va_end(va);
}
