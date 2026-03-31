/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) double external_func2(double x) { return x * 3.14159; }
__attribute__((noinline)) void* external_func3(void* p) { return (void*)((uintptr_t)p + 1); }

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_fptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables of different types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed + 0x1234;
    long long v4 = (long long)seed * 0xABCDEF;
    float v5 = (float)seed * 1.234f;
    double v6 = (double)seed * 5.678;
    char v7 = (char)(seed & 0xFF);
    short v8 = (short)(seed * 3);
    int* v9 = (int*)&seed;
    double v10 = v6 * 2.0;
    
    /* Intermediate result that will be used conditionally */
    int critical_value = v1 + v2 + v3;
    double fp_critical = v5 + v6;
    
    /* Complex nested conditional structure */
    if (v1 > 100) {
        /* First level */
        if (v2 < 500) {
            /* Use critical_value in inline asm with many clobbered registers */
            asm volatile (
                "add %0, %0, #1\n\t"
                : "+r" (critical_value)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                  "r8", "r9", "r10", "r11", "r12", "memory"
            );
            
            /* Call non-inlineable function with the value */
            critical_value = external_func1(critical_value);
        } else {
            /* Different path using floating point */
            fp_critical = external_func2(fp_critical);
        }
        
        /* Switch inside if */
        switch (v7 & 0x3) {
            case 0:
                critical_value += v8;
                /* Use goto with computed label */
                void* target = &&label_case0;
                volatile_fptr = target;
                goto *volatile_fptr;
            label_case0:
                break;
            case 1:
                critical_value *= 2;
                break;
            case 2:
                /* More inline asm with different mode (double) */
                asm volatile (
                    "fadd.d %0, %0, %1\n\t"
                    : "+f" (fp_critical)
                    : "f" (v10)
                    : "memory"
                );
                break;
            default:
                critical_value = external_func1(critical_value ^ v3);
        }
    } else if (v1 < -50) {
        /* Another branch with different register usage */
        for (int i = 0; i < 3; i++) {
            critical_value += i * v2;
            if (i == 1) {
                /* Use critical_value in asm with byte mode */
                char temp = (char)critical_value;
                asm volatile (
                    "and %0, %0, #0x7F\n\t"
                    : "+r" (temp)
                    :
                    : "cc", "memory"
                );
                critical_value = temp;
            }
        }
    } else {
        /* Default path with mixed computations */
        v4 = v4 + (long long)critical_value;
        critical_value = (int)(v4 & 0xFFFFFFFF);
    }
    
    /* More complex control flow with label addresses */
    static void* labels[] = { &&label1, &&label2, &&label3 };
    volatile int label_selector = v1 % 3;
    
    /* Force register pressure by using all variables before jump */
    int sum = v1 + v2 + v3 + (int)v4 + (int)v5 + (int)v6 + v7 + v8 + *v9;
    
    goto *labels[label_selector];
    
label1:
    critical_value += sum;
    goto after_labels;
    
label2:
    critical_value -= sum;
    /* Additional floating point use with different mode */
    asm volatile (
        "fmul.s %0, %0, %1\n\t"
        : "+f" (v5)
        : "f" (fp_critical)
        : "memory"
    );
    critical_value += (int)v5;
    goto after_labels;
    
label3:
    critical_value *= sum;
    /* Use pointer mode */
    v9 = external_func3(v9);
    critical_value += *v9;
    goto after_labels;
    
after_labels:
    
    /* Final computation using critical_value (makes it live across complex CFG) */
    int result = critical_value;
    
    /* Use all variables to keep them live and increase pressure */
    result += v2 - v3;
    result += (int)v4;
    result += (int)v5;
    result += (int)v6;
    result += v7;
    result += v8;
    if (v9) result += 1;
    result += (int)v10;
    
    /* More conditional use with volatile to prevent optimization */
    volatile int volatile_cond = seed;
    if (volatile_cond & 0x1) {
        asm volatile (
            "eor %0, %0, #0xFF\n\t"
            : "+r" (result)
            :
            : "cc"
        );
    }
    
    return result;
}

/* Additional stress functions with different patterns */
__attribute__((noinline))
int stress_function2(int base) {
    /* Different data types and modes */
    short s1 = base & 0xFFFF;
    int i1 = base * 2;
    long long ll1 = (long long)base * 1000;
    float f1 = (float)base / 3.0f;
    double d1 = (double)base / 7.0;
    
    /* Critical value with mixed usage */
    int crit = s1 + i1;
    
    /* Complex switch with fallthrough */
    switch (base % 5) {
        case 0:
            crit += 10;
            /* Fall through */
        case 1:
            crit *= 2;
            /* Inline asm with word mode */
            asm volatile (
                "mov %0, %0, lsl #2\n\t"
                : "+r" (crit)
                :
                : "cc"
            );
            break;
        case 2:
            /* Use floating point */
            f1 = f1 * 2.0f;
            crit += (int)f1;
            break;
        case 3:
            /* Use double mode */
            d1 = external_func2(d1);
            crit += (int)d1;
            break;
        case 4:
            /* Use long long mode */
            ll1 += crit;
            crit = (int)(ll1 & 0x7FFFFFFF);
            break;
    }
    
    /* Use crit after switch in another conditional */
    if (crit > 1000) {
        asm volatile (
            "sub %0, %0, #1000\n\t"
            : "+r" (crit)
            :
            : "cc"
        );
    }
    
    return crit + (int)f1 + (int)d1 + (int)ll1;
}

/* Main function that calls stress functions */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Call stress functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result ^= stress_function(argc + i * 100);
        result += stress_function2(argc + i * 50);
    }
    
    /* Use result to prevent elimination */
    volatile int* dummy = (volatile int*)malloc(sizeof(int));
    if (dummy) {
        *dummy = result;
        free((void*)dummy);
    }
    
    return result & 0xFF;
}
