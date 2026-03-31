/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Alternative: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline, noclone))
int external_func(int x, int y) {
    volatile int dummy = x + y;
    return dummy;
}

/* Another non-inlineable function */
__attribute__((noinline, noclone))
double external_double(double a, double b) {
    volatile double dummy = a * b;
    return dummy;
}

/* Function pointer with volatile to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = &external_func;

/* Stress function with complex control flow and register pressure */
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
    short s1 = seed & 0x7FFF;
    long long ll1 = (long long)seed * 1000000LL;
    int *ptr1 = &v1;
    float *ptr2 = &f1;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double double_key = 0.0;
    
    /* Complex computation creating register pressure */
    for (int i = 0; i < 10; i++) {
        v1 += i;
        v2 *= (i + 1);
        v3 -= i;
        v4 ^= i;
        f1 += i * 0.5f;
        f2 *= 1.1f;
        d1 += i * 0.25;
        d2 /= 1.05;
        c1 += i;
        s1 -= i;
        ll1 += i * 1000LL;
    }
    
    /* Store label addresses for complex control flow */
    void* labels[5];
    labels[0] = &&label0;
    labels[1] = &&label1;
    labels[2] = &&label2;
    labels[3] = &&label3;
    labels[4] = &&label4;
    
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* Complex conditional structure with deeply nested blocks */
    key_result = v1 + v2 + v3 + v4;
    double_key = d1 * d2 + f1 + f2;
    
    /* Use volatile variables in conditional expressions */
    volatile int cond1 = key_result > 1000;
    volatile int cond2 = double_key < 5000.0;
    volatile int cond3 = c1 > 50;
    
    /* First level if-else */
    if (cond1) {
        /* Nested if-else inside */
        if (cond2) {
            /* Switch statement inside nested if */
            switch (seed % 4) {
                case 0:
                    key_result += 100;
                    /* Use key_result in inline assembly with many clobbers */
                    __asm__ volatile (
                        "add %0, %0, #1\n\t"
                        "mov r0, %0\n\t"
                        "mov r1, %0\n\t"
                        "mov r2, %0\n\t"
                        "mov r3, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    break;
                case 1:
                    key_result -= 50;
                    /* Call non-inlineable function using key_result */
                    key_result = volatile_func_ptr(key_result, v1);
                    break;
                case 2:
                    /* More complex use with different data types */
                    double_key = external_double(double_key, d1);
                    key_result = (int)double_key;
                    break;
                default:
                    /* Use goto with computed label */
                    goto *volatile_label_ptr;
            }
        } else {
            /* Another path with register pressure */
            int temp = key_result * 2;
            for (int j = 0; j < 5; j++) {
                temp += j;
                __asm__ volatile (
                    "nop\n\t"
                    "nop\n\t"
                    : 
                    : 
                    : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
                );
            }
            key_result = temp;
        }
        
        /* Deeply nested if */
        if (cond3 && (s1 > 100)) {
            /* Use key_result in another inline asm */
            __asm__ volatile (
                "mov %0, %0, lsl #2\n\t"
                : "+r" (key_result)
                :
                : "cc", "memory"
            );
            
            /* Create more register pressure */
            float ftemp = f1 * f2;
            double dtemp = d1 / d2;
            int itemp = (int)(ftemp + dtemp);
            key_result += itemp;
        }
    } else {
        /* Else branch with different computation */
        key_result = v1 * v2 - v3;
        
        /* Switch with multiple cases */
        switch (c1 % 3) {
            case 0:
                key_result += ll1 & 0xFFFFFFFF;
                break;
            case 1:
                key_result -= s1;
                /* Inline asm with clobbered registers */
                __asm__ volatile (
                    "sub %0, %0, #10\n\t"
                    : "+r" (key_result)
                    :
                    : "r0", "r1", "r2", "memory"
                );
                break;
            case 2:
                /* Complex floating point computation */
                double_key = d1 * 2.0 + d2 * 3.0;
                key_result += (int)double_key;
                break;
        }
    }
    
    /* Label for computed goto */
label0:
    key_result += 10;
    goto label_end;
    
label1:
    key_result += 20;
    goto label_end;
    
label2:
    key_result += 30;
    goto label_end;
    
label3:
    key_result += 40;
    goto label_end;
    
label4:
    key_result += 50;
    /* Fall through */
    
label_end:
    
    /* Use key_result again outside conditional blocks */
    int final_result = key_result;
    
    /* More computations to keep variables live */
    final_result += (int)f1;
    final_result += (int)d1;
    final_result += c1;
    final_result += s1;
    final_result += ll1 & 0xFFFF;
    
    /* Final inline asm with many clobbers */
    __asm__ volatile (
        "add %0, %0, #100\n\t"
        "mov r0, %0\n\t"
        "mov r1, %0\n\t"
        "mov r2, %0\n\t"
        : "+r" (final_result)
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory"
    );
    
    return final_result;
}

int main() {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
