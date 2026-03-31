/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* Non-inlineable function that creates massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = v1 ^ v2;
    short v4 = (short)(input3 & 0xFFFF);
    short v5 = (short)(input4 | 0xAAAA);
    char v6 = (char)(input1 * 3);
    char v7 = (char)(input2 + 5);
    long v8 = (long)input1 * (long)input2;
    long v9 = (long)input3 * (long)input4;
    int v10 = v1 + v2;
    int v11 = v3 * v4;
    int v12 = v5 - v6;
    int v13 = v7 ^ v8;
    int v14 = v9 & v10;
    int v15 = v11 | v12;
    int v16 = v13 << 2;
    int v17 = v14 >> 1;
    int v18 = v15 + v16;
    int v19 = v17 - v18;
    int v20 = v19 * v1;
    int v21 = v2 & v20;
    int v22 = v3 | v21;
    int v23 = v4 ^ v22;
    int v24 = v5 + v23;
    int v25 = v6 - v24;
    int v26 = v7 * v25;
    int v27 = v8 & v26;
    int v28 = v9 | v27;
    int v29 = v10 ^ v28;
    int v30 = v11 + v29;
    
    /* Complex addressing mode forcing - use variables as array indices */
    volatile int array[32];
    for (int i = 0; i < 32; i++) {
        array[i] = i * global_seed;
    }
    
    /* Force memory accesses with complex addressing */
    v1 = array[v2 & 31] + array[v3 & 31];
    v4 = (short)(array[v5 & 31] ^ array[v6 & 31]);
    v7 = (char)(array[v8 & 31] | array[v9 & 31]);
    
    /* Inline assembly to create register constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (v10)
        : "r" (v11), "r" (v12)
        : "cc"
    );
    
    /* More complex expressions requiring multiple registers */
    v13 = (v14 * v15) + (v16 << (v17 & 3)) - (v18 >> (v19 & 3));
    v20 = (v21 & v22) | (v23 << v24) ^ (v25 >> v26);
    
    /* Force spill/reload by using all variables in one massive expression */
    int result = 
        (v1 * v2) + (v3 & v4) - (v5 | v6) + 
        (v7 ^ v8) * (v9 - v10) + (v11 << (v12 & 7)) - 
        (v13 >> (v14 & 7)) | (v15 & v16) ^ (v17 | v18) + 
        (v19 * v20) - (v21 & v22) | (v23 ^ v24) << 
        (v25 & 3) + (v26 - v27) * (v28 | v29) ^ v30;
    
    /* Use inline assembly with memory constraint to force reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+m" (result)
        : "r" (global_seed)
        : "eax", "cc"
    );
    
    /* More operations to keep variables live */
    v1 = v2 + v3;
    v4 = (short)(v5 ^ v6);
    v7 = (char)(v8 & v9);
    v10 = v11 * v12;
    v13 = v14 | v15;
    v16 = v17 - v18;
    v19 = v20 ^ v21;
    v22 = v23 << 2;
    v24 = v25 >> 1;
    v26 = v27 + v28;
    v29 = v30 - v1;
    
    /* Final complex expression using all variables */
    result ^= 
        (v1 + v2) | (v3 & v4) ^ (v5 - v6) + 
        (v7 * v8) & (v9 | v10) - (v11 ^ v12) << 
        (v13 & 3) | (v14 + v15) & (v16 - v17) ^ 
        (v18 * v19) + (v20 | v21) - (v22 ^ v23) >> 
        (v24 & 3) | (v25 + v26) & (v27 - v28) ^ v29;
    
    return result & global_mask;
}

/* Another pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different type mixing */
    unsigned int u1 = (unsigned int)base;
    unsigned short u2 = (unsigned short)(base * 2);
    unsigned char u3 = (unsigned char)(base + 3);
    signed int s1 = -base;
    signed short s2 = (signed short)(-base * 2);
    signed char s3 = (signed char)(-base - 1);
    
    /* Complex bit manipulation */
    u1 = (u1 << 3) | (u2 >> 5);
    u2 = (u2 ^ u3) & 0x7FFF;
    u3 = (u3 * 7) + 1;
    
    s1 = (s1 & 0x0F0F0F0F) | (s2 << 16);
    s2 = (s2 ^ s3) | 0x8000;
    s3 = (s3 * 3) - 2;
    
    /* Force addressing mode complexity */
    volatile unsigned int u_array[16];
    volatile signed int s_array[16];
    
    for (int i = 0; i < 16; i++) {
        u_array[i] = u1 + i;
        s_array[i] = s1 - i;
    }
    
    /* Mixed type operations requiring conversions */
    int mixed1 = (int)u1 + (int)s1;
    int mixed2 = (int)u2 * (int)s2;
    int mixed3 = (int)u3 - (int)s3;
    
    /* Inline assembly with multiple constraints */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0\n\t"
        : "+r" (mixed1)
        : "r" (mixed2), "rm" (mixed3)
        : "cc"
    );
    
    /* Access arrays with complex indices */
    mixed2 = u_array[(u1 ^ u2) & 15] + s_array[(s1 | s2) & 15];
    mixed3 = u_array[(u3 * 2) & 15] - s_array[(s3 / 2) & 15];
    
    return mixed1 ^ mixed2 ^ mixed3;
}

int main(int argc, char **argv) {
    /* Use argc to make values unknown at compile time */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Create varying inputs to prevent constant propagation */
        int input1 = argc + i;
        int input2 = argc * 2 + i;
        int input3 = argc * 3 + i;
        int input4 = argc * 4 + i;
        
        /* Call the high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        int result2 = secondary_pressure(input1 ^ input2);
        
        /* Combine results to prevent dead code elimination */
        total_result ^= result1;
        total_result += result2;
        
        /* Modify global to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result == 0) ? 0 : 1;
}
