/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
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

/* Force register pressure with many live variables */
__attribute__((noinline))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1 + 1;
    register int r1 asm ("r13") = a2 + 2;
    volatile int v1 = a3;
    volatile int v2 = a4;
    volatile int v3 = a5;
    volatile int v4 = a6;
    volatile int v5 = a7;
    volatile int v6 = a8;
    volatile int v7 = a9;
    volatile int v8 = a10;
    
    int b1 = barrier(a1);
    int b2 = barrier(a2);
    int b3 = barrier(a3);
    int b4 = barrier(a4);
    int b5 = barrier(a5);
    int b6 = barrier(a6);
    int b7 = barrier(a7);
    int b8 = barrier(a8);
    int b9 = barrier(a9);
    int b10 = barrier(a10);
    
    /* Complex array access with SIB-like addressing */
    int array[256][8];
    volatile int idx1 = b1 % 256;
    volatile int idx2 = b2 % 8;
    
    /* Force memory addressing with complex computation */
    for (int i = 0; i < 10; i++) {
        /* Complex addressing: array[idx1 + i][idx2 + i*2] */
        array[idx1 + i][idx2 + i * 2] = 
            b1 + b2 * i + b3 * (i * i) + b4 * (i * i * i);
    }
    
    /* Mixed integer/float operations */
    float f1 = (float)b1 / (b2 + 1);
    float f2 = (float)b3 / (b4 + 1);
    int if1 = *(int*)&f1;  /* Type punning */
    int if2 = *(int*)&f2;
    
    /* Inline assembly that clobbers many registers */
    int result;
    __asm__ volatile (
        "mov %[val1], %[tmp1]\n\t"
        "mov %[val2], %[tmp2]\n\t"
        "add %[tmp1], %[tmp2], %[out]\n\t"
        : [out] "=r" (result)
        : [val1] "m" (array[idx1][idx2]),  /* Memory constraint */
          [val2] "r" (b5),                  /* Register constraint */
          [tmp1] "r" (r0),
          [tmp2] "r" (r1)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Atomic operations with complex addressing */
    _Atomic int atomic_var;
    __atomic_store_n(&atomic_var, result, __ATOMIC_RELAXED);
    
    /* More complex addressing with structure */
    struct nested {
        int x[4];
        struct {
            int y[2];
            int z[3];
        } inner;
    } s;
    
    volatile int idx = b6 % 4;
    s.x[idx] = b7;
    s.inner.y[idx % 2] = b8;
    s.inner.z[idx % 3] = b9;
    
    /* Access with double register addressing simulation */
    int *ptr1 = &s.x[0];
    int *ptr2 = &s.inner.y[0];
    int offset = b10 % 2;
    
    /* Force potential secondary reload */
    int final;
    __asm__ volatile (
        "ldr %[out], [%[base], %[idx], lsl #2]\n\t"
        : [out] "=r" (final)
        : [base] "r" (ptr1),
          [idx] "r" (offset)
        : "memory"
    );
    
    /* Use vector extensions if available */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {b1, b2, b3, b4};
    v4si vec2 = {b5, b6, b7, b8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract element - forces move between vector and scalar regs */
    int vec_elem;
    __asm__ volatile (
        "umov %w[out], %[vec].s[0]\n\t"
        : [out] "=r" (vec_elem)
        : [vec] "w" (vec3)
    );
    
    /* Long dependency chain */
    int chain = b1;
    chain = chain * b2 + b3;
    chain = chain / (b4 + 1) + b5;
    chain = chain ^ b6 ^ b7;
    chain = chain | b8 | b9;
    chain = chain & b10 & final;
    chain = chain + vec_elem + result;
    chain = chain + s.x[0] + s.inner.y[0] + s.inner.z[0];
    chain = chain + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    
    /* Access array with complex index computation */
    volatile int* volatile_ptr = array[idx1];
    for (int i = 0; i < 8; i++) {
        chain += volatile_ptr[i * 2] * (i + 1);
    }
    
    return chain + __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
}

/* Another function to create more reload contexts */
__attribute__((noinline))
static int secondary_reload_test(volatile int* mem, int idx) {
    /* Force memory-to-register with complex addressing */
    int val1, val2;
    
    /* Complex addressing mode that might need secondary reload */
    __asm__ volatile (
        "ldr %[v1], [%[addr], %[off], lsl #2]\n\t"
        "add %[v2], %[v1], #1\n\t"
        "str %[v2], [%[addr], %[off], lsl #2]\n\t"
        : [v1] "=&r" (val1),
          [v2] "=&r" (val2)
        : [addr] "r" (mem),
          [off] "r" (idx)
        : "memory"
    );
    
    /* Use register variable in output constraint */
    register int out_reg asm ("r10") = 0;
    __asm__ volatile (
        "mov %[out], %[in]\n\t"
        : [out] "=r" (out_reg)
        : [in] "m" (mem[idx]),
          "[out]" (out_reg)
    );
    
    return val1 + val2 + out_reg;
}

int main(int argc, char** argv) {
    /* Initialize many variables to prevent constant propagation */
    int vars[20];
    for (int i = 0; i < 20; i++) {
        vars[i] = barrier(argc + i * 3);
    }
    
    /* Create complex memory layout */
    volatile int* heap_array = (volatile int*)malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        heap_array[i] = barrier(i * 7);
    }
    
    /* Call test function with many arguments */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9]
    );
    
    /* Call secondary reload test */
    volatile int idx = vars[10] % 64;
    int sec_result = secondary_reload_test((int*)heap_array, idx);
    
    /* More arithmetic to keep variables live */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += vars[i] * (i + 1);
    }
    
    /* Complex addressing in main too */
    struct {
        int a[8];
        int b[8];
        int c[8];
    } data;
    
    volatile int idx2 = vars[11] % 8;
    data.a[idx2] = vars[12];
    data.b[idx2 * 2 % 8] = vars[13];
    data.c[(idx2 + 3) % 8] = vars[14];
    
    /* Force spilling around function call */
    int temp = vars[15];
    temp = barrier(temp);
    temp += data.a[0] + data.b[0] + data.c[0];
    
    /* Final computation */
    long final_result = result + sec_result + sum + temp;
    
    printf("Result: %ld\n", final_result);
    
    free((void*)heap_array);
    return (final_result > 0) ? 0 : 1;
}
