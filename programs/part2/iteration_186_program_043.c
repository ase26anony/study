/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Complex test function with high register pressure and control flow */
static int __attribute__((noinline)) test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mixed types for additional pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* ptr0 = &v0, *ptr1 = &v1;
    
    int result = 0;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v3 * v4;
        v2 = v5 - v6;
        v3 = v7 / (v8 + 1);
        v4 = v9 ^ v10;
        v5 = v11 | v12;
        v6 = v13 & v14;
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 * v20;
        
        /* Inline assembly with register clobbering */
        /* For x86 */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v11), "=r"(v12)
            : "0"(v21), "1"(v22)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        if (v0 % 3 == 0) {
            v12 = v13 + v14;
            if (v1 > v2) {
                v13 = v15 * v16;
                /* Another inline asm with different clobbers */
                asm volatile (
                    "# More pressure\n"
                    : "+r"(v14), "+r"(v15)
                    :
                    : "esi", "edi", "memory"
                );
            } else {
                v13 = v17 - v18;
                goto label_a;  /* Create control flow complexity */
            }
        } else if (v0 % 3 == 1) {
            v12 = v19 ^ v20;
            switch (v1 % 5) {
                case 0: v13 = v21 + v22; break;
                case 1: v13 = v23 - v24; break;
                case 2: v13 = v25 * v26; break;
                case 3: v13 = v27 / (v28 + 1); break;
                case 4: v13 = v29 ^ v0; break;
                default: v13 = 1; break;
            }
        } else {
            v12 = v23 | v24;
            /* Float operations for FP register pressure */
            f0 = f1 * 2.0f;
            d0 = d1 / 3.0;
        }
        
        /* Continue with more operations */
        v14 = v25 + v26;
        v15 = v27 * v28;
        
        label_a:
        v16 = v29 + v0;
        
        /* Another switch with many cases */
        switch (i % 8) {
            case 0: v17 = v1 + v2; break;
            case 1: v17 = v3 - v4; break;
            case 2: v17 = v5 * v6; break;
            case 3: v17 = v7 / (v8 + 1); break;
            case 4: v17 = v9 ^ v10; break;
            case 5: v17 = v11 | v12; break;
            case 6: v17 = v13 & v14; break;
            case 7: v17 = v15 << 1; break;
        }
        
        /* Pointer arithmetic for address calculations */
        ptr0 = (char*)ptr0 + v17;
        ptr1 = (char*)ptr1 + v18;
        
        /* Mix integer and float */
        v18 = (int)(f0 * 10.0f) + v19;
        v19 = (int)(d0 * 100.0) + v20;
        
        /* Complex nested loop */
        for (int j = 0; j < 3; j++) {
            v20 = v21 + v22 + j;
            if (j == 1) {
                v21 = v23 * v24;
                continue;  /* Create loop exit/entry points */
            }
            v22 = v25 - v26;
            if (j == 2) {
                break;
            }
            v23 = v27 + v28;
        }
        
        /* More operations to ensure all variables are used */
        v24 = v29 ^ v0;
        v25 = v1 | v2;
        v26 = v3 & v4;
        v27 = v5 << 3;
        v28 = v6 >> 2;
        v29 = v7 + v8;
        
        /* Accumulate result */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                  v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                  v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    }
    
    return result;
}

/* Another complex function to prevent interprocedural optimization */
static int __attribute__((noinline)) test_mcf_control_flow(int base) {
    volatile int a = base, b = base + 1, c = base + 2;
    volatile int d = base + 3, e = base + 4, f = base + 5;
    int sum = 0;
    
    /* Deep if-else chain */
    if (a > 0) {
        b = c * d;
        if (b < 100) {
            c = d + e;
            goto middle;
        } else {
            d = e - f;
        }
    } else if (a < 0) {
        e = f * a;
        if (e > 50) {
            f = a + b;
        } else {
            a = b - c;
        }
    } else {
        b = c + d;
    }
    
    middle:
    /* Switch with many cases */
    switch (a % 10) {
        case 0: sum += b; break;
        case 1: sum += c; break;
        case 2: sum += d; break;
        case 3: sum += e; break;
        case 4: sum += f; break;
        case 5: sum += a + b; break;
        case 6: sum += c + d; break;
        case 7: sum += e + f; break;
        case 8: sum += a * b; break;
        case 9: sum += c * d; break;
    }
    
    /* Loop with multiple exit points */
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            a = b + c;
            if (a > 100) break;
        }
        if (i == 3) {
            b = c + d;
            continue;
        }
        if (i == 4) {
            goto final;
        }
        c = d + e;
    }
    
    final:
    /* Final inline asm with many clobbers */
    asm volatile (
        "# Final register pressure\n"
        : "+r"(a), "+r"(b), "+r"(c)
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "memory"
    );
    
    return sum + a + b + c + d + e + f;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pressure(i, 5);
        total += test_mcf_control_flow(i * 10);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
