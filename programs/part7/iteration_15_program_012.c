/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFFFFFF;

/* NOINLINE function to create register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3, int input4) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 | 0xAA;
    int v4 = input4 ^ 0x55;
    short v5 = (short)(v1 + v2);
    char v6 = (char)(v3 & 0xFF);
    long v7 = (long)v1 * (long)v2;
    int v8 = v3 << 2;
    int v9 = v4 >> 1;
    int v10 = v5 * 3;
    int v11 = v6 + 100;
    int v12 = (int)(v7 & 0xFFFFFFFF);
    int v13 = v8 | v9;
    int v14 = v10 ^ v11;
    int v15 = v12 + v13;
    int v16 = v14 * v15;
    unsigned int v17 = (unsigned int)v16;
    unsigned short v18 = (unsigned short)v17;
    unsigned char v19 = (unsigned char)v18;
    int v20 = v19 * 7;
    int v21 = v20 - v1;
    int v22 = v21 + v2;
    int v23 = v22 * v3;
    int v24 = v23 / (v4 + 1);
    int v25 = v24 << v5;
    int v26 = v25 >> v6;
    int v27 = v26 & v7;
    int v28 = v27 | v8;
    int v29 = v28 ^ v9;
    int v30 = v29 + v10;
    
    /* Complex addressing mode operations */
    volatile int mem1 = v11;
    volatile int mem2 = v12;
    volatile int mem3 = v13;
    
    /* Force memory accesses that compete for address registers */
    int* ptr1 = (int*)&mem1;
    int* ptr2 = (int*)&mem2;
    int* ptr3 = (int*)&mem3;
    
    /* Multi-operand expressions requiring specific registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3)) - v6;
    v7 = (v8 * v9) + (v10 << (v11 % 8)) - v12;
    v13 = (v14 | v15) ^ (v16 & v17) + (v18 * v19);
    v20 = (v21 + v22) * (v23 - v24) / (v25 + 1);
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2) : : "cc", "memory");
    asm volatile("" : "+m"(*ptr1), "+r"(v3) : : "cc");
    asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6) : : "cc");
    
    /* More complex expressions with many live values */
    v26 = ((v27 * v28) + (v29 << 2)) | ((v30 & v1) ^ (v2 | v3));
    v7 = v4 * v5 + v6 * v8 - v9 * v10 + v11 * v12;
    v13 = (v14 & v15) | (v16 ^ v17) + (v18 << v19) - (v20 >> v21);
    
    /* Access volatiles to force loads */
    v22 = *ptr1 + *ptr2 - *ptr3;
    v23 = mem1 * mem2 + mem3;
    
    /* Even more operations keeping all values live */
    v24 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v25 = v9 * v10 * v11 * v12 * v13 * v14;
    v26 = v15 & v16 & v17 & v18 & v19 & v20;
    v27 = v21 | v22 | v23 | v24 | v25 | v26;
    v28 = v27 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5;
    v29 = v6 + v7 + v8 + v9 + v10 + v11;
    v30 = v12 * v13 - v14 * v15 + v16 * v17;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= *ptr1 ^ *ptr2 ^ *ptr3;
    result ^= mem1 ^ mem2 ^ mem3;
    
    return result & global_mask;
}

/* Another pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different type mixing */
    unsigned long ul1 = (unsigned long)base * 3;
    unsigned int ui2 = (unsigned int)ul1 + 1;
    unsigned short us3 = (unsigned short)ui2;
    unsigned char uc4 = (unsigned char)us3;
    
    int i5 = (int)ul1;
    short s6 = (short)ui2;
    char c7 = (char)us3;
    
    /* Complex bit manipulation */
    i5 = ((i5 << 3) | (i5 >> 29)) ^ 0xDEADBEEF;
    s6 = (s6 * 7) & 0x7FFF;
    c7 = ((c7 ^ 0x55) << 2) | (c7 & 0x03);
    
    /* Force spills with many intermediate calculations */
    int t1 = i5 + s6;
    int t2 = t1 * c7;
    int t3 = t2 - i5;
    int t4 = t3 | s6;
    int t5 = t4 ^ c7;
    int t6 = t5 << 2;
    int t7 = t6 >> 1;
    int t8 = t7 & 0xFF;
    int t9 = t8 * 3;
    int t10 = t9 - t1;
    
    /* Use inline asm with specific constraints */
    asm volatile("" : "+r"(t1), "+r"(t2), "+r"(t3) : : "cc");
    asm volatile("" : "+m"(ul1), "+r"(ui2) : : "cc");
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

int main(int argc, char **argv) {
    /* Use volatile to prevent loop optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Create varying inputs from globals and loop counter */
        int input1 = global_seed + i;
        int input2 = argc * 3;
        int input3 = (int)((long)&global_seed & 0xFFFF);
        int input4 = i * i;
        
        /* Call the high-pressure function */
        int result1 = create_reload_pressure(input1, input2, input3, input4);
        
        /* Call secondary function */
        int result2 = secondary_pressure(result1 + i);
        
        /* Mix results */
        total_result ^= result1;
        total_result += result2;
        
        /* Modify global to prevent optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
