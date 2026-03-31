/* mcf_coverage.c - Program to trigger MCF pass debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int g_volatile_sink = 0;

/* Large test function with complex control flow and high register pressure */
int __attribute__((noinline)) test_mcf_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int v30, v31, v32, v33, v34, v35, v36, v37, v38, v39;
    float f0, f1, f2, f3, f4;
    int *p0, *p1, *p2;
    int result = seed;
    
    /* Initialize variables with complex dependencies */
    v0 = seed;
    v1 = seed * 2;
    v2 = seed + 1;
    v3 = seed - 1;
    v4 = seed ^ 0x55;
    
    /* Create data dependencies across variables */
    for (int i = 0; i < 5; i++) {
        v5 = v0 + v1;
        v6 = v2 * v3;
        v7 = v4 ^ v5;
        v8 = v6 - v7;
        v9 = v8 >> 2;
        
        v10 = v9 + i;
        v11 = v10 * v5;
        v12 = v11 & 0xFF;
        v13 = v12 | v6;
        v14 = v13 ^ v7;
        
        /* Use inline assembly to clobber registers */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v15), "=r"(v16)
            : "0"(v14), "1"(v13)
            : "eax", "ebx", "ecx", "edx", "esi", "edi"
        );
        
        v17 = v15 + v16;
        v18 = v17 * v8;
        v19 = v18 % 97;
        
        /* Complex conditional creating basic blocks */
        if (v19 & 1) {
            v20 = v19 * 3;
            v21 = v20 + 1;
            
            /* More inline assembly with clobbers */
            asm volatile (
                "# Another clobber point\n"
                : "=r"(v22)
                : "r"(v21)
                : "r8", "r9", "r10", "r11"
            );
        } else {
            v20 = v19 / 2;
            v21 = v20 - 1;
            v22 = v21 ^ 0xAA;
        }
        
        v23 = v22 + v17;
        v24 = v23 * i;
        v25 = v24 & 0xFFFF;
        
        /* Nested conditionals */
        if (v25 > 1000) {
            v26 = v25 - 1000;
            if (v26 < 100) {
                v27 = v26 * 5;
            } else {
                v27 = v26 / 5;
            }
        } else if (v25 > 500) {
            v26 = v25 - 500;
            v27 = v26 * 3;
        } else {
            v26 = v25;
            v27 = v26;
        }
        
        v28 = v27 + v22;
        v29 = v28 ^ v23;
        
        /* Use all variables to prevent elimination */
        v30 = v0 + v29;
        v31 = v1 + v30;
        v32 = v2 + v31;
        v33 = v3 + v32;
        v34 = v4 + v33;
        
        /* Float operations for FP register pressure */
        f0 = (float)v34;
        f1 = f0 * 1.5f;
        f2 = f1 + (float)v29;
        f3 = f2 / 2.0f;
        f4 = f3 - f0;
        
        /* Pointer operations */
        p0 = &v34;
        p1 = &v29;
        p2 = p0;
        *p2 = (int)f4;
        
        v35 = *p0 + *p1;
        v36 = v35 << 2;
        v37 = v36 >> 1;
        v38 = v37 ^ 0x1234;
        v39 = v38 % 89;
        
        /* Update initial variables for next iteration */
        v0 = v39;
        v1 = v38;
        v2 = v37;
        v3 = v36;
        v4 = v35;
        
        result += v39;
    }
    
    /* Large switch statement creating many basic blocks */
    switch (result & 0xF) {
        case 0: result ^= v0; break;
        case 1: result += v1; break;
        case 2: result -= v2; break;
        case 3: result *= v3; break;
        case 4: result |= v4; break;
        case 5: result &= v5; break;
        case 6: result ^= v6; break;
        case 7: result += v7; break;
        case 8: result -= v8; break;
        case 9: result *= v9; break;
        case 10: result |= v10; break;
        case 11: result &= v11; break;
        case 12: result ^= v12; break;
        case 13: result += v13; break;
        case 14: result -= v14; break;
        case 15: result *= v15; break;
        default: result = 0;
    }
    
    /* Complex loop with break/continue */
    for (int j = 0; j < 10; j++) {
        if (j & 1) {
            v16 = result + j;
            if (v16 > 50) {
                result -= 5;
                continue;
            }
        } else {
            v17 = result - j;
            if (v17 < 0) {
                result += 10;
                break;
            }
        }
        
        /* Label and goto for additional control flow */
        if (j == 5) {
            goto special_case;
        }
        
        v18 = result * j;
        result = v18 % 100;
        
        if (j == 8) {
            /* Another inline assembly with many clobbers */
            asm volatile (
                "# Final clobber\n"
                : "=r"(result)
                : "r"(result)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
            );
        }
        
        continue;
        
    special_case:
        result ^= 0xDEADBEEF;
    }
    
    /* Prevent dead code elimination */
    g_volatile_sink = result;
    
    return result;
}

/* Second complex function to create interprocedural effects */
int __attribute__((noinline)) another_complex_function(int base) {
    int a = base, b = base + 1, c = base + 2;
    int d, e, f, g, h;
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < 20; i++) {
        d = a + b;
        e = b + c;
        f = c + d;
        g = d + e;
        h = e + f;
        
        /* Conditional with multiple exits */
        if (h > 1000) {
            a = h % 100;
            if (a < 10) return a;
        } else if (h < 100) {
            b = h * 2;
            if (b > 150) goto early_exit;
        }
        
        c = g - f;
        
        /* Use computed goto-like pattern */
        switch (i % 4) {
            case 0: a += 1; break;
            case 1: b += 2; break;
            case 2: c += 3; break;
            case 3: a = b = c = h; break;
        }
    }
    
    return a + b + c;
    
early_exit:
    return b * 2;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        int r1 = test_mcf_function(i);
        int r2 = another_complex_function(i);
        total += r1 + r2;
        
        /* Mix in some volatile operations */
        asm volatile (
            "# Main loop clobber\n"
            : 
            : "r"(total)
            : "memory"
        );
    }
    
    printf("Result: %d\n", total);
    printf("Volatile sink: %d\n", g_volatile_sink);
    
    return total != 0 ? 0 : 1;
}
