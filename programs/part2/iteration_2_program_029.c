/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
int __attribute__((noinline)) external_func1(int x) { return x * 3; }
int __attribute__((noinline)) external_func2(int x) { return x / 2; }
float __attribute__((noinline)) external_func3(float x) { return x * 2.5f; }
double __attribute__((noinline)) external_func4(double x) { return x / 1.5; }

/* Volatile function pointer to prevent optimization */
volatile void* volatile_fptr = NULL;

/* Stress function with complex control flow and register pressure */
int __attribute__((noinline, optimize("O0"))) stress_function(int seed) {
    /* Create many local variables of different types to increase register pressure */
    volatile int v1 = seed;
    int a1 = v1 + 1, a2 = v1 + 2, a3 = v1 + 3, a4 = v1 + 4;
    int b1 = v1 * 2, b2 = v1 * 3, b3 = v1 * 4, b4 = v1 * 5;
    float f1 = (float)v1 * 1.1f, f2 = (float)v1 * 2.2f, f3 = (float)v1 * 3.3f;
    double d1 = (double)v1 * 1.11, d2 = (double)v1 * 2.22, d3 = (double)v1 * 3.33;
    char c1 = (char)(v1 & 0xFF), c2 = (char)((v1 >> 8) & 0xFF);
    short s1 = (short)(v1 & 0xFFFF), s2 = (short)((v1 >> 16) & 0xFFFF);
    long long ll1 = (long long)v1 * 1000LL, ll2 = (long long)v1 * 2000LL;
    
    /* Intermediate result that will be used conditionally */
    int intermediate_result = 0;
    float float_intermediate = 0.0f;
    double double_intermediate = 0.0;
    
    /* Complex computation creating register pressure */
    intermediate_result = a1 + a2 + a3 + a4;
    intermediate_result += b1 - b2 + b3 - b4;
    
    /* Use volatile variables in conditional expressions */
    volatile int cond1 = 1, cond2 = 0, cond3 = 1, cond4 = 0;
    volatile int switch_cond = seed % 5;
    
    /* Define labels for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    
    /* Store label address in volatile pointer */
    volatile_fptr = labels[switch_cond % 5];
    
    /* Complex nested if-else structure */
    if (cond1) {
        if (cond2) {
            intermediate_result = external_func1(intermediate_result);
            float_intermediate = f1 + f2;
        } else {
            intermediate_result = external_func2(intermediate_result);
            float_intermediate = f2 + f3;
            
            if (cond3) {
                /* Nested conditional block where intermediate_result is used */
                int temp = intermediate_result;
                
                /* Inline assembly that clobbers many registers */
                __asm__ volatile (
                    "mov %0, %%eax\n\t"
                    "add $100, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r" (temp)
                    : 
                    : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                );
                
                intermediate_result = temp;
                float_intermediate = external_func3(float_intermediate);
            }
        }
        
        /* Switch statement with fall-through cases */
        switch (switch_cond) {
            case 0:
                intermediate_result += c1;
                /* Fall through */
            case 1:
                intermediate_result += s1;
                /* Use intermediate_result in conditional way */
                if (cond4) {
                    /* Another conditional use with inline assembly */
                    int local_copy = intermediate_result;
                    __asm__ volatile (
                        "imul $7, %0\n\t"
                        : "+r" (local_copy)
                        :
                        : "cc", "memory"
                    );
                    intermediate_result = local_copy;
                }
                break;
            case 2:
                intermediate_result *= 2;
                break;
            case 3:
                /* Conditional goto */
                goto *volatile_fptr;
            case 4:
                intermediate_result = intermediate_result >> 1;
                break;
            default:
                intermediate_result = 0;
        }
    } else {
        intermediate_result = -intermediate_result;
    }
    
    /* Label definitions for goto targets */
label1:
    intermediate_result += 100;
    goto label_end;
    
label2:
    intermediate_result += 200;
    /* Use different data types */
    double_intermediate = d1 + d2;
    goto label_end;
    
label3:
    intermediate_result += 300;
    float_intermediate = f3 * 2.0f;
    goto label_end;
    
label4:
    intermediate_result += 400;
    /* Use long long */
    ll1 += intermediate_result;
    goto label_end;
    
label5:
    intermediate_result += 500;
    /* Mixed type computation */
    double_intermediate = external_func4((double)intermediate_result);
    goto label_end;
    
label_end:
    
    /* Use intermediate_result again after conditional blocks */
    intermediate_result = intermediate_result * 2;
    
    /* More computations to keep variables live */
    a1 = intermediate_result + b1;
    a2 = intermediate_result + b2;
    
    /* Use all variables in final computation to prevent elimination */
    int final_result = intermediate_result;
    final_result += a1 + a2 + a3 + a4;
    final_result += b1 + b2 + b3 + b4;
    final_result += (int)f1 + (int)f2 + (int)f3;
    final_result += (int)d1 + (int)d2 + (int)d3;
    final_result += c1 + c2 + s1 + s2;
    final_result += (int)(ll1 & 0xFFFFFFFF) + (int)(ll2 & 0xFFFFFFFF);
    final_result += (int)float_intermediate + (int)double_intermediate;
    
    return final_result;
}

/* Another function with different mode types */
double __attribute__((noinline)) mixed_mode_function(int seed) {
    volatile char vc = (char)seed;
    volatile short vs = (short)seed;
    volatile int vi = seed;
    volatile long long vll = (long long)seed * 100;
    volatile float vf = (float)seed * 1.5f;
    volatile double vd = (double)seed * 2.5;
    
    char c = vc + 10;
    short s = vs + 100;
    int i = vi + 1000;
    long long ll = vll + 10000;
    float f = vf + 100.0f;
    double d = vd + 1000.0;
    
    /* Conditional use of different typed variables */
    double result = 0.0;
    
    if (vc > 0) {
        /* Use char in computation */
        result += c;
        
        if (vs > 100) {
            /* Use short */
            result += s;
            
            /* Inline assembly with char/short modes */
            short temp_s = s;
            __asm__ volatile (
                "addw $50, %0\n\t"
                : "+r" (temp_s)
                :
                : "cc"
            );
            s = temp_s;
        }
    }
    
    if (vi % 2) {
        /* Use int */
        result += i;
        
        /* Complex conditional block */
        for (int j = 0; j < 3; j++) {
            if (j == 1) {
                int temp_i = i;
                /* Assembly clobbering registers */
                __asm__ volatile (
                    "addl $1000, %0\n\t"
                    : "+r" (temp_i)
                    :
                    : "cc", "memory"
                );
                i = temp_i;
            }
            result += j;
        }
    }
    
    if (vll > 0) {
        /* Use long long (DImode) */
        result += (double)ll;
    }
    
    if (vf > 0.0f) {
        /* Use float (SFmode) */
        result += (double)f;
        
        float temp_f = f;
        __asm__ volatile (
            "fadds %0, %0, %0\n\t"
            : "+f" (temp_f)
            :
        );
        f = temp_f;
    }
    
    if (vd > 0.0) {
        /* Use double (DFmode) */
        result += d;
        
        double temp_d = d;
        __asm__ volatile (
            "faddd %0, %0, %0\n\t"
            : "+f" (temp_d)
            :
        );
        d = temp_d;
    }
    
    return result;
}

int main() {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        total += (int)mixed_mode_function(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
