/* early_remat_test.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early_remat_test.c -o test
 * Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat early_remat_test.c -o test32
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with unknown target */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_func;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables of mixed types to increase register pressure */
    volatile int v1 = seed;
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 100, s2 = 200, s3 = 300;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    long long ll1, ll2, ll3, ll4;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    void *ptr1, *ptr2, *ptr3;
    
    /* Complex computation creating intermediate values */
    i1 = v1 * 2;
    i2 = i1 + s1;
    i3 = i2 * c1;
    ll1 = (long long)i3 * 1000LL;
    f1 = (float)i3 * 3.14f;
    d1 = (double)ll1 * 2.71828;
    
    /* Key intermediate result - this register will have complex live range */
    int key_result = i3 * 7;
    
    /* Volatile control variables to prevent optimization */
    volatile int control1 = 1;
    volatile int control2 = 0;
    volatile int control3 = 1;
    
    /* Label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr;
    
    /* Deeply nested conditional blocks */
    if (control1) {
        if (external_func(seed) > 10) {
            switch (seed % 5) {
                case 0:
                    /* Use key_result in inline asm with many clobbers */
                    asm volatile (
                        "add %0, %0, #5\n\t"
                        : "+r" (key_result)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory"
                    );
                    volatile_label_ptr = labels[0];
                    break;
                    
                case 1:
                    /* Different mode usage - char */
                    char temp_char = (char)key_result;
                    asm volatile (
                        "and %0, %0, #0xFF\n\t"
                        : "+r" (temp_char)
                        :
                        : "r0", "r1", "r2", "memory"
                    );
                    key_result = temp_char;
                    volatile_label_ptr = labels[1];
                    break;
                    
                case 2:
                    /* Short mode */
                    short temp_short = (short)key_result;
                    asm volatile (
                        "lsl %0, %0, #2\n\t"
                        : "+r" (temp_short)
                        :
                        : "r0", "r1", "memory"
                    );
                    key_result = temp_short;
                    volatile_label_ptr = labels[2];
                    break;
                    
                case 3:
                    /* Long long mode */
                    long long temp_ll = (long long)key_result * 100LL;
                    asm volatile (
                        "mov x0, %0\n\t"
                        "add x0, x0, #100\n\t"
                        "mov %0, x0\n\t"
                        : "+r" (temp_ll)
                        :
                        : "x0", "x1", "x2", "x3", "memory"
                    );
                    key_result = (int)(temp_ll / 100);
                    volatile_label_ptr = labels[3];
                    break;
                    
                case 4:
                    /* Float mode */
                    float temp_float = (float)key_result;
                    asm volatile (
                        "fadd s0, s0, s0\n\t"
                        : "+w" (temp_float)
                        :
                        : "s0", "s1", "s2", "s3", "memory"
                    );
                    key_result = (int)temp_float;
                    volatile_label_ptr = labels[4];
                    break;
            }
            
            /* Another level of nesting */
            if (control2) {
                /* Use goto with computed label */
                goto *volatile_label_ptr;
                
                label1:
                key_result += 100;
                goto after_labels;
                
                label2:
                key_result += 200;
                goto after_labels;
                
                label3:
                key_result += 300;
                goto after_labels;
                
                label4:
                key_result += 400;
                goto after_labels;
                
                label5:
                key_result += 500;
                goto after_labels;
                
                after_labels:
                ;
            }
        } else {
            /* Alternative path using key_result */
            for (int j = 0; j < 3; j++) {
                key_result += external_func(j);
            }
        }
    }
    
    /* More register pressure with mixed types */
    i4 = key_result * 2;
    i5 = i4 + s2;
    i6 = i5 * c2;
    ll2 = (long long)i6 * 2000LL;
    f2 = (float)i6 * 1.618f;
    d2 = (double)ll2 * 3.14159;
    
    /* Use key_result again outside conditional blocks */
    int final_result = key_result;
    
    /* Complex loop with switch to split live ranges */
    for (int k = 0; k < 10; k++) {
        switch (k % 4) {
            case 0:
                final_result += i1;
                break;
            case 1:
                final_result += i2;
                /* Inline asm that uses and modifies final_result */
                asm volatile (
                    "mov r0, %0\n\t"
                    "add r0, r0, #42\n\t"
                    "mov %0, r0\n\t"
                    : "+r" (final_result)
                    :
                    : "r0", "r1", "memory"
                );
                break;
            case 2:
                final_result += i3;
                break;
            case 3:
                final_result += i4;
                /* Function call through volatile pointer */
                final_result += volatile_func_ptr(final_result);
                break;
        }
        
        /* More mixed-type computations to keep variables live */
        f3 = f1 + f2;
        d3 = d1 + d2;
        ll3 = ll1 + ll2;
        ptr1 = &i1;
        ptr2 = &i2;
        ptr3 = &i3;
    }
    
    /* Final aggregation using all variables */
    final_result += (int)f1 + (int)f2 + (int)f3;
    final_result += (int)d1 + (int)d2 + (int)d3;
    final_result += (int)ll1 + (int)ll2 + (int)ll3;
    final_result += (int)(intptr_t)ptr1 + (int)(intptr_t)ptr2 + (int)(intptr_t)ptr3;
    final_result += c1 + c2 + c3;
    final_result += s1 + s2 + s3;
    
    return final_result;
}

/* Another layer to increase complexity */
__attribute__((noinline))
int wrapper_function(int base) {
    int total = 0;
    
    /* Multiple calls with different parameters */
    for (int i = 0; i < 5; i++) {
        total += stress_function(base + i);
        
        /* Conditional with volatile variable */
        volatile int cond = i % 2;
        if (cond) {
            /* Additional computation path */
            total += external_func(total);
        } else {
            /* Alternative path */
            total -= external_func(total) / 2;
        }
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    printf("Starting early remat test with seed=%d\n", seed);
    
    int result = wrapper_function(seed);
    
    printf("Result: %d\n", result);
    
    /* Use result to prevent optimization */
    if (result > 1000000) {
        printf("Unexpectedly large result\n");
    }
    
    return result % 100;
}
