/* reload_stress.c - Stress GCC's reload pass to cover rld initialization block */
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

/* Complex structure with mixed types */
struct nested {
    int a[3];
    long b[2];
    float c;
    double d;
    struct nested *next;
};

/* Multi-dimensional array */
static volatile int md_array[4][8][16];

/* Global to force memory operations */
volatile int global_index = 0;

/* Test function with many parameters to force register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
long test_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    long b1, long b2, long b3, long b4, long b5,
    float f1, float f2, double d1, double d2
) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f3, f4, f5;
    double d3, d4;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + 1);
    v2 = barrier(a2 + v1);
    v3 = barrier(a3 + v2);
    v4 = barrier(a4 + v3);
    v5 = barrier(a5 + v4);
    v6 = barrier(a6 + v5);
    v7 = barrier(a7 + v6);
    v8 = barrier(a8 + v7);
    v9 = barrier(a9 + v8);
    v10 = barrier(a10 + v9);
    
    /* Use register variables in complex expressions */
    r0 = (r0 * r1) + (v1 << 2);
    r1 = (r1 / (r0 ? r0 : 1)) | v2;
    
    /* Inline asm that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "add %0, %1, %2\n"
        "sub %1, %2, %3\n"
        : "+r" (r0), "+r" (r1)
        : "r" (v3), "r" (v4)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More arithmetic creating long dependency chain */
    v11 = v10 * r0;
    v12 = v11 + r1;
    v13 = v12 - v1;
    v14 = v13 ^ v2;
    v15 = v14 | v3;
    v16 = v15 & v4;
    v17 = v16 << v5;
    v18 = v17 >> (v6 & 31);
    v19 = v18 + v7;
    v20 = v19 * v8;
    
    /* Long operations */
    l1 = b1 + v20;
    l2 = b2 * l1;
    l3 = b3 + l2;
    l4 = b4 ^ l3;
    l5 = b5 | l4;
    l6 = l5 << (v9 & 63);
    l7 = l6 >> (v10 & 63);
    l8 = l7 + v11;
    l9 = l8 * v12;
    l10 = l9 - v13;
    
    /* Floating point operations to use different register classes */
    f3 = f1 * f2 + (float)v14;
    f4 = f3 / (f2 ? f2 : 1.0f);
    f5 = f4 - (float)v15;
    
    d3 = d1 * d2 + (double)l1;
    d4 = d3 / (d2 ? d2 : 1.0);
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } pun;
    pun.f = f5;
    v16 = pun.i + v16;
    
    /* Complex addressing modes - SIB addressing on x86 */
    volatile int* volatile ptr = (volatile int*)md_array;
    int idx1 = barrier(v17) & 0x3;      /* 0-3 */
    int idx2 = barrier(v18) & 0x7;      /* 0-7 */
    int idx3 = barrier(v19) & 0xF;      /* 0-15 */
    
    /* Access with complex addressing that may need secondary reload */
    int mem_val = md_array[idx1][idx2][idx3];
    
    /* More complex: array[index*scale + base] */
    int* base = (int*)md_array;
    int index = barrier(v20) & 0x7F;
    int scale = 4;
    int complex_addr_val = base[index * scale + idx1];
    
    /* Use atomic operations to force specific reload patterns */
    __atomic_store_n(&md_array[0][0][0], mem_val + complex_addr_val, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&md_array[idx1][idx2][idx3], __ATOMIC_RELAXED);
    
    /* Another inline asm with memory constraint */
    __asm__ volatile (
        "# Memory constraint with complex address\n"
        "ldr %0, [%1, %2, lsl #2]\n"  /* ARM-style, will be adapted by GCC */
        : "=r" (v17)
        : "r" (base), "r" (index)
        : "memory"
    );
    
    /* Structure access with pointer arithmetic */
    struct nested nested_array[8];
    struct nested* nptr = &nested_array[idx2];
    nptr->a[idx3 & 0x3] = atomic_val;
    nptr->b[idx1 & 0x1] = l10;
    nptr->c = f5;
    nptr->d = d4;
    
    /* Access through pointer with offset */
    int struct_val = nptr->a[(idx1 + idx2) & 0x3];
    
    /* Final computation using all variables */
    long result = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                  l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
                  (long)f3 + (long)f4 + (long)f5 +
                  (long)d3 + (long)d4 +
                  mem_val + complex_addr_val + atomic_val + struct_val +
                  r0 + r1;
    
    return barrier(result);
}

int main(int argc, char *argv[]) {
    /* Initialize multi-dimensional array */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                md_array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Initialize with pseudo-random values based on argc */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    int a1 = barrier(base + 1);
    int a2 = barrier(base + 2);
    int a3 = barrier(base + 3);
    int a4 = barrier(base + 4);
    int a5 = barrier(base + 5);
    int a6 = barrier(base + 6);
    int a7 = barrier(base + 7);
    int a8 = barrier(base + 8);
    int a9 = barrier(base + 9);
    int a10 = barrier(base + 10);
    
    long b1 = barrier(base + 100);
    long b2 = barrier(base + 200);
    long b3 = barrier(base + 300);
    long b4 = barrier(base + 400);
    long b5 = barrier(base + 500);
    
    float f1 = (float)barrier(base + 1000) / 100.0f;
    float f2 = (float)barrier(base + 2000) / 100.0f;
    double d1 = (double)barrier(base + 3000) / 100.0;
    double d2 = (double)barrier(base + 4000) / 100.0;
    
    /* Call test function multiple times with different args */
    long result1 = test_function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                                b1, b2, b3, b4, b5, f1, f2, d1, d2);
    
    long result2 = test_function(a10, a9, a8, a7, a6, a5, a4, a3, a2, a1,
                                b5, b4, b3, b2, b1, f2, f1, d2, d1);
    
    long final_result = result1 + result2;
    
    /* Use the result so it can't be optimized away */
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
