/* early-remat-test.c
 * Designed to trigger early_remat::privatize_cond_register_use
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-test.c -o early-remat-test
 * Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat early-remat-test.c -o early-remat-test-32
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_helper(int x, int y) {
    volatile int result = x * y + 37;
    return result;
}

/* Another non-inlineable function */
__attribute__((noinline))
double double_helper(double a, double b) {
    volatile double temp = a * b;
    return temp / 2.0;
}

/* Function pointer with unknown target */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = external_helper;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed + 1;
    volatile int v2 = seed * 2;
    volatile int v3 = seed / 3;
    volatile int v4 = seed - 4;
    volatile int v5 = seed + 5;
    volatile int v6 = seed * 6;
    volatile int v7 = seed / 7;
    volatile int v8 = seed - 8;
    
    /* Mixed data types for different machine modes */
    char c1 = (char)(seed + 10);
    short s1 = (short)(seed * 20);
    int i1 = seed * 30;
    long long ll1 = (long long)seed * 40LL;
    float f1 = (float)seed * 1.5f;
    double d1 = (double)seed * 2.5;
    
    /* Pointer variables */
    int *p1 = &v1;
    int *p2 = &v2;
    double *dp1 = &d1;
    
    /* Critical intermediate result - this is the register we want to stress */
    int critical_value = seed * 100 + 12345;
    
    /* Complex conditional structure with deeply nested blocks */
    volatile int control = seed % 7;
    
    /* Label addresses for goto* - creates irreducible control flow */
    void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    volatile void *volatile_label_ptr = labels[control % 5];
    
    /* First conditional block */
    if (control > 0) {
        /* Nested if-else chain */
        if (control > 2) {
            /* Use critical_value in inline asm with many clobbered registers */
            /* This creates a conditional use that might need privatization */
            asm volatile (
                "/* Critical value usage in conditional path */\n\t"
                "add %0, %0, #1\n\t"
                : "+r" (critical_value)
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "memory", "cc"
            );
            
            /* Call non-inlineable function with critical_value */
            critical_value = volatile_func_ptr(critical_value, v1);
            
            /* Switch statement inside nested if */
            switch (control) {
                case 3:
                    critical_value += c1 * 2;  /* char mode */
                    break;
                case 4:
                    critical_value += s1 * 3;  /* short mode */
                    break;
                case 5:
                    critical_value += (int)(f1 * 4.0f);  /* float mode */
                    break;
                case 6:
                    critical_value += (int)(d1 * 5.0);   /* double mode */
                    break;
                default:
                    critical_value += ll1 % 100;  /* long long mode */
            }
        } else {
            /* Alternative path with different register usage */
            asm volatile (
                "/* Alternative path register usage */\n\t"
                "sub %0, %0, #2\n\t"
                : "+r" (critical_value)
                :
                : "r0", "r1", "r2", "memory", "cc"
            );
        }
        
        /* Indirect goto to create complex control flow */
        if (v1 % 3 == 0) {
            goto *volatile_label_ptr;
        }
    }
    
    /* Comeback point after potential jumps */
    int intermediate_result = critical_value;
    
label0:
    /* Use critical_value with different data types */
    f1 = (float)critical_value * 0.5f;
    d1 = (double)critical_value * 0.25;
    
label1:
    /* More register pressure */
    v1 = v2 + v3;
    v4 = v5 * v6;
    
    /* Use critical_value in another conditional */
    if (v7 % 2 == 0) {
        asm volatile (
            "/* Another conditional asm use */\n\t"
            "mov %0, %0, LSL #2\n\t"
            : "+r" (critical_value)
            :
            : "r0", "r1", "cc"
        );
        
        /* Mixed type computation */
        ll1 = (long long)critical_value * 1000LL;
        critical_value = (int)(ll1 % 10000);
    }
    
label2:
    /* Loop to create more register pressure */
    for (int i = 0; i < 5; i++) {
        v1 += v2;
        v3 -= v4;
        v5 *= v6;
        
        /* Use critical_value inside loop */
        if (i % 2 == 0) {
            critical_value += external_helper(i, v1);
        }
    }
    
label3:
    /* Pointer arithmetic with critical_value */
    *p1 = critical_value + 10;
    *p2 = critical_value - 20;
    
    /* Double precision computation */
    *dp1 = double_helper(d1, (double)critical_value);
    
label4:
    /* Final aggregation of all values to prevent elimination */
    int final_result = critical_value;
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    final_result += c1 + s1 + i1;
    final_result += (int)ll1;
    final_result += (int)f1;
    final_result += (int)d1;
    final_result += *p1 + *p2;
    
    return final_result;
}

/* Another stress function with different patterns */
__attribute__((noinline, optimize("no-tree-loop-optimize")))
int stress_function2(int base) {
    /* Variables with different lifetimes */
    int a = base * 3;
    int b = base * 5;
    int c = base * 7;
    volatile int d = base * 11;
    
    /* Critical value with complex computation */
    long long critical_ll = (long long)base * 1000000LL;
    
    /* Complex switch with fall-through */
    volatile int switch_val = base % 10;
    
    switch (switch_val) {
        case 0:
            critical_ll += a * 100LL;
            /* Fall through */
        case 1:
            critical_ll += b * 200LL;
            /* Fall through */
        case 2:
            critical_ll += c * 300LL;
            {
                /* Nested block with asm */
                int temp = (int)(critical_ll % 1000);
                asm volatile (
                    "/* Nested switch asm */\n\t"
                    "add %0, %0, %1\n\t"
                    : "+r" (temp)
                    : "r" (d)
                    : "r0", "r1", "r2", "cc"
                );
                critical_ll = temp;
            }
            break;
        case 3:
        case 4:
            critical_ll = critical_ll / 2;
            break;
        default:
            critical_ll = critical_ll * 2;
    }
    
    /* Use critical_ll after switch */
    int result = (int)critical_ll;
    
    /* Conditional with inline asm that clobbers many registers */
    if (result > 1000) {
        asm volatile (
            "/* Many register clobber */\n\t"
            "mov %0, %0, ASR #1\n\t"
            : "+r" (result)
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory", "cc"
        );
    }
    
    return result + a + b + c + d;
}

int main(int argc, char **argv) {
    int seed = 42;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    printf("Testing early rematerialization privatization...\n");
    
    /* Call stress functions multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
        total += stress_function2(seed + i * 7);
    }
    
    printf("Result: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
