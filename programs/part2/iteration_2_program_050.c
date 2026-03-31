/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) double external_func2(double x) { return x * 3.14159; }
__attribute__((noinline)) void external_side_effect(void) { asm volatile("" ::: "memory"); }

/* Volatile function pointer to prevent optimization */
static void (*volatile volatile_func_ptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 100;
    long long v4 = seed * 1000LL;
    float v5 = seed * 1.5f;
    double v6 = seed * 2.71828;
    char v7 = seed & 0xFF;
    short v8 = seed & 0xFFFF;
    int *v9 = &v2;
    double v10 = 0.0;
    float v11 = 0.0f;
    int v12 = 0;
    long long v13 = 0;
    double v14 = 0.0;
    int v15 = 0;
    float v16 = 0.0f;
    int v17 = 0;
    double v18 = 0.0;
    int v19 = 0;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 + v2 + v3;
    
    /* Complex conditional structure with deeply nested blocks */
    if (v1 > 0) {
        if (v2 % 3 == 0) {
            switch (v3 & 7) {
                case 0:
                    critical_value = external_func1(critical_value);
                    /* Use critical_value in inline asm with many clobbers */
                    asm volatile (
                        "add %[val], %[val], #1\n\t"
                        : [val] "+r" (critical_value)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "memory"
                    );
                    v4 = critical_value * 2LL;
                    break;
                case 1:
                    critical_value *= 2;
                    /* Another asm with different clobbers */
                    asm volatile (
                        "mov %[val], %[val], lsl #2\n\t"
                        : [val] "+r" (critical_value)
                        :
                        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
                    );
                    v5 = critical_value * 0.5f;
                    break;
                default:
                    critical_value = critical_value ^ 0x12345678;
                    v6 = external_func2(critical_value);
                    break;
            }
            
            /* Nested if inside switch case */
            if (v4 > 1000LL) {
                int temp = critical_value;
                /* Mixed type computation */
                v10 = (double)temp * v6;
                v11 = (float)temp + v5;
                
                /* Use goto with computed label address */
                void *label_ptr = &&label1;
                volatile_func_ptr = label_ptr;
                if (v1 & 1) {
                    goto *label_ptr;
                }
            }
        } else if (v2 % 5 == 0) {
            /* Different conditional path */
            critical_value = critical_value << (v1 & 3);
            v7 = critical_value & 0xFF;
            v8 = critical_value & 0xFFFF;
            
            /* Create irreducible control flow with label addresses */
            static void *labels[] = { &&label2, &&label3, &&label4 };
            void *jump_target = labels[v3 % 3];
            volatile_func_ptr = jump_target;
            
            if (v1 & 2) {
                goto *jump_target;
            }
        }
    } else {
        /* Alternative path with different register usage */
        critical_value = -critical_value;
        asm volatile (
            "sub %[val], %[val], #100\n\t"
            : [val] "+r" (critical_value)
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "memory"
        );
    }
    
    /* Use critical_value after conditional blocks */
    v12 = critical_value * 3;
    v13 = (long long)critical_value * v4;
    v14 = (double)critical_value / (v6 + 1.0);
    
    /* More computations to keep variables live */
    v15 = v12 + v2;
    v16 = v5 + v11;
    v17 = v3 * critical_value;
    v18 = v10 + v14;
    v19 = external_func1(v15);
    
    /* Label for goto targets */
label1:
    v15 += 100;
    
label2:
    v16 += 20.0f;
    
label3:
    v17 *= 2;
    
label4:
    v18 *= 1.5;
    
    /* Final aggregation to prevent elimination */
    int result = critical_value + v12 + v15 + v17 + v19;
    result += (int)v5 + (int)v6 + (int)v10 + (int)v11 + (int)v14 + (int)v16 + (int)v18;
    result += (int)(v4 & 0xFFFFFFFF) + (int)(v13 & 0xFFFFFFFF);
    result += v7 + v8;
    
    return result;
}

/* Additional test with different modes */
__attribute__((noinline, optimize("no-gcse")))
long long test_mixed_modes(int base) {
    volatile char c1 = base & 0xFF;
    volatile short s1 = base & 0xFFFF;
    volatile int i1 = base;
    volatile long long ll1 = (long long)base * 100;
    volatile float f1 = base * 1.234f;
    volatile double d1 = base * 5.678;
    
    /* Variable used conditionally with different modes */
    long long mixed_value = ll1;
    
    /* Complex conditional with mixed type operations */
    if (c1 > 50) {
        mixed_value += i1;
        /* Use in asm with char mode */
        asm volatile (
            "add %[val], %[val], #10\n\t"
            : [val] "+r" (mixed_value)
            :
            : "r0", "r1", "r2", "r3", "r4", "memory"
        );
        
        if (s1 < 1000) {
            /* Convert to float and back */
            f1 = (float)mixed_value;
            mixed_value = (long long)f1;
            
            /* Jump to label via volatile pointer */
            void *target = &&mixed_label;
            volatile_func_ptr = target;
            if (i1 & 4) {
                goto *target;
            }
        }
    }
    
    /* Use mixed_value after conditional */
    d1 = (double)mixed_value * 2.0;
    ll1 = mixed_value * 3;
    
mixed_label:
    return mixed_value + ll1 + (long long)d1 + (long long)f1 + i1 + s1 + c1;
}

int main(void) {
    int total = 0;
    
    /* Test with different seeds to explore various paths */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        total += test_mixed_modes(i) & 0xFFFFFFFF;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
