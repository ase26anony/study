/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to generate complex flow graph with many basic blocks */
__attribute__((noinline))
static unsigned long test_mcf_pressure(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed, v1 = seed + 1, v2 = seed + 2, v3 = seed + 3;
    volatile int v4 = seed + 4, v5 = seed + 5, v6 = seed + 6, v7 = seed + 7;
    volatile int v8 = seed + 8, v9 = seed + 9, v10 = seed + 10, v11 = seed + 11;
    volatile int v12 = seed + 12, v13 = seed + 13, v14 = seed + 14, v15 = seed + 15;
    volatile int v16 = seed + 16, v17 = seed + 17, v18 = seed + 18, v19 = seed + 19;
    volatile int v20 = seed + 20, v21 = seed + 21, v22 = seed + 22, v23 = seed + 23;
    volatile int v24 = seed + 24, v25 = seed + 25, v26 = seed + 26, v27 = seed + 27;
    volatile int v28 = seed + 28, v29 = seed + 29;
    
    volatile float f0 = seed * 0.1f, f1 = seed * 0.2f, f2 = seed * 0.3f;
    volatile float f3 = seed * 0.4f, f4 = seed * 0.5f;
    
    volatile void* p0 = &v0, *p1 = &v1, *p2 = &v2, *p3 = &v3;
    
    unsigned long checksum = 0;
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
        v7 = v15 << 2;
        v8 = v16 >> 1;
        v9 = v17 + v18;
        v10 = v19 - v20;
        
        /* Mix float operations */
        f0 = f1 * f2;
        f1 = f3 + f4;
        f2 = f0 - f1;
        
        /* Pointer arithmetic */
        p0 = (char*)p1 + v0;
        p1 = (char*)p2 - v1;
        
        /* Complex conditional chain creating many basic blocks */
        if (i % 3 == 0) {
            v11 = v21 * v22;
            if (i % 5 == 0) {
                v12 = v23 + v24;
                if (i % 7 == 0) {
                    v13 = v25 - v26;
                } else {
                    v13 = v26 - v25;
                }
            } else {
                v12 = v24 - v23;
            }
        } else if (i % 3 == 1) {
            v11 = v22 * v21;
            /* Inline asm with register clobbers to force graph transformations */
            asm volatile (
                "# Force register pressure\n"
                : "=r"(v14), "=r"(v15)
                : "0"(v27), "1"(v28)
                : "eax", "ebx", "ecx", "edx", "memory"
            );
        } else {
            v11 = v21 + v22;
            /* Another asm with different clobbers */
            asm volatile (
                "# More register pressure\n"
                : "=r"(v16), "=r"(v17)
                : "0"(v28), "1"(v29)
                : "esi", "edi", "ebp", "memory"
            );
        }
        
        /* Switch statement with many cases - creates multiple basic blocks */
        switch (i % 13) {
            case 0: v18 = v0 + v1; break;
            case 1: v18 = v1 + v2; break;
            case 2: v18 = v2 + v3; break;
            case 3: v18 = v3 + v4; break;
            case 4: v18 = v4 + v5; break;
            case 5: v18 = v5 + v6; break;
            case 6: v18 = v6 + v7; break;
            case 7: v18 = v7 + v8; break;
            case 8: v18 = v8 + v9; break;
            case 9: v18 = v9 + v10; break;
            case 10: v18 = v10 + v11; break;
            case 11: v18 = v11 + v12; break;
            case 12: v18 = v12 + v13; break;
        }
        
        /* Nested loops with breaks/continues */
        for (j = 0; j < 10; j++) {
            if (j % 2 == 0) {
                v19 = v18 * j;
                if (v19 > 100) {
                    v20 = v19 / 2;
                    continue;
                }
            } else {
                v20 = v18 + j;
                if (v20 < 50) {
                    break;
                }
            }
            v21 = v19 + v20;
        }
        
        /* Goto labels to create additional control flow edges */
        if (v21 > 1000) {
            goto large_value;
        } else {
            v22 = v21 * 2;
            goto continue_loop;
        }
        
    large_value:
        v22 = v21 / 2;
        
    continue_loop:
        /* More arithmetic with all variables */
        v23 = v0 + v1 + v2 + v3 + v4 + v5;
        v24 = v6 + v7 + v8 + v9 + v10 + v11;
        v25 = v12 + v13 + v14 + v15 + v16 + v17;
        v26 = v18 + v19 + v20 + v21 + v22;
        
        /* Update checksum */
        checksum += v0 ^ v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9;
        checksum += v10 ^ v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17;
        checksum += v18 ^ v19 ^ v20 ^ v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26;
        checksum += (unsigned long)(f0 * 1000) ^ (unsigned long)(f1 * 1000);
        
        /* Force side effects */
        asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
    }
    
    return checksum;
}

/* Another complex function to ensure MCF runs multiple times */
__attribute__((noinline))
static unsigned long test_mcf_pressure2(int seed) {
    volatile int a0 = seed, a1 = seed * 2, a2 = seed * 3, a3 = seed * 4;
    volatile int a4 = seed * 5, a5 = seed * 6, a6 = seed * 7, a7 = seed * 8;
    volatile int a8 = seed * 9, a9 = seed * 10, a10 = seed * 11, a11 = seed * 12;
    volatile int a12 = seed * 13, a13 = seed * 14, a14 = seed * 15, a15 = seed * 16;
    
    unsigned long sum = 0;
    int i;
    
    for (i = 0; i < 50; i++) {
        /* Complex expression with many intermediate values */
        a0 = ((a1 + a2) * (a3 - a4)) / ((a5 | a6) + 1);
        a1 = ((a7 ^ a8) << (i % 4)) + ((a9 & a10) >> 1);
        a2 = a11 * a12 - a13 / (a14 + 1);
        
        /* Conditional with both sides having register pressure */
        if (a0 > a1) {
            a3 = a2 * 3;
            a4 = a3 + a0;
            /* Inline asm creating artificial constraints */
            asm volatile (
                "# Create compensation code needs\n"
                : "+r"(a5), "+r"(a6)
                : "r"(a0), "r"(a1)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
            );
        } else {
            a3 = a2 / 3;
            a4 = a3 - a0;
            /* Different asm with overlapping clobbers */
            asm volatile (
                "# Alternative constraint path\n"
                : "+r"(a7), "+r"(a8)
                : "r"(a0), "r"(a2)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
            );
        }
        
        sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    }
    
    return sum;
}

int main(void) {
    unsigned long total = 0;
    int i;
    
    printf("Starting MCF coverage test...\n");
    
    /* Call test functions multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        total += test_mcf_pressure(i * 17);
        total += test_mcf_pressure2(i * 23);
    }
    
    printf("Final checksum: %lu\n", total);
    return 0;
}
