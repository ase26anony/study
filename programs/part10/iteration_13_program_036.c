/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

/* Prevent optimizations that would eliminate reloads */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Opaque function to prevent register reuse */
extern int barrier(int) __asm__("barrier");
int barrier(int x) { return x ^ 0x55AA55AA; }

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

/* Register variables to force specific register allocation */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");

/* Function to create register pressure and complex addressing */
NOINLINE static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                                  int a6, int a7, int a8, int a9, int a10,
                                  long l1, long l2, long l3, long l4, long l5)
{
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8;
    float f1, f2, f3, f4;
    double d1, d2;
    
    /* Complex array with SIB-like addressing potential */
    int array[256];
    struct nested nested_arr[8];
    
    /* Initialize with function args to prevent constant propagation */
    v1 = barrier(a1); v2 = barrier(a2); v3 = barrier(a3); 
    v4 = barrier(a4); v5 = barrier(a5); v6 = barrier(a6);
    v7 = barrier(a7); v8 = barrier(a8); v9 = barrier(a9);
    v10 = barrier(a10);
    
    /* Create long dependency chain */
    v11 = v1 + v2; v12 = v3 + v4; v13 = v5 + v6; 
    v14 = v7 + v8; v15 = v9 + v10;
    v16 = v11 * v12; v17 = v13 * v14; v18 = v15 * v16;
    v19 = v17 ^ v18; v20 = v19 & 0xFFFF;
    
    /* Use register variables in complex expressions */
    reg_var1 = v1 + v20;
    reg_var2 = v2 * v19;
    
    /* Inline assembly that clobbers many registers */
    /* This forces reloads around the asm block */
    asm volatile (
        "/* Begin clobbering block */\n\t"
        "mov %0, %0\n\t"  /* Use input operands */
        "mov %1, %1\n\t"
        : "+r" (reg_var1), "+r" (reg_var2)
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Complex addressing mode: array[base + index*scale] */
    /* This may require secondary reloads on some architectures */
    int scale = 4;
    int base = idx1;
    int index = idx2;
    
    /* Force SIB addressing with all components */
    for (int i = 0; i < 8; i++) {
        /* array[base + i*scale] - may need reload for index*scale */
        array[base + i * scale] = v1 + i;
        
        /* Nested array access with multiple indices */
        nested_arr[i].a[index] = v2 + i;
        nested_arr[i].inner.b[i % 2] = l1 + i;
        
        /* Volatile access to prevent optimization */
        nested_arr[i].inner.c = v3 ^ i;
    }
    
    /* Mixed register class operations */
    /* Integer to float moves may need reloads */
    f1 = (float)v4;
    f2 = (float)v5;
    f3 = f1 * f2;
    
    /* Use atomic operations to force specific memory addressing */
    int atomic_var = 0;
    __atomic_store_n(&array[base + index * 2], v6, __ATOMIC_RELAXED);
    atomic_var = __atomic_load_n(&nested_arr[0].a[index], __ATOMIC_RELAXED);
    
    /* More complex inline assembly with memory constraints */
    /* This specifically stresses reload record creation */
    volatile int* volatile_ptr = &array[128];
    int temp;
    
    asm volatile (
        "mov %[tmp], %[ptr]\n\t"
        "ldr %[tmp], [%[tmp]]\n\t"
        "add %[tmp], %[tmp], %[val]\n\t"
        "str %[tmp], [%[ptr]]\n\t"
        : [tmp] "=&r" (temp), [ptr] "+m" (*volatile_ptr)
        : [val] "r" (v7)
        : "memory"
    );
    
    /* Use all variables in final computation to prevent dead code elimination */
    lv1 = (long)v8 + (long)v9 + (long)v10;
    lv2 = (long)v11 * (long)v12;
    lv3 = (long)v13 | (long)v14;
    lv4 = (long)v15 ^ (long)v16;
    lv5 = (long)v17 & (long)v18;
    lv6 = (long)v19 + (long)v20;
    lv7 = (long)reg_var1 * (long)reg_var2;
    lv8 = (long)atomic_var + (long)temp;
    
    d1 = (double)f3;
    d2 = (double)lv1;
    
    /* Final complex expression using all values */
    long result = lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8;
    result += (long)d1 + (long)d2;
    result += array[base + index * scale];
    result += nested_arr[idx3].a[idx1];
    
    return result;
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) 
{
    /* Initialize many variables with non-constant values */
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
    
    long l1 = (long)barrier(argc + 11) * 1000L;
    long l2 = (long)barrier(argc + 12) * 2000L;
    long l3 = (long)barrier(argc + 13) * 3000L;
    long l4 = (long)barrier(argc + 14) * 4000L;
    long l5 = (long)barrier(argc + 15) * 5000L;
    
    /* Additional local variables to increase pressure */
    int extra1 = a1 * 2, extra2 = a2 * 3, extra3 = a3 * 4;
    int extra4 = a4 * 5, extra5 = a5 * 6, extra6 = a6 * 7;
    int extra7 = a7 * 8, extra8 = a8 * 9, extra9 = a9 * 10;
    volatile int extra10 = a10 * 11;
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               l1, l2, l3, l4, l5);
    
    /* Modify variables and call again to prevent optimization */
    a1 = barrier(a1); a2 = barrier(a2); a3 = barrier(a3);
    l1 = l1 ^ 0x12345678; l2 = l2 ^ 0x87654321;
    
    long result2 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               l1, l2, l3, l4, l5);
    
    /* Use all variables to prevent dead code elimination */
    long final_result = result1 + result2 + extra1 + extra2 + extra3 + 
                       extra4 + extra5 + extra6 + extra7 + extra8 + 
                       extra9 + extra10;
    
    /* Print result to ensure side effects are observable */
    volatile long output = final_result;
    
    return (int)(output & 0x7FFFFFFF);
}

/* Dummy definition of barrier to satisfy linker */
int barrier(int x) {
    volatile int y = x;
    return y ^ 0x55AA55AA;
}
