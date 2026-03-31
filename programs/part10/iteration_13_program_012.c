/* reload_test.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
    __asm__ volatile ("" : "+r" (x) : : "memory");
    return x;
}

/* Complex structure with mixed types */
struct ComplexStruct {
    int a;
    long b;
    float c;
    double d;
    int arr[4];
    struct ComplexStruct *next;
};

/* Volatile global to force memory operations */
volatile int volatile_global = 42;
volatile long volatile_long = 123456789L;

/* Multi-dimensional array with volatile indices */
static int multi_array[8][8][8];

/* Function with many arguments to increase register pressure */
__attribute__((noinline, optimize("O0")))
long test_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    long l1, long l2, long l3, long l4, long l5,
    float f1, float f2, double d1, double d2
) {
    /* Declare many local variables to exhaust registers */
    register int reg_var1 asm ("r12") = a1;
    register int reg_var2 asm ("r13") = a2;
    int local1, local2, local3, local4, local5;
    int local6, local7, local8, local9, local10;
    int local11, local12, local13, local14, local15;
    long llocal1, llocal2, llocal3, llocal4, llocal5;
    float flocal1, flocal2, flocal3;
    double dlocal1, dlocal2;
    
    /* Complex addressing with SIB-like computation */
    volatile int idx1 = barrier(a1) % 8;
    volatile int idx2 = barrier(a2) % 8;
    volatile int idx3 = barrier(a3) % 8;
    
    /* Force multiple reloads with complex addressing */
    local1 = multi_array[idx1][idx2][idx3];
    local2 = multi_array[idx2][idx3][idx1];
    local3 = multi_array[idx3][idx1][idx2];
    
    /* Create long dependency chain */
    local1 = barrier(local1 + a1);
    local2 = barrier(local2 + a2);
    local3 = barrier(local3 + a3);
    local4 = barrier(local1 * local2 - local3);
    local5 = barrier(local4 + a4);
    local6 = barrier(local5 * a5);
    local7 = barrier(local6 - a6);
    local8 = barrier(local7 + a7);
    local9 = barrier(local8 * a8);
    local10 = barrier(local9 - a9);
    
    /* Inline assembly that clobbers many registers */
    /* This forces the compiler to spill/reload around it */
    __asm__ volatile (
        "/* Begin clobbering block */\n\t"
        "movl $0x12345678, %%eax\n\t"
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        "/* End clobbering block */"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Continue dependency chain after asm */
    local11 = barrier(local10 + a10);
    local12 = barrier(local11 * reg_var1);
    local13 = barrier(local12 - reg_var2);
    
    /* Mixed integer/long operations */
    llocal1 = l1 + local1;
    llocal2 = l2 * local2;
    llocal3 = l3 - local3;
    llocal4 = l4 + local4;
    llocal5 = l5 * local5;
    
    /* Force spills with many live variables */
    llocal1 = barrier(llocal1 + 1);
    llocal2 = barrier(llocal2 + 2);
    llocal3 = barrier(llocal3 + 3);
    llocal4 = barrier(llocal4 + 4);
    llocal5 = barrier(llocal5 + 5);
    
    /* Floating point operations to engage different register classes */
    flocal1 = f1 + (float)local1;
    flocal2 = f2 * (float)local2;
    flocal3 = flocal1 - flocal2;
    
    dlocal1 = d1 + (double)llocal1;
    dlocal2 = d2 * (double)llocal2;
    
    /* Type punning between float and int */
    union {
        float f;
        int i;
    } punner;
    
    punner.f = flocal3;
    local14 = barrier(punner.i + local13);
    
    /* Atomic operations that may need special reload handling */
    int atomic_val = __atomic_load_n(&volatile_global, __ATOMIC_RELAXED);
    local15 = barrier(local14 + atomic_val);
    
    __atomic_store_n(&volatile_global, local15, __ATOMIC_RELAXED);
    
    /* Complex addressing with structure */
    struct ComplexStruct cs;
    cs.a = local1;
    cs.b = llocal1;
    cs.c = flocal1;
    cs.d = dlocal1;
    cs.arr[0] = local2;
    cs.arr[1] = local3;
    cs.arr[2] = local4;
    cs.arr[3] = local5;
    
    /* Access structure with variable index */
    int sidx = barrier(local6) % 4;
    local15 = cs.arr[sidx] + cs.arr[sidx ^ 1];
    
    /* Another inline asm with complex constraints */
    /* This should trigger secondary reloads */
    int input_for_asm = barrier(local15);
    int output_from_asm;
    
    __asm__ volatile (
        "movl %[input], %%eax\n\t"
        "addl $0x100, %%eax\n\t"
        "movl %%eax, %[output]"
        : [output] "=r" (output_from_asm)
        : [input] "mr" (input_for_asm)  /* "mr" constraint may need reload */
        : "eax", "memory"
    );
    
    /* More register pressure */
    local1 = barrier(local1 + output_from_asm);
    local2 = barrier(local2 - output_from_asm);
    local3 = barrier(local3 * output_from_asm);
    
    /* Final computation using all variables */
    long result = (long)local1 + (long)local2 + (long)local3 + 
                  (long)local4 + (long)local5 + (long)local6 +
                  (long)local7 + (long)local8 + (long)local9 +
                  (long)local10 + (long)local11 + (long)local12 +
                  (long)local13 + (long)local14 + (long)local15 +
                  llocal1 + llocal2 + llocal3 + llocal4 + llocal5 +
                  (long)flocal1 + (long)flocal2 + (long)flocal3 +
                  (long)dlocal1 + (long)dlocal2;
    
    return barrier(result);
}

/* Initialize multi-dimensional array */
void init_array(void) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_array[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Initialize with somewhat random values */
    init_array();
    
    /* Create many live variables */
    int v1 = barrier(argc + 1);
    int v2 = barrier(argc + 2);
    int v3 = barrier(argc + 3);
    int v4 = barrier(argc + 4);
    int v5 = barrier(argc + 5);
    int v6 = barrier(argc + 6);
    int v7 = barrier(argc + 7);
    int v8 = barrier(argc + 8);
    int v9 = barrier(argc + 9);
    int v10 = barrier(argc + 10);
    int v11 = barrier(argc + 11);
    int v12 = barrier(argc + 12);
    int v13 = barrier(argc + 13);
    int v14 = barrier(argc + 14);
    int v15 = barrier(argc + 15);
    int v16 = barrier(argc + 16);
    int v17 = barrier(argc + 17);
    int v18 = barrier(argc + 18);
    int v19 = barrier(argc + 19);
    int v20 = barrier(argc + 20);
    
    long l1 = barrier(v1 * 100L);
    long l2 = barrier(v2 * 200L);
    long l3 = barrier(v3 * 300L);
    long l4 = barrier(v4 * 400L);
    long l5 = barrier(v5 * 500L);
    
    float f1 = (float)barrier(v6) / 10.0f;
    float f2 = (float)barrier(v7) / 20.0f;
    
    double d1 = (double)barrier(v8) / 30.0;
    double d2 = (double)barrier(v9) / 40.0;
    
    /* Call test function multiple times with different args */
    long result1 = test_function(
        v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
        l1, l2, l3, l4, l5,
        f1, f2, d1, d2
    );
    
    long result2 = test_function(
        v11, v12, v13, v14, v15, v16, v17, v18, v19, v20,
        l5, l4, l3, l2, l1,
        f2, f1, d2, d1
    );
    
    long final_result = barrier(result1 + result2);
    
    printf("Result: %ld\n", final_result);
    
    /* Use result to prevent dead code elimination */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return (final_result > 0) ? 0 : 1;
}
