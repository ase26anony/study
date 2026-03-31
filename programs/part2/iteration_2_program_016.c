/* early-remat-test.c
 * Test program to trigger early rematerialization privatization logic
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int y = x * 2;
    return y + 1;
}

__attribute__((noinline)) float external_func2(float x) {
    volatile float y = x * 3.14f;
    return y - 1.0f;
}

__attribute__((noinline)) double external_func3(double x) {
    volatile double y = x / 2.0;
    return y + 3.14159;
}

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = v1 + 1;
    int v3 = v2 * 2;
    int v4 = v3 - 5;
    int v5 = v4 / 2;
    int v6 = v5 * 3;
    int v7 = v6 + 7;
    int v8 = v7 - v1;
    int v9 = v8 * v2;
    int v10 = v9 / 3;
    
    /* Mixed data types for different machine modes */
    char c1 = (char)(v1 & 0xFF);
    short s1 = (short)(v2 * 2);
    long long ll1 = (long long)v3 * 1000000LL;
    float f1 = (float)v4 * 1.5f;
    double d1 = (double)v5 * 3.14159;
    
    /* More variables for additional pressure */
    int v11 = v10 + 11;
    int v12 = v11 * 12;
    int v13 = v12 - 13;
    int v14 = v13 / 14;
    int v15 = v14 * 15;
    float f2 = f1 * 2.0f;
    double d2 = d1 / 2.0;
    long long ll2 = ll1 + 999999LL;
    
    /* Critical intermediate result that will be used conditionally */
    int critical_value = v1 * v2 * v3;
    float critical_float = f1 * 2.0f;
    double critical_double = d1 * 3.0;
    
    /* Label addresses for complex control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[seed % 5];
    
    /* Complex nested conditional structure */
    if (v1 > 0) {
        if (v2 < 100) {
            switch (v3 % 4) {
                case 0:
                    /* Use critical_value in inline asm with many clobbers */
                    asm volatile (
                        "mov %0, %%eax\n\t"
                        "add $1, %%eax\n\t"
                        "mov %%eax, %0\n\t"
                        : "+r" (critical_value)
                        : 
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    
                    /* Call non-inlineable function using critical value */
                    v4 = external_func1(critical_value);
                    
                    /* Deeply nested if */
                    if (critical_value > 50) {
                        /* Use critical_float with floating point operations */
                        asm volatile (
                            "flds %0\n\t"
                            "fadd %%st(0), %%st(0)\n\t"
                            "fstps %0\n\t"
                            : "+m" (critical_float)
                            :
                            : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
                        );
                        
                        f2 = external_func2(critical_float);
                    }
                    break;
                    
                case 1:
                    /* Alternative path with different register usage */
                    critical_value += external_func1(v5);
                    
                    /* Complex computation with many temporary values */
                    for (int i = 0; i < 10; i++) {
                        v6 += critical_value * i;
                        v7 -= v6 / (i + 1);
                    }
                    break;
                    
                case 2:
                    /* Use goto with computed label */
                    if (v4 % 3 == 0) {
                        goto *volatile_label_ptr;
                    }
                    critical_value = v8 * v9;
                    break;
                    
                case 3:
                    /* Use critical_double with double precision */
                    asm volatile (
                        "fldl %0\n\t"
                        "fmul %%st(0), %%st(0)\n\t"
                        "fstpl %0\n\t"
                        : "+m" (critical_double)
                        :
                        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
                    );
                    
                    d2 = external_func3(critical_double);
                    break;
            }
            
            /* More conditional blocks */
            if (v10 > 20) {
                int temp = critical_value;
                for (int j = 0; j < 5; j++) {
                    temp += external_func1(j * critical_value);
                }
                critical_value = temp;
                
                /* Nested switch */
                switch (v11 % 3) {
                    case 0:
                        v12 = critical_value * 2;
                        break;
                    case 1:
                        v13 = critical_value + ll1;
                        break;
                    case 2:
                        v14 = critical_value - (int)critical_float;
                        break;
                }
            }
        } else {
            /* Else branch with its own complex logic */
            critical_value = external_func1(v15) * 3;
            
            /* Loop with conditional break */
            for (int k = 0; k < 8; k++) {
                if (k == critical_value % 4) {
                    /* Inline asm that clobbers many registers */
                    asm volatile (
                        "push %%eax\n\t"
                        "push %%ebx\n\t"
                        "mov %0, %%eax\n\t"
                        "mov %%eax, %%ebx\n\t"
                        "add $100, %%ebx\n\t"
                        "mov %%ebx, %0\n\t"
                        "pop %%ebx\n\t"
                        "pop %%eax\n\t"
                        : "+r" (critical_value)
                        :
                        : "cc", "memory"
                    );
                    break;
                }
                critical_value += k;
            }
        }
    } else {
        /* Outer else branch */
        critical_value = -critical_value;
        
        /* Use computed goto */
        if (v2 % 2 == 0) {
            void* target = labels[v3 % 5];
            volatile_func_ptr = target;
            if (v4 > 0) {
                goto *target;
            }
        }
    }

label1:
    /* Use critical_value after conditional block */
    v15 = critical_value * 3 + v14;
    
label2:
    /* More computations mixing different data types */
    f1 = critical_float + f2;
    ll1 = (long long)critical_value * 1000LL + ll2;
    
label3:
    /* Additional conditional use of critical_value */
    if (v15 % 7 == 0) {
        asm volatile (
            "imul $137, %0, %0\n\t"
            : "+r" (critical_value)
            :
            : "cc"
        );
    }
    
label4:
    /* Final aggregation of all values to prevent elimination */
    int result = critical_value + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15;
    result += (int)c1 + (int)s1 + (int)ll1 + (int)f1 + (int)d1;
    result += (int)f2 + (int)d2 + (int)ll2;
    
label5:
    return result;
}

