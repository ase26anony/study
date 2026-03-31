/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing -m32 -march=i686 -fno-dse -o test test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) int external_func2(int x) { return x ^ 0xAA55AA55; }
__attribute__((noinline)) float external_func3(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func4(double x) { return x * 2.5; }

/* Volatile function pointer to create opaque control flow */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump_target = NULL;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    int v4 = seed - 50;
    float f1 = seed * 1.1f;
    float f2 = seed * 2.2f;
    double d1 = seed * 3.3;
    double d2 = seed * 4.4;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000LL;
    int *ptr1 = &v2;
    float *ptr2 = &f1;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 * v2 + v3 - v4;
    float critical_float = f1 * f2 + (float)critical_value;
    double critical_double = d1 * d2 + (double)critical_float;
    
    /* Complex nested conditional structure */
    if (v1 > 0) {
        /* First level */
        if (v2 < 1000) {
            /* Second level */
            switch (v3 % 5) {
                case 0: {
                    /* Deeply nested block where critical_value is used */
                    int temp = critical_value;
                    
                    /* Inline assembly that clobbers many registers */
                    __asm__ volatile (
                        "movl %0, %%eax\n\t"
                        "addl $100, %%eax\n\t"
                        "movl %%eax, %0\n\t"
                        : "+r" (temp)
                        : 
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    
                    critical_value = temp + external_func1(v4);
                    
                    /* Use different data types */
                    f1 = external_func3(f1 + (float)critical_value);
                    d1 = external_func4(d1 + (double)critical_value);
                    
                    /* Conditional goto using label address */
                    void* target = &&label_special;
                    volatile_jump_target = (jump_func_t)target;
                    break;
                }
                case 1:
                    critical_value = external_func2(critical_value);
                    /* Fall through */
                case 2:
                    /* More register pressure */
                    ll1 += (long long)critical_value;
                    c1 = (char)(critical_value & 0xFF);
                    s1 = (short)(critical_value & 0xFFFF);
                    break;
                default:
                    /* Mixed mode operations */
                    critical_float = (float)critical_value * 1.234f;
                    critical_double = (double)critical_value * 2.345;
                    break;
            }
            
            /* Use volatile variable to prevent optimization */
            volatile int volatile_check = v1;
            if (volatile_check > 50) {
                /* Another conditional use of critical_value */
                __asm__ volatile (
                    "imull $7, %0\n\t"
                    : "+r" (critical_value)
                    :
                    : "cc", "memory"
                );
            }
        } else {
            /* Alternative path with different register usage */
            for (int i = 0; i < 10; i++) {
                v4 += external_func1(v4);
                f2 += external_func3(f2);
                d2 += external_func4(d2);
            }
        }
        
        /* Label for goto target */
        label_special:
        /* Use critical_value after conditional block */
        critical_value = critical_value * 2 + v1;
        
        /* Potential goto through volatile pointer */
        if (volatile_jump_target && (v1 % 3 == 0)) {
            goto *volatile_jump_target;
        }
    } else {
        /* Else branch with its own complex flow */
        int counter = 0;
        while (counter < 5) {
            critical_value += external_func2(critical_value + counter);
            
            /* Mixed type operations to generate different RTL modes */
            switch (counter % 4) {
                case 0:
                    f1 = (float)critical_value / 2.0f;
                    break;
                case 1:
                    d1 = (double)critical_value / 3.0;
                    break;
                case 2:
                    ll1 = (long long)critical_value * 100LL;
                    break;
                case 3:
                    c1 = (char)(critical_value >> 8);
                    s1 = (short)(critical_value);
                    break;
            }
            counter++;
        }
    }
    
    /* Final computation using all variables to prevent elimination */
    int result = critical_value;
    result += (int)f1 + (int)f2;
    result += (int)d1 + (int)d2;
    result += c1 + s1;
    result += (int)(ll1 & 0xFFFFFFFF);
    result += *ptr1;
    result += (int)*ptr2;
    
    /* One more conditional jump possibility */
    void* labels[] = { &&end_label, &&extra_compute };
    volatile int idx = v1 % 2;
    
    if (idx) {
        goto *labels[idx];
    }
    
    extra_compute:
    result = external_func1(result);
    
    end_label:
    return result;
}

/* Main function with different test cases */
int main() {
    int total = 0;
    
    /* Test with different seeds to explore various paths */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Alternate between different volatile jump targets */
        static void* targets[] = { NULL, &&dummy_label };
        volatile_jump_target = (jump_func_t)targets[i % 2];
    }
    
    dummy_label:
    printf("Result: %d\n", total);
    return 0;
}
