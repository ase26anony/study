/* early-remat-test.c
 * Designed to trigger early_remat::privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 * For 32-bit: gcc -O2 -m32 -march=i686 -fno-dse -fearly-remat early-remat-test.c -o early-remat-test-32
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
    volatile void* q = p;
    return (void*)((uintptr_t)q + 1);
}

/* Volatile function pointer to prevent inlining */
typedef int (*volatile func_ptr_t)(int);
func_ptr_t volatile volatile_fp = external_func1;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    char v6 = seed & 0xFF;
    short v7 = seed & 0xFFFF;
    int* v8 = (int*)&seed;
    double v9 = seed * 9.0;
    float v10 = seed * 10.0f;
    long long v11 = seed * 11LL;
    int v12 = seed * 12;
    double v13 = seed * 13.0;
    float v14 = seed * 14.0f;
    int v15 = seed * 15;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex conditional computation with many branches */
    if (v1 > 100) {
        key_result = v2 + v3;
        
        if (v4 > 50.0f) {
            key_result += (int)v5;
            
            /* Nested switch inside if */
            switch (v6 % 4) {
                case 0:
                    key_result += v7 * 2;
                    /* Use inline assembly that clobbers many registers */
                    __asm__ volatile (
                        "mov %0, %%eax\n\t"
                        "add $100, %%eax\n\t"
                        "mov %%eax, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    break;
                case 1:
                    key_result -= v7;
                    break;
                case 2:
                    key_result *= 3;
                    break;
                case 3:
                    key_result /= 2;
                    break;
            }
            
            /* Another level of nesting */
            if (v8 != NULL) {
                key_result += *v8;
                
                /* Use the key_result in a non-inlineable function call */
                key_result = volatile_fp(key_result);
                
                /* More register pressure */
                v9 = external_func2(v9);
                v10 = v9 + v10;
            }
        } else {
            key_result = v2 - v3;
        }
    } else if (v1 > 50) {
        key_result = v3 - v2;
        
        /* Deeply nested if-else chain */
        if (v5 > 25.0) {
            if (v6 < 0) {
                if (v7 > 1000) {
                    key_result += 1000;
                    
                    /* Mixed type computation forcing mode conversions */
                    double temp_d = (double)key_result;
                    float temp_f = (float)temp_d;
                    long long temp_ll = (long long)temp_f;
                    key_result = (int)temp_ll;
                    
                    /* Inline assembly with different modes */
                    __asm__ volatile (
                        "mov %0, %%eax\n\t"
                        "cvtsi2sd %%eax, %%xmm0\n\t"
                        "addsd %1, %%xmm0\n\t"
                        "cvttsd2si %%xmm0, %%eax\n\t"
                        "mov %%eax, %0\n\t"
                        : "+r" (key_result)
                        : "m" (v13)
                        : "eax", "xmm0", "xmm1", "xmm2", "memory"
                    );
                }
            }
        }
    } else {
        key_result = v2 * v3;
    }
    
    /* Use goto with computed labels for irreducible control flow */
    void* label_ptr = NULL;
    int label_choice = v1 % 3;
    
    /* Define labels */
    void* labels[] = { &&label0, &&label1, &&label2 };
    
    /* Store label address in volatile pointer */
    volatile void* volatile_label_ptr = labels[label_choice];
    label_ptr = (void*)volatile_label_ptr;
    
    /* Jump based on volatile pointer */
    goto *label_ptr;
    
label0:
    /* Use key_result after conditional jump */
    key_result += v12;
    goto after_labels;
    
label1:
    /* Different computation path */
    key_result -= v15;
    
    /* Another conditional block inside label path */
    if (v14 > 20.0f) {
        /* Use key_result in complex expression */
        key_result = (key_result * 3) / 2;
        
        /* More inline assembly with register clobbering */
        __asm__ volatile (
            "mov %0, %%ecx\n\t"
            "imul $7, %%ecx\n\t"
            "mov %%ecx, %0\n\t"
            : "+r" (key_result)
            : 
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    }
    goto after_labels;
    
label2:
    /* Third path with mixed mode operations */
    {
        short temp_short = (short)key_result;
        char temp_char = (char)temp_short;
        key_result = temp_char * 4;
    }
    goto after_labels;
    
after_labels:
    
    /* Switch statement with fall-through to create complex CFG */
    int switch_var = v1 % 5;
    switch (switch_var) {
        case 0:
            key_result += 10;
            /* Fall through */
        case 1:
            key_result += 20;
            /* Use external function */
            v8 = external_func3(v8);
            if (v8) {
                key_result += 30;
            }
            break;
        case 2:
            key_result -= 40;
            /* Nested switch */
            switch (v6 % 2) {
                case 0:
                    key_result *= 2;
                    break;
                case 1:
                    key_result /= 2;
                    break;
            }
            break;
        case 3:
            key_result *= 3;
            /* Loop to extend live ranges */
            for (int i = 0; i < 3; i++) {
                key_result += i;
                /* Use all variables to keep them live */
                v4 += i;
                v5 += i;
                v9 += i;
            }
            break;
        case 4:
            key_result = key_result > 0 ? key_result : -key_result;
            break;
    }
    
    /* Final aggregation using all variables to prevent elimination */
    int final_result = key_result;
    final_result += v2;
    final_result += (int)v3;
    final_result += (int)v4;
    final_result += (int)v5;
    final_result += v6;
    final_result += v7;
    final_result += (v8 ? *v8 : 0);
    final_result += (int)v9;
    final_result += (int)v10;
    final_result += (int)v11;
    final_result += v12;
    final_result += (int)v13;
    final_result += (int)v14;
    final_result += v15;
    
    return final_result;
}

/* Another function with different patterns */
__attribute__((noinline))
int alternate_stress(int base) {
    /* Variables with different modes */
    unsigned char c1 = base & 0xFF;
    unsigned short s1 = base & 0xFFFF;
    unsigned int i1 = base;
    unsigned long long ll1 = (unsigned long long)base * 1000;
    float f1 = base * 1.5f;
    double d1 = base * 2.5;
    
    /* Conditional register use with pointer aliasing */
    int* ptr1 = &i1;
    int* ptr2 = (int*)&ll1;
    volatile int* volatile_ptr = ptr1;
    
    int result = 0;
    
    /* Complex condition based on volatile read */
    if (*volatile_ptr > 50) {
        result = c1 + s1;
        
        /* Inline assembly that uses and modifies result */
        __asm__ volatile (
            "mov %1, %%eax\n\t"
            "add %2, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (result)
            : "r" (result), "r" (i1)
            : "eax", "memory"
        );
        
        /* Use result in floating point computation */
        f1 = f1 + (float)result;
        d1 = d1 * (double)result;
        
        /* Convert back to integer */
        result = (int)(f1 + d1);
    } else {
        result = i1 - c1;
    }
    
    /* Use computed goto again */
    static void* alt_labels[] = { &&alt_a, &&alt_b, &&alt_c };
    volatile int label_idx = base % 3;
    goto *alt_labels[label_idx];
    
alt_a:
    result += 100;
    goto alt_end;
    
alt_b:
    /* Use mixed modes in computation */
    {
        double temp = (double)result;
        __asm__ volatile (
            "movq %1, %%xmm0\n\t"
            "addsd %2, %%xmm0\n\t"
            "movq %%xmm0, %0\n\t"
            : "=m" (temp)
            : "m" (temp), "m" (d1)
            : "xmm0", "memory"
        );
        result = (int)temp;
    }
    goto alt_end;
    
alt_c:
    result *= 3;
    goto alt_end;
    
alt_end:
    
    /* Force spill/reload patterns */
    for (int i = 0; i < 10; i++) {
        result += i;
        /* Use all variables in loop */
        c1 += i;
        s1 += i;
        i1 += i;
        ll1 += i;
        f1 += i;
        d1 += i;
    }
    
    return result + c1 + s1 + i1 + (int)ll1 + (int)f1 + (int)d1;
}

int main(int argc, char** argv) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result1 = stress_function(seed);
    printf("Stress function result: %d\n", result1);
    
    int result2 = alternate_stress(seed * 2);
    printf("Alternate stress result: %d\n", result2);
    
    /* Use results to prevent dead code elimination */
    volatile int final_check = result1 + result2;
    if (final_check > 1000000) {
        printf("Unexpected large result\n");
    }
    
    return final_check & 0xFF;
}
