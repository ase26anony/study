/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_coverage(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    /* Mix different types to increase pressure */
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile double d0 = seed * 0.01, d1 = seed * 0.02;
    volatile void* ptr0 = &v0, *ptr1 = &v1;
    
    int result = 0;
    
    /* Complex control flow with many basic blocks */
    for (int i = 0; i < 100; i++) {
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
            "# Force register pressure\n\t"
            : 
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3), "r" (v4)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
        
        /* Complex conditional chain creating many basic blocks */
        switch (i % 13) {
            case 0: v11 = v21 + v22; break;
            case 1: v12 = v23 - v24; break;
            case 2: v13 = v25 * v26; break;
            case 3: v14 = v27 / (v28 + 1); break;
            case 4: v15 = v29 ^ v0; break;
            case 5: v16 = v1 | v2; break;
            case 6: v17 = v3 & v4; break;
            case 7: v18 = v5 << 1; break;
            case 8: v19 = v6 >> 2; break;
            case 9: v20 = v7 + v8; break;
            case 10: v21 = v9 * v10; break;
            case 11: v22 = v11 - v12; break;
            case 12: v23 = v13 / (v14 + 1); break;
        }
        
        /* Nested conditionals */
        if (i & 1) {
            if (i & 2) {
                v24 = v15 + v16;
                if (i & 4) {
                    v25 = v17 * v18;
                    goto label1;
                } else {
                    v26 = v19 - v20;
                }
            } else {
                v27 = v21 ^ v22;
            }
        } else {
            if (i & 8) {
                v28 = v23 | v24;
            } else {
                v29 = v25 & v26;
            }
        }
        
        label1:
        /* More arithmetic with floating point mixing */
        f0 = f1 * 1.1f + v0;
        f1 = f2 / 2.0f - v1;
        f2 = f0 + f1 + v2;
        
        d0 = d1 * 1.01 + v3;
        d1 = d0 / 1.5 + v4;
        
        /* Pointer arithmetic */
        ptr0 = (char*)ptr1 + v5;
        ptr1 = (char*)ptr0 - v6;
        
        /* Another inline assembly with different clobbers */
        asm volatile (
            "# More register pressure\n\t"
            : 
            : "r" (v10), "r" (v11), "r" (v12), "r" (v13), "r" (v14),
              "r" (v15), "r" (v16)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Loop with break/continue creating exit-like regions */
        for (int j = 0; j < 5; j++) {
            if (j == 2 && (i % 7 == 0)) {
                v0 = v1 + j;
                continue;
            }
            if (j == 4 && (i % 11 == 0)) {
                v1 = v2 * j;
                break;
            }
            v2 = v3 + v4 + j;
        }
        
        /* Compute checksum */
        result += v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9;
        result += v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19;
        result += v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29;
        result += (int)f0 ^ (int)f1 ^ (int)f2;
        result += (int)d0 ^ (int)d1;
        result += (long)ptr0 ^ (long)ptr1;
        
        NO_OPTIMIZE(result);
    }
    
    return result;
}

/* Wrapper with even more complexity */
static int __attribute__((noinline))
complex_wrapper(int base) {
    int total = 0;
    
    /* Multiple calls with different seeds */
    for (int k = 0; k < 10; k++) {
        int r = test_mcf_coverage(base + k * 17);
        
        /* Complex post-processing */
        switch (k % 8) {
            case 0: total += r * 2; break;
            case 1: total += r / 3; break;
            case 2: total += r ^ 0xABCD; break;
            case 3: total += r | 0x1234; break;
            case 4: total += r & 0xF0F0; break;
            case 5: total += r << 3; break;
            case 6: total += r >> 2; break;
            case 7: total += ~r; break;
        }
        
        /* Create more control flow */
        if (k & 1) {
            if (total > 1000) {
                total -= 500;
                goto adjust;
            }
        } else {
            if (total < 0) {
                total = -total;
            }
        }
        
        adjust:
        /* Force spill/reload */
        asm volatile (
            "# Force memory operations\n\t"
            : 
            : "r" (total)
            : "memory"
        );
    }
    
    return total;
}

int main(void) {
    int final_result = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call multiple times to prevent interprocedural optimization */
    for (int run = 0; run < 5; run++) {
        int r = complex_wrapper(run * 1000 + 12345);
        final_result ^= r;
        printf("Run %d: result = %d\n", run, r);
    }
    
    printf("Final checksum: %d\n", final_result);
    printf("Compile with: gcc -O2 -fdump-rtl-mcf -fdump-rtl-mcf-details mcf_coverage.c\n");
    printf("Or: gcc -O3 -fdump-rtl-all -fno-peephole2 mcf_coverage.c\n");
    
    return final_result != 0 ? 0 : 1;
}
