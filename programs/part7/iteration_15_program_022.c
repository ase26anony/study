/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Force register pressure function to not be optimized away */
__attribute__((noinline, noipa, optimize("O0")))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare MANY local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = input3 & 0xFF;
    short v4 = (short)(input1 ^ input2);
    char v5 = (char)(input3 >> 4);
    long v6 = (long)input1 * input2;
    int v7 = v1 + v2;
    int v8 = v3 | v4;
    int v9 = v5 * 3;
    long v10 = v6 + v7;
    int v11 = v8 ^ v9;
    short v12 = (short)(v10 & 0xFFFF);
    char v13 = (char)(v11 >> 8);
    int v14 = v12 + v13;
    long v15 = v10 * v14;
    int v16 = v11 & v14;
    int v17 = v7 << 2;
    int v18 = v8 >> 1;
    int v19 = v9 + v16;
    int v20 = v17 | v18;
    int v21 = v19 ^ v20;
    long v22 = v15 + v21;
    int v23 = v16 * v17;
    int v24 = v18 + v19;
    int v25 = v20 & v21;
    short v26 = (short)(v22 & 0xFFFF);
    char v27 = (char)(v23 >> 8);
    int v28 = v24 | v25;
    long v29 = v22 * v28;
    int v30 = v26 + v27;
    
    /* Complex addressing mode forcing - use array indexing with variables */
    volatile int memory[32];
    for (int i = 0; i < 8; i++) {
        memory[i] = i * input1;
    }
    
    /* Force memory accesses with complex addressing */
    v1 = memory[v2 & 7] + v3;
    v4 = (short)(memory[v5 & 7] ^ v6);
    v7 = memory[v8 & 7] * v9;
    
    /* Inline assembly to create specific register constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "xorl %2, %0\n\t"
        : "+r" (v10)
        : "r" (v11), "r" (v12)
        : "cc"
    );
    
    /* More complex expressions with multiple uses of variables */
    v13 = (char)(((v14 * v15) + (v16 << v17) - v18) & 0xFF);
    v19 = ((v20 & v21) | (v22 >> v23)) + v24;
    v25 = (v26 * v27) - (v28 ^ v29);
    v30 = ((v1 << v2) | (v3 & v4)) + ((v5 * v6) - (v7 ^ v8));
    
    /* Even more variables to increase pressure */
    int v31 = v10 + v11 + v12;
    int v32 = v13 * v14 - v15;
    int v33 = v16 | v17 ^ v18;
    int v34 = v19 << (v20 & 3);
    int v35 = v21 >> (v22 & 3);
    int v36 = v23 + v24 * v25;
    int v37 = v26 & v27 | v28;
    int v38 = v29 ^ v30 + v31;
    int v39 = v32 - v33 * v34;
    int v40 = v35 | v36 & v37;
    
    /* Use all variables in a complex computation to keep them live */
    long result = (long)v1 * v2;
    result += (long)v3 * v4;
    result += (long)v5 * v6;
    result += (long)v7 * v8;
    result += (long)v9 * v10;
    result += (long)v11 * v12;
    result += (long)v13 * v14;
    result += (long)v15 * v16;
    result += (long)v17 * v18;
    result += (long)v19 * v20;
    result += (long)v21 * v22;
    result += (long)v23 * v24;
    result += (long)v25 * v26;
    result += (long)v27 * v28;
    result += (long)v29 * v30;
    result += (long)v31 * v32;
    result += (long)v33 * v34;
    result += (long)v35 * v36;
    result += (long)v37 * v38;
    result += (long)v39 * v40;
    
    /* More inline assembly with memory constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (memory[0])
        : "r" (v1), "r" (v2)
        : "%eax", "cc"
    );
    
    /* Use volatile memory access to force reloads */
    v1 = memory[0] + memory[1];
    v2 = memory[2] - memory[3];
    
    /* Final complex expression using most variables */
    return (int)(result ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 
                 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20
                 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30
                 ^ v31 ^ v32 ^ v33 ^ v34 ^ v35 ^ v36 ^ v37 ^ v38 ^ v39 ^ v40);
}

/* Another pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different data types to stress the register allocator */
    unsigned char c1 = base & 0xFF;
    unsigned short s1 = (base >> 8) & 0xFFFF;
    unsigned int i1 = base * 3;
    unsigned long l1 = (unsigned long)base * base;
    
    /* Force many intermediate calculations */
    for (int i = 0; i < 4; i++) {
        c1 = (c1 * 17 + i) & 0xFF;
        s1 = (s1 ^ (s1 << 3)) + c1;
        i1 = (i1 * 3 - s1) | l1;
        l1 = l1 + (i1 * i1);
        
        /* Inline assembly with clobbered registers */
        asm volatile (
            "rorl $3, %0\n\t"
            "roll $5, %1\n\t"
            : "+r" (i1), "+r" (s1)
            : 
            : "cc"
        );
    }
    
    return (int)(c1 + s1 + i1 + (l1 & 0xFFFFFFFF));
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure the function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile inputs to prevent constant propagation */
        volatile int input1 = global_seed + i;
        volatile int input2 = global_mask ^ i;
        volatile int input3 = argc * i;
        
        /* Call the high-pressure function */
        int result1 = create_reload_pressure(input1, input2, input3);
        
        /* Call secondary pressure function */
        int result2 = secondary_pressure(result1 + i);
        
        /* Combine results in a non-trivial way */
        total_result ^= result1;
        total_result += result2;
        total_result = (total_result << 3) | (total_result >> 29); /* rotate */
        
        /* Force memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
