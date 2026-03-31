/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debugging output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int external_counter = 0;

/* Complex function with high register pressure and control flow */
int complex_mcf_function(int seed) {
    /* Declare many local variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    volatile int barrier = seed; /* Prevent optimization */
    
    /* Initialize variables with dependent computations */
    v0 = seed + 1;
    v1 = seed * 2;
    v2 = seed ^ 0x55AA;
    v3 = seed << 3;
    v4 = seed >> 2;
    v5 = seed | 0xFF00;
    v6 = seed & 0x00FF;
    v7 = ~seed;
    v8 = seed + 100;
    v9 = seed - 50;
    
    /* Complex control flow with many basic blocks */
    if (barrier & 1) {
        v10 = v0 + v1;
        v11 = v2 * v3;
        
        /* Inline assembly to force register constraints */
        asm volatile (
            "# Force register pressure\n"
            : "=r"(v12), "=r"(v13)
            : "0"(v4), "1"(v5)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    } else {
        v10 = v1 - v0;
        v11 = v3 / (v2 ? v2 : 1);
        
        /* Different inline assembly with different clobbers */
        asm volatile (
            "# More register pressure\n"
            : "=r"(v12), "=r"(v13)
            : "0"(v6), "1"(v7)
            : "esi", "edi", "ebp", "memory"
        );
    }
    
    /* Switch statement creating multiple control flow paths */
    switch (barrier % 10) {
        case 0: v14 = v8 + v9; v15 = v10 * v11; break;
        case 1: v14 = v8 - v9; v15 = v10 / (v11 ? v11 : 1); break;
        case 2: v14 = v8 ^ v9; v15 = v10 & v11; break;
        case 3: v14 = v8 | v9; v15 = v10 | v11; break;
        case 4: v14 = v8 & v9; v15 = v10 ^ v11; break;
        case 5: v14 = v8 << 2; v15 = v11 >> 1; break;
        case 6: v14 = v9 << 3; v15 = v10 >> 2; break;
        case 7: v14 = ~v8; v15 = ~v9; break;
        case 8: v14 = v8 * 3; v15 = v9 * 7; break;
        case 9: v14 = v8 / 2; v15 = v9 / 4; break;
        default: v14 = 0; v15 = 0; break;
    }
    
    /* Nested loops with complex exit conditions */
    for (int i = 0; i < 5; i++) {
        v16 = v14 + i;
        for (int j = 0; j < 3; j++) {
            v17 = v15 + j;
            if (i == j) {
                v18 = v16 * v17;
                
                /* More inline assembly in loop */
                asm volatile (
                    "# Loop register pressure\n"
                    : "+r"(v18)
                    :
                    : "eax", "ebx", "ecx", "memory"
                );
            } else {
                v18 = v16 + v17;
            }
            
            /* Conditional goto creating additional control flow */
            if (v18 > 1000) {
                v19 = v18 >> 2;
                goto special_case;
            }
            v19 = v18 << 2;
        }
        
        /* Mix variable types to increase pressure */
        float f1 = v16 * 0.5f;
        float f2 = v17 * 0.25f;
        v20 = (int)(f1 + f2);
        
        /* Pointer arithmetic creating aliasing concerns */
        int* ptr = &v20;
        v21 = *ptr + i;
    }
    
    /* Label for goto creating irregular control flow */
special_case:
    v22 = v19 * 2;
    
    /* Deeply nested conditionals */
    if (v22 > 500) {
        if (v22 < 1000) {
            v23 = v22 + 100;
            if (v23 & 1) {
                v24 = v23 * 3;
            } else {
                v24 = v23 / 3;
            }
        } else {
            v23 = v22 - 100;
            v24 = v23 ^ 0x1234;
        }
    } else {
        v23 = v22 * 2;
        v24 = v23 | 0xABCD;
    }
    
    /* More variables with overlapping live ranges */
    v25 = v24 + v23;
    v26 = v25 - v22;
    v27 = v26 * v21;
    v28 = v27 ^ v20;
    v29 = v28 & 0xFFFF;
    
    /* Final computation that uses all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Use external volatile to prevent dead code elimination */
    external_counter += result;
    
    return result;
}

/* Second complex function with different patterns */
int another_mcf_function(int base) {
    int a0 = base, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int b0, b1, b2, b3, b4, b5, b6, b7, b8, b9;
    
    /* Chain of dependent computations */
    a1 = a0 * 2;
    a2 = a1 + 17;
    a3 = a2 ^ a1;
    a4 = a3 << 1;
    a5 = a4 >> 2;
    a6 = a5 | 0xFF;
    a7 = a6 & 0xF0;
    a8 = a7 * 3;
    a9 = a8 - 1;
    
    /* Loop with early exit creating multiple exits */
    for (int i = 0; i < 20; i++) {
        b0 = a9 + i;
        if (b0 > 100) {
            b1 = b0 - 50;
            break;
        }
        if (b0 < 0) {
            b1 = b0 + 50;
            continue;
        }
        b1 = b0 * 2;
        
        /* Inline assembly with many clobbers */
        asm volatile (
            "# Extensive clobber list\n"
            : "=r"(b2), "=r"(b3)
            : "0"(b1), "1"(i)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
        );
    }
    
    /* Complex switch with fallthrough */
    switch (b2 % 8) {
        case 0: b4 = b3 + 1; /* Fallthrough */
        case 1: b4 += b2; /* Fallthrough */
        case 2: b4 *= 2; break;
        case 3: b4 = b3 - b2; break;
        case 4: b4 = b3 ^ b2; break;
        case 5: b4 = b3 | b2; break;
        case 6: b4 = b3 & b2; break;
        case 7: b4 = ~b3; break;
    }
    
    /* More computations */
    b5 = b4 << 3;
    b6 = b5 >> 1;
    b7 = b6 ^ 0xAA;
    b8 = b7 | 0x55;
    b9 = b8 & 0xF0F0;
    
    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 +
           b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
}

int main() {
    int total = 0;
    
    /* Call functions multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += complex_mcf_function(i);
        total += another_mcf_function(i * 3);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile ("# Loop barrier" ::: "memory");
        }
    }
    
    printf("Result: %d (external: %d)\n", total, external_counter);
    return total != 0 ? 0 : 1;
}
