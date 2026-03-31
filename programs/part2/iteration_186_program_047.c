/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all variables alive */
#define USE(x) asm volatile("" : : "r"(x))

/* Complex test function with high register pressure */
static int __attribute__((noinline)) 
test_mcf_coverage(int seed) {
    /* Declare many variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed % 7;
    volatile int v4 = seed ^ 0x55;
    volatile int v5 = seed | 0xAA;
    volatile int v6 = seed & 0xFF;
    volatile int v7 = seed << 2;
    volatile int v8 = seed >> 1;
    volatile int v9 = ~seed;
    
    volatile int v10 = v0 + v1;
    volatile int v11 = v2 * v3;
    volatile int v12 = v4 ^ v5;
    volatile int v13 = v6 | v7;
    volatile int v14 = v8 & v9;
    volatile int v15 = v10 << v11;
    volatile int v16 = v12 >> v13;
    volatile int v17 = v14 + v15;
    volatile int v18 = v16 * v17;
    volatile int v19 = v18 ^ v0;
    
    volatile int v20 = v1 + v19;
    volatile int v21 = v2 * v20;
    volatile int v22 = v3 ^ v21;
    volatile int v23 = v4 | v22;
    volatile int v24 = v5 & v23;
    volatile int v25 = v6 << v24;
    volatile int v26 = v7 >> v25;
    volatile int v27 = v8 + v26;
    volatile int v28 = v9 * v27;
    volatile int v29 = v10 ^ v28;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    
    /* First level of branching */
    if (v0 > 100) {
        result += v1;
        if (v2 < 50) {
            result += v3;
            
            /* Inline assembly to force register constraints */
            asm volatile (
                "# Force register pressure\n"
                : 
                : "r"(v4), "r"(v5), "r"(v6), "r"(v7), "r"(v8)
                : "eax", "ebx", "ecx", "edx", "esi", "edi"
            );
            
            /* Switch creates many basic blocks */
            switch (v9 % 10) {
                case 0: result += v10; break;
                case 1: result += v11; break;
                case 2: result += v12; break;
                case 3: result += v13; break;
                case 4: result += v14; break;
                case 5: result += v15; break;
                case 6: result += v16; break;
                case 7: result += v17; break;
                case 8: result += v18; break;
                case 9: result += v19; break;
                default: result += 1;
            }
        } else {
            result -= v4;
            goto label1;  /* Create control flow edge */
        }
    } else {
        result = v5;
    }
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < v6; i++) {
        if (i % 3 == 0) {
            result += v20 + i;
            continue;
        } else if (i % 3 == 1) {
            result += v21 * i;
            
            /* More inline assembly with clobbers */
            asm volatile (
                "# More register pressure\n"
                : 
                : "r"(v22), "r"(v23), "r"(v24)
                : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            if (result > 1000) {
                break;  /* Early exit from loop */
            }
        } else {
            result += v25 - i;
        }
        
        /* Nested loop */
        for (int j = 0; j < 5; j++) {
            result += (v26 + j) * (v27 - i);
            if (result % 7 == 0) {
                goto label2;  /* Another control flow edge */
            }
        }
        
    label2:
        result += 1;
    }
    
label1:
    /* More complex branching */
    if (v7 != 0) {
        result /= (v7 + 1);
    } else {
        result *= 2;
    }
    
    /* Compute checksum using all variables */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                   v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                   v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Final computation with mixed types to stress register allocator */
    float f1 = v0 * 1.5f;
    float f2 = v1 * 2.5f;
    double d1 = v2 * 3.14159;
    double d2 = v3 * 2.71828;
    
    /* Pointer arithmetic */
    int* ptr1 = &v0;
    int* ptr2 = &v1;
    long ptr_diff = ptr2 - ptr1;
    
    /* Force use of all computed values */
    result += (int)f1 + (int)f2 + (int)d1 + (int)d2 + (int)ptr_diff;
    result ^= checksum;
    
    /* Final assembly to ensure all values are used */
    asm volatile (
        "# Final use of variables\n"
        : 
        : "r"(result), "r"(checksum), "r"(v29), "r"(v28)
        : "memory"
    );
    
    return result;
}

/* Second complex function to create interprocedural pressure */
static int __attribute__((noinline))
another_complex_function(int base) {
    volatile int a = base * 3;
    volatile int b = base + 17;
    volatile int c = base ^ 0x1234;
    
    int sum = 0;
    
    /* Deeply nested conditionals */
    if (a > b) {
        if (c > a) {
            sum = a + c;
        } else if (b > c) {
            sum = b - c;
            
            /* Switch with many cases */
            switch (base % 12) {
                case 0: sum += 1; break;
                case 1: sum += 2; break;
                case 2: sum += 3; break;
                case 3: sum += 4; break;
                case 4: sum += 5; break;
                case 5: sum += 6; break;
                case 6: sum += 7; break;
                case 7: sum += 8; break;
                case 8: sum += 9; break;
                case 9: sum += 10; break;
                case 10: sum += 11; break;
                case 11: sum += 12; break;
            }
        } else {
            for (int i = 0; i < 20; i++) {
                sum += i * base;
                if (sum > 1000) goto early_exit;
            }
        }
    } else {
        sum = a * b * c;
    }
    
early_exit:
    return sum;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += test_mcf_coverage(i);
        total += another_complex_function(i);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile("# Loop barrier" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
