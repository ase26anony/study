/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register pressure with many live variables */
__attribute__((noinline))
int test_reloads(int seed) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    int v0 = seed * 3;
    int v1 = seed * 5;
    int v2 = seed * 7;
    int v3 = seed * 11;
    int v4 = seed * 13;
    int v5 = seed * 17;
    int v6 = seed * 19;
    int v7 = seed * 23;
    int v8 = seed * 29;
    int v9 = seed * 31;
    long l0 = seed * 37;
    long l1 = seed * 41;
    long l2 = seed * 43;
    long l3 = seed * 47;
    long l4 = seed * 53;
    float f0 = seed * 2.0f;
    float f1 = seed * 3.0f;
    double d0 = seed * 5.0;
    double d1 = seed * 7.0;
    
    /* Complex array with volatile index to prevent optimization */
    volatile int idx = seed % 16;
    int array[32][32];
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            array[i][j] = i * 100 + j;
        }
    }
    
    /* Complex addressing mode: array[idx*2 + v0][idx*3 + v1] */
    /* This may require secondary reloads on many architectures */
    int temp = array[idx*2 + v0][idx*3 + v1];
    
    /* Inline assembly that clobbers many registers */
    /* Forces reloads around the asm block */
    asm volatile (
        "mov %0, %0\n\t"
        "mov %1, %1\n\t"
        :
        : "r" (temp), "r" (v0)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Arithmetic creating long dependency chain */
    v0 = v0 + v1 + v2;
    v1 = v1 + v3 + v4;
    v2 = v2 + v5 + v6;
    v3 = v3 + v7 + v8;
    v4 = v4 + v9 + temp;
    v5 = v5 + v0 + v1;
    v6 = v6 + v2 + v3;
    v7 = v7 + v4 + v5;
    v8 = v8 + v6 + v7;
    v9 = v9 + v8 + v0;
    
    /* Mix integer and floating point operations */
    /* Forces moves between different register classes */
    union {
        int i;
        float f;
    } pun;
    pun.f = f0;
    v0 = v0 + pun.i;
    
    f0 = f0 + f1 + (float)v0;
    d0 = d0 + d1 + (double)v1;
    
    /* Atomic operations with complex addressing */
    volatile _Atomic int atomic_var = 0;
    int* volatile ptr = &v0;
    
    __atomic_store_n(ptr, v0 + v1 + v2, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More complex array access with SIB-like addressing */
    /* array[base + index*scale] where scale != 1,2,4,8 may need reload */
    int* base_ptr = &array[0][0];
    int index = idx + v0;
    int scale = 3; /* Non-power-of-two scale */
    int offset = index * scale + v1;
    
    /* This complex address may require secondary reload */
    int complex_load = *(base_ptr + offset);
    
    /* Another inline asm with memory constraint */
    /* Forces the reload pass to handle memory operands */
    asm volatile (
        "add %1, %0\n\t"
        : "+m" (complex_load)
        : "r" (v2)
        : "cc"
    );
    
    /* Use register variables in complex expressions */
    r0 = r0 + r1 + v0 + v1 + v2;
    r1 = r1 + v3 + v4 + v5;
    
    /* Long chain of arithmetic to keep all variables live */
    l0 = l0 + l1 + v0;
    l1 = l1 + l2 + v1;
    l2 = l2 + l3 + v2;
    l3 = l3 + l4 + v3;
    l4 = l4 + v4 + v5;
    
    /* Final computation using all variables */
    int result = r0 + r1 + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9
                 + (int)l0 + (int)l1 + (int)l2 + (int)l3 + (int)l4
                 + (int)f0 + (int)d0 + temp + complex_load + loaded;
    
    /* Prevent tail call optimization */
    asm volatile ("nop");
    
    return barrier(result);
}

/* Simple barrier implementation */
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55AA55AA;
}

int main(int argc, char** argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Call test function multiple times with different seeds */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += test_reloads(seed + i);
    }
    
    printf("Result: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
