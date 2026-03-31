/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent constant propagation */
volatile int global_seed = 42;
volatile int global_mask = 0xFFFF;

/* Non-inlineable function that creates massive register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 * 2;
    int v3 = v1 ^ v2;
    short v4 = (short)(input3 & 0xFF);
    char v5 = (char)(input1 >> 8);
    long v6 = (long)input2 * input3;
    int v7 = v1 | v2;
    int v8 = v3 & 0x7F;
    short v9 = (short)(v4 + v5);
    char v10 = (char)(v6 & 0xFF);
    int v11 = v7 - v8;
    long v12 = v6 >> 4;
    int v13 = v11 * v3;
    short v14 = (short)(v9 | 0x80);
    char v15 = (char)(v10 ^ 0x55);
    int v16 = v13 + v11;
    long v17 = v12 * 3;
    int v18 = v16 & v13;
    short v19 = (short)(v14 - v9);
    char v20 = (char)(v15 + 1);
    int v21 = v18 | v16;
    long v22 = v17 << 2;
    int v23 = v21 ^ v18;
    short v24 = (short)(v19 * 2);
    char v25 = (char)(v20 & 0xF);
    int v26 = v23 + v21;
    long v27 = v22 >> 1;
    int v28 = v26 - v23;
    short v29 = (short)(v24 ^ 0x7F);
    char v30 = (char)(v25 | 0xA);
    
    /* Complex expressions with many live values to force spills/reloads */
    /* First complex expression - many operands */
    int expr1 = ((v1 * v2) + (v3 << v4)) - (v5 & v6) | (v7 ^ v8);
    
    /* Use inline assembly to create register constraints */
    asm volatile("" : "+r"(expr1) : : "cc", "memory");
    
    /* Second complex expression - different operands */
    long expr2 = ((v9 * v10) | (v11 & v12)) + ((v13 << v14) - (v15 ^ v16));
    
    /* Third complex expression with volatile memory access pattern */
    volatile int mem_var = global_seed;
    int expr3 = (v17 & mem_var) | (v18 << (v19 & 0x3)) - (v20 * v21);
    
    /* Fourth complex expression */
    int expr4 = ((v22 & 0xFF) * v23) + ((v24 | v25) << 2) - (v26 ^ v27);
    
    /* Fifth complex expression with inline assembly constraints */
    int expr5;
    asm volatile("mov %1, %0\n\t"
                 "add %2, %0\n\t"
                 "sub %3, %0"
                 : "=r"(expr5)
                 : "r"(v28), "r"(v29), "r"(v30)
                 : "cc");
    
    /* More operations keeping many values live */
    int tmp1 = expr1 & expr2;
    long tmp2 = expr3 | expr4;
    short tmp3 = (short)(expr5 ^ tmp1);
    char tmp4 = (char)(tmp2 & 0xFF);
    
    /* Complex addressing mode simulation */
    int* ptr_arr[8];
    for (int i = 0; i < 8; i++) {
        ptr_arr[i] = &expr1 + i;
    }
    
    /* Use all variables in final computation to prevent elimination */
    int result = (tmp1 ^ tmp2) | (tmp3 & tmp4);
    result += (v1 + v2 + v3 + v4 + v5);
    result += (v6 ^ v7 ^ v8 ^ v9 ^ v10);
    result += (v11 | v12 | v13 | v14 | v15);
    result += (v16 & v17 & v18 & v19 & v20);
    result += (v21 * v22 * v23 * v24 * v25) & 0xFF;
    result += (v26 + v27 + v28 + v29 + v30);
    
    /* Final inline assembly to ensure values are used */
    asm volatile("" : "+r"(result) : : "cc", "memory");
    
    return result & global_mask;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base, int iter) {
    /* Chain of dependent calculations */
    int a = base;
    int b = a * iter;
    int c = b >> 2;
    int d = c ^ a;
    int e = d & b;
    int f = e | c;
    int g = f - d;
    int h = g * 3;
    int i = h << 1;
    int j = i ^ f;
    int k = j & 0xFF;
    int l = k | g;
    int m = l - h;
    int n = m * iter;
    int o = n >> 3;
    int p = o ^ l;
    int q = p & m;
    int r = q | n;
    int s = r - p;
    int t = s * 5;
    
    /* Use inline assembly with memory constraint */
    int u;
    asm volatile("movl %1, %%eax\n\t"
                 "addl %2, %%eax\n\t"
                 "movl %%eax, %0"
                 : "=r"(u)
                 : "m"(t), "r"(s)
                 : "%eax", "cc");
    
    return (u + a + b + c + d + e + f + g + h + i + 
            j + k + l + m + n + o + p + q + r + s + t) & 0xFFFF;
}

int main(int argc, char *argv[]) {
    /* Use argc to create variable loop count */
    volatile int loop_limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (loop_limit < 10) loop_limit = 10;
    if (loop_limit > 1000) loop_limit = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure functions are called multiple times */
    for (volatile int i = 0; i < loop_limit; i++) {
        /* Create varying inputs to prevent optimization */
        int input1 = global_seed + i;
        int input2 = argc * i;
        int input3 = (int)((long)argv & 0xFF) + i;
        
        /* Call high-pressure functions */
        int res1 = create_reload_pressure(input1, input2, input3);
        int res2 = secondary_pressure(res1, i + 1);
        
        /* Combine results */
        total_result ^= res1;
        total_result += res2;
        total_result &= 0xFFFFFF;
        
        /* Modify global to prevent loop optimization */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%06x\n", total_result);
    
    return total_result & 0xFF;
}
