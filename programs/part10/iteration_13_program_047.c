/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization of reload records
 * Compile with: gcc -O1 -fschedule-insns -fno-omit-frame-pointer -m32 -march=i686 -fno-pic -fno-optimize-sibling-calls -fno-crossjumping reload_stress_test.c -o reload_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure to force complex addressing */
struct nested {
    int a[3];
    struct {
        long b[2];
        volatile int c;
    } inner;
    float f;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Global arrays to force memory operands */
int global_arr[100];
long global_long_arr[50];
struct nested nested_arr[10];

/* Function with many arguments to force register pressure */
__attribute__((noinline))
long test_function(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   long l1, long l2, long l3, long l4, long l5,
                   float f1, float f2, float f3) {
    
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = a1;
    register int r2 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    long x1, x2, x3, x4, x5, x6, x7, x8;
    float y1, y2, y3, y4, y5;
    volatile int *volatile_ptr = &global_arr[0];
    
    /* Initialize with complex expressions to prevent constant propagation */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 ^ a6);
    v4 = barrier(a7 | a8);
    v5 = barrier(a9 & a10);
    v6 = barrier(r1 + r2);
    v7 = barrier(v1 * v2);
    v8 = barrier(v3 ^ v4);
    v9 = barrier(v5 | v6);
    v10 = barrier(v7 & v8);
    
    w1 = barrier(v9 + v10);
    w2 = barrier(w1 * a1);
    w3 = barrier(w2 ^ a2);
    w4 = barrier(w3 | a3);
    w5 = barrier(w4 & a4);
    w6 = barrier(w5 + a5);
    w7 = barrier(w6 * a6);
    w8 = barrier(w7 ^ a7);
    w9 = barrier(w8 | a8);
    w10 = barrier(w9 & a9);
    
    /* Force register pressure with long variables */
    x1 = l1 + l2;
    x2 = l3 * l4;
    x3 = x1 ^ x2;
    x4 = l5 + x3;
    x5 = x4 * l1;
    x6 = x5 ^ l2;
    x7 = x6 | l3;
    x8 = x7 & l4;
    
    /* Mixed floating point operations */
    y1 = f1 + f2;
    y2 = f3 * y1;
    y3 = y2 - f1;
    y4 = y3 / f2;
    y5 = y4 * f3;
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly to force reloads\n"
        "movl %[v1], %%eax\n"
        "movl %[v2], %%ebx\n"
        "addl %%ebx, %%eax\n"
        "movl %%eax, %[v3]\n"
        : [v3] "=m" (v3)
        : [v1] "mr" (v1), [v2] "mr" (v2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Complex addressing modes - SIB addressing on x86 */
    /* array[base + index*scale] where scale=4 for int array */
    int scale = 4;
    int *base = &global_arr[20];
    int index = barrier(idx1);
    
    /* This should force complex addressing reloads */
    volatile int complex_load = base[index * scale / 4];
    complex_load += base[(index + 1) * scale / 4];
    complex_load += base[(index + 2) * scale / 4];
    
    /* More complex: nested array with structure */
    int nidx = barrier(idx2) % 10;
    int sidx = barrier(idx3) % 3;
    
    /* Complex memory access pattern */
    nested_arr[nidx].a[sidx] = barrier(v1 + v2);
    nested_arr[nidx].inner.b[sidx % 2] = barrier(x1 + x2);
    nested_arr[nidx].inner.c = barrier(w1 + w2);
    nested_arr[nidx].f = y1 + y2;
    
    /* Force secondary reloads: memory to register with intermediate */
    volatile long *vlptr = &global_long_arr[0];
    register long temp_reg asm ("ebx");
    
    /* This sequence should trigger secondary reloads */
    __asm__ volatile (
        "# Force secondary reload with memory constraint\n"
        "movl (%[ptr]), %%eax\n"
        "movl 4(%[ptr]), %%edx\n"
        : "=A" (temp_reg)
        : [ptr] "r" (vlptr)
        : "memory"
    );
    
    /* Use the register in another operation */
    temp_reg = barrier(temp_reg + x1);
    
    /* Atomic operations to force specific reload patterns */
    int atomic_var = 0;
    __atomic_store_n(&atomic_var, v1 + v2, __ATOMIC_RELAXED);
    int atomic_load = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Type punning between int and float to force register class changes */
    union {
        int i;
        float f;
    } punner;
    
    punner.i = barrier(v3 + v4);
    y1 = punner.f + y3;  /* Force move between integer and FP registers */
    
    /* Another complex addressing mode with multiple components */
    struct nested *nptr = &nested_arr[0];
    int offset = barrier(idx1 + idx2);
    
    /* Access with complex computation */
    int complex_addr_result = nptr[offset % 5].a[(offset + 1) % 3] +
                              nptr[(offset + 2) % 5].inner.b[offset % 2];
    
    /* Final computation using all variables to prevent dead code elimination */
    long checksum = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                    w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 +
                    x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 +
                    (long)complex_load + atomic_load + complex_addr_result +
                    (long)temp_reg + (long)(y1 * 100) + (long)(y5 * 100);
    
    return barrier(checksum);
}

int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = barrier(i * 3 + 1);
    }
    for (int i = 0; i < 50; i++) {
        global_long_arr[i] = barrier(i * 5 + 2);
    }
    
    /* Initialize many variables to force register pressure */
    int a1 = barrier(argc + 1);
    int a2 = barrier(argc + 2);
    int a3 = barrier(argc + 3);
    int a4 = barrier(argc + 4);
    int a5 = barrier(argc + 5);
    int a6 = barrier(argc + 6);
    int a7 = barrier(argc + 7);
    int a8 = barrier(argc + 8);
    int a9 = barrier(argc + 9);
    int a10 = barrier(argc + 10);
    
    long l1 = barrier(argc + 100);
    long l2 = barrier(argc + 200);
    long l3 = barrier(argc + 300);
    long l4 = barrier(argc + 400);
    long l5 = barrier(argc + 500);
    
    float f1 = barrier(argc + 1000) / 100.0f;
    float f2 = barrier(argc + 2000) / 100.0f;
    float f3 = barrier(argc + 3000) / 100.0f;
    
    /* Call test function multiple times with different arguments */
    long result1 = test_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                l1, l2, l3, l4, l5, f1, f2, f3);
    
    /* Modify some values and call again */
    a1 = barrier(a1 + 1);
    a2 = barrier(a2 + 2);
    l1 = barrier(l1 + 1000);
    f1 = f1 + 1.5f;
    
    long result2 = test_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                l1, l2, l3, l4, l5, f1, f2, f3);
    
    /* Final result to prevent optimization */
    long final_result = barrier(result1 + result2);
    
    printf("Result: %ld\n", final_result);
    
    /* Use all variables to prevent dead store elimination */
    volatile int dummy = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    
    return (final_result > 0) ? 0 : 1;
}

/* Define the barrier function to prevent optimization */
int barrier(int x) {
    volatile int y = x;
    __asm__ volatile ("" : "+r" (y));
    return y;
}
