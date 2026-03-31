/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Alternative: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_helper(int x, int y) {
    volatile int dummy = x + y;
    return dummy * 2;
}

/* Another non-inlineable function with different signature */
__attribute__((noinline))
double double_helper(double a, double b) {
    volatile double tmp = a;
    return tmp * b;
}

/* Function pointer with volatile to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = &external_helper;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 7;
    int v4 = seed - 3;
    int v5 = seed * 3;
    int v6 = seed / 2;
    int v7 = seed % 11;
    int v8 = seed ^ 0x55;
    int v9 = seed | 0xAA;
    int v10 = seed & 0xFF;
    
    /* Mixed data types for different machine modes */
    char c1 = seed & 0xFF;
    short s1 = seed * 2;
    long long ll1 = (long long)seed * 1000;
    float f1 = seed * 1.5f;
    double d1 = seed * 2.5;
    
    /* Pointer variables */
    int *p1 = &v1;
    int *p2 = &v2;
    
    /* Key intermediate result that will be used conditionally */
    int critical_value = v1 + v2 + v3 + v4;
    double critical_double = d1 + f1;
    long long critical_ll = ll1 + s1;
    
    /* Complex conditional structure with nested blocks */
    volatile int control = seed % 17;
    
    /* Label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr;
    
    /* First level if-else */
    if (control < 5) {
        volatile_label_ptr = labels[0];
        /* Nested switch inside if */
        switch (seed % 7) {
            case 0:
                critical_value = external_helper(critical_value, v5);
                /* Use critical_value in inline asm with many clobbers */
                asm volatile (
                    "add %[val], %[val], #1\n\t"
                    : [val] "+r" (critical_value)
                    : 
                    : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                      "memory", "cc"
                );
                break;
            case 1:
                critical_double = double_helper(critical_double, d1);
                /* Different mode (double) */
                asm volatile (
                    "fadd d0, d0, d1\n\t"
                    : "+w" (critical_double)
                    : "w" (d1)
                    : "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
                      "memory"
                );
                break;
            case 2:
                /* Use critical_ll (DImode) */
                asm volatile (
                    "add x0, x0, x1\n\t"
                    : "+r" (critical_ll)
                    : "r" (ll1)
                    : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
                      "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
                      "memory"
                );
                break;
            default:
                critical_value += v6;
        }
        goto *volatile_label_ptr;
    } else if (control < 10) {
        volatile_label_ptr = labels[1];
        /* Another nested conditional */
        for (int i = 0; i < 3; i++) {
            if (i % 2 == 0) {
                critical_value = volatile_func_ptr(critical_value, v7);
                /* More register pressure */
                asm volatile (
                    "mov %0, %0, lsl #2\n\t"
                    : "+r" (critical_value)
                    :
                    : "r0", "r1", "r2", "r3", "r4", "r5", "memory", "cc"
                );
            }
        }
        goto *volatile_label_ptr;
    } else {
        volatile_label_ptr = labels[2];
        /* Deeply nested if-else chain */
        if (v8 > 100) {
            if (v9 < 50) {
                if (v10 != 0) {
                    critical_value = external_helper(critical_value, v8);
                    /* Use with char mode */
                    asm volatile (
                        "and %[val], %[val], #0xFF\n\t"
                        : [val] "+r" (c1)
                        :
                        : "r0", "r1", "r2", "memory", "cc"
                    );
                    critical_value += c1;
                }
            }
        }
        goto *volatile_label_ptr;
    }

label1:
    /* Use critical_value after conditional block */
    v1 = critical_value * 2;
    
    /* More computations to keep variables live */
    v2 = critical_value + v3;
    v3 = critical_value - v4;
    
    /* Switch with multiple cases using the critical value */
    switch (critical_value % 5) {
        case 0:
            v5 = critical_value & 0xF;
            break;
        case 1:
            v6 = critical_value | 0xF0;
            break;
        case 2:
            v7 = critical_value ^ 0xAA;
            break;
        case 3:
            v8 = critical_value << 2;
            break;
        case 4:
            v9 = critical_value >> 1;
            break;
    }
    
    /* Use double value */
    d1 = critical_double * 2.0;
    f1 = (float)critical_double;
    
    /* Use long long */
    ll1 = critical_ll * 3;
    s1 = (short)(critical_ll & 0xFFFF);

label2:
    /* More conditional use */
    if (v1 > v2) {
        critical_value = external_helper(critical_value, v1);
    }

label3:
    /* Loop with conditional break */
    for (int i = 0; i < 10; i++) {
        if (i == critical_value % 10) {
            critical_value += i;
            break;
        }
        v10 += i;
    }

label4:
    /* Final aggregation to prevent elimination */
    int result = critical_value + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += (int)c1 + s1 + (int)ll1 + (int)f1 + (int)d1;
    
    return result;

label5:
    /* Alternative path */
    return seed * 2;
}

/* Main function with different seeds */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call stress function multiple times with different inputs */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        total += stress_function(i * 2);
        total += stress_function(i * 3 + 1);
    }
    
    /* Use result to prevent dead code elimination */
    volatile int final_result = total;
    
    return final_result % 256;
}
