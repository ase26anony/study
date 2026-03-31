/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with high register pressure and control flow */
static int __attribute__((noinline)) complex_mcf_function(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0x5678;
    volatile int v5 = seed & 0x9ABC;
    volatile int v6 = seed << 2;
    volatile int v7 = seed >> 1;
    volatile int v8 = seed + v0;
    volatile int v9 = v1 * v2;
    volatile int v10 = v3 ^ v4;
    volatile int v11 = v5 | v6;
    volatile int v12 = v7 & v8;
    volatile int v13 = v9 << 1;
    volatile int v14 = v10 >> 2;
    volatile int v15 = v11 + v12;
    volatile int v16 = v13 * v14;
    volatile int v17 = v15 ^ v16;
    volatile int v18 = v0 | v17;
    volatile int v19 = v1 & v18;
    volatile int v20 = v2 + v19;
    volatile int v21 = v3 * v20;
    volatile int v22 = v4 ^ v21;
    volatile int v23 = v5 | v22;
    volatile int v24 = v6 & v23;
    volatile int v25 = v7 + v24;
    volatile int v26 = v8 * v25;
    volatile int v27 = v9 ^ v26;
    volatile int v28 = v10 | v27;
    volatile int v29 = v11 & v28;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
        /* Nested conditionals creating control flow splits */
        if (i % 3 == 0) {
            v0 = v1 + v2;
            /* Inline assembly with register clobbers to force graph transformations */
            asm volatile (
                "# Force register pressure\n"
                : 
                : "r"(v0), "r"(v1), "r"(v2)
                : "eax", "ebx", "ecx", "edx", "esi", "edi"
            );
            
            if (i % 5 == 0) {
                v3 = v4 * v5;
                result += v3;
                
                /* More inline assembly */
                asm volatile (
                    "# Additional clobber\n"
                    : 
                    : "r"(v3), "r"(v4)
                    : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
                );
                
                /* Switch statement creating multiple basic blocks */
                switch (i % 7) {
                    case 0: v6 = v7 + v8; break;
                    case 1: v6 = v7 - v8; break;
                    case 2: v6 = v7 * v8; break;
                    case 3: v6 = v7 / (v8 ? v8 : 1); break;
                    case 4: v6 = v7 ^ v8; break;
                    case 5: v6 = v7 | v8; break;
                    case 6: v6 = v7 & v8; break;
                }
            } else {
                v9 = v10 ^ v11;
                result ^= v9;
            }
        } else if (i % 3 == 1) {
            v12 = v13 | v14;
            result |= v12;
            
            /* Loop with break/continue creating exit-like regions */
            for (int j = 0; j < 10; j++) {
                if (j % 2 == 0) {
                    v15 = v16 + j;
                    continue;
                } else {
                    v17 = v18 * j;
                    if (j == 5) break;
                }
                v19 = v20 ^ v21;
            }
        } else {
            v22 = v23 & v24;
            result &= v22;
            
            /* Goto creating additional control flow edges */
            if (v22 > 1000) {
                goto special_label;
            }
            
            v25 = v26 + v27;
            result += v25;
            
            special_label:
            v28 = v29 * seed;
            result *= v28;
        }
        
        /* Mix different types to increase pressure */
        float f1 = v0 * 1.5f;
        float f2 = v1 * 2.5f;
        volatile float f3 = f1 + f2;
        
        double d1 = v2 * 3.14159;
        double d2 = v3 * 2.71828;
        volatile double d3 = d1 - d2;
        
        /* Pointer operations */
        int* ptr1 = &v4;
        int* ptr2 = &v5;
        volatile int ptr_diff = ptr2 - ptr1;
        
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        
        /* Another inline assembly with many clobbers */
        asm volatile (
            "# Complex constraint\n"
            : "+r"(v0), "+r"(v1), "+r"(v2)
            : "r"(v3), "r"(v4), "r"(v5)
            : "memory", "cc", 
              "rax", "rbx", "rcx", "rdx", 
              "rsi", "rdi", "r8", "r9", "r10", 
              "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3",
              "xmm4", "xmm5", "xmm6", "xmm7"
        );
    }
    
    /* Final computation using all variables */
    result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
             v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
             v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int __attribute__((noinline)) another_mcf_function(int base) {
    volatile int a = base, b = base * 2, c = base * 3;
    volatile int d, e, f, g, h, i, j, k, l, m;
    
    /* Complex switch with many cases */
    switch (base % 13) {
        case 0: d = a + b; break;
        case 1: d = a - b; break;
        case 2: d = a * b; break;
        case 3: d = b / (a ? a : 1); break;
        case 4: d = a ^ b; break;
        case 5: d = a | b; break;
        case 6: d = a & b; break;
        case 7: d = ~a; break;
        case 8: d = a << 2; break;
        case 9: d = b >> 1; break;
        case 10: d = a + c; break;
        case 11: d = b - c; break;
        case 12: d = c * 3; break;
    }
    
    /* Nested loops with early exits */
    for (int x = 0; x < 50; x++) {
        e = d + x;
        for (int y = 0; y < 20; y++) {
            f = e * y;
            if (f > 1000) {
                g = f / 2;
                break;
            } else {
                g = f * 2;
                continue;
            }
            h = g + 1; /* Unreachable but creates control flow */
        }
        
        /* More inline assembly */
        asm volatile (
            "# More register pressure\n"
            : "+r"(e), "+r"(f)
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
        );
    }
    
    return d + e + f + g;
}

int main(void) {
    int total = 0;
    
    /* Call functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += complex_mcf_function(i);
        total += another_mcf_function(i * 7);
        
        /* Prevent loop unrolling */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