/* Another complex function with different patterns */
__attribute__((noinline, optimize("no-tree-loop-optimize")))
int secondary_function(int base) {
    volatile int trigger = base;
    
    /* Array of values creating SSA web */
    int values[20];
    for (int i = 0; i < 20; i++) {
        values[i] = base * i + (i % 3);
    }
    
    /* Complex conditional computation */
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < 20; i++) {
        if (trigger % (i + 2) == 0) {
            /* Conditional use of intermediate value */
            int temp = values[i] * 3;
            
            /* Inline asm that forces register allocation */
            asm volatile (
                "mov %0, %%ecx\n\t"
                "lea (%%ecx, %%ecx, 2), %%ecx\n\t"
                "mov %%ecx, %0\n\t"
                : "+r" (temp)
                :
                : "ecx", "cc"
            );
            
            sum += temp;
            
            /* Nested condition */
            if (temp > 100) {
                prod *= temp;
                
                /* More register pressure */
                asm volatile (
                    "push %%eax\n\t"
                    "push %%ebx\n\t"
                    "push %%ecx\n\t"
                    "push %%edx\n\t"
                    "mov %0, %%eax\n\t"
                    "mov %%eax, %%ebx\n\t"
                    "imul %%ebx, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    "pop %%edx\n\t"
                    "pop %%ecx\n\t"
                    "pop %%ebx\n\t"
                    "pop %%eax\n\t"
                    : "+r" (prod)
                    :
                    : "cc", "memory"
                );
            }
        } else {
            /* Alternative path */
            sum -= values[i];
        }
        
        /* Modify trigger to create complex control flow */
        if (i % 4 == 0) {
            trigger = external_func1(trigger + i);
        }
    }
    
    /* Final conditional merge */
    return (trigger > 0) ? sum : prod;
}

int main(int argc, char *argv[]) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
        total += secondary_function(seed - i);
    }
    
    printf("Result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    return sink % 256;
}
