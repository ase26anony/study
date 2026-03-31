/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
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
static volatile int md_array[8][8][8];

/* Force register pressure with many live variables */
__attribute__((noinline, optimize("O0")))
int test_function(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10,
                  long p11, long p12, long p13, long p14, long p15,
                  float p16, double p17, int p18, int p19, int p20) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = p1 + 1;
    register int r1 asm ("r13") = p2 + 2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10, v9 = p18, v10 = p19;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14, l5 = p15;
    float f1 = p16, f2 = p16 * 2.0f;
    double d1 = p17, d2 = p17 * 3.0;
    volatile int vi1 = p20;
    volatile long vl1 = p11;
    
    /* Complex addressing with SIB-like computation */
    int *volatile ptr1 = (int*)&md_array[0][0][0];
    volatile int idx1 = barrier(p1) % 8;
    volatile int idx2 = barrier(p2) % 8;
    volatile int idx3 = barrier(p3) % 8;
    
    /* Force multiple reloads with complex addressing */
    int addr1 = idx1 * 64 + idx2 * 8 + idx3;  /* Scale-Index-Base computation */
    int val1 = ptr1[addr1];  /* Complex memory access */
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[v1], %[v2]\n\t"
        "add %[v3], %[v4]\n\t"
        : [v1] "+r" (v1), [v2] "+r" (v2), [v3] "+r" (v3)
        : [v4] "r" (v4)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* Mixed register class operations */
    int int_from_float = *(int*)&f1;  /* Type punning float->int */
    float float_from_int = *(float*)&v1;  /* Type punning int->float */
    
    /* Atomic operations with memory ordering */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(0);
    __atomic_store_n(&atomic_var, v1 + v2, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure */
    struct nested ns1, ns2;
    ns1.a[0] = v1; ns1.a[1] = v2; ns1.a[2] = v3;
    ns1.b[0] = l1; ns1.b[1] = l2;
    ns1.c = f1; ns1.d = d1;
    ns1.next = &ns2;
    
    ns2.a[0] = v4; ns2.a[1] = v5; ns2.a[2] = v6;
    ns2.b[0] = l3; ns2.b[1] = l4;
    ns2.c = f2; ns2.d = d2;
    ns2.next = &ns1;
    
    /* Complex structure access with variable index */
    volatile int sidx = barrier(v1) % 3;
    int sval = ns1.next->next->a[sidx];  /* Nested access with variable index */
    
    /* Force secondary reloads with register variables in complex constraints */
    int out1, out2;
    __asm__ volatile (
        "# Force secondary reload\n"
        "mov %[out1], %[r0]\n\t"
        "add %[out1], %[val1]\n\t"
        "mov %[out2], %[r1]\n\t"
        "imul %[out2], %[sval]\n\t"
        : [out1] "=r" (out1), [out2] "=r" (out2)
        : [r0] "r" (r0), [r1] "r" (r1), 
          [val1] "rm" (val1),  /* Memory or register constraint */
          [sval] "rm" (sval)   /* Forces reload decisions */
        : "cc"
    );
    
    /* Long dependency chain using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += l1 + l2 + l3 + l4 + l5;
    result += (int)f1 + (int)f2;
    result += (int)d1 + (int)d2;
    result += vi1 + (int)vl1;
    result += val1 + atomic_val + sval + out1 + out2;
    result += int_from_float + (int)float_from_int;
    
    /* More memory operations with complex addresses */
    for (int i = 0; i < 4; i++) {
        volatile int *volatile ptr2 = &md_array[i][i][i];
        *ptr2 = result + i;
        result += *ptr2;
    }
    
    return barrier(result);
}

int main(int argc, char **argv) {
    /* Initialize with non-constant values */
    int base = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Create many distinct variables */
    int v1 = base + 1, v2 = base + 2, v3 = base + 3, v4 = base + 4, v5 = base + 5;
    int v6 = base + 6, v7 = base + 7, v8 = base + 8, v9 = base + 9, v10 = base + 10;
    long l1 = base + 11, l2 = base + 12, l3 = base + 13, l4 = base + 14, l5 = base + 15;
    float f1 = base * 1.5f;
    double d1 = base * 2.5;
    int v11 = base + 16, v12 = base + 17, v13 = base + 18;
    
    /* Initialize array with volatile to prevent optimization */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                md_array[i][j][k] = i * 64 + j * 8 + k + base;
            }
        }
    }
    
    /* Call test function with many arguments */
    int result = test_function(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
                               l1, l2, l3, l4, l5, f1, d1, v11, v12, v13);
    
    printf("Result: %d\n", result);
    return 0;
}
