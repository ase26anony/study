/* test_mcf_coverage.c
 * Designed to trigger debug dumps in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based IRA algorithm */
#define IRA_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers - adjust if needed */
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))

/* Function to create extreme register pressure */
IRA_ATTR TARGET_ATTR
void extreme_register_pressure(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent optimization */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4));
    asm volatile("" : "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8));
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12));
    asm volatile("" : "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16));
    
    /* Complex control flow to create many live ranges across blocks */
    if (v1 > 0) {
        /* Use all variables in block 1 */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        v1 = v2 + v3 + v4 + v5;
        
        /* Clobber many registers to increase pressure */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", 
                     "r7", "r8", "r9", "r10", "r12", "memory");
    } else {
        /* Different computation in block 2, keeping variables live */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 ^ v14;
        v15 = v16 | v1;
        v2 = v3 + v4 + v5 + v6;
        
        /* Clobber different registers */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
    }
    
    /* Nested if to create more control flow edges */
    if (v2 > 100) {
        v7 = v8 + v9 + v10;
        v11 = v12 * v13 * v14;
        asm volatile("" : : : "r0", "r1", "r2", "r3", "memory");
    } else if (v2 > 50) {
        v15 = v16 + v1 + v2;
        v3 = v4 * v5 * v6;
        asm volatile("" : : : "r4", "r5", "r6", "r7", "memory");
    } else {
        v8 = v9 + v10 + v11;
        v12 = v13 * v14 * v15;
        asm volatile("" : : : "r8", "r9", "r10", "r12", "memory");
    }
    
    /* Switch statement for additional control flow complexity */
    switch (v3 & 0x7) {
        case 0:
            v4 = v5 + v6 + v7 + v8;
            break;
        case 1:
            v9 = v10 + v11 + v12 + v13;
            break;
        case 2:
            v14 = v15 + v16 + v1 + v2;
            break;
        case 3:
            v3 = v4 + v5 + v6 + v7;
            break;
        case 4:
            v8 = v9 + v10 + v11 + v12;
            break;
        case 5:
            v13 = v14 + v15 + v16 + v1;
            break;
        case 6:
            v2 = v3 + v4 + v5 + v6;
            break;
        default:
            v7 = v8 + v9 + v10 + v11;
            break;
    }
    
    /* Final computations keeping all variables live until the end */
    v16 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v15 = v9 + v10 + v11 + v12 + v13 + v14 + v16;
    
    /* Force all variables to be used in output */
    asm volatile("" 
                 : /* outputs */ 
                 : /* inputs */ 
                   "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                   "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                   "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                   "r"(v13), "r"(v14), "r"(v15), "r"(v16)
                 : "memory");
}

/* Alternative: x86 version with different register clobbering */
#ifdef __x86_64__
IRA_ATTR __attribute__((target("arch=x86-64")))
void x86_register_pressure(void) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18;
    
    asm volatile("" : "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4));
    asm volatile("" : "=r"(a5), "=r"(a6), "=r"(a7), "=r"(a8));
    asm volatile("" : "=r"(a9), "=r"(a10), "=r"(a11), "=r"(a12));
    asm volatile("" : "=r"(a13), "=r"(a14), "=r"(a15), "=r"(a16));
    asm volatile("" : "=r"(a17), "=r"(a18));
    
    /* Complex loop to create many overlapping live ranges */
    for (int i = 0; i < 10; i++) {
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 ^ a12;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                         "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
        } else {
            a13 = a14 + a15;
            a16 = a17 * a18;
            a2 = a3 - a4;
            a5 = a6 ^ a7;
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "memory");
        }
        
        /* Cross-block variable usage */
        a8 = a9 + a10 + a11 + a12 + a13;
        a14 = a15 + a16 + a17 + a18 + a1;
    }
    
    /* Force all to be used */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                   "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                   "r"(a11), "r"(a12), "r"(a13), "r"(a14), "r"(a15),
                   "r"(a16), "r"(a17), "r"(a18) : "memory");
}
#endif

/* Simple main to make file compilable */
int main(void) {
    extreme_register_pressure();
    #ifdef __x86_64__
    x86_register_pressure();
    #endif
    return 0;
}
