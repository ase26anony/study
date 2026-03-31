/* early-remat-test.c
 * Test program to trigger early rematerialization privatization logic
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_helper(int x, int y) {
    volatile int result = x * y;
    return result + 1;
}

/* Another non-inlineable function */
__attribute__((noinline))
double external_double_helper(double a, double b) {
    volatile double temp = a / b;
    return temp * 2.0;
}

/* Function pointer with volatile to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = &external_helper;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    int v3 = seed * 2;
    int v4 = seed / 3;
    int v5 = seed - 5;
    int v6 = seed + 10;
    int v7 = seed * 3;
    int v8 = seed / 2;
    int v9 = seed - 8;
    int v10 = seed + 15;
    
    /* Mixed data types for different machine modes */
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    long long ll1 = (long long)seed * 1000;
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    void *ptr1 = &v1;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 + v2 + v3 + v4;
    
    /* Complex conditional with deeply nested structure */
    volatile int control = seed % 100;
    
    /* Label addresses for goto to create irreducible CFG */
    void *labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void *volatile_label_ptr = labels[control % 5];
    
    /* First conditional block */
    if (control < 20) {
        /* Nested if-else chain */
        if (control < 10) {
            /* Use critical_value in inline asm with many clobbers */
            asm volatile (
                "/* Using critical_value: %0 */"
                : 
                : "r" (critical_value)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory", "cc"
            );
            
            /* Call non-inlineable function */
            critical_value = volatile_func_ptr(critical_value, v5);
        } else {
            /* Different computation path */
            critical_value += external_helper(v6, v7);
        }
        
        /* Switch inside if to create more complex CFG */
        switch (control % 4) {
            case 0:
                critical_value *= 2;
                /* Use mixed types */
                f1 = critical_value * 0.5f;
                break;
            case 1:
                critical_value /= 2;
                d1 = critical_value * 0.25;
                break;
            case 2:
                critical_value <<= 1;
                ll1 = critical_value * 100LL;
                break;
            case 3:
                critical_value >>= 1;
                /* Use pointer arithmetic */
                ptr1 = (char*)ptr1 + critical_value;
                break;
        }
    } 
    else if (control < 40) {
        /* Second major branch with its own nested conditions */
        volatile int inner_control = control % 30;
        
        for (int i = 0; i < 3; i++) {
            if (inner_control < 15) {
                /* Use critical_value in computation with double */
                d1 = external_double_helper(critical_value, v8 + i);
                critical_value = (int)d1;
            } else {
                /* Use critical_value with float */
                f1 = critical_value * 1.1f;
                critical_value = (int)f1;
            }
            
            /* Indirect jump to create irreducible CFG */
            if (i == 1 && inner_control > 20) {
                goto *volatile_label_ptr;
            }
        }
        
label2:
        /* More computations after label */
        critical_value += v9 * v10;
    }
    else if (control < 60) {
        /* Third branch with switch statement */
        switch (control % 6) {
            case 0:
            case 1:
                /* Use critical_value with char/short types */
                c1 = critical_value & 0xFF;
                s1 = critical_value & 0xFFFF;
                critical_value = c1 + s1;
                break;
            case 2:
            case 3:
                /* Long long operations */
                ll1 = critical_value * 1000LL;
                critical_value = (int)(ll1 / 100);
                break;
            case 4:
            case 5:
                /* Float/double operations */
                f1 = critical_value * 3.14f;
                d1 = f1 * 2.0;
                critical_value = (int)d1;
                break;
        }
        
        /* Another inline asm with register clobbering */
        asm volatile (
            "/* Second asm use of critical_value: %0 */"
            : 
            : "r" (critical_value)
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
    }
    else if (control < 80) {
label3:
        /* Loop with conditional use */
        int sum = 0;
        for (int i = 0; i < 5; i++) {
            if (i % 2 == 0) {
                sum += critical_value * i;
            } else {
                sum += critical_value / (i + 1);
            }
            
            /* Volatile store to prevent optimization */
            volatile int temp = sum;
            (void)temp;
        }
        critical_value = sum;
    }
    else {
label4:
        /* Final branch with more complex operations */
        double temp_d = critical_value;
        for (int i = 0; i < 4; i++) {
            temp_d = temp_d * 1.1 + i;
            
            /* Conditional goto within loop */
            if (i == 2 && control > 90) {
                goto *labels[control % 5];
            }
        }
        critical_value = (int)temp_d;
    }

label1:
    /* Use critical_value again after conditional blocks */
    int final_result = critical_value;
    
    /* More computations using all variables to keep them live */
    final_result += v1 - v2 + v3 * v4 - v5 + v6 / (v7 + 1);
    final_result += (int)f1 + (int)d1 + (int)ll1 + c1 + s1;
    final_result += (intptr_t)ptr1 & 0xFF;
    
    /* Final inline asm to increase register pressure */
    asm volatile (
        "/* Final computation with many clobbers */"
        : "+r" (final_result)
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "memory", "cc"
    );
    
    return final_result;
}

label5:
/* Another label for goto targets */

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        int seed = i * 37 + argc;
        int result = stress_function(seed);
        total += result;
        
        /* Volatile to prevent loop optimization */
        volatile int temp = result;
        (void)temp;
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different data patterns */
    if (argc > 1) {
        int extra_seed = atoi(argv[1]);
        for (int i = 0; i < 50; i++) {
            total += stress_function(extra_seed + i * 19);
        }
        printf("Final total: %d\n", total);
    }
    
    return total & 0xFF;
}
