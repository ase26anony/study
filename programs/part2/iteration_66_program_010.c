/* caller-save-test.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering block
 * (lines 905-913 in caller-save.cc)
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent inlining and optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Function that creates high register pressure with multiple calls */
NOINLINE USED int test_function1(int param) {
    /* Many local variables that must stay live across calls */
    int a = param + 1;
    int b = param + 2;
    int c = param + 3;
    int d = param + 4;
    int e = param + 5;
    int f = param + 6;
    int g = param + 7;
    int h = param + 8;
    int i = param + 9;
    int j = param + 10;
    int k = param + 11;
    int l = param + 12;
    int m = param + 13;
    int n = param + 14;
    int o = param + 15;
    int p = param + 16;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a * b - c / (d + 1) + e % (f + 1) +
           g * h - i / (j + 1) + k % (l + 1) + m * n - o / (p + 1);
}

/* Function with control flow variations */
NOINLINE USED int test_function2(int param, int flag) {
    /* Many variables with different lifetimes */
    register int r1 = param;  /* Hint for register allocation */
    int r2 = param * 2;
    int r3 = param * 3;
    int r4 = param * 4;
    int r5 = param * 5;
    int r6 = param * 6;
    int r7 = param * 7;
    int r8 = param * 8;
    
    /* Take addresses to force stack allocation for some */
    int *ptr1 = &r1;
    int *ptr2 = &r2;
    
    if (flag > 0) {
        /* Branch with call and variable usage */
        clobber_many_regs1();
        
        /* Complex computation keeping many vars live */
        r1 = r2 + r3;
        r4 = r5 * r6;
        r7 = r8 - param;
        
        clobber_many_regs2();
        
        /* More computations */
        r2 = r3 + r4;
        r5 = r6 * r7;
        r8 = r1 - param;
    } else {
        /* Alternative branch with different pattern */
        clobber_many_regs3();
        
        r3 = r4 + r5;
        r6 = r7 * r8;
        r1 = r2 - param;
        
        clobber_many_regs1();
        
        r4 = r5 + r6;
        r7 = r8 * r1;
        r2 = r3 - param;
    }
    
    /* Use all variables at the end */
    return *ptr1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Function with loop and calls inside */
NOINLINE USED int test_function3(int param, int iterations) {
    int accum = 0;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Local variables inside loop - high pressure */
        int v1 = param + idx;
        int v2 = param * idx;
        int v3 = param - idx;
        int v4 = param ^ idx;
        int v5 = param | idx;
        int v6 = param & idx;
        int v7 = param << (idx & 3);
        int v8 = param >> (idx & 3);
        
        /* Call that clobbers registers */
        if (idx % 2 == 0) {
            clobber_many_regs1();
        } else {
            clobber_many_regs2();
        }
        
        /* Use all variables */
        accum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        
        /* Another call */
        if (idx % 3 == 0) {
            clobber_many_regs3();
        }
        
        /* More variable usage */
        accum -= v1 * v2 - v3 / (v4 + 1) + v5 % (v6 + 1);
    }
    
    return accum;
}

/* Function with mixed caller/callee saved usage */
NOINLINE USED int test_function4(int p1, int p2, int p3, int p4, 
                                 int p5, int p6, int p7, int p8) {
    /* Parameters already in registers - some caller-saved, some callee-saved */
    int sum = p1 + p2 + p3 + p4;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use parameters again - may need reloading */
    sum += p5 + p6 + p7 + p8;
    
    /* Local variables competing for registers */
    int l1 = sum * 2;
    int l2 = sum * 3;
    int l3 = sum * 4;
    int l4 = sum * 5;
    int l5 = sum * 6;
    int l6 = sum * 7;
    int l7 = sum * 8;
    int l8 = sum * 9;
    
    /* Another call */
    clobber_many_regs2();
    
    /* Complex expression using all locals */
    return l1 + l2 - l3 * l4 / (l5 + 1) + l6 % (l7 + 1) - l8;
}

/* Inline assembly to simulate register-clobbering functions */
void clobber_many_regs1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
        "# clobber caller-saved regs\n"
        :
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15"
    );
}

void clobber_many_regs2(void) {
    /* Different clobber pattern */
    asm volatile (
        "# clobber different caller-saved regs\n"
        :
        :
        : "rax", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
    );
}

void clobber_many_regs3(void) {
    /* Yet another clobber pattern */
    asm volatile (
        "# clobber more caller-saved regs\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"
    );
}

int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int seed = 0;
    for (int i = 0; i < argc; i++) {
        for (char *p = argv[i]; *p; p++) {
            seed = seed * 31 + *p;
        }
    }
    
    int result = 0;
    
    /* Call test functions with different patterns */
    result += test_function1(seed);
    result += test_function2(seed, argc);
    result += test_function3(seed, argc > 1 ? atoi(argv[1]) % 10 + 1 : 5);
    result += test_function4(seed, seed+1, seed+2, seed+3, 
                            seed+4, seed+5, seed+6, seed+7);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
