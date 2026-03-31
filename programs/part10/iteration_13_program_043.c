/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register pressure with many live variables */
__attribute__((noinline))
int test_function(int a, int b, int c, int d, int e,
                  int f, int g, int h, int i, int j,
                  int k, int l, int m, int n, int o,
                  int p, int q, int r, int s, int t) {
    
    /* Declare many local variables to exhaust registers */
    volatile int v1 = a + 1;
    volatile int v2 = b + 2;
    volatile int v3 = c + 3;
    volatile int v4 = d + 4;
    volatile int v5 = e + 5;
    volatile int v6 = f + 6;
    volatile int v7 = g + 7;
    volatile int v8 = h + 8;
    volatile int v9 = i + 9;
    volatile int v10 = j + 10;
    volatile int v11 = k + 11;
    volatile int v12 = l + 12;
    volatile int v13 = m + 13;
    volatile int v14 = n + 14;
    volatile int v15 = o + 15;
    volatile int v16 = p + 16;
    volatile int v17 = q + 17;
    volatile int v18 = r + 18;
    volatile int v19 = s + 19;
    volatile int v20 = t + 20;
    
    /* Complex array with SIB addressing (x86) */
    int array[256][16];
    volatile int idx1 = barrier(a) % 256;
    volatile int idx2 = barrier(b) % 16;
    volatile int scale = 4; /* Force scale factor */
    
    /* Force secondary reloads with complex addressing */
    int *base = &array[0][0];
    int offset = idx1 * 16 + idx2;
    
    /* This may need secondary reload on some architectures */
    int temp1 = base[offset * scale / 4];
    
    /* Inline assembly that clobbers many registers */
    /* Generic version that works on multiple architectures */
    int result = 0;
    asm volatile (
        "/* Begin massive clobber */\n\t"
        "mov %[val1], %[res]\n\t"
        "add %[val2], %[res]\n\t"
        "add %[val3], %[res]\n\t"
        "add %[val4], %[res]\n\t"
        "add %[val5], %[res]"
        : [res] "=r" (result)
        : [val1] "r" (v1), [val2] "r" (v2), [val3] "r" (v3),
          [val4] "r" (v4), [val5] "r" (v5)
        : "memory", "cc"
    );
    
    /* More arithmetic to keep variables live */
    v6 = barrier(v6 + result);
    v7 = barrier(v7 + v6);
    v8 = barrier(v8 + v7);
    v9 = barrier(v9 + v8);
    v10 = barrier(v10 + v9);
    
    /* Mixed integer/float operations for different register classes */
    {
        union {
            int i;
            float f;
        } pun;
        pun.i = v11;
        float fval = pun.f * 1.5f;
        pun.f = fval;
        v11 = pun.i;
    }
    
    /* Atomic operations with memory ordering */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(0);
    __atomic_store(&atomic_var, &v12, __ATOMIC_RELAXED);
    __atomic_load(&atomic_var, &v13, __ATOMIC_RELAXED);
    
    /* Complex structure access */
    struct nested {
        int a[8];
        struct {
            int x;
            int y[4];
        } inner;
        int b[8];
    } nested_struct;
    
    volatile int struct_idx = barrier(c) % 8;
    nested_struct.a[struct_idx] = v14;
    nested_struct.inner.y[struct_idx % 4] = v15;
    
    /* Access with double register addressing simulation */
    int *ptr1 = &nested_struct.a[0];
    int *ptr2 = &nested_struct.inner.y[0];
    int temp2 = ptr1[struct_idx] + ptr2[struct_idx % 4];
    
    /* More inline asm with specific register constraints */
    register int reg_var asm ("r12") = v16;
    int output;
    asm volatile (
        "mov %[input], %[output]\n\t"
        "add $0x1234, %[output]"
        : [output] "=r" (output)
        : [input] "r" (reg_var)
        : /* empty clobber - reg_var is in r12 */
    );
    v16 = output;
    
    /* Force spills with many live values across function call */
    v17 = barrier(v17 + temp1);
    v18 = barrier(v18 + temp2);
    v19 = barrier(v19 + v17 + v18);
    v20 = barrier(v20 + v19);
    
    /* Final computation using all variables */
    int checksum = v1 + v2 + v3 + v4 + v5 +
                   v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 +
                   v16 + v17 + v18 + v19 + v20 +
                   result + temp1 + temp2;
    
    return checksum;
}

/* Barrier implementation to prevent optimization */
int barrier(int x) {
    /* Use inline asm to prevent optimization */
    asm volatile ("" : "+r" (x));
    return x;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = (argc > 1) ? atoi(argv[1]) + i : i * 3 + 1;
    }
    
    /* Call test function with many arguments */
    int result = test_function(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9],
        vals[10], vals[11], vals[12], vals[13], vals[14],
        vals[15], vals[16], vals[17], vals[18], vals[19]
    );
    
    printf("Result: %d\n", result);
    return 0;
}
