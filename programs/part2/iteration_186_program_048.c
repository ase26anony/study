/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 0, v1 = seed + 1, v2 = seed + 2;
    volatile int v3 = seed + 3, v4 = seed + 4, v5 = seed + 5;
    volatile int v6 = seed + 6, v7 = seed + 7, v8 = seed + 8;
    volatile int v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14;
    volatile int v15 = seed + 15, v16 = seed + 16, v17 = seed + 17;
    volatile int v18 = seed + 18, v19 = seed + 19, v20 = seed + 20;
    volatile int v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26;
    volatile int v27 = seed + 27, v28 = seed + 28, v29 = seed + 29;
    
    /* Additional variables with different types to increase pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* p0 = &v0, *p1 = &v1;
    
    int result = 0;
    
    /* Complex loop with many iterations */
    for (int i = 0; i < iterations; i++) {
        /* Create data dependencies between variables */
        v0 = v1 + v2;
        v1 = v2 * v3;
        v2 = v3 - v4;
        v3 = v4 ^ v5;
        v4 = v5 | v6;
        v5 = v6 & v7;
        v6 = v7 + v8;
        v7 = v8 * v9;
        v8 = v9 - v10;
        v9 = v10 ^ v11;
        
        /* Mix in floating point operations */
        f0 = f1 * 1.1f;
        f1 = f0 / 2.0f;
        d0 = d1 * 1.01;
        d1 = d0 / 2.0;
        
        /* Complex conditional structure creating many basic blocks */
        if (i % 3 == 0) {
            v10 = v11 + v12;
            v11 = v12 * v13;
            
            /* Inline assembly with register clobbers to force graph modifications */
            asm volatile(
                "# Force register pressure\n\t"
                : 
                : "r"(v10), "r"(v11)
                : "eax", "ebx", "ecx", "edx", "esi", "edi"
            );
        } else if (i % 3 == 1) {
            v12 = v13 - v14;
            v13 = v14 ^ v15;
            
            /* Different clobber set */
            asm volatile(
                "# More register pressure\n\t"
                : 
                : "r"(v12), "r"(v13)
                : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        } else {
            v14 = v15 | v16;
            v15 = v16 & v17;
        }
        
        /* Nested switch with many cases */
        switch (i % 7) {
            case 0:
                v16 = v17 + v18;
                v17 = v18 * v19;
                break;
            case 1:
                v18 = v19 - v20;
                v19 = v20 ^ v21;
                break;
            case 2:
                v20 = v21 | v22;
                v21 = v22 & v23;
                break;
            case 3:
                v22 = v23 + v24;
                v23 = v24 * v25;
                break;
            case 4:
                v24 = v25 - v26;
                v25 = v26 ^ v27;
                break;
            case 5:
                v26 = v27 | v28;
                v27 = v28 & v29;
                break;
            case 6:
                v28 = v29 + v0;
                v29 = v0 * v1;
                break;
        }
        
        /* More arithmetic chains */
        v0 = v0 + v1 - v2 * v3 / (v4 + 1);
        v1 = v1 ^ v2 | v3 & v4;
        v2 = (v2 << 2) | (v3 >> 3);
        v3 = v3 + v4 - v5;
        v4 = v4 * v5 / (v6 + 1);
        
        /* Use goto to create additional control flow edges */
        if (i % 11 == 0) {
            goto special_path;
        }
        
        continue;
        
    special_path:
        v5 = v6 + v7;
        v6 = v8 * v9;
        result += v5 + v6;
    }
    
    /* Final computation using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    result += v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    result += (int)f0 + (int)f1 + (int)d0 + (int)d1;
    result += (long)p0 % 256 + (long)p1 % 256;
    
    return result;
}

/* Another complex function with different patterns */
static int __attribute__((noinline))
test_mcf_control_flow(int base) {
    volatile int a = base, b = base + 1, c = base + 2;
    volatile int d = base + 3, e = base + 4, f = base + 5;
    volatile int g = base + 6, h = base + 7, i = base + 8;
    volatile int j = base + 9, k = base + 10, l = base + 11;
    
    int result = 0;
    
    /* Deeply nested conditionals */
    if (a > 0) {
        if (b > a) {
            if (c > b) {
                if (d > c) {
                    a = b + c;
                    
                    /* Force register spilling with inline asm */
                    asm volatile(
                        "# Complex clobber set\n\t"
                        : 
                        : "r"(a), "r"(b), "r"(c), "r"(d)
                        : "rax", "rbx", "rcx", "rdx", 
                          "rsi", "rdi", "r8", "r9", "r10",
                          "r11", "r12", "r13", "r14", "r15",
                          "xmm0", "xmm1", "xmm2", "xmm3"
                    );
                } else {
                    b = c + d;
                }
            } else {
                c = d + e;
            }
        } else {
            d = e + f;
        }
    } else {
        e = f + g;
    }
    
    /* Loop with break/continue creating complex CFG */
    for (int x = 0; x < 100; x++) {
        if (x % 2 == 0) {
            f = g + h;
            if (x % 4 == 0) {
                continue;
            }
        } else {
            g = h + i;
            if (x % 5 == 0) {
                break;
            }
        }
        
        h = i + j;
        i = j + k;
        j = k + l;
        
        /* Another inline asm with different constraints */
        asm volatile(
            "# More pressure\n\t"
            : "=r"(k)
            : "r"(h), "r"(i), "r"(j)
            : "memory"
        );
    }
    
    result = a + b + c + d + e + f + g + h + i + j + k + l;
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pressure(i * 100, 50);
        total += test_mcf_control_flow(i * 50);
    }
    
    printf("Result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
