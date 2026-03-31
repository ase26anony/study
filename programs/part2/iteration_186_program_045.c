/* mcf_coverage.c - Program to trigger GCC's Minimum Cost Flow debug output */
#include <stdio.h>
#include <stdlib.h>

/* Force compiler to keep all computations */
static volatile int external_counter = 0;

/* Large test function with high register pressure */
int __attribute__((noinline)) test_mcf_pressure(int seed) {
    /* Declare many local variables to create register pressure */
    volatile int v0 = seed + 1;
    volatile int v1 = seed * 2;
    volatile int v2 = seed / 3;
    volatile int v3 = seed ^ 0x1234;
    volatile int v4 = seed | 0x5678;
    volatile int v5 = seed & 0x9ABC;
    volatile int v6 = seed << 2;
    volatile int v7 = seed >> 1;
    volatile int v8 = ~seed;
    volatile int v9 = seed + 100;
    
    volatile int v10 = v0 + v1;
    volatile int v11 = v2 * v3;
    volatile int v12 = v4 ^ v5;
    volatile int v13 = v6 | v7;
    volatile int v14 = v8 & v9;
    volatile int v15 = v10 << 1;
    volatile int v16 = v11 >> 2;
    volatile int v17 = v12 + v13;
    volatile int v18 = v14 * v15;
    volatile int v19 = v16 ^ v17;
    
    volatile int v20 = v18 | v19;
    volatile int v21 = v0 & v10;
    volatile int v22 = v1 ^ v11;
    volatile int v23 = v2 | v12;
    volatile int v24 = v3 & v13;
    volatile int v25 = v4 ^ v14;
    volatile int v26 = v5 | v15;
    volatile int v27 = v6 & v16;
    volatile int v28 = v7 ^ v17;
    volatile int v29 = v8 | v18;
    
    /* Complex control flow with many basic blocks */
    int result = 0;
    
    /* First level of conditionals */
    if (v0 > v1) {
        result += v0;
        if (v2 < v3) {
            result += v2;
            /* Inline asm to force register constraints */
            asm volatile ("" : : : "eax", "ebx", "ecx", "edx");
        } else {
            result += v3;
            asm volatile ("" : : : "esi", "edi", "ebp");
        }
    } else {
        result += v1;
        if (v4 > v5) {
            result += v4;
            asm volatile ("" : : : "r8", "r9", "r10", "r11");
        } else {
            result += v5;
            asm volatile ("" : : : "r12", "r13", "r14", "r15");
        }
    }
    
    /* Switch statement creating multiple control flow paths */
    switch (seed % 10) {
        case 0:
            result += v6 + v7;
            asm volatile ("" : : : "xmm0", "xmm1");
            break;
        case 1:
            result += v8 - v9;
            asm volatile ("" : : : "xmm2", "xmm3");
            break;
        case 2:
            result += v10 * v11;
            asm volatile ("" : : : "xmm4", "xmm5");
            break;
        case 3:
            result += v12 ^ v13;
            asm volatile ("" : : : "xmm6", "xmm7");
            break;
        case 4:
            result += v14 | v15;
            asm volatile ("" : : : "mm0", "mm1");
            break;
        case 5:
            result += v16 & v17;
            asm volatile ("" : : : "mm2", "mm3");
            break;
        case 6:
            result += v18 << 1;
            asm volatile ("" : : : "st", "st(1)");
            break;
        case 7:
            result += v19 >> 2;
            asm volatile ("" : : : "st(2)", "st(3)");
            break;
        case 8:
            result += v20 + v21;
            asm volatile ("" : : : "st(4)", "st(5)");
            break;
        case 9:
            result += v22 - v23;
            asm volatile ("" : : : "st(6)", "st(7)");
            break;
        default:
            result += v24;
            break;
    }
    
    /* Loop with complex exit conditions */
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            result += v25;
            /* More inline asm with clobbers */
            asm volatile ("" : : : "eax", "ebx", "ecx");
        } else {
            result += v26;
            asm volatile ("" : : : "edx", "esi", "edi");
        }
        
        /* Nested loop */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 3 == 0) {
                result += v27;
                asm volatile ("" : : : "r8", "r9");
            } else if ((i + j) % 3 == 1) {
                result += v28;
                asm volatile ("" : : : "r10", "r11");
            } else {
                result += v29;
                asm volatile ("" : : : "r12", "r13");
            }
        }
    }
    
    /* Goto labels creating additional control flow edges */
    if (result > 1000) {
        goto large_result;
    } else if (result < 0) {
        goto negative_result;
    }
    
    /* Normal path */
    result = result * 2 - seed;
    goto finish;
    
large_result:
    result = result / 2 + seed;
    /* Force spill/reload */
    asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    goto finish;
    
negative_result:
    result = -result + seed;
    asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                  "xmm12", "xmm13", "xmm14", "xmm15");
    goto finish;
    
finish:
    /* Final computation mixing all variables */
    result += v0 - v1 + v2 * v3 - v4 / (v5 ? v5 : 1) + 
              (v6 ^ v7) | (v8 & v9) + v10 << (v11 % 4) -
              v12 >> (v13 % 4) + v14 + v15 - v16 * v17 +
              v18 ^ v19 | v20 & v21 + v22 - v23 * v24 +
              v25 / (v26 ? v26 : 1) + (v27 ^ v28) | (v29 & seed);
    
    return result;
}

/* Another complex function to increase overall compilation complexity */
int __attribute__((noinline)) secondary_pressure(int base) {
    volatile double d0 = base * 1.1;
    volatile double d1 = base * 2.2;
    volatile double d2 = base * 3.3;
    volatile float f0 = base * 0.5f;
    volatile float f1 = base * 1.5f;
    
    int r = base;
    
    /* Mix float and int computations */
    if (d0 > d1) {
        r += (int)d0;
        asm volatile ("" : : : "xmm0", "xmm1", "xmm2");
    } else {
        r += (int)d1;
        asm volatile ("" : : : "xmm3", "xmm4", "xmm5");
    }
    
    if (f0 < f1) {
        r -= (int)f0;
        asm volatile ("" : : : "xmm6", "xmm7");
    } else {
        r -= (int)f1;
        asm volatile ("" : : : "xmm8", "xmm9");
    }
    
    /* Pointer arithmetic to create address computations */
    volatile int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = base + i;
    }
    
    volatile int *ptr = arr;
    for (int i = 0; i < 5; i++) {
        r += *ptr++;
        asm volatile ("" : : : "rax", "rbx");
    }
    
    return r;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += test_mcf_pressure(i);
        total += secondary_pressure(i);
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
