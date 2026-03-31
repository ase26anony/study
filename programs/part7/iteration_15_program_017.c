/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types */
    int v1 = input1;
    int v2 = input2 + 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 | 0x1234);
    char v6 = (char)(input3 ^ 0x55);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 * input3;
    int v9 = v1 + v2;
    int v10 = v3 - v1;
    int v11 = v2 * v3;
    int v12 = v1 | v2;
    int v13 = v3 & v4;
    int v14 = v5 ^ v6;
    int v15 = v7 >> 2;
    int v16 = v8 << 1;
    int v17 = v9 + v10;
    int v18 = v11 - v12;
    int v19 = v13 * v14;
    int v20 = v15 | v16;
    int v21 = v17 & v18;
    int v22 = v19 ^ v20;
    int v23 = v21 + v22;
    int v24 = v23 - v17;
    int v25 = v18 * v19;
    int v26 = v20 | v21;
    int v27 = v22 & v23;
    int v28 = v24 ^ v25;
    int v29 = v26 + v27;
    int v30 = v28 - v29;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 >> 1);
    v10 = (v11 | v12) ^ (v13 & v14);
    v15 = (v16 + v17) * (v18 - v19);
    v20 = (v21 << 2) | (v22 >> 3);
    v23 = (v24 * v25) + (v26 & v27);
    v28 = (v29 ^ v30) - (v1 | v2);
    
    /* Inline assembly to force specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5) : : "cc");
    
    /* More complex expressions with volatile memory accesses */
    volatile int mem1 = v6;
    volatile int mem2 = v7;
    v8 = mem1 + mem2;
    v9 = v8 * (mem1 - mem2);
    
    /* Nested expressions creating intermediate values */
    v10 = ((v11 + v12) * (v13 - v14)) >> ((v15 & 3) + 1);
    v16 = ((v17 | v18) & (v19 ^ v20)) + ((v21 << v22) >> v23);
    v24 = (v25 * v26) - (v27 / (v28 | 1)) + (v29 ^ v30);
    
    /* Force address register pressure with array accesses */
    volatile int arr[8] = {v1, v2, v3, v4, v5, v6, v7, v8};
    v9 = arr[0] + arr[1] + arr[2] + arr[3];
    v10 = arr[4] * arr[5] - arr[6] * arr[7];
    
    /* More inline assembly with mixed constraints */
    asm volatile(
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r"(v11)
        : "r"(v12), "r"(v13)
        : "cc"
    );
    
    /* Complex bit manipulation */
    v14 = ((v15 << 4) | (v16 >> 4)) & ((v17 << 8) | (v18 >> 8));
    v19 = (v20 ^ v21) + (v22 | v23) - (v24 & v25);
    v26 = ((v27 * 3) + (v28 * 5)) / ((v29 | 1) + (v30 & 0xFF));
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    volatile int seed = argc;
    int total = 0;
    
    /* Loop to prevent optimization and ensure execution */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile inputs to prevent constant propagation */
        int input1 = seed + i;
        int input2 = global_seed * (i + 1);
        int input3 = global_mask & (seed ^ i);
        
        /* Call the high-pressure function */
        int result = create_reload_pressure(input1, input2, input3);
        total += result;
        
        /* Modify globals to create side effects */
        global_seed ^= result;
        global_mask = (global_mask << 1) | (global_mask >> 31);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total);
    
    return total & 1;
}
