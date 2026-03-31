/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
#define NO_OPTIMIZE(x) asm volatile("" : "+r" (x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_pressure(int seed, int iterations) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    int v1 = seed * 2, v2 = seed / 3, v3 = seed ^ 0x55;
    int v4 = seed << 2, v5 = seed >> 1, v6 = seed | 0xFF;
    int v7 = seed & 0xF0, v8 = seed + 100, v9 = seed - 50;
    int v10 = seed * 3, v11 = seed / 4, v12 = seed ^ 0xAA;
    int v13 = seed << 1, v14 = seed >> 2, v15 = seed | 0x0F;
    int v16 = seed & 0x0F, v17 = seed + 200, v18 = seed - 100;
    int v19 = seed * 5, v20 = seed / 5, v21 = seed ^ 0xCC;
    int v22 = seed << 3, v23 = seed >> 3, v24 = seed | 0xF0;
    int v25 = seed & 0x33, v26 = seed + 300, v27 = seed - 150;
    int v28 = seed * 7, v29 = seed / 7, v30 = seed ^ 0x33;
    
    /* Mix different types to increase pressure */
    float f1 = seed * 1.5f, f2 = seed * 0.5f;
    double d1 = seed * 2.5, d2 = seed * 0.25;
    void *ptr1 = &v0, *ptr2 = &v1;
    
    int result = 0;
    
    /* Complex loop with many basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Chain of dependent operations */
        v0 = v1 + v2;
        v1 = v3 ^ v4;
        v2 = v5 | v6;
        v3 = v7 & v8;
        v4 = v9 * v10;
        v5 = v11 / (v12 + 1);
        v6 = v13 << (v14 & 3);
        v7 = v15 >> (v16 & 3);
        
        /* Inline assembly with register clobbering */
        /* Force compiler to work around clobbered registers */
        asm volatile (
            "# Start of critical section\n\t"
            "mov %0, %0\n\t"
            "# End of critical section"
            : "+r" (v8), "+r" (v9), "+r" (v10)
            :
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Complex conditional structure creating many basic blocks */
        switch (i & 0xF) {  /* 16-way switch */
            case 0: v8 = v17 + v18; break;
            case 1: v9 = v19 - v20; break;
            case 2: v10 = v21 * v22; break;
            case 3: v11 = v23 / (v24 + 1); break;
            case 4: v12 = v25 | v26; break;
            case 5: v13 = v27 & v28; break;
            case 6: v14 = v29 ^ v30; break;
            case 7: v15 = v0 << 1; break;
            case 8: v16 = v1 >> 1; break;
            case 9: v17 = v2 + v3; break;
            case 10: v18 = v4 - v5; break;
            case 11: v19 = v6 * v7; break;
            case 12: v20 = v8 / (v9 + 1); break;
            case 13: v21 = v10 | v11; break;
            case 14: v22 = v12 & v13; break;
            case 15: v23 = v14 ^ v15; break;
        }
        
        /* Nested conditionals */
        if (i & 1) {
            if (i & 2) {
                v24 = v16 + v17;
                if (i & 4) {
                    v25 = v18 - v19;
                    goto label1;
                } else {
                    v26 = v20 * v21;
                }
            } else {
                v27 = v22 / (v23 + 1);
            }
        } else {
            if (i & 8) {
                v28 = v24 | v25;
            } else {
                v29 = v26 & v27;
            }
        }
        
        /* Use goto to create additional control flow edges */
        if (i & 16) {
            goto label2;
        }
        
    label1:
        v30 = v28 ^ v29;
        
        /* More arithmetic with floating point mixing */
        f1 = f1 * 1.1f + v0;
        f2 = f2 * 0.9f + v1;
        d1 = d1 * 1.01 + v2;
        d2 = d2 * 0.99 + v3;
        
        /* Pointer arithmetic */
        ptr1 = (char*)ptr1 + v4;
        ptr2 = (char*)ptr2 - v5;
        
    label2:
        /* Final computation for this iteration */
        result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
        result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
        result += v18 + v19 + v20 + v21 + v22 + v23 + v24 + v25;
        result += v26 + v27 + v28 + v29 + v30;
        result += (int)f1 + (int)f2 + (int)d1 + (int)d2;
        result += (int)(long)ptr1 + (int)(long)ptr2;
        
        /* Prevent loop unrolling */
        NO_OPTIMIZE(result);
    }
    
    return result;
}

/* Another complex function with different pattern */
static int __attribute__((noinline))
test_mcf_alternate(int base) {
    int a[32];
    volatile int sum = 0;
    
    /* Initialize array with complex pattern */
    for (int i = 0; i < 32; i++) {
        a[i] = base + i * 7;
    }
    
    /* Complex loop with early exits and continues */
    for (int i = 0; i < 100; i++) {
        if (i < 10) {
            for (int j = 0; j < 32; j++) {
                a[j] = a[j] * 3 + i;
                if (a[j] & 1) {
                    a[j] ^= 0xABCD;
                    continue;
                } else {
                    a[j] |= 0x1234;
                    if (a[j] > 10000) break;
                }
            }
        } else if (i < 50) {
            int k = i;
            while (k > 0) {
                a[k % 32] += k;
                k /= 2;
                if (k & 1) goto early_update;
            }
        } else {
            /* Deeply nested conditionals */
            if (i & 1) {
                if (i & 2) {
                    if (i & 4) {
                        a[0] += a[1];
                    } else {
                        a[1] += a[2];
                    }
                } else {
                    if (i & 8) {
                        a[2] += a[3];
                    } else {
                        a[3] += a[4];
                    }
                }
            }
        }
        
    early_update:
        /* Another inline assembly with different clobbers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop"
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
        );
        
        /* Update sum */
        for (int j = 0; j < 32; j += 2) {
            sum += a[j] - a[j + 1];
        }
        
        NO_OPTIMIZE(sum);
    }
    
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        total += test_mcf_pressure(i * 100, 50 + i * 5);
        total += test_mcf_alternate(i * 50);
        
        /* Prevent interprocedural optimization */
        NO_OPTIMIZE(total);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
