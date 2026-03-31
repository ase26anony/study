/* reload_stress_test.c
 * A test program designed to stress GCC's reload pass and trigger
 * initialization of reload records, particularly those requiring
 * secondary reloads.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Use inline asm to make this opaque to the optimizer */
    __asm__ volatile ("" : "+r"(x));
    return x;
}

/* Force register pressure with many live variables */
__attribute__((noinline))
static int test_function(int a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j,
                         int k, int l, int m, int n, int o,
                         int p, int q, int r, int s, int t) {
    /* Declare many local variables to exhaust registers */
    volatile int v1 = barrier(a);
    volatile int v2 = barrier(b);
    volatile int v3 = barrier(c);
    volatile int v4 = barrier(d);
    volatile int v5 = barrier(e);
    volatile int v6 = barrier(f);
    volatile int v7 = barrier(g);
    volatile int v8 = barrier(h);
    volatile int v9 = barrier(i);
    volatile int v10 = barrier(j);
    volatile int v11 = barrier(k);
    volatile int v12 = barrier(l);
    volatile int v13 = barrier(m);
    volatile int v14 = barrier(n);
    volatile int v15 = barrier(o);
    volatile int v16 = barrier(p);
    volatile int v17 = barrier(q);
    volatile int v18 = barrier(r);
    volatile int v19 = barrier(s);
    volatile int v20 = barrier(t);
    
    /* Complex arithmetic with long dependency chain */
    int sum = v1;
    sum = sum * 31 + v2;
    sum = sum * 31 + v3;
    sum = sum * 31 + v4;
    sum = sum * 31 + v5;
    sum = sum * 31 + v6;
    sum = sum * 31 + v7;
    sum = sum * 31 + v8;
    sum = sum * 31 + v9;
    sum = sum * 31 + v10;
    sum = sum * 31 + v11;
    sum = sum * 31 + v12;
    sum = sum * 31 + v13;
    sum = sum * 31 + v14;
    sum = sum * 31 + v15;
    sum = sum * 31 + v16;
    sum = sum * 31 + v17;
    sum = sum * 31 + v18;
    sum = sum * 31 + v19;
    sum = sum * 31 + v20;
    
    /* Complex multi-dimensional array access with volatile indices */
    volatile int idx1 = barrier(sum) % 8;
    volatile int idx2 = barrier(sum + 1) % 8;
    volatile int idx3 = barrier(sum + 2) % 8;
    
    int array[8][8][8];
    for (int i1 = 0; i1 < 8; i1++) {
        for (int i2 = 0; i2 < 8; i2++) {
            for (int i3 = 0; i3 < 8; i3++) {
                array[i1][i2][i3] = barrier(i1 * 64 + i2 * 8 + i3);
            }
        }
    }
    
    /* Complex addressing mode: array[idx1][idx2][idx3] */
    int array_val = array[idx1][idx2][idx3];
    sum = sum * 31 + array_val;
    
    /* Inline assembly that clobbers many registers */
    int asm_result;
    __asm__ volatile (
        /* Complex addressing mode in input */
        "mov %[array_val], %[temp]\n\t"
        "add %[sum], %[temp]\n\t"
        "mov %[temp], %[result]\n\t"
        : [result] "=r" (asm_result)
        : [sum] "r" (sum), [array_val] "m" (array[idx1][idx2][idx3])
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    sum = asm_result;
    
    /* Mixed data types to force moves between register classes */
    union {
        int i;
        float f;
    } pun;
    
    pun.i = sum;
    float fval = pun.f * 1.5f;
    pun.f = fval;
    sum = pun.i;
    
    /* Use explicit register variables to create conflicts */
    register int reg_var1 asm ("r12") = barrier(sum + 100);
    register int reg_var2 asm ("r11") = barrier(sum + 200);
    
    /* Complex expression using register variables */
    int complex_expr = (reg_var1 * reg_var2) + (reg_var1 >> 3) - (reg_var2 << 2);
    
    /* Another inline asm with memory constraint and complex address */
    int final_result;
    __asm__ volatile (
        "mov %[complex], %[final]\n\t"
        "add %[idx1], %[final]\n\t"
        "add %[idx2], %[final]\n\t"
        "add %[idx3], %[final]\n\t"
        : [final] "=r" (final_result)
        : [complex] "r" (complex_expr),
          [idx1] "m" (array[idx1][0][0]),  /* Complex memory address */
          [idx2] "m" (array[0][idx2][0]),  /* Another complex address */
          [idx3] "m" (array[0][0][idx3])   /* Yet another */
        : "memory"
    );
    
    /* Atomic operations to prevent optimization */
    volatile int atomic_var = 0;
    __atomic_store_n(&atomic_var, final_result, __ATOMIC_RELAXED);
    int loaded = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* More arithmetic to ensure all values are used */
    loaded = loaded * 31 + v1;
    loaded = loaded * 31 + v2;
    loaded = loaded * 31 + v3;
    loaded = loaded * 31 + v4;
    loaded = loaded * 31 + v5;
    loaded = loaded * 31 + v6;
    loaded = loaded * 31 + v7;
    loaded = loaded * 31 + v8;
    loaded = loaded * 31 + v9;
    loaded = loaded * 31 + v10;
    
    return loaded;
}

/* Secondary reload stress test */
__attribute__((noinline))
static int secondary_reload_test(void) {
    /* Create a volatile memory location */
    volatile int mem_buffer[256];
    for (int i = 0; i < 256; i++) {
        mem_buffer[i] = barrier(i * 3);
    }
    
    /* Use complex SIB addressing (for x86) */
    volatile int index = barrier(42) % 256;
    volatile int scale = 4;  /* sizeof(int) */
    volatile int base = barrier(100);
    
    /* This should force secondary reloads on some architectures */
    int result;
    __asm__ volatile (
        /* Try to use complex addressing mode */
        "mov (%[base], %[index], %[scale]), %[result]\n\t"
        : [result] "=r" (result)
        : [base] "r" (&mem_buffer[0]),
          [index] "r" (index),
          [scale] "i" (4)
        : "memory"
    );
    
    /* More complex: register variable with specific constraint */
    register int reg1 asm ("ebx") = result;
    register int reg2 asm ("ecx") = barrier(result + 1);
    
    /* Force spill and reload */
    int temp_array[10];
    for (int i = 0; i < 10; i++) {
        temp_array[i] = barrier(reg1 + i * reg2);
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += temp_array[i];
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vals[20];
    for (int i = 0; i < 20; i++) {
        vals[i] = barrier(argc + i * 3);
    }
    
    /* Call test function with many arguments */
    int result1 = test_function(
        vals[0], vals[1], vals[2], vals[3], vals[4],
        vals[5], vals[6], vals[7], vals[8], vals[9],
        vals[10], vals[11], vals[12], vals[13], vals[14],
        vals[15], vals[16], vals[17], vals[18], vals[19]
    );
    
    /* Call secondary reload test */
    int result2 = secondary_reload_test();
    
    /* Combine results to ensure all code is live */
    int final_result = barrier(result1) + barrier(result2 * 3);
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
