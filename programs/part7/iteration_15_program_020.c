/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 54321;

/* Non-inlineable function with massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = input3 * 2;
    short v4 = (short)(input1 & 0xFFFF);
    short v5 = (short)(input2 & 0xFFFF);
    char v6 = (char)(input3 & 0xFF);
    char v7 = (char)((input1 + input2) & 0xFF);
    long v8 = (long)input1 * input2;
    long v9 = (long)input2 * input3;
    long v10 = (long)input3 * input1;
    
    int v11 = v1 * v2;
    int v12 = v2 + v3;
    int v13 = v3 - v1;
    short v14 = (short)(v4 + v5);
    short v15 = (short)(v5 - v4);
    char v16 = (char)(v6 ^ v7);
    char v17 = (char)(v6 & v7);
    long v18 = v8 + v9;
    long v19 = v9 - v10;
    long v20 = v10 * v8;
    
    int v21 = v11 << (v12 & 3);
    int v22 = v12 >> (v13 & 3);
    int v23 = v13 | v11;
    short v24 = (short)(v14 * v15);
    short v25 = (short)(v15 + v14);
    char v26 = (char)(v16 | v17);
    char v27 = (char)(v16 & v17);
    long v28 = v18 ^ v19;
    long v29 = v19 | v20;
    long v30 = v20 & v18;
    
    /* Complex expressions requiring multiple registers */
    int v31 = (v21 * v22) + (v23 << 2) - (v11 >> 1);
    int v32 = (v22 & v23) | (v21 ^ v12);
    short v33 = (short)((v24 + v25) * (v14 - v15));
    short v34 = (short)((v25 << 1) | (v24 >> 1));
    char v35 = (char)((v26 * v27) + (v16 - v17));
    char v36 = (char)((v27 << 2) ^ (v26 >> 2));
    long v37 = (v28 * v29) + (v30 << 3);
    long v38 = (v29 ^ v30) | (v28 & v19);
    long v39 = (v30 + v28) - (v29 * 2);
    
    /* More variables to increase pressure */
    int v40 = v31 + v32 + v1;
    int v41 = v32 - v31 + v2;
    int v42 = v31 * v32 + v3;
    short v43 = (short)(v33 + v34 + v4);
    short v44 = (short)(v34 - v33 + v5);
    char v45 = (char)(v35 ^ v36 ^ v6);
    char v46 = (char)(v36 & v35 & v7);
    long v47 = v37 + v38 + v8;
    long v48 = v38 - v37 + v9;
    long v49 = v37 * v38 + v10;
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v40), "+r"(v41) : : "cc", "memory");
    asm volatile("" : "+r"(v42), "+r"(v43) : : "cc", "memory");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v40;
    int* ptr2 = &v41;
    int* ptr3 = &v42;
    
    /* Force memory accesses that compete for address registers */
    volatile int mem1 = *ptr1 + *ptr2;
    volatile int mem2 = *ptr2 - *ptr3;
    volatile int mem3 = *ptr3 * *ptr1;
    
    /* More operations keeping many values live */
    v40 = v40 + mem1;
    v41 = v41 - mem2;
    v42 = v42 * mem3;
    
    /* Even more variables */
    int v50 = v40 << (v41 & 7);
    int v51 = v41 >> (v42 & 7);
    int v52 = v42 | v40;
    int v53 = v50 ^ v51;
    int v54 = v51 & v52;
    int v55 = v52 + v53;
    int v56 = v53 - v54;
    int v57 = v54 * v55;
    int v58 = v55 / (v56 ? v56 : 1);
    int v59 = v56 % (v57 ? v57 : 1);
    int v60 = v57 ^ v58;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v11 ^ v12 ^ v13 ^ v21 ^ v22 ^ v23;
    result += v31 ^ v32 ^ v40 ^ v41 ^ v42 ^ v50 ^ v51 ^ v52;
    result += v53 ^ v54 ^ v55 ^ v56 ^ v57 ^ v58 ^ v59 ^ v60;
    result += (int)(v8 & 0xFFFFFFFF) ^ (int)(v9 & 0xFFFFFFFF);
    result += (int)(v10 & 0xFFFFFFFF) ^ (int)(v18 & 0xFFFFFFFF);
    result += (int)(v19 & 0xFFFFFFFF) ^ (int)(v20 & 0xFFFFFFFF);
    result += (int)(v28 & 0xFFFFFFFF) ^ (int)(v29 & 0xFFFFFFFF);
    result += (int)(v30 & 0xFFFFFFFF) ^ (int)(v37 & 0xFFFFFFFF);
    result += (int)(v38 & 0xFFFFFFFF) ^ (int)(v39 & 0xFFFFFFFF);
    result += (int)(v47 & 0xFFFFFFFF) ^ (int)(v48 & 0xFFFFFFFF);
    result += (int)(v49 & 0xFFFFFFFF);
    
    /* Use all variables in return calculation */
    return result + v4 + v5 + v6 + v7 + v14 + v15 + v16 + v17 +
           v24 + v25 + v26 + v27 + v33 + v34 + v35 + v36 +
           v43 + v44 + v45 + v46;
}

int main(int argc, char *argv[]) {
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 1) loop_limit = 1;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Use volatile globals and argc to prevent constant propagation */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc + i * 2;
        int input3 = global_seed3 ^ argc ^ i;
        
        total ^= create_reload_pressure(input1, input2, input3);
        
        /* Modify globals slightly to change inputs */
        global_seed1 = (global_seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        global_seed2 = (global_seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        global_seed3 = (global_seed3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    /* Print result to create side effect */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
