/* early-remat-trigger.c
 * Program designed to trigger early rematerialization's privatize_cond_register_use
 * logic in GCC's early-remat.cc (lines 930-937)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) int external_func1(int x) {
    volatile int y = x;
    return y * 2;
}

__attribute__((noinline)) double external_func2(double x) {
    volatile double y = x;
    return y * 3.14159;
}

__attribute__((noinline)) void* external_func3(void* p) {
    volatile void* q = p;
    return (void*)((uintptr_t)q + 1);
}

/* Volatile function pointer to prevent inlining */
volatile void (*volatile_fp)(void) = NULL;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Many local variables of different types to create register pressure */
    volatile int v1 = seed;  /* Prevent optimization */
    int a = v1 + 1;
    int b = a * 2;
    int c = b / 3;
    long long d = (long long)a * b * c;
    float e = (float)a / 2.0f;
    double f = (double)b * 3.14159;
    char g = (char)(a & 0xFF);
    short h = (short)(b & 0xFFFF);
    int i = c ^ 0x12345678;
    long long j = d + 0xFFFFFFFF;
    float k = e * 2.71828f;
    double l = f / 1.41421;
    void* m = (void*)(uintptr_t)a;
    int n = i * 2;
    int o = n + 1;
    int p = o * 3;
    int q = p / 2;
    long long r = (long long)p * q;
    float s = (float)q / 3.0f;
    double t = (double)r * 1.23456;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = a + b + c + (int)d;
    
    /* Complex nested conditional structure */
    volatile int cond1 = key_result % 7;
    volatile int cond2 = key_result % 11;
    volatile int cond3 = key_result % 13;
    
    /* Label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = NULL;
    
    /* Deeply nested conditional blocks */
    if (cond1 > 3) {
        if (cond2 < 5) {
            switch (cond3 % 4) {
                case 0:
                    /* Use key_result in inline assembly with many clobbers */
                    asm volatile (
                        "/* Using key_result: %0 */"
                        : 
                        : "r" (key_result)
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory", "cc"
                    );
                    
                    /* Call non-inlineable function using key_result */
                    key_result = external_func1(key_result);
                    volatile_label_ptr = labels[0];
                    break;
                    
                case 1:
                    /* Different use with floating point */
                    double temp_f = external_func2((double)key_result);
                    key_result += (int)temp_f;
                    volatile_label_ptr = labels[1];
                    break;
                    
                case 2:
                    /* Pointer manipulation */
                    m = external_func3(m);
                    key_result += (int)(uintptr_t)m;
                    volatile_label_ptr = labels[2];
                    break;
                    
                default:
                    /* Mixed type computation */
                    key_result = (key_result * 3) / 2;
                    volatile_label_ptr = labels[3];
                    break;
            }
            
            /* Nested if inside switch case */
            if (key_result & 1) {
                /* More register pressure */
                asm volatile (
                    "/* More clobbers */"
                    :
                    :
                    : "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp",
                      "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                      "xmm6", "xmm7", "memory"
                );
                
                /* Use different modes */
                short temp_short = (short)key_result;
                char temp_char = (char)(key_result >> 8);
                key_result = temp_short * temp_char;
            }
        } else {
            /* Another conditional path */
            volatile int inner_cond = key_result % 17;
            for (int x = 0; x < 3; x++) {
                if (inner_cond > x) {
                    /* Loop creates live range splits */
                    key_result += x * 7;
                    
                    /* Use in arithmetic with different types */
                    long long temp_ll = (long long)key_result * 100;
                    key_result = (int)(temp_ll / 50);
                }
            }
            volatile_label_ptr = labels[4];
        }
    } else {
        /* Alternative path with different computation */
        key_result = key_result * key_result - key_result;
    }
    
    /* Opaque control flow using goto */
    if (volatile_label_ptr) {
        goto *volatile_label_ptr;
    }
    
label1:
    /* Use key_result after conditional block with different mode */
    double mode_test1 = (double)key_result;
    key_result += (int)(mode_test1 * 2.0);
    goto join_point;
    
label2:
    /* Different type usage */
    float mode_test2 = (float)key_result;
    key_result += (int)(mode_test2 * 3.0f);
    goto join_point;
    
label3:
    /* Char/short mode usage */
    char mode_test3 = (char)key_result;
    short mode_test4 = (short)key_result;
    key_result = mode_test3 * mode_test4;
    goto join_point;
    
label4:
    /* Long long mode usage */
    long long mode_test5 = (long long)key_result * 1000LL;
    key_result = (int)(mode_test5 / 100);
    goto join_point;
    
label5:
    /* Pointer mode usage */
    intptr_t mode_test6 = (intptr_t)key_result;
    key_result = (int)(mode_test6 ^ 0x5555);
    goto join_point;
    
join_point:
    /* Final computation using all variables to keep them live */
    int final_result = key_result;
    final_result += a + b + c;
    final_result += (int)d;
    final_result += (int)e;
    final_result += (int)f;
    final_result += g + h + i;
    final_result += (int)j;
    final_result += (int)k;
    final_result += (int)l;
    final_result += (int)(uintptr_t)m;
    final_result += n + o + p + q;
    final_result += (int)r;
    final_result += (int)s;
    final_result += (int)t;
    
    /* More inline assembly to increase register pressure */
    asm volatile (
        "/* Final clobber */"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",
          "r24", "r25", "r26", "r27", "r28", "r29", "r30", "r31",
          "memory", "cc"
    );
    
    return final_result;
}

/* Another layer of complexity */
__attribute__((noinline))
int wrapper_function(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Switch with multiple cases creating control flow complexity */
        switch (i % 5) {
            case 0:
                total += stress_function(i * 3);
                break;
            case 1:
                total += stress_function(i * 7 + 1);
                break;
            case 2:
                total += stress_function(i * 11 + 2);
                /* Nested switch */
                switch (total % 3) {
                    case 0:
                        total += 100;
                        break;
                    case 1:
                        total += 200;
                        break;
                    case 2:
                        total += 300;
                        break;
                }
                break;
            case 3:
                total += stress_function(i * 13 + 3);
                break;
            case 4:
                total += stress_function(i * 17 + 4);
                break;
        }
        
        /* Conditional goto within loop */
        if (i % 7 == 0) {
            volatile void* loop_label = &&loop_continue;
            if (total % 2 == 0) {
                goto *loop_label;
            }
        }
        
    loop_continue:
        /* Empty but creates control flow edge */
        ;
    }
    
    return total;
}

int main(int argc, char** argv) {
    int iterations = 10;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
        if (iterations > 100) iterations = 100; /* Prevent excessive runtime */
    }
    
    printf("Testing early rematerialization privatization logic...\n");
    
    int result = wrapper_function(iterations);
    
    printf("Result: %d\n", result);
    printf("Test completed.\n");
    
    return 0;
}
