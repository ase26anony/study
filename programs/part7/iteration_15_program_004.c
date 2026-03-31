/* test_reload.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 13579;

/* Non-inlineable function to create maximum register pressure */
__attribute__((noinline, noipa))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables with different types */
    int v1 = input1;
    int v2 = input2;
    int v3 = input3;
    short v4 = (short)(input1 + 1);
    short v5 = (short)(input2 - 1);
    char v6 = (char)(input3 ^ 0xFF);
    long v7 = (long)input1 * input2;
    long v8 = (long)input2 * input3;
    int v9 = v1 + v2;
    int v10 = v2 - v3;
    int v11 = v3 * v1;
    short v12 = (short)(v4 | v5);
    char v13 = (char)(v6 & 0x7F);
    long v14 = v7 ^ v8;
    int v15 = v9 << 2;
    int v16 = v10 >> 1;
    int v17 = v11 & 0xFFFF;
    short v18 = (short)(v12 + v13);
    char v19 = (char)(v13 - v6);
    long v20 = v14 | v7;
    int v21 = v15 * v16;
    int v22 = v17 / (v1 ? v1 : 1);
    int v23 = v18 * v19;
    int v24 = v20 & 0xFFFFFFFF;
    int v25 = v21 ^ v22;
    int v26 = v23 | v24;
    int v27 = v25 - v26;
    int v28 = v27 + v15;
    int v29 = v28 * v16;
    int v30 = v29 & v17;
    
    /* Complex expressions requiring multiple registers */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v2 = v6 * v7 - v8 / (v9 ? v9 : 1);
    v3 = (v10 ^ v11) + (v12 | v13);
    v4 = (short)((v14 & 0xFFFF) + (v15 >> 4));
    v5 = (short)(v16 * v17 - v18);
    v6 = (char)((v19 << 2) | (v20 & 0xFF));
    
    /* More complex operations with data dependencies */
    v7 = (v21 * v22) + (v23 << (v24 & 3)) - v25;
    v8 = (v26 | v27) ^ (v28 & v29) + v30;
    v9 = v1 * v2 - v3 / (v4 ? v4 : 1);
    v10 = (v5 << 3) | (v6 & 0x1F);
    v11 = v7 ^ v8 + v9 * v10;
    v12 = (short)(v11 & 0xFFFF);
    v13 = (char)((v12 >> 8) & 0xFF);
    
    /* Inline assembly to force specific register usage */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+r"(v4), "+r"(v5), "+r"(v6) : : "cc", "memory");
    
    /* More operations keeping values live */
    v14 = v1 * v2 + v3 * v4 - v5 * v6;
    v15 = (v7 & v8) | (v9 ^ v10);
    v16 = v11 << (v12 & 3);
    v17 = v13 * v14 / (v15 ? v15 : 1);
    v18 = (short)(v16 | v17);
    v19 = (char)(v18 & 0x7F);
    
    /* Additional complex expressions */
    v20 = (v1 + v2) * (v3 - v4) / (v5 + 1);
    v21 = (v6 << 4) | (v7 >> 4);
    v22 = v8 ^ v9 & v10 | v11;
    v23 = v12 * v13 + v14 - v15;
    v24 = v16 & v17 | v18 ^ v19;
    v25 = v20 * v21 - v22 * v23;
    v26 = v24 | v25;
    v27 = v26 << 2;
    v28 = v27 >> 1;
    v29 = v28 & 0x0F0F0F0F;
    v30 = v29 ^ 0xFFFFFFFF;
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6;
    result += v7 ^ v8 ^ v9 ^ v10 ^ v11 ^ v12;
    result += v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18;
    result += v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24;
    result += v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    
    /* More inline assembly */
    asm volatile("" : "+r"(result) : : "cc");
    
    return result;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable naming pattern */
    int a = base;
    int b = base + 1;
    int c = base * 2;
    int d = base - 1;
    int e = base ^ 0xAA;
    int f = base | 0x55;
    int g = base & 0xFF;
    int h = base << 3;
    int i = base >> 2;
    int j = a + b;
    int k = c - d;
    int l = e * f;
    int m = g ^ h;
    int n = i | j;
    int o = k & l;
    int p = m + n;
    int q = o - p;
    int r = q * a;
    int s = r / (b ? b : 1);
    int t = s ^ c;
    
    /* Complex addressing-like operations */
    int* ptr = &a;
    volatile int mem1 = *ptr;
    volatile int mem2 = *(ptr + 1);
    
    /* Operations that might need address registers */
    int addr1 = (int)(ptr);
    int addr2 = addr1 + sizeof(int) * 5;
    
    /* Force memory accesses */
    asm volatile("" : : "r"(addr1), "r"(addr2) : "memory");
    
    /* More computations */
    int u = t << (mem1 & 3);
    int v = mem2 >> 2;
    int w = u | v;
    int x = w & 0x0F0F0F0F;
    int y = x ^ 0x12345678;
    int z = y + a + b + c + d + e + f + g + h + i + j;
    
    return z;
}

int main(int argc, char** argv) {
    /* Use command line arguments to create variable inputs */
    int limit = (argc > 1) ? atoi(argv[1]) : 100;
    if (limit < 1) limit = 1;
    if (limit > 1000) limit = 1000;
    
    /* Volatile loop counter to prevent optimization */
    volatile int volatile_limit = limit;
    
    int total_result = 0;
    
    /* Loop to ensure multiple executions */
    for (volatile int i = 0; i < volatile_limit; i++) {
        /* Mix inputs from various sources */
        int input1 = global_seed1 + i;
        int input2 = global_seed2 - i;
        int input3 = global_seed3 ^ i;
        
        /* Call high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3);
        int result2 = secondary_pressure(result1 + i);
        
        /* Combine results */
        total_result ^= result1;
        total_result += result2;
        
        /* Modify globals slightly */
        global_seed1 ^= result1;
        global_seed2 += result2;
        global_seed3 = global_seed3 * 1103515245 + 12345;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: 0x%08x\n", total_result & 0xFFFFFFFF);
    
    return (total_result & 0xFF);
}
