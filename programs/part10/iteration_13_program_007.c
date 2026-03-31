/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int v = x;
    return v + 1;
}

/* Complex structure for difficult addressing */
struct nested {
    int a[3];
    struct {
        long x;
        volatile int y;
    } inner;
    float f;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Inline assembly that clobbers many registers */
#define CLOBBER_ASM() __asm__ volatile ( \
    "# Clobber many registers\n" \
    "movl $0, %%eax\n" \
    "movl $0, %%ebx\n" \
    "movl $0, %%ecx\n" \
    "movl $0, %%edx\n" \
    "movl $0, %%esi\n" \
    "movl $0, %%edi\n" \
    : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory")

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
int test_function(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long l1, long l2, long l3, long l4) {
    
    /* Many local variables to exhaust registers */
    register int r1 asm ("r12") = a1;
    register int r2 asm ("r13") = a2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7;
    int v6 = a8, v7 = a9, v8 = a10;
    long lv1 = l1, lv2 = l2, lv3 = l3, lv4 = l4;
    int v9, v10, v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Complex array with SIB addressing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    /* Force register pressure with arithmetic chain */
    v9 = barrier(v1 + v2);
    v10 = barrier(v3 * v4);
    v11 = barrier(v5 ^ v6);
    v12 = barrier(v7 | v8);
    v13 = barrier(r1 + r2);
    v14 = barrier(lv1 + lv2);
    v15 = barrier(lv3 * lv4);
    
    /* Inline assembly with constraints causing reloads */
    int mem_val;
    __asm__ volatile (
        "movl %[addr], %[out]\n"
        : [out] "=r" (mem_val)
        : [addr] "m" (array[idx1 * 4 + idx2])  /* Complex SIB addressing */
        : "memory"
    );
    
    /* More arithmetic to keep values live */
    v16 = barrier(v9 + v10);
    v17 = barrier(v11 - v12);
    v18 = barrier(v13 ^ v14);
    v19 = barrier(v15 + mem_val);
    
    /* Clobber registers to force reloads */
    CLOBBER_ASM();
    
    /* Mixed integer/float operations */
    {
        union {
            int i;
            float f;
        } pun;
        pun.i = v16;
        float f1 = pun.f * 1.5f;
        pun.f = f1;
        v20 = pun.i;
    }
    
    /* Complex structure access with volatile */
    struct nested ns[5];
    for (int i = 0; i < 5; i++) {
        ns[i].inner.x = i * 100;
        ns[i].inner.y = i * 200;
        ns[i].f = i * 1.0f;
        for (int j = 0; j < 3; j++) {
            ns[i].a[j] = i * 10 + j;
        }
    }
    
    /* Difficult addressing: array of structures with variable indices */
    volatile int *volatile_ptr = &ns[idx1].a[idx2];
    int struct_val = *volatile_ptr;
    
    /* Atomic operations that need specific handling */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v17 + v18, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More arithmetic with all variables */
    int result = r1 + r2 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 +
                 v17 + v18 + v19 + v20 + mem_val + struct_val + atomic_val +
                 (int)lv1 + (int)lv2 + (int)lv3 + (int)lv4;
    
    /* Another clobber to force output reloads */
    CLOBBER_ASM();
    
    return barrier(result);
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int a1 = barrier(argc);
    int a2 = barrier(argc + 1);
    int a3 = barrier(argc + 2);
    int a4 = barrier(argc + 3);
    int a5 = barrier(argc + 4);
    int a6 = barrier(argc + 5);
    int a7 = barrier(argc + 6);
    int a8 = barrier(argc + 7);
    int a9 = barrier(argc + 8);
    int a10 = barrier(argc + 9);
    
    long l1 = barrier(argc) * 100L;
    long l2 = barrier(argc + 1) * 200L;
    long l3 = barrier(argc + 2) * 300L;
    long l4 = barrier(argc + 3) * 400L;
    
    /* Additional local variables in main */
    int b1 = a1 * 2, b2 = a2 * 3, b3 = a3 * 4, b4 = a4 * 5;
    int b5 = a5 * 6, b6 = a6 * 7, b7 = a7 * 8, b8 = a8 * 9;
    int b9 = a9 * 10, b10 = a10 * 11;
    
    /* Call test function multiple times with different args */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += test_function(a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
                            a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
                            l1 + i, l2 + i, l3 + i, l4 + i);
        
        /* Modify indices to change addressing patterns */
        idx1 = (idx1 + 1) % 4;
        idx2 = (idx2 + 2) % 4;
    }
    
    /* Use all local variables to prevent elimination */
    sum += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
    
    printf("Result: %d\n", sum);
    return 0;
}
