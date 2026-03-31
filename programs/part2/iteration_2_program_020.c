/* Test case for early-remat.cc privatization logic */
/* Compile with: -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline, noclone))
int external_helper(int x) {
    volatile static int counter = 0;
    counter += x;
    return counter;
}

/* Another non-inlineable function */
__attribute__((noinline, noclone))
double external_double_helper(double x) {
    volatile static double accumulator = 0.0;
    accumulator += x;
    return accumulator;
}

/* Function pointer to prevent inlining */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_helper;

/* Label address storage for complex control flow */
static void* volatile label_ptr = NULL;

__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    float f1 = seed * 1.5f;
    float f2 = seed * 2.5f;
    double d1 = seed * 3.14159;
    double d2 = seed * 2.71828;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000LL;
    long long ll2 = (long long)seed * 2000LL;
    
    /* Pointer variables */
    int* p1 = &v2;
    int* p2 = &v3;
    float* fp1 = &f1;
    double* dp1 = &d1;
    
    /* Intermediate result that will be used conditionally */
    int intermediate_result = 0;
    double float_intermediate = 0.0;
    
    /* Complex computation creating register pressure */
    intermediate_result = v1 + v2 + v3 + v4;
    intermediate_result *= (c1 + s1);
    
    /* Mixed type computations forcing mode conversions */
    float_intermediate = (double)intermediate_result + d1 + d2;
    float_intermediate *= (f1 + f2);
    
    /* Store label addresses for goto */
    void* label1 = &&LABEL_1;
    void* label2 = &&LABEL_2;
    void* label3 = &&LABEL_3;
    
    /* Volatile store to prevent optimization */
    volatile int control = seed % 7;
    
    /* Complex nested conditional structure */
    if (control > 0) {
        /* First level nesting */
        intermediate_result = external_helper(intermediate_result);
        
        if (control > 2) {
            /* Second level nesting */
            float_intermediate = external_double_helper(float_intermediate);
            
            /* Deeply nested switch */
            switch (control) {
                case 3:
                    /* Use intermediate_result in inline asm with many clobbers */
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        : "+r" (intermediate_result)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory"
                    );
                    label_ptr = label1;
                    break;
                    
                case 4:
                    /* Different mode usage - double */
                    asm volatile (
                        "fadd d0, d0, d1\n\t"
                        : "+w" (float_intermediate)
                        :
                        : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
                          "memory"
                    );
                    label_ptr = label2;
                    break;
                    
                case 5:
                    /* Mixed mode usage */
                    intermediate_result = (int)float_intermediate;
                    asm volatile (
                        "mov %0, %0, lsl #2\n\t"
                        : "+r" (intermediate_result)
                        :
                        : "r0", "r1", "r2", "r3", "memory"
                    );
                    label_ptr = label3;
                    break;
                    
                default:
                    /* Use function pointer to create opaque control flow */
                    intermediate_result = volatile_func_ptr(intermediate_result);
                    break;
            }
            
            /* Conditional goto based on volatile */
            if (v1 > 100) {
                goto *label_ptr;
            }
        } else if (control == 2) {
            /* Another conditional path */
            intermediate_result *= 3;
            float_intermediate /= 2.0;
        }
        
        /* Use intermediate_result after conditional block */
        intermediate_result += v1 * v2;
        
        LABEL_1:
        intermediate_result -= v3 * v4;
        
    } else {
        /* Alternative path */
        intermediate_result = v4 - v3;
        float_intermediate = d2 - d1;
        
        LABEL_2:
        intermediate_result *= 2;
    }
    
    /* Loop to create more register pressure */
    for (int i = 0; i < 10; i++) {
        v1 += i;
        v2 -= i;
        v3 *= (i + 1);
        v4 /= (i > 0 ? i : 1);
        
        /* Use intermediate_result in loop */
        intermediate_result += v1 + v2 + v3 + v4;
        
        /* More mixed type operations */
        if (i % 3 == 0) {
            float_intermediate += (double)intermediate_result / (i + 1);
            
            /* Inline asm with clobbers in loop */
            asm volatile (
                "cmp %0, #100\n\t"
                "ite gt\n\t"
                "subgt %0, %0, #50\n\t"
                "addle %0, %0, #10\n\t"
                : "+r" (intermediate_result)
                :
                : "cc", "r0", "r1", "memory"
            );
        }
    }
    
    LABEL_3:
    
    /* Final computation using all variables */
    int final_result = intermediate_result;
    final_result += (int)float_intermediate;
    final_result += (int)d1;
    final_result += (int)d2;
    final_result += f1 + f2;
    final_result += c1;
    final_result += s1;
    final_result += (int)(ll1 % 1000);
    final_result += (int)(ll2 % 1000);
    final_result += *p1 + *p2;
    final_result += (int)(*fp1);
    final_result += (int)(*dp1);
    
    /* Prevent tail call optimization */
    asm volatile ("" : : "r"(final_result) : "memory");
    
    return final_result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to exercise different paths */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Volatile operation to prevent loop optimization */
        volatile int dummy = i;
        (void)dummy;
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
