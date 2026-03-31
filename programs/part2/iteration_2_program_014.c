/* Compilation flags for best coverage:
   x86-64: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing -fearly-remat -c test.c
   x86-32: gcc -O2 -m32 -march=i686 -fno-dse -fearly-remat -c test.c
   ARM:    gcc -O2 -fno-schedule-insns -fearly-remat -c test.c
*/

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) float external_func2(float x) { return x * 1.5f; }
__attribute__((noinline)) double external_func3(double x) { return x / 3.14159; }

/* Volatile function pointer to prevent optimization */
static volatile void (*volatile_jump_target)(void) = 0;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables of different types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    short v3 = (short)(seed & 0xFFFF);
    char v4 = (char)(seed & 0xFF);
    long long v5 = (long long)seed * 1000LL;
    float v6 = (float)seed * 1.234f;
    double v7 = (double)seed * 3.456;
    int *v8 = &v2;
    float *v9 = &v6;
    double *v10 = &v7;
    
    int i, j;
    int result = 0;
    
    /* Complex computation with intermediate value that will be used conditionally */
    int intermediate = 0;
    for (i = 0; i < 100; i++) {
        intermediate += v1 * i;
        intermediate ^= v2;
        intermediate += (int)v3 * v4;
        
        /* Use volatile to prevent optimization of control flow */
        volatile int control = i % 7;
        
        /* Deeply nested conditional blocks */
        if (control == 0) {
            /* Nested if-else chain */
            if (v1 > 100) {
                intermediate = external_func1(intermediate);
                if (v2 < 50) {
                    /* Use intermediate in inline asm with many clobbered registers */
                    __asm__ volatile (
                        "movl %0, %%eax\n\t"
                        "addl $100, %%eax\n\t"
                        : 
                        : "r" (intermediate)
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                } else {
                    intermediate = intermediate * 3;
                }
            }
        } else if (control == 1) {
            /* Switch statement inside conditional */
            switch (v4 % 4) {
                case 0:
                    intermediate += v5 & 0xFFFFFFFF;
                    break;
                case 1:
                    intermediate -= (int)(v6 * 10.0f);
                    break;
                case 2:
                    intermediate ^= (int)v7;
                    break;
                case 3:
                    intermediate = intermediate >> 3;
                    break;
            }
        } else if (control == 2) {
            /* Use intermediate with floating point operations */
            v6 = external_func2((float)intermediate);
            intermediate = (int)v6;
        } else if (control == 3) {
            /* Use intermediate with double operations */
            v7 = external_func3((double)intermediate);
            intermediate = (int)v7;
        } else if (control == 4) {
            /* Pointer arithmetic using intermediate */
            int temp[10];
            for (j = 0; j < 10; j++) {
                temp[j] = intermediate + j;
            }
            intermediate = temp[intermediate % 10];
        } else if (control == 5) {
            /* Label addresses and computed goto */
            static void* labels[] = { &&label1, &&label2, &&label3 };
            volatile int label_idx = intermediate % 3;
            
            /* Store label address in volatile function pointer */
            volatile_jump_target = (void (*)(void))labels[label_idx];
            
            /* Conditional goto */
            if (label_idx == 0) {
                goto *labels[0];
            } else if (label_idx == 1) {
                goto *labels[1];
            } else {
                goto *labels[2];
            }
            
        label1:
            intermediate += 1000;
            goto after_labels;
        label2:
            intermediate += 2000;
            goto after_labels;
        label3:
            intermediate += 3000;
            goto after_labels;
        after_labels:
            /* Continue execution */
            ;
        } else {
            /* Default case with more register pressure */
            long long ll1 = v5 + intermediate;
            long long ll2 = ll1 * 2;
            double d1 = v7 + intermediate;
            double d2 = d1 * 2.0;
            intermediate = (int)(ll2 + (long long)d2);
        }
        
        /* Use intermediate after conditional block - this creates the live range
           that might need privatization of conditional uses */
        result += intermediate;
        
        /* More operations to keep variables live */
        v2 = external_func1(v2);
        v3 = (short)(v3 + 1);
        v4 = (char)(v4 ^ 0x55);
        v5 += intermediate;
        v6 += (float)intermediate * 0.01f;
        v7 += (double)intermediate * 0.001;
    }
    
    /* Final aggregation using all variables */
    result += v2;
    result += (int)v3;
    result += (int)v4;
    result += (int)(v5 & 0xFFFFFFFF);
    result += (int)v6;
    result += (int)v7;
    result += *v8;
    result += (int)*v9;
    result += (int)*v10;
    
    return result;
}

/* Another function with different data types to test various modes */
__attribute__((noinline, optimize("no-tree-vectorize")))
long long mixed_type_stress(int iterations) {
    volatile char c1 = 'A', c2 = 'B';
    volatile short s1 = 1000, s2 = 2000;
    volatile int i1 = 100000, i2 = 200000;
    volatile long long ll1 = 10000000000LL, ll2 = 20000000000LL;
    volatile float f1 = 1.234f, f2 = 5.678f;
    volatile double d1 = 9.87654321, d2 = 1.23456789;
    
    long long accumulator = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Complex conditional computation with mixed types */
        volatile int selector = i % 8;
        long long temp = 0;
        
        switch (selector) {
            case 0:
                temp = (long long)c1 * c2;
                /* Inline asm with clobbered registers */
                __asm__ volatile (
                    "movq %0, %%rax\n\t"
                    "addq $100, %%rax\n\t"
                    : 
                    : "r" (temp)
                    : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", 
                      "r10", "r11", "r12", "r13", "r14", "r15", "memory"
                );
                break;
            case 1:
                temp = (long long)s1 * s2;
                break;
            case 2:
                temp = (long long)i1 * i2;
                break;
            case 3:
                temp = ll1 + ll2;
                break;
            case 4:
                temp = (long long)(f1 * f2 * 1000.0f);
                break;
            case 5:
                temp = (long long)(d1 * d2 * 1000.0);
                break;
            case 6:
                /* Nested conditionals */
                if (c1 > 'M') {
                    temp = s1 * 10;
                    if (i1 < 150000) {
                        temp += ll1 % 1000;
                    }
                } else {
                    temp = s2 * 20;
                }
                break;
            case 7:
                /* Computed goto with label addresses */
                {
                    static void* labels[] = { &&L1, &&L2, &&L3, &&L4 };
                    volatile int idx = i % 4;
                    goto *labels[idx];
                L1:
                    temp = 1111;
                    goto after_switch;
                L2:
                    temp = 2222;
                    goto after_switch;
                L3:
                    temp = 3333;
                    goto after_switch;
                L4:
                    temp = 4444;
                    goto after_switch;
                after_switch:
                    break;
                }
        }
        
        /* Use temp after switch - creates conditional use scenario */
        accumulator += temp;
        
        /* Modify variables to prevent dead code elimination */
        c1 ^= 0x20;
        c2 += 1;
        s1 = (short)(s1 + i);
        s2 = (short)(s2 - i);
        i1 = external_func1(i1);
        i2 = external_func1(i2);
        ll1 += temp;
        ll2 -= temp;
        f1 = external_func2(f1);
        f2 = external_func2(f2);
        d1 = external_func3(d1);
        d2 = external_func3(d2);
    }
    
    return accumulator;
}

int main(void) {
    int result1 = stress_function(42);
    long long result2 = mixed_type_stress(50);
    
    /* Use results to prevent optimization */
    volatile int final_result = result1 + (int)(result2 & 0xFFFFFFFF);
    
    return final_result % 256;
}
