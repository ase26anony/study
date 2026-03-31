/* early-remat-trigger.c
 * Program designed to trigger GCC's early rematerialization privatization logic
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int y = x * 2;
    return y + 1;
}

__attribute__((noinline)) double external_func2(double x) {
    volatile double y = x * 3.14159;
    return y / 2.0;
}

__attribute__((noinline)) void* external_func3(void* p) {
    volatile intptr_t addr = (intptr_t)p;
    return (void*)(addr + 16);
}

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_fptr)(void);

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    int v5 = seed * 3;
    int v6 = seed / 2;
    int v7 = seed % 13;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = seed ^ 0xFF;
    
    /* Mixed data types for different machine modes */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    long long ll1 = (long long)seed * 1000000LL;
    float f1 = seed * 1.5f;
    double d1 = seed * 3.14159;
    void* ptr1 = &seed;
    
    /* More variables for additional pressure */
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f2, f3, f4;
    double d2, d3;
    char c2, c3;
    short s2, s3;
    long long ll2, ll3;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex conditional computation with many branches */
    if (v1 > 100) {
        key_result = v2 + v3;
        
        if (v4 < 0) {
            key_result *= 2;
            
            /* Nested switch inside if */
            switch (v5 % 5) {
                case 0:
                    key_result += external_func1(v6);
                    /* Use inline assembly that clobbers many registers */
                    __asm__ volatile (
                        "mov %0, %%eax\n\t"
                        "add $100, %%eax\n\t"
                        : 
                        : "r" (key_result)
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    break;
                case 1:
                    key_result -= v7;
                    /* Another inline assembly with clobbers */
                    __asm__ volatile (
                        "mov %0, %%ebx\n\t"
                        "imul $3, %%ebx\n\t"
                        : 
                        : "r" (key_result)
                        : "eax", "ebx", "ecx", "edx", "memory"
                    );
                    break;
                case 2:
                    key_result = key_result ^ v8;
                    break;
                case 3:
                    key_result = key_result | v9;
                    break;
                case 4:
                    key_result = key_result & v10;
                    break;
            }
            
            /* Use volatile variable in condition */
            volatile int vol_cond = v1;
            if (vol_cond > 150) {
                /* Use key_result in floating point computation */
                f1 = (float)key_result / 2.0f;
                d1 = external_func2((double)key_result);
                
                /* More register pressure */
                f2 = f1 * 2.0f;
                f3 = f1 + 1.0f;
                f4 = f1 - 0.5f;
            }
        } else {
            key_result = v3 - v2;
        }
    } else if (v1 > 50) {
        key_result = v4 * v5;
        
        /* Deeply nested if-else chain */
        if (v6 != 0) {
            if (v7 > 10) {
                if (v8 < 1000) {
                    /* Use key_result with different data types */
                    ll1 = (long long)key_result * 100LL;
                    c1 = (char)(key_result & 0xFF);
                    s1 = (short)key_result;
                    
                    /* Inline assembly using the value */
                    __asm__ volatile (
                        "mov %0, %%ecx\n\t"
                        "add %%ecx, %%ecx\n\t"
                        : 
                        : "r" (key_result)
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory"
                    );
                }
            }
        }
    } else {
        key_result = v9 / (v10 ? v10 : 1);
    }
    
    /* Label addresses for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile void* volatile_label_ptr = labels[v1 % 4];
    
    /* Use goto with computed label */
    goto *volatile_label_ptr;
    
label1:
    /* Use key_result after conditional block */
    v11 = key_result + 100;
    v12 = key_result * 2;
    goto after_labels;
    
label2:
    v11 = key_result - 50;
    v12 = key_result / 2;
    goto after_labels;
    
label3:
    v11 = key_result ^ 0xAAAA;
    v12 = key_result | 0x5555;
    goto after_labels;
    
label4:
    v11 = key_result & 0x3333;
    v12 = key_result << 1;
    goto after_labels;
    
after_labels:
    
    /* More computations to keep variables live */
    v13 = v11 + v12;
    v14 = v13 * key_result;
    v15 = v14 - v1;
    
    /* Use all the float/double variables */
    d2 = d1 + (double)v15;
    d3 = external_func2(d2);
    f2 = (float)d3;
    f3 = f1 + f2;
    f4 = f3 * 2.0f;
    
    /* Use char/short/long long variables */
    c2 = c1 + (char)v15;
    c3 = c2 * 2;
    s2 = s1 + (short)v15;
    s3 = s2 - 100;
    ll2 = ll1 + (long long)v15;
    ll3 = ll2 * 2LL;
    
    /* Pointer arithmetic */
    ptr1 = external_func3(ptr1);
    void* ptr2 = (char*)ptr1 + v15;
    
    /* Final aggregation to prevent dead code elimination */
    int final_result = v11 + v12 + v13 + v14 + v15;
    final_result += (int)f4 + (int)d3 + c3 + s3 + (int)(ll3 & 0xFFFFFFFF);
    final_result += (intptr_t)ptr2 & 0xFF;
    
    return final_result;
}

/* Another function with switch-based control flow */
__attribute__((noinline))
int switch_based_function(int x) {
    int result = x;
    
    /* Complex switch with many cases */
    switch (x % 8) {
        case 0:
            result += 100;
            /* Conditional register use inside case */
            {
                volatile int cond = x;
                if (cond > 1000) {
                    __asm__ volatile (
                        "mov %0, %%r8d\n\t"
                        "add $42, %%r8d\n\t"
                        : 
                        : "r" (result)
                        : "r8", "r9", "r10", "r11", "memory"
                    );
                }
            }
            break;
        case 1:
            result -= 50;
            break;
        case 2:
            result *= 3;
            break;
        case 3:
            result /= 2;
            break;
        case 4:
            result ^= 0xFFFF;
            break;
        case 5:
            result |= 0xAAAA;
            break;
        case 6:
            result &= 0x5555;
            break;
        case 7:
            result <<= 2;
            break;
    }
    
    /* Use result after switch */
    int y = result * 2;
    int z = y + result;
    
    /* More register pressure */
    float f1 = (float)result;
    float f2 = f1 * 3.14f;
    double d1 = (double)result;
    double d2 = d1 / 2.71828;
    
    __asm__ volatile (
        "mov %0, %%eax\n\t"
        "mov %1, %%ebx\n\t"
        "add %%ebx, %%eax\n\t"
        : 
        : "r" (z), "r" (result)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return z + (int)f2 + (int)d2;
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        total += stress_function(seed + i);
        total += switch_based_function(seed - i);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
