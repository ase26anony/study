/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex control flow and register pressure */
#define USE_VOLATILE(var) do { asm volatile("" : "+r" (var)); } while(0)

/* Large test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed;
    int v1 = v0 * 2;
    int v2 = v1 + 1;
    int v3 = v2 ^ v0;
    int v4 = v3 << 1;
    int v5 = v4 - v2;
    int v6 = v5 | v3;
    int v7 = v6 & 0xFF;
    int v8 = v7 * 3;
    int v9 = v8 / 2;
    int v10 = v9 % 7;
    int v11 = v10 + v0;
    int v12 = v11 ^ v1;
    int v13 = v12 << 2;
    int v14 = v13 >> 1;
    int v15 = v14 | v2;
    int v16 = v15 & v3;
    int v17 = v16 * 5;
    int v18 = v17 - v4;
    int v19 = v18 ^ v5;
    int v20 = v19 + v6;
    int v21 = v20 * 7;
    int v22 = v21 / 3;
    int v23 = v22 % 11;
    int v24 = v23 | v7;
    int v25 = v24 & v8;
    int v26 = v25 ^ v9;
    int v27 = v26 + v10;
    int v28 = v27 * 13;
    int v29 = v28 - v11;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        /* Inline assembly to clobber registers and force graph transformations */
        asm volatile (
            "# MCF test - clobber multiple registers\n"
            "movl %%eax, %%ebx\n"
            "movl %%ecx, %%edx\n"
            : 
            : 
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Deep conditional chain creating many basic blocks */
        if (v0 & 0x01) {
            v1 = v29 ^ i;
            if (v1 > v2) {
                v3 = v1 * v2;
                /* Force register spill/reload */
                USE_VOLATILE(v3);
            } else {
                v4 = v2 / (v1 ? v1 : 1);
                asm volatile("# Branch A" : : : "esi", "edi");
            }
        } else if (v0 & 0x02) {
            v5 = v28 | i;
            switch (v5 % 8) {
                case 0: v6 = v27 + 1; break;
                case 1: v6 = v26 - 1; break;
                case 2: v6 = v25 * 2; break;
                case 3: v6 = v24 / 2; break;
                case 4: v6 = v23 ^ 0xFF; break;
                case 5: v6 = v22 | 0xAA; break;
                case 6: v6 = v21 & 0x55; break;
                case 7: v6 = v20 << 1; break;
                default: v6 = 0;
            }
            asm volatile("# Switch case" : : : "r8", "r9", "r10");
        } else if (v0 & 0x04) {
            /* Nested loops with register pressure */
            for (int j = 0; j < 3; j++) {
                v7 = (v19 + j) * (v18 - j);
                v8 = v7 ^ v17;
                USE_VOLATILE(v8);
            }
        }
        
        /* More arithmetic creating data dependencies */
        v9 = v0 + v1 + v2 + v3;
        v10 = v4 * v5 * v6;
        v11 = v7 ^ v8 ^ v9;
        v12 = v10 - v11 + v12;
        
        /* Another complex conditional region */
        if (i % 3 == 0) {
            v13 = v12 << (i & 0x3);
            goto label1;
        } else if (i % 3 == 1) {
            v14 = v13 >> (i & 0x3);
            goto label2;
        } else {
            v15 = v14 | v13;
            /* Fall through */
        }
        
        /* Labels creating additional control flow edges */
        label1:
        v16 = v15 & 0xFFFF;
        asm volatile("# Label1" : : : "r11", "r12");
        
        label2:
        v17 = v16 ^ 0xAAAA;
        
        /* Mix float operations to pressure different register classes */
        {
            float f1 = v17 * 0.5f;
            float f2 = f1 * f1;
            int vi = (int)f2;
            v18 = v17 + vi;
            USE_VOLATILE(f1);
            USE_VOLATILE(f2);
        }
        
        /* Pointer arithmetic for additional pressure */
        {
            int arr[4] = {v18, v19, v20, v21};
            int *ptr = arr;
            v19 = *(ptr + (i & 0x3));
            asm volatile("# Pointer access" : : : "r13", "r14", "r15");
        }
        
        /* Update all variables to keep them live */
        v20 = v19 + v18;
        v21 = v20 * v19;
        v22 = v21 ^ v20;
        v23 = v22 | v21;
        v24 = v23 & v22;
        v25 = v24 - v23;
        v26 = v25 + v24;
        v27 = v26 * v25;
        v28 = v27 ^ v26;
        v29 = v28 | v27;
        
        /* Accumulate result with complex expression */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        
        /* Rotate values to create changing patterns */
        int temp = v0;
        v0 = v29;
        v29 = v28;
        v28 = v27;
        v27 = v26;
        v26 = v25;
        v25 = v24;
        v24 = v23;
        v23 = v22;
        v22 = v21;
        v21 = v20;
        v20 = v19;
        v19 = v18;
        v18 = v17;
        v17 = v16;
        v16 = v15;
        v15 = v14;
        v14 = v13;
        v13 = v12;
        v12 = v11;
        v11 = v10;
        v10 = v9;
        v9 = v8;
        v8 = v7;
        v7 = v6;
        v6 = v5;
        v5 = v4;
        v4 = v3;
        v3 = v2;
        v2 = v1;
        v1 = temp;
    }
    
    /* Final complex computation */
    result = (result ^ v0) | (v1 & v2) | (v3 ^ v4) | (v5 & v6);
    
    /* More inline assembly to force graph fixups */
    asm volatile (
        "# Final MCF transformation trigger\n"
        "pushl %%eax\n"
        "popl %%eax\n"
        : 
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    return result;
}

/* Second test function with different patterns */
static int __attribute__((noinline))
test_mcf_alternate(int base) {
    volatile int a = base;
    int b = a + 1, c = b * 2, d = c ^ a, e = d << 1;
    
    /* Large switch statement creating many basic blocks */
    switch (base % 12) {
        case 0: a = b + c; break;
        case 1: a = c - d; break;
        case 2: a = d * e; break;
        case 3: a = e ^ b; break;
        case 4: a = b & c; break;
        case 5: a = c | d; break;
        case 6: a = d << 2; break;
        case 7: a = e >> 1; break;
        case 8: a = ~b; break;
        case 9: a = -c; break;
        case 10: a = b ^ c ^ d; break;
        case 11: a = (b + c) * d; break;
    }
    
    /* Loop with early exit creating exit-like regions */
    for (int i = 0; i < 100; i++) {
        if (i == a % 50) {
            asm volatile("# Early exit" : : : "rax", "rbx");
            goto early_exit;
        }
        b = (b + i) % 1000;
        if (i % 7 == 0) continue;
        c = c ^ i;
        if (c > 1000) break;
    }
    
    early_exit:
    
    /* Compute final value using all variables */
    return a + b + c + d + e;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total ^= test_mcf_pressure(i, 5 + (i % 3));
        total += test_mcf_alternate(i * 7);
        
        /* Prevent optimization of loop */
        asm volatile("# Main loop iteration %0" : : "r" (i) : "memory");
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
