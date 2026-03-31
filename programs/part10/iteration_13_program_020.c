/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
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
struct ComplexData {
    int a[8];
    long b[4];
    float c[4];
    double d[2];
    volatile int volatile_member;
};

/* Global arrays to force complex addressing */
static int global_array[256];
static struct ComplexData global_struct[16];

/* Test function with many parameters to force register pressure */
__attribute__((noinline, noipa))
long test_reloads(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10,
                  long p11, long p12, long p13, long p14,
                  float p15, float p16, double p17, double p18) {
    /* Declare many local variables to exhaust registers */
    register int r1 asm ("r12") = p1;
    register int r2 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14;
    float f1 = p15, f2 = p16;
    double d1 = p17, d2 = p18;
    
    /* Additional locals for more pressure */
    int v9 = barrier(v1), v10 = barrier(v2);
    int v11 = barrier(v3), v12 = barrier(v4);
    int v13 = barrier(v5), v14 = barrier(v6);
    int v15 = barrier(v7), v16 = barrier(v8);
    long l5 = barrier(l1), l6 = barrier(l2);
    long l7 = barrier(l3), l8 = barrier(l4);
    
    /* Volatile variables to prevent optimization */
    volatile int volatile_idx1 = barrier(v1) % 256;
    volatile int volatile_idx2 = barrier(v2) % 16;
    volatile int volatile_idx3 = barrier(v3) % 8;
    
    /* Complex addressing with SIB-like calculation (for x86) */
    /* array[base + index*scale] where scale=4 */
    int complex_addr_result = 0;
    for (int i = 0; i < 4; i++) {
        /* Force SIB addressing with all components */
        int base = volatile_idx1;
        int index = barrier(i);
        int scale = 4;
        /* This should require complex addressing mode */
        complex_addr_result += global_array[base + index * scale];
    }
    
    /* Nested structure access with volatile indices */
    struct ComplexData *ptr = &global_struct[volatile_idx2];
    ptr->a[volatile_idx3] = barrier(v9);
    ptr->volatile_member = barrier(v10);
    
    /* Mixed register class operations */
    /* Integer to float and back */
    int int_from_float = (int)f1;
    float float_from_int = (float)(v11 + v12);
    
    /* Use inline asm that clobbers many registers */
    /* This forces reloads around the asm block */
    __asm__ volatile (
        "/* Clobber many registers */\n\t"
        "mov %0, %0\n\t"
        "mov %1, %1\n\t"
        : "+r" (r1), "+r" (r2)
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* More arithmetic to create dependency chain */
    v1 = v1 + v2 * v3 - v4 / (v5 + 1);
    v2 = v2 + v3 * v4 - v5 / (v6 + 1);
    v3 = v3 + v4 * v5 - v6 / (v7 + 1);
    v4 = v4 + v5 * v6 - v7 / (v8 + 1);
    v5 = v5 + v6 * v7 - v8 / (v9 + 1);
    v6 = v6 + v7 * v8 - v9 / (v10 + 1);
    v7 = v7 + v8 * v9 - v10 / (v11 + 1);
    v8 = v8 + v9 * v10 - v11 / (v12 + 1);
    
    l1 = l1 + l2 * l3 - l4 / (l5 + 1);
    l2 = l2 + l3 * l4 - l5 / (l6 + 1);
    l3 = l3 + l4 * l5 - l6 / (l7 + 1);
    l4 = l4 + l5 * l6 - l7 / (l8 + 1);
    
    /* Atomic operations with complex addressing */
    int *atomic_ptr = &ptr->a[barrier(v13) % 8];
    int old_val = __atomic_load_n(atomic_ptr, __ATOMIC_RELAXED);
    __atomic_store_n(atomic_ptr, old_val + complex_addr_result, __ATOMIC_RELAXED);
    
    /* Another inline asm with memory constraint */
    /* This should force memory reloads */
    int mem_var = barrier(v14);
    __asm__ volatile (
        "add %[mem], %[val]\n\t"
        : [val] "+r" (v15)
        : [mem] "m" (mem_var)
        : "cc"
    );
    
    /* Type punning through union to force moves between register classes */
    union {
        float f;
        int i;
    } pun;
    pun.f = f1 + f2;
    v16 = pun.i + v15;
    
    /* Use all variables in final computation */
    long result = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                  v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 +
                  l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 +
                  (long)int_from_float + (long)float_from_int +
                  (long)complex_addr_result + (long)old_val;
    
    return barrier(result);
}

int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_array[i] = barrier(i);
    }
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            global_struct[i].a[j] = barrier(i * j);
        }
        global_struct[i].volatile_member = barrier(i);
    }
    
    /* Create many live variables */
    int a1 = barrier(argc), a2 = barrier(argc + 1);
    int a3 = barrier(argc + 2), a4 = barrier(argc + 3);
    int a5 = barrier(argc + 4), a6 = barrier(argc + 5);
    int a7 = barrier(argc + 6), a8 = barrier(argc + 7);
    int a9 = barrier(argc + 8), a10 = barrier(argc + 9);
    long b1 = barrier(argc + 10), b2 = barrier(argc + 11);
    long b3 = barrier(argc + 12), b4 = barrier(argc + 13);
    float c1 = barrier(argc + 14) * 1.0f;
    float c2 = barrier(argc + 15) * 1.0f;
    double d1 = barrier(argc + 16) * 1.0;
    double d2 = barrier(argc + 17) * 1.0;
    
    /* Additional variables for more pressure */
    int x1 = a1, x2 = a2, x3 = a3, x4 = a4;
    int x5 = a5, x6 = a6, x7 = a7, x8 = a8;
    int x9 = a9, x10 = a10;
    
    /* Call test function multiple times with different args */
    long sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += test_reloads(
            barrier(a1 + i), barrier(a2 + i), barrier(a3 + i),
            barrier(a4 + i), barrier(a5 + i), barrier(a6 + i),
            barrier(a7 + i), barrier(a8 + i), barrier(a9 + i),
            barrier(a10 + i), barrier(b1 + i), barrier(b2 + i),
            barrier(b3 + i), barrier(b4 + i),
            c1 + i, c2 + i, d1 + i, d2 + i
        );
        
        /* Modify variables to prevent optimization */
        x1 = barrier(x1 + x2);
        x2 = barrier(x2 + x3);
        x3 = barrier(x3 + x4);
        x4 = barrier(x4 + x5);
        x5 = barrier(x5 + x6);
        x6 = barrier(x6 + x7);
        x7 = barrier(x7 + x8);
        x8 = barrier(x8 + x9);
        x9 = barrier(x9 + x10);
        x10 = barrier(x10 + x1);
    }
    
    /* Use all variables to prevent dead code elimination */
    int final_check = barrier(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                             x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10);
    
    printf("Result: %ld (check: %d)\n", sum, final_check);
    return (sum > 0) ? 0 : 1;
}
