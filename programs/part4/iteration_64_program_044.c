/* test-mcf-debug.c
 * Designed to trigger debug dumping of fixup graph with NEW_ENTRY/NEW_EXIT nodes
 * when compiled with GCC built with --enable-checking (MCF_DEBUG defined).
 * Coverage target: mcf.cc lines 151-162 in dump_fixup_edge()
 */

/* Force priority-based IRA algorithm which uses min-cost flow solver */
#ifdef __GNUC__
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_IRA
#endif

/* Target ARM for limited register set (16 GP registers, some reserved) */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Prevent optimization of variables */
#define USE(var) asm volatile("" : : "r"(var) :)

/* Clobber many registers to increase pressure */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* For x86 testing alternative */
#define CLOBBER_X86 asm volatile("" : : : \
    "eax", "ebx", "ecx", "edx", "esi", "edi", \
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory")

/* Main high-pressure function */
FORCE_PRIORITY_IRA TARGET_ARM
void high_pressure_func(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with computations to create dependencies */
    v1 = cond1 * 2;
    v2 = cond2 + 5;
    v3 = cond3 - 3;
    v4 = v1 + v2;
    v5 = v2 * v3;
    v6 = v3 - v1;
    v7 = v4 * 7;
    v8 = v5 / 2;
    v9 = v6 + 11;
    v10 = v7 - v8;
    v11 = v8 * v9;
    v12 = v9 + v10;
    v13 = v10 * 3;
    v14 = v11 - 7;
    v15 = v12 + v13;
    v16 = v14 * v15;
    
    CLOBBER_REGS;  /* Force many registers to appear clobbered */
    
    /* Complex control flow to create different live ranges across blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 + v12;
        v13 = v14 * v15;
        v16 = v1 + v4;
        USE(v7); USE(v10); USE(v13); USE(v16);
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Different subset in nested block */
            v2 = v3 * v4;
            v5 = v6 + v7;
            v8 = v9 - v10;
            v11 = v12 * v13;
            v14 = v15 + v16;
            USE(v2); USE(v5); USE(v8); USE(v11); USE(v14);
            CLOBBER_REGS;
        } else {
            /* Alternative subset */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
            v15 = v16 * v1;
            USE(v3); USE(v6); USE(v9); USE(v12); USE(v15);
            CLOBBER_REGS;
        }
    } else {
        /* Else branch uses different variables */
        v2 = v4 + v6;
        v3 = v5 * v7;
        v8 = v9 - v11;
        v10 = v12 + v14;
        v13 = v15 * v16;
        USE(v2); USE(v3); USE(v8); USE(v10); USE(v13);
        CLOBBER_REGS;
        
        if (cond3 != 0) {
            v1 = v8 + v10;
            v4 = v13 * v2;
            v6 = v3 - v1;
            v9 = v4 / 2;
            v11 = v6 * 7;
            v14 = v9 + v11;
            USE(v1); USE(v4); USE(v6); USE(v9); USE(v11); USE(v14);
            CLOBBER_REGS;
        }
    }
    
    /* Final use of all variables to extend live ranges */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Volatile asm to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Second function with switch statement for more complex CFG */
FORCE_PRIORITY_IRA TARGET_ARM
void switch_pressure_func(int selector) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = selector;
    a2 = a1 * 2;
    a3 = a2 + 1;
    a4 = a3 - selector;
    a5 = a4 * 3;
    a6 = a5 / 2;
    a7 = a6 + a1;
    a8 = a7 * a2;
    a9 = a8 - a3;
    a10 = a9 + a4;
    a11 = a10 * a5;
    a12 = a11 - a6;
    a13 = a12 + a7;
    a14 = a13 * a8;
    
    CLOBBER_REGS;
    
    /* Switch creates multiple basic blocks */
    switch (selector & 0x7) {  /* 8 cases */
        case 0:
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            USE(a1); USE(a4); USE(a7);
            break;
        case 1:
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 + a13;
            USE(a2); USE(a5); USE(a8); USE(a11);
            break;
        case 2:
            a3 = a4 + a5;
            a6 = a7 * a8;
            a9 = a10 - a11;
            a12 = a13 + a14;
            USE(a3); USE(a6); USE(a9); USE(a12);
            break;
        case 3:
            a4 = a5 + a6;
            a7 = a8 * a9;
            a10 = a11 - a12;
            a13 = a14 + a1;
            USE(a4); USE(a7); USE(a10); USE(a13);
            break;
        case 4:
            a5 = a6 + a7;
            a8 = a9 * a10;
            a11 = a12 - a13;
            a14 = a1 + a2;
            USE(a5); USE(a8); USE(a11); USE(a14);
            break;
        case 5:
            a6 = a7 + a8;
            a9 = a10 * a11;
            a12 = a13 - a14;
            a1 = a2 + a3;
            USE(a6); USE(a9); USE(a12); USE(a1);
            break;
        case 6:
            a7 = a8 + a9;
            a10 = a11 * a12;
            a13 = a14 - a1;
            a2 = a3 + a4;
            USE(a7); USE(a10); USE(a13); USE(a2);
            break;
        case 7:
            a8 = a9 + a10;
            a11 = a12 * a13;
            a14 = a1 - a2;
            a3 = a4 + a5;
            USE(a8); USE(a11); USE(a14); USE(a3);
            break;
    }
    
    /* Force all variables live at end */
    int total = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + 
                a9 + a10 + a11 + a12 + a13 + a14;
    asm volatile("" : : "r"(total) : "memory");
}

/* Simple main to make file compilable */
int main() {
    high_pressure_func(1, -1, 0);
    switch_pressure_func(3);
    return 0;
}
