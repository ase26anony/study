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

/* Complex structure with nested arrays */
struct nested {
    int data[4][8];
    volatile long offsets[3];
    float fp_values[6];
};

/* Global volatile to force memory operations */
volatile int global_index = 0;
volatile long global_base = 1000;

/* Test function with high register pressure */
__attribute__((noinline, optimize("no-omit-frame-pointer")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  struct nested* np, volatile int* vptr) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;  /* Explicit register variable */
    int v1 = a2, v2 = a3, v3 = a4, v4 = a5;
    int v5 = a6, v6 = a7, v7 = a8, v8 = a9, v9 = a10;
    long l1 = a1 * 2L, l2 = a2 * 3L, l3 = a3 * 4L, l4 = a4 * 5L;
    float f1 = a5 * 1.5f, f2 = a6 * 2.5f;
    double d1 = a7 * 3.14;
    
    /* Complex addressing with SIB-like calculation (for x86) */
    int idx = barrier(global_index);
    long base = barrier(global_base);
    int scale = 4;
    
    /* Force multiple reloads with volatile accesses */
    *vptr = barrier(*vptr + 1);
    
    /* Long dependency chain using all variables */
    v1 = barrier(v1 + r0);
    v2 = barrier(v2 + v1);
    v3 = barrier(v3 + v2);
    v4 = barrier(v4 + v3);
    v5 = barrier(v5 + v4);
    v6 = barrier(v6 + v5);
    v7 = barrier(v7 + v6);
    v8 = barrier(v8 + v7);
    v9 = barrier(v9 + v8);
    
    /* Mixed integer operations */
    l1 = barrier(l1 + v1);
    l2 = barrier(l2 + v2);
    l3 = barrier(l3 + v3);
    l4 = barrier(l4 + v4);
    
    /* Floating point operations (different register class) */
    f1 = barrier((int)f1) + v5 * 0.5f;
    f2 = barrier((int)f2) + v6 * 0.25f;
    d1 = barrier((int)d1) + v7 * 1.1;
    
    /* Inline assembly that clobbers many registers */
    /* This forces reloads around the asm */
    __asm__ volatile (
        "# Complex inline assembly\n\t"
        "mov %[val1], %[tmp]\n\t"
        "add %[val2], %[tmp]\n\t"
        : [tmp] "=r" (v1)
        : [val1] "m" (np->data[idx][0]),  /* Memory constraint with complex address */
          [val2] "r" (v2)                  /* Register constraint */
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    /* Access complex nested array with variable indices */
    /* This may require secondary reloads for addressing */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        volatile int* addr = &np->data[i][idx * scale + i];
        sum += barrier(*addr);
        
        /* More complex addressing with multiple components */
        long offset = np->offsets[i];
        int* addr2 = (int*)((char*)np->data + offset + i * sizeof(int) * 8);
        sum += barrier(*addr2);
    }
    
    /* Atomic operations that force specific reload patterns */
    _Atomic int atomic_val = 0;
    __atomic_store_n(&atomic_val, sum, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(&atomic_val, __ATOMIC_RELAXED);
    
    /* Type punning between int and float (different register classes) */
    union {
        int i;
        float f;
    } pun;
    pun.i = loaded;
    f1 = pun.f * 2.0f;
    pun.f = f1;
    v1 = pun.i;
    
    /* Another inline asm with complex constraints */
    /* Forces input/output reloads with potential secondary reloads */
    int result;
    __asm__ volatile (
        "# More complex assembly\n\t"
        "imul %[in1], %[in2], %[out]\n\t"
        : [out] "=r" (result)
        : [in1] "r" (v1),
          [in2] "m" (np->fp_values[idx])  /* Memory operand needing reload */
        : "cc", "memory"
    );
    
    /* Final computation using all variables */
    long final = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 l1 + l2 + l3 + l4 + (int)f1 + (int)f2 + (int)d1 +
                 sum + loaded + result;
    
    return barrier(final);
}

int main(int argc, char** argv) {
    /* Initialize many variables to prevent constant propagation */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = barrier(argc + i * 3);
    }
    
    /* Initialize complex structure */
    struct nested n;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            n.data[i][j] = barrier(i * 8 + j + argc);
        }
    }
    for (int i = 0; i < 3; i++) {
        n.offsets[i] = barrier(i * sizeof(int) * 4);
    }
    for (int i = 0; i < 6; i++) {
        n.fp_values[i] = barrier(i) * 1.5f;
    }
    
    volatile int volatile_var = 42;
    
    /* Call test function with many arguments */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        &n, &volatile_var
    );
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", result);
    
    /* Additional arithmetic with remaining variables */
    int checksum = 0;
    for (int i = 10; i < 20; i++) {
        checksum += barrier(vars[i]);
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
