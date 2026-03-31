/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function with extreme register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(v3 & 0xFFFF);
    char v5 = (char)(v3 >> 8);
    long v6 = (long)v1 * (long)v2;
    int v7 = v3 ^ v1;
    int v8 = v2 | v3;
    short v9 = (short)(v7 + v8);
    char v10 = (char)(v8 - v7);
    int v11 = v4 * v5;
    long v12 = v6 + (long)v11;
    int v13 = ~v7;
    int v14 = v8 << 2;
    short v15 = (short)(v9 >> 1);
    char v16 = (char)(v10 ^ 0x55);
    int v17 = v11 + v13;
    long v18 = v12 - (long)v14;
    int v19 = v13 & v14;
    int v20 = v15 | v16;
    short v21 = (short)(v17 & 0xFF);
    char v22 = (char)(v18 & 0xFF);
    int v23 = v19 ^ v20;
    long v24 = v18 + (long)v23;
    int v25 = v21 * v22;
    int v26 = v23 << v5;
    short v27 = (short)(v24 & 0xFFFF);
    char v28 = (char)(v25 & 0xFF);
    int v29 = v26 | v27;
    int v30 = v28 + v29;
    
    /* Complex addressing mode simulation with volatile memory access */
    volatile int mem_var = global_seed;
    int v31 = v30 + mem_var;
    
    /* Inline assembly to force specific register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    
    /* Dense sequence of arithmetic/logical operations keeping values live */
    v4 = (v1 & v2) | (v3 << 1);
    v5 = (char)((v4 ^ v31) & 0xFF);
    v6 = (long)v1 * (long)v2 + (long)v3;
    v7 = (v4 << v5) | (v31 >> 2);
    v8 = v7 - v6;
    v9 = (short)((v8 & 0xFFFF) ^ (v4 & 0xFFFF));
    v10 = (char)(v9 | v5);
    v11 = v8 * v9;
    v12 = v6 - (long)v11;
    v13 = ~v7 & v8;
    v14 = v9 << (v10 & 3);
    v15 = (short)(v11 >> 4);
    v16 = (char)(v12 ^ v13);
    v17 = v14 + v15 - v16;
    v18 = v12 * (long)v13;
    v19 = v14 | v15;
    v20 = v16 & v17;
    v21 = (short)(v18 & 0xFFFF);
    v22 = (char)(v19 ^ v20);
    v23 = v21 * v22;
    v24 = v18 + (long)v23;
    v25 = v19 << (v20 & 7);
    v26 = v21 | v22;
    v27 = (short)(v23 & 0xFF);
    v28 = (char)(v24 & 0xFF);
    v29 = v25 ^ v26;
    v30 = v27 + v28 - v29;
    
    /* More inline assembly with memory constraints */
    asm volatile("" : : "r"(v1), "r"(v2), "m"(mem_var) : "cc");
    
    /* Additional complex expressions with multiple operands */
    v31 = ((v1 * v2) + (v3 << v4) - v5) | (v6 & v7);
    int v32 = (v8 ^ v9) * (v10 + v11);
    short v33 = (short)((v12 & v13) | (v14 << 1));
    char v34 = (char)((v15 + v16) ^ (v17 & 0xFF));
    long v35 = (long)v18 * (long)v19 - (long)v20;
    int v36 = v21 | v22 | v23;
    int v37 = v24 ^ v25 ^ v26;
    short v38 = (short)(v27 * v28);
    char v39 = (char)(v29 + v30);
    int v40 = v31 & v32 & v33;
    
    /* Force another memory access with addressing mode */
    volatile int* mem_ptr = &mem_var;
    int v41 = *mem_ptr + v34;
    
    /* Complex expression that likely needs reloads */
    int result = (v35 & 0xFFFFFFFF) + 
                 (v36 << 2) - 
                 (v37 >> 1) + 
                 (v38 * v39) + 
                 (v40 ^ v41);
    
    /* Use all variables in final computation to keep them live */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    result += v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 + v40 + v41;
    
    return result & global_mask;  /* Use volatile global */
}

/* Another non-inlineable function to create different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base, int modifier) {
    /* Different variable types and patterns */
    unsigned int u1 = (unsigned int)base;
    unsigned short u2 = (unsigned short)modifier;
    unsigned char u3 = (unsigned char)(base ^ modifier);
    
    /* Complex bit manipulation */
    u1 = (u1 << 3) | (u1 >> 29);
    u2 = (u2 ^ 0xAAAA) + u3;
    u3 = (u3 * 7) & 0xFF;
    
    /* Force addressing mode with array access */
    volatile unsigned int array[4] = {u1, u2, u3, u1 ^ u2};
    unsigned int temp = array[0] + array[1] - array[2] + array[3];
    
    /* Multiple intermediate computations */
    for (int i = 0; i < 3; i++) {
        temp = (temp << 1) | (temp >> 31);
        temp ^= array[i & 3];
    }
    
    return (int)temp;
}

int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile loop counter to prevent optimization */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Create varying inputs from argc and argv */
    int input1 = argc;
    int input2 = (argv[0] != NULL) ? argv[0][0] : 0;
    int input3 = (argc > 1) ? argv[1][0] : 'A';
    
    printf("Starting reload pressure test with %d iterations...\n", iterations);
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Mix both pressure functions */
        int r1 = create_reload_pressure(input1 + i, input2, input3);
        int r2 = secondary_pressure(input2 + i, input3);
        
        total += r1 ^ r2;
        
        /* Modify inputs slightly each iteration */
        input1 = (input1 * 1103515245 + 12345) & 0x7FFFFFFF;
        input2 = (input2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        input3 = (input3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%08X\n", total & 0xFFFFFFFF);
    
    return (total & 0xFF);  /* Return non-zero to indicate execution */
}
