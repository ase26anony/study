/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa)) 
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1;
    int v2 = input2 + 1;
    int v3 = input3 * 2;
    int v4 = input4 & 0xFF;
    short v5 = (short)(input1 >> 8);
    char v6 = (char)(input2 & 0xFF);
    long v7 = (long)input3 * 3;
    int v8 = v1 + v2;
    int v9 = v3 - v4;
    short v10 = (short)(v5 + v6);
    char v11 = (char)(v6 * 2);
    long v12 = v7 + v1;
    int v13 = v8 ^ v9;
    int v14 = v4 | v3;
    int v15 = v2 << 3;
    int v16 = v1 >> 2;
    int v17 = v13 & v14;
    int v18 = v15 + v16;
    short v19 = (short)(v10 - v11);
    char v20 = (char)(v11 + 5);
    long v21 = v12 * 2;
    int v22 = v17 | v18;
    int v23 = v13 ^ v14;
    int v24 = v15 & v16;
    int v25 = v22 + v23;
    int v26 = v24 * 3;
    int v27 = v25 - v26;
    int v28 = v27 << 1;
    int v29 = v28 >> 2;
    int v30 = v29 & 0x7F;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (v7 * v8) - (v9 ^ v10);
    v11 = ((v12 & v13) | (v14 << v15)) + v16;
    v17 = (v18 * v19) / (v20 + 1);
    v21 = (v22 | v23) & (v24 ^ v25);
    v26 = (v27 << 2) + (v28 >> 1);
    v29 = (v30 * v1) - (v2 & v3);
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+r"(v3), "+r"(v4) : : "cc", "memory");
    
    /* More complex operations with mixed types */
    v5 = (short)((v6 * v7) & 0xFFFF);
    v8 = v9 + ((v10 & 0xFF) << 8);
    v11 = (char)((v12 ^ v13) & 0xFF);
    v14 = v15 | ((v16 & 0xF) << 4);
    v17 = (v18 + v19) * (v20 - v21);
    v22 = (v23 << v24) | (v25 >> v26);
    v27 = v28 ^ v29 ^ v30;
    
    /* Address calculations to stress address registers */
    int* ptr1 = &v1;
    int* ptr2 = &v2;
    int* ptr3 = &v3;
    
    /* Volatile memory accesses */
    volatile int mem1 = *ptr1;
    volatile int mem2 = *ptr2;
    volatile int mem3 = *ptr3;
    
    /* More operations using memory values */
    v4 = mem1 + mem2;
    v5 = (short)(mem3 & mem1);
    v6 = (char)((mem2 >> 8) & 0xFF);
    
    /* Additional inline assembly with memory constraints */
    asm volatile("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3) : "memory");
    
    /* Even more variables to increase pressure */
    int v31 = v1 + v2 + v3;
    int v32 = v4 * v5 * v6;
    int v33 = v7 ^ v8 ^ v9;
    int v34 = v10 | v11 | v12;
    int v35 = v13 & v14 & v15;
    int v36 = v16 << v17;
    int v37 = v18 >> v19;
    int v38 = v20 + v21 + v22;
    int v39 = v23 - v24 - v25;
    int v40 = v26 * v27 * v28;
    
    /* Complex expression combining many variables */
    int result = (v31 & v32) | (v33 ^ v34) + (v35 << 2) - 
                 (v36 >> 1) * (v37 & 0xF) + (v38 | v39) ^ v40;
    
    /* Final inline assembly to prevent optimization */
    asm volatile("" : "+r"(result) : : "cc");
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable types and patterns */
    unsigned int u1 = base;
    unsigned short u2 = base >> 16;
    unsigned char u3 = base & 0xFF;
    signed int s1 = -base;
    signed short s2 = base & 0x7FFF;
    signed char s3 = -(base & 0x7F);
    
    /* Mixed signed/unsigned operations */
    int t1 = u1 + s1;
    int t2 = u2 * s2;
    int t3 = u3 - s3;
    int t4 = t1 ^ t2;
    int t5 = t2 | t3;
    int t6 = t3 & t4;
    int t7 = t4 << 2;
    int t8 = t5 >> 1;
    int t9 = t6 * 3;
    int t10 = t7 - t8;
    
    /* Complex addressing with array */
    int arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = base + i;
    }
    
    /* Use array elements in calculations */
    t1 = arr[0] + arr[1];
    t2 = arr[2] * arr[3];
    t3 = arr[4] ^ arr[5];
    t4 = arr[6] | arr[7];
    
    /* Force spills with many live values */
    int r1 = t1 + t2;
    int r2 = t3 - t4;
    int r3 = t1 * t3;
    int r4 = t2 / (t4 + 1);
    int r5 = r1 ^ r2;
    int r6 = r3 | r4;
    int r7 = r5 << 3;
    int r8 = r6 >> 2;
    int r9 = r7 & r8;
    int r10 = r9 + t10;
    
    return r10;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = global_seed + i;
        volatile int input2 = global_mask - i;
        volatile int input3 = argc * i;
        volatile int input4 = (i << 8) | (i & 0xFF);
        
        /* Call high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        int result2 = secondary_pressure(input1 + input2);
        
        /* Combine results to prevent dead code elimination */
        total_result ^= result1;
        total_result += result2;
        
        /* Modify globals to create side effects */
        global_seed ^= result1;
        global_mask &= result2;
    }
    
    /* Use result to prevent optimization */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result == 0) ? 0 : 1;
}
