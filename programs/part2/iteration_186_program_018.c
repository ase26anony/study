/* test_mcf_coverage.c
 * Designed to trigger GCC's Minimum Cost Flow pass debugging output
 * Compile with: gcc -O2 -fdump-rtl-mcf -fdump-rtl-mcf-details -fno-schedule-insns test_mcf_coverage.c -o test_mcf_coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep variables alive across basic blocks */
#define KEEP_ALIVE(var) asm volatile("" : "+r"(var))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) complex_mcf_test(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0xABCD;
    volatile int v5 = seed & 0xF0F0;
    volatile int v6 = seed << 3;
    volatile int v7 = seed >> 2;
    volatile int v8 = seed % 17;
    volatile int v9 = seed * seed;
    volatile int v10 = seed + 100;
    volatile int v11 = seed - 50;
    volatile int v12 = seed * 3;
    volatile int v13 = seed / 5;
    volatile int v14 = seed ^ 0x5678;
    volatile int v15 = seed | 0xDCBA;
    volatile int v16 = seed & 0x0F0F;
    volatile int v17 = seed << 1;
    volatile int v18 = seed >> 4;
    volatile int v19 = seed % 23;
    volatile int v20 = seed * 7;
    volatile int v21 = seed + 200;
    volatile int v22 = seed - 100;
    volatile int v23 = seed * 11;
    volatile int v24 = seed / 7;
    volatile int v25 = seed ^ 0x9ABC;
    volatile int v26 = seed | 0xEFEF;
    volatile int v27 = seed & 0xAAAA;
    volatile int v28 = seed << 2;
    volatile int v29 = seed >> 1;
    
    /* Mix different types to increase pressure */
    volatile float f0 = seed * 1.5f;
    volatile float f1 = seed / 2.0f;
    volatile double d0 = seed * 3.14159;
    volatile void* p0 = &v0;
    volatile void* p1 = &v1;
    
    int result = 0;
    int i, j;
    
    /* Complex loop with many basic blocks */
    for (i = 0; i < 100; i++) {
        /* Create data dependencies across variables */
        v0 = v1 + v2;
        v1 = v3 - v4;
        v2 = v5 * v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << (i & 3);
        v8 = v16 >> (i & 2);
        
        /* Inline assembly with register clobbering */
        /* Force compiler to work around clobbered registers */
        asm volatile (
            "# Force register pressure\n"
            : 
            : "r"(v0), "r"(v1), "r"(v2), "r"(v3)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v9 = v17 + v18;
            v10 = v19 * v20;
            
            /* Nested conditionals */
            if (v9 > v10) {
                v11 = v21 - v22;
                asm volatile("# Branch 1" : : : "esi", "edi");
            } else {
                v11 = v22 - v21;
                asm volatile("# Branch 2" : : : "ebp", "esp");
            }
        } else if (i % 3 == 1) {
            v12 = v23 / (v24 + 1);
            v13 = v25 ^ v26;
            
            /* Another inline asm with different clobbers */
            asm volatile (
                "movl %0, %%eax\n"
                "addl %1, %%eax\n"
                : 
                : "r"(v12), "r"(v13)
                : "eax", "cc"
            );
        } else {
            v14 = v27 | v28;
            v15 = v29 & i;
            
            /* Force spill/reload */
            for (j = 0; j < 5; j++) {
                v14 = v14 + j;
                v15 = v15 - j;
                KEEP_ALIVE(v14);
                KEEP_ALIVE(v15);
            }
        }
        
        /* Switch statement with many cases - creates multiple basic blocks */
        switch (i % 10) {
            case 0:
                v16 = v0 + v1;
                asm volatile("# Case 0" : : : "eax");
                break;
            case 1:
                v17 = v2 - v3;
                asm volatile("# Case 1" : : : "ebx");
                break;
            case 2:
                v18 = v4 * v5;
                asm volatile("# Case 2" : : : "ecx");
                break;
            case 3:
                v19 = v6 / (v7 + 1);
                asm volatile("# Case 3" : : : "edx");
                break;
            case 4:
                v20 = v8 ^ v9;
                asm volatile("# Case 4" : : : "esi");
                break;
            case 5:
                v21 = v10 | v11;
                asm volatile("# Case 5" : : : "edi");
                break;
            case 6:
                v22 = v12 & v13;
                asm volatile("# Case 6" : : : "ebp");
                break;
            case 7:
                v23 = v14 << 1;
                asm volatile("# Case 7" : : : "r8");
                break;
            case 8:
                v24 = v15 >> 2;
                asm volatile("# Case 8" : : : "r9");
                break;
            case 9:
                v25 = v16 % 17;
                asm volatile("# Case 9" : : : "r10");
                break;
        }
        
        /* Use all variables to prevent elimination */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Mix in float/double/pointer operations */
        f0 = f0 + i * 0.1f;
        f1 = f1 - i * 0.2f;
        d0 = d0 * 1.01;
        p0 = (char*)p0 + 1;
        p1 = (char*)p1 - 1;
        
        /* Use goto to create additional control flow edges */
        if (i % 7 == 0) {
            goto special_handler;
        }
        
        continue;
        
    special_handler:
        /* Compensation code block */
        v26 = v27 ^ v28;
        v27 = v29 | i;
        asm volatile(
            "# Special handler\n"
            : 
            : "r"(v26), "r"(v27)
            : "rax", "rbx", "rcx", "memory"
        );
    }
    
    /* Final computation using all variables */
    result = result ^ v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5;
    result = result | v6 | v7 | v8 | v9 | v10;
    result = result & v11 & v12 & v13 & v14 & v15;
    result = result + v16 + v17 + v18 + v19 + v20;
    result = result - v21 - v22 - v23 - v24 - v25;
    result = result * (v26 % 31) * (v27 % 37);
    result = result / ((v28 & 0xFF) + 1);
    result = result ^ v29;
    
    /* Use float results */
    result += (int)f0 + (int)f1 + (int)d0;
    
    /* Use pointer results */
    result += ((long)p0 % 1000) + ((long)p1 % 1000);
    
    return result;
}

/* Another complex function to increase overall compilation complexity */
static int __attribute__((noinline)) secondary_test(int x) {
    volatile int a = x * 3;
    volatile int b = x / 2;
    volatile int c = x + 100;
    volatile int d = x - 50;
    
    int sum = 0;
    
    /* Loop with early exits creating multiple exit blocks */
    for (int i = 0; i < 50; i++) {
        if (i % 11 == 0) {
            a = b + c;
            asm volatile("# Early exit path 1" : : : "eax", "ebx");
            if (a > 1000) break;
        }
        
        if (i % 13 == 0) {
            b = c - d;
            asm volatile("# Early exit path 2" : : : "ecx", "edx");
            if (b < 0) return sum;
        }
        
        if (i % 17 == 0) {
            c = d * a;
            asm volatile("# Early exit path 3" : : : "esi", "edi");
            if (c > 10000) goto final_calc;
        }
        
        sum += a + b + c + d;
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 3; j++) {
            d = (d + j) * 2;
            KEEP_ALIVE(d);
        }
    }
    
final_calc:
    return sum * 2;
}

int main(void) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        int r1 = complex_mcf_test(i * 100 + 123);
        int r2 = secondary_test(i * 50 + 456);
        
        total += r1 + r2;
        
        printf("Iteration %d: r1=%d, r2=%d, total=%d\n", 
               i, r1, r2, total);
    }
    
    printf("Final result: %d\n", total);
    
    /* Ensure result is used */
    if (total == 0) {
        printf("Unexpected zero result\n");
        return 1;
    }
    
    return 0;
}
