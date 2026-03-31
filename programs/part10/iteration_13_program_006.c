/* reload_stress.c - Stress GCC's reload pass to cover rld initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register pressure with many live variables */
__attribute__((noinline))
static long test_reloads(int a1, int a2, int a3, int a4, int a5,
                         int a6, int a7, int a8, int a9, int a10,
                         int a11, int a12, int a13, int a14, int a15,
                         int a16, int a17, int a18, int a19, int a20,
                         volatile int* mem_base, int idx1, int idx2) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1 = a3, v2 = a4, v3 = a5, v4 = a6, v5 = a7;
    int v6 = a8, v7 = a9, v8 = a10, v9 = a11, v10 = a12;
    int v11 = a13, v12 = a14, v13 = a15, v14 = a16, v15 = a17;
    int v16 = a18, v17 = a19, v18 = a20;
    long l1 = a1, l2 = a2, l3 = a3, l4 = a4, l5 = a5;
    float f1 = a6 * 0.5f, f2 = a7 * 0.25f;
    double d1 = a8 * 0.125, d2 = a9 * 0.0625;
    
    /* Complex addressing with SIB-like computation (for x86) */
    volatile int* volatile_ptr = mem_base;
    volatile int idx_volatile = idx1;
    
    /* Force multiple reloads with complex addressing */
    int addr1 = idx_volatile * 4 + idx2 * 2 + 16;
    int addr2 = idx_volatile * 8 + idx2 * 4 + 32;
    
    /* Memory access with complex addressing - may need secondary reload */
    int mem_val1 = volatile_ptr[addr1 / sizeof(int)];
    int mem_val2 = volatile_ptr[addr2 / sizeof(int)];
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp]\n"
        "add %[val2], %[tmp]\n"
        : [tmp] "=r" (r0)
        : [val1] "m" (mem_val1), 
          [val2] "r" (mem_val2)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* More arithmetic to create dependency chain */
    v1 = v1 + r0 + r1;
    v2 = v2 * v1 - r0;
    v3 = v3 ^ v2 | v1;
    v4 = v4 + v3 * 2;
    v5 = v5 - v4 / 3;
    v6 = v6 & v5 ^ v4;
    v7 = v7 | v6 & v5;
    v8 = v8 + v7 - v6;
    v9 = v9 * v8 + v7;
    v10 = v10 ^ v9 | v8;
    
    /* Type mixing between integer and float */
    f1 = f1 + (float)v10;
    d1 = d1 + (double)v9;
    v11 = v11 + (int)f1;
    v12 = v12 + (int)d1;
    
    /* Another inline asm with memory constraint */
    int tmp_out;
    asm volatile (
        "# Secondary reload test\n"
        "ldr %[out], [%[base], %[index], lsl #2]\n"
        : [out] "=r" (tmp_out)
        : [base] "r" (volatile_ptr),
          [index] "r" (idx_volatile)
        : "memory"
    );
    
    v13 = v13 + tmp_out;
    
    /* Atomic operations for additional reload complexity */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(0);
    __atomic_store_n(&atomic_var, v13, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    v14 = v14 + atomic_val;
    
    /* Use all variables in final computation */
    long result = (long)r0 + r1 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18
                + (long)l1 + l2 + l3 + l4 + l5
                + (long)f1 + (long)f2 + (long)d1 + (long)d2
                + (long)mem_val1 + mem_val2 + tmp_out + atomic_val;
    
    /* Force another barrier */
    result = barrier(result);
    
    return result;
}

/* Multi-dimensional array access to stress addressing modes */
__attribute__((noinline))
static int complex_array_access(int size, volatile int idx) {
    int arr[32][32];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            arr[i][j] = i * 32 + j;
        }
    }
    
    /* Complex addressing with variable indices */
    for (int i = 0; i < size; i++) {
        /* This addressing may need secondary reload on some arches */
        int val = arr[idx % 32][(idx + i) % 32];
        sum += val;
        
        /* More complex: arr[i][idx*2 + i*3] */
        int idx2 = (idx * 2 + i * 3) % 32;
        sum += arr[i % 32][idx2];
    }
    
    return sum;
}

/* Union for type-punning between int and float */
union type_pun {
    int i;
    float f;
    unsigned u;
};

__attribute__((noinline))
static int type_punning_test(int x, float y) {
    union type_pun u1, u2;
    u1.i = x;
    u2.f = y;
    
    /* Force moves between different register classes */
    int result = u1.i + (int)u2.f;
    u1.f = (float)u2.i;
    result += (int)u1.f;
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
    int vars[30];
    for (int i = 0; i < 30; i++) {
        vars[i] = (argc > 1) ? atoi(argv[1]) + i : i * 3 + 1;
    }
    
    /* Volatile memory region */
    volatile int mem_region[256];
    for (int i = 0; i < 256; i++) {
        mem_region[i] = i * 7;
    }
    
    volatile int idx1 = 10, idx2 = 20;
    
    /* Call test function with many arguments */
    long result = test_reloads(
        vars[0], vars[1], vars[2], vars[3], vars[4],
        vars[5], vars[6], vars[7], vars[8], vars[9],
        vars[10], vars[11], vars[12], vars[13], vars[14],
        vars[15], vars[16], vars[17], vars[18], vars[19],
        mem_region, idx1, idx2
    );
    
    /* Additional tests for different reload patterns */
    int array_result = complex_array_access(16, idx1);
    int pun_result = type_punning_test(vars[20], vars[21] * 0.5f);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %ld\n", result + array_result + pun_result);
    
    /* More register pressure in main */
    int r1 = vars[22], r2 = vars[23], r3 = vars[24], r4 = vars[25];
    int r5 = vars[26], r6 = vars[27], r7 = vars[28], r8 = vars[29];
    
    /* Long dependency chain */
    for (int i = 0; i < 100; i++) {
        r1 = r1 + r2 * 3;
        r2 = r2 ^ r3;
        r3 = r3 | r4;
        r4 = r4 + r5 - r6;
        r5 = r5 * r7 / (r8 + 1);
        r6 = r6 & r1;
        r7 = r7 ^ r2;
        r8 = r8 + r3;
    }
    
    printf("Final: %d\n", r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8);
    
    return 0;
}

/* Dummy barrier function definition */
int barrier(int x) {
    return x ^ 0x55AA55AA;
}
