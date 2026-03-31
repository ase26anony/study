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

/* Complex structure with nested arrays */
struct nested {
    int a[3][4];
    long b[2][5];
    volatile int sync;
};

/* Global volatile to force memory operations */
volatile int global_seed = 42;

/* Test function with many registers and complex addressing */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  struct nested* np, int idx1, int idx2) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3;
    volatile int* volatile_ptr;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + global_seed);
    v2 = barrier(a2 * v1);
    v3 = barrier(a3 ^ v2);
    v4 = barrier(a4 + v3);
    v5 = barrier(a5 - v4);
    v6 = barrier(a6 | v5);
    v7 = barrier(a7 & v6);
    v8 = barrier(a8 * v7);
    v9 = barrier(a9 + v8);
    v10 = barrier(a10 ^ v9);
    
    /* More variables for register pressure */
    v11 = barrier(v1 * 3);
    v12 = barrier(v2 + 7);
    v13 = barrier(v3 - 11);
    v14 = barrier(v4 ^ 13);
    v15 = barrier(v5 | 17);
    v16 = barrier(v6 & 19);
    v17 = barrier(v7 * 23);
    v18 = barrier(v8 + 29);
    v19 = barrier(v9 - 31);
    v20 = barrier(v10 ^ 37);
    
    /* Long variables */
    l1 = (long)v1 * v2;
    l2 = (long)v3 * v4;
    l3 = (long)v5 * v6;
    l4 = (long)v7 * v8;
    l5 = (long)v9 * v10;
    
    /* Floating point to engage different register classes */
    f1 = (float)v11 / 2.0f;
    f2 = (float)v12 * 3.14f;
    f3 = (float)v13 + f1 - f2;
    
    /* Complex addressing with SIB-like calculation */
    /* This often requires secondary reloads on x86 */
    volatile_ptr = &np->sync;
    
    /* Inline asm that clobbers many registers */
    /* Forces reloads around the asm block */
    __asm__ volatile (
        "# Complex addressing with forced reloads\n"
        "movl %[idx1], %%eax\n"
        "movl %[idx2], %%ebx\n"
        "leal (%%eax, %%ebx, 2), %%ecx\n"
        "movl %[val], %%edx\n"
        : 
        : [idx1] "rm" (idx1), 
          [idx2] "rm" (idx2),
          [val] "rm" (v1)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Access multi-dimensional array with variable indices */
    /* Compiler may need secondary reload for addressing */
    int array_val = np->a[idx1 % 3][idx2 % 4];
    
    /* More inline asm with explicit register constraints */
    /* Forces specific register allocation and potential spills */
    int temp1, temp2;
    __asm__ volatile (
        "# Force register constraints and reloads\n"
        "movl %2, %0\n"
        "addl %3, %0\n"
        "movl %0, %1\n"
        : "=&r" (temp1), "=m" (np->b[0][idx1 % 5])
        : "r" (array_val), "r" (v2), "m" (np->b[0][idx1 % 5])
        : "cc"
    );
    
    /* Atomic operation with memory constraint */
    /* May require special reload handling */
    int old = __atomic_exchange_n(&np->sync, v3, __ATOMIC_RELAXED);
    
    /* Use register variables in complex expressions */
    r0 = r0 + r1 * 2;
    __asm__ volatile ("" : "+r" (r0));
    
    /* Mixed-type operations forcing moves between register classes */
    int int_from_float = (int)f3;
    float float_from_int = (float)(v4 ^ int_from_float);
    
    /* Another complex addressing mode */
    long* addr = &np->b[1][idx2 % 5];
    *addr = l1 + l2 + (long)temp1;
    
    /* Final computation using all variables */
    long result = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  l1 + l2 + l3 + l4 + l5 +
                  (long)int_from_float + (long)float_from_int +
                  array_val + old + r0 + temp1;
    
    /* Memory barrier to prevent reordering */
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    
    return result;
}

int main(int argc, char** argv) {
    /* Initialize many variables to prevent constant propagation */
    int vars[30];
    for (int i = 0; i < 30; i++) {
        vars[i] = barrier(argc + i * 3);
    }
    
    /* Initialize nested structure */
    struct nested n;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            n.a[i][j] = barrier(i * 10 + j);
        }
    }
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            n.b[i][j] = barrier(i * 20 + j * 3);
        }
    }
    n.sync = 0;
    
    /* Call test function with many arguments */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        &n, vars[10] % 3, vars[11] % 4
    );
    
    /* Additional arithmetic to use remaining variables */
    for (int i = 12; i < 30; i++) {
        result += barrier(vars[i] * (i - 10));
    }
    
    /* Print result to ensure side effects are observable */
    printf("Result: %ld\n", result);
    
    return (int)(result % 256);
}
