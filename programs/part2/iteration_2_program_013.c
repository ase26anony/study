/* early-remat-trigger.c
 * Program designed to trigger early rematerialization's 
 * privatize_cond_register_use function in GCC.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-trigger.c -o trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_helper(int x, int y) {
    volatile int result = x * y + 37;
    return result;
}

/* Another non-inlineable function */
__attribute__((noinline))
double fp_helper(double a, double b) {
    volatile double temp = a / (b + 1.0);
    return temp * 2.0;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Main stress function with high register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    int v3 = seed * 2;
    int v4 = seed / 3;
    int v5 = seed % 7;
    int v6 = seed << 2;
    int v7 = seed >> 1;
    int v8 = seed ^ 0x55;
    int v9 = seed | 0xAA;
    int v10 = seed & 0xFF;
    
    /* Mixed floating point types */
    float f1 = seed * 1.5f;
    float f2 = seed * 0.25f;
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    
    /* Pointer variables */
    int *p1 = &v1;
    int *p2 = &v2;
    volatile int *p3 = &v3;
    
    /* Character types */
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000000LL;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double fp_key_result = 0.0;
    
    /* Complex conditional computation */
    if (v1 > 0) {
        key_result = v2 + v3;
        if (v4 < 100) {
            key_result *= v5;
            if (v6 != 0) {
                /* Nested conditional block with register pressure */
                int temp = key_result;
                
                /* Use inline assembly that clobbers many registers */
                __asm__ volatile (
                    /* Clobber many registers to increase pressure */
                    "mov %0, %%eax\n\t"
                    "add $1, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r" (temp)
                    : 
                    : "eax", "ebx", "ecx", "edx", "esi", "edi", 
                      "memory", "cc"
                );
                
                key_result = temp + v7;
                
                /* Call non-inlineable function */
                key_result = external_helper(key_result, v8);
            } else {
                key_result = v9 - v10;
            }
            
            /* Switch statement for additional complexity */
            switch (v1 & 0x3) {
                case 0:
                    key_result += 100;
                    /* Use floating point computation */
                    fp_key_result = d1 * 2.0;
                    break;
                case 1:
                    key_result -= 50;
                    fp_key_result = d2 / 2.0;
                    break;
                case 2:
                    key_result *= 2;
                    fp_key_result = fp_helper(d1, d2);
                    break;
                default:
                    key_result /= 2;
                    fp_key_result = d1 + d2;
                    break;
            }
        } else {
            key_result = v2 * v3 - v4;
        }
    } else {
        if (v1 < -10) {
            key_result = v5 * v6;
        } else {
            key_result = v7 + v8 + v9;
        }
    }
    
    /* More register pressure with mixed types */
    long long ll2 = ll1 + key_result;
    double d3 = d1 + d2 + fp_key_result;
    float f3 = f1 * f2;
    
    /* Use goto with computed labels for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile int label_selector = (key_result >> 2) & 0x3;
    
    /* Store label address in volatile function pointer */
    volatile_func_ptr = (void (*)(void))labels[label_selector];
    
    /* Conditional jump based on volatile */
    if (v1 & 0x1) {
        goto *labels[0];
    } else {
        goto *labels[label_selector];
    }
    
label1:
    /* Use key_result in inline assembly with different mode */
    __asm__ volatile (
        "addl $0x10, %0\n\t"
        : "+r" (key_result)
        : 
        : "cc"
    );
    goto after_labels;
    
label2:
    /* Use floating point version */
    __asm__ volatile (
        "fldl %0\n\t"
        "fadd %%st(0), %%st(0)\n\t"
        "fstpl %0\n\t"
        : "+m" (d3)
        :
        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)"
    );
    goto after_labels;
    
label3:
    /* Mixed mode use */
    {
        short temp_short = (short)key_result;
        __asm__ volatile (
            "addw $5, %0\n\t"
            : "+r" (temp_short)
            :
            : "cc"
        );
        key_result = temp_short;
    }
    goto after_labels;
    
label4:
    /* Character mode use */
    {
        char temp_char = (char)key_result;
        __asm__ volatile (
            "addb $1, %0\n\t"
            : "+r" (temp_char)
            :
            : "cc"
        );
        key_result = temp_char;
    }
    /* fall through */
    
after_labels:
    /* Use all variables again to keep them live */
    int final_result = key_result;
    final_result += v1 + v2 + v3 + v4 + v5;
    final_result += v6 + v7 + v8 + v9 + v10;
    final_result += (int)f1 + (int)f2;
    final_result += (int)d1 + (int)d2;
    final_result += (int)fp_key_result;
    final_result += *p1 + *p2 + *p3;
    final_result += c1 + s1;
    final_result += (int)(ll1 & 0xFFFFFFFF);
    final_result += (int)(ll2 & 0xFFFFFFFF);
    final_result += (int)f3;
    final_result += (int)d3;
    
    /* Final computation using key_result */
    if (final_result > 1000) {
        final_result = final_result % 1000;
    }
    
    return final_result;
}

/* Another complex function with different patterns */
__attribute__((noinline))
int secondary_stress(int base) {
    volatile int trigger = base;
    
    /* Array to create more pressure */
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = base + i;
    }
    
    double darr[10];
    for (int i = 0; i < 10; i++) {
        darr[i] = base * 1.1 * i;
    }
    
    int sum = 0;
    double dsum = 0.0;
    
    /* Complex loop with conditionals */
    for (int i = 0; i < 20; i++) {
        if (i & 1) {
            sum += arr[i];
            if (i < 10) {
                dsum += darr[i];
                /* Inline asm with clobber */
                __asm__ volatile (
                    "mov %0, %%eax\n\t"
                    "imul $3, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r" (arr[i])
                    :
                    : "eax", "edx", "cc"
                );
            }
        } else {
            sum -= arr[i];
        }
        
        /* Nested switch */
        switch (i % 4) {
            case 0:
                sum += 1;
                break;
            case 1:
                sum += trigger;
                break;
            case 2:
                /* Use different mode */
                {
                    long long lltemp = (long long)sum * i;
                    __asm__ volatile (
                        "movq %0, %%mm0\n\t"
                        "psrlq $1, %%mm0\n\t"
                        "movq %%mm0, %0\n\t"
                        : "+m" (lltemp)
                        :
                        : "mm0"
                    );
                    sum = (int)(lltemp & 0xFFFFFFFF);
                }
                break;
            case 3:
                sum = sum / 2;
                break;
        }
    }
    
    return sum + (int)dsum;
}

int main(int argc, char *argv[]) {
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    int result1 = stress_function(seed);
    printf("Result 1: %d\n", result1);
    
    int result2 = secondary_stress(seed);
    printf("Result 2: %d\n", result2);
    
    /* Mix results to prevent optimization */
    int final = result1 + result2;
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int output = final;
    
    return output % 256;
}
