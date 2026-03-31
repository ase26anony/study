/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline asm to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10) {
    /* Declare many scalar variables to exhaust registers */
    volatile int v1 = a1 + 1;
    volatile int v2 = a2 + 2;
    volatile int v3 = a3 + 3;
    volatile int v4 = a4 + 4;
    volatile int v5 = a5 + 5;
    volatile int v6 = a6 + 6;
    volatile int v7 = a7 + 7;
    volatile int v8 = a8 + 8;
    volatile int v9 = a9 + 9;
    volatile int v10 = a10 + 10;
    
    /* More variables for additional pressure */
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex expressions to prevent elimination */
    v11 = barrier(v1 * v2);
    v12 = barrier(v3 ^ v4);
    v13 = barrier(v5 | v6);
    v14 = barrier(v7 & v8);
    v15 = barrier(v9 + v10);
    v16 = barrier(v11 - v12);
    v17 = barrier(v13 * v14);
    v18 = barrier(v15 ^ v16);
    v19 = barrier(v17 | v18);
    v20 = barrier(v19 + v1);
    
    /* Create long dependency chain */
    v21 = v11 + v12 + v13 + v14 + v15;
    v22 = v16 + v17 + v18 + v19 + v20;
    v23 = v21 * v22 - v1;
    v24 = v23 / (v2 + 1) + v3;
    v25 = v24 ^ v4 | v5;
    v26 = v25 & v6 + v7;
    v27 = v26 * v8 - v9;
    v28 = v27 + v10 * v11;
    v29 = v28 ^ v12 | v13;
    v30 = v29 & v14 + v15;
    
    /* Complex addressing mode: SIB addressing with all components */
    volatile int array[256][16];
    volatile int index1 = v1 % 256;
    volatile int index2 = v2 % 16;
    volatile int scale = sizeof(int);
    volatile int base = (intptr_t)array;
    
    /* Force complex memory addressing that may need secondary reload */
    int mem_val;
    __asm__ volatile (
        "movl (%[base], %[index1], %[scale]), %[val]\n\t"
        : [val] "=r" (mem_val)
        : [base] "r" (base),
          [index1] "r" (index1),
          [scale] "i" (4)
        : "memory"
    );
    
    /* Use the value in further computations */
    v30 += mem_val;
    
    /* Access multi-dimensional array with variable indices */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            /* Complex addressing: array[i][j] with i,j in registers */
            volatile int idx_i = (v1 + i) % 256;
            volatile int idx_j = (v2 + j) % 16;
            v30 += array[idx_i][idx_j];
        }
    }
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Clobber many registers\n\t"
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        "movl $0, %%esi\n\t"
        "movl $0, %%edi\n\t"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Mixed data types to engage different register classes */
    union {
        float f;
        int i;
    } pun;
    
    pun.i = v30;
    float fval = pun.f * 1.5f;
    pun.f = fval;
    v30 = pun.i;
    
    /* Atomic operations with complex addressing */
    volatile _Atomic int atomic_var = 0;
    int* volatile ptr = (int*)&atomic_var;
    
    /* Complex address calculation for atomic operation */
    int offset = v1 % 64;
    int* atomic_ptr = ptr + offset;
    
    __atomic_store_n(atomic_ptr, v30, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(atomic_ptr, __ATOMIC_RELAXED);
    
    /* Register variables with explicit registers */
    register int reg_var1 asm ("r12") = v1;
    register int reg_var2 asm ("r13") = v2;
    register int reg_var3 asm ("r14") = v3;
    
    /* Force these into complex expressions */
    reg_var1 = reg_var1 * reg_var2 + reg_var3;
    reg_var2 = reg_var1 ^ reg_var2 | reg_var3;
    reg_var3 = reg_var1 & reg_var2 - reg_var3;
    
    /* Inline asm using register variables with complex constraints */
    int result;
    __asm__ volatile (
        "imull %%ecx, %%edx\n\t"
        "addl %%ebx, %%edx\n\t"
        "movl %%edx, %[res]\n\t"
        : [res] "=m" (result)  /* Memory output constraint */
        : "b" (reg_var1),      /* ebx input */
          "c" (reg_var2),      /* ecx input */
          "d" (reg_var3)       /* edx input */
        : "memory"
    );
    
    /* More arithmetic to keep all variables live */
    v30 += result;
    v30 += v21 + v22 + v23 + v24 + v25;
    v30 += v26 + v27 + v28 + v29;
    
    /* Final barrier to prevent tail optimization */
    return barrier(v30);
}

/* Structure with nested arrays for complex addressing */
struct nested {
    int data[8][8];
    struct {
        int x[4];
        int y[4];
    } inner[2];
};

__attribute__((noinline, noipa))
static int test_complex_addressing(void) {
    volatile struct nested n;
    volatile int idx1 = 3, idx2 = 5, idx3 = 1, idx4 = 2;
    
    /* Very complex addressing: n.inner[idx1].x[idx2] + n.data[idx3][idx4] */
    int val1 = n.inner[idx1].x[idx2];
    int val2 = n.data[idx3][idx4];
    
    /* Force addressing computation to remain */
    __asm__ volatile (
        "# Complex addressing stress\n\t"
        "addl %%ebx, %%eax\n\t"
        : "+a" (val1)
        : "b" (val2)
        : "cc"
    );
    
    return val1;
}

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Create many distinct values */
    int vals[10];
    for (int i = 0; i < 10; i++) {
        vals[i] = base + i * 7;
    }
    
    /* Call test function with many arguments */
    long result = test_reloads(vals[0], vals[1], vals[2], vals[3], vals[4],
                               vals[5], vals[6], vals[7], vals[8], vals[9]);
    
    /* Additional test for complex addressing */
    result += test_complex_addressing();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    return (int)(result % 256);
}
