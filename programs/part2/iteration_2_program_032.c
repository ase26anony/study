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
    volatile int dummy = x;
    return dummy * 2;
}

__attribute__((noinline)) double external_func2(double x) {
    volatile double dummy = x;
    return dummy * 3.14159;
}

__attribute__((noinline)) void* external_func3(void* p) {
    volatile void* dummy = p;
    return dummy;
}

/* Volatile function pointer to prevent optimization */
volatile void (*volatile_fp)(void) = NULL;

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of different types to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed + 1;
    long long v3 = seed * 2LL;
    float v4 = seed * 1.5f;
    double v5 = seed * 2.71828;
    char v6 = seed & 0xFF;
    short v7 = seed & 0xFFFF;
    int* v8 = (int*)&seed;
    double v9 = 0.0;
    float v10 = 0.0f;
    long long v11 = 0;
    int v12 = 0;
    int v13 = 0;
    int v14 = 0;
    int v15 = 0;
    int v16 = 0;
    int v17 = 0;
    int v18 = 0;
    int v19 = 0;
    int v20 = 0;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    double fp_key_result = 0.0;
    
    /* Complex computation creating register pressure */
    for (int i = 0; i < 10; i++) {
        v1 = v1 * 1103515245 + 12345;
        v2 = v2 ^ (v1 >> 16);
        v3 = v3 + v1 * v2;
        v4 = v4 * 1.1f + v2 * 0.01f;
        v5 = v5 * 1.01 + v3 * 0.001;
        v6 = (v6 + v1) & 0xFF;
        v7 = (v7 + v2) & 0x7FFF;
        
        /* Mix in some inline assembly to clobber registers */
        __asm__ volatile (
            "# Clobber many registers to increase pressure\n"
            :
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory"
        );
    }
    
    /* Compute key intermediate results */
    key_result = v1 + v2 + v3 + v6 + v7;
    fp_key_result = v4 + v5 + key_result;
    
    /* Store label addresses for goto-based control flow */
    void* labels[10];
    labels[0] = &&label0;
    labels[1] = &&label1;
    labels[2] = &&label2;
    labels[3] = &&label3;
    labels[4] = &&label4;
    labels[5] = &&label5;
    labels[6] = &&label6;
    labels[7] = &&label7;
    labels[8] = &&label8;
    labels[9] = &&label9;
    
    /* Volatile variable to control conditional execution */
    volatile int control = v1 & 0xF;
    
    /* Complex nested conditional structure */
    if (control & 1) {
        /* Nested if-else chain */
        if (control & 2) {
            /* Deeply nested conditional block where key_result is used */
            if (control & 4) {
                /* Use key_result in inline assembly with many clobbers */
                int temp = key_result;
                __asm__ volatile (
                    "# Conditional use of key_result\n"
                    "add %0, %0, #1\n"
                    : "+r" (temp)
                    :
                    : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
                );
                key_result = temp;
                
                /* Call non-inlineable function that uses the result */
                v9 = external_func2(fp_key_result + temp);
            } else {
                /* Alternative path with different register usage */
                double temp_d = fp_key_result;
                __asm__ volatile (
                    "# Alternative FP use\n"
                    : "+r" (temp_d)
                    :
                    : "r0", "r1", "r2", "r3", "memory"
                );
                fp_key_result = temp_d;
            }
            
            /* Switch statement inside nested if */
            switch (control & 3) {
                case 0:
                    v10 = key_result * 0.5f;
                    /* Use key_result in complex expression */
                    v11 = key_result * v3;
                    break;
                case 1:
                    v10 = key_result * 0.25f;
                    v11 = key_result * v2;
                    break;
                case 2:
                    v10 = key_result * 0.125f;
                    v11 = key_result * v1;
                    /* Another inline assembly with clobbers */
                    __asm__ volatile (
                        "# More register pressure\n"
                        :
                        :
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "memory"
                    );
                    break;
                default:
                    v10 = key_result * 0.0625f;
                    v11 = key_result * v7;
            }
        } else {
            /* Another conditional block */
            if (control & 8) {
                /* Use goto with computed label */
                goto *labels[control & 7];
            }
        }
    } else {
        /* Outer else branch with its own complexity */
        volatile int alt_control = v2 & 0x7;
        
        if (alt_control == 0) {
            /* Use key_result differently */
            long long temp_ll = key_result;
            temp_ll = temp_ll * temp_ll;
            __asm__ volatile (
                "# Square operation with clobbers\n"
                : "+r" (temp_ll)
                :
                : "r0", "r1", "r2", "r3", "memory"
            );
            v12 = temp_ll;
        }
    }
    
    /* Label-based control flow resumes here */
label0:
    /* Use key_result after conditional blocks */
    v13 = key_result * 2;
    
    /* More register pressure */
    for (int i = 0; i < 5; i++) {
        v14 = v14 + key_result + i;
        v15 = v15 * v13;
        
        __asm__ volatile (
            "# Loop clobber\n"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "memory"
        );
    }
    
label1:
    /* Different data type usage */
    v16 = (short)key_result;
    v17 = (char)key_result;
    
label2:
    /* Pointer arithmetic using key_result */
    v8 = v8 + (key_result & 0x3);
    v18 = *v8;
    
label3:
    /* Floating point usage */
    v19 = fp_key_result * 1000;
    
label4:
    /* Mixed type computation */
    v20 = key_result + (int)fp_key_result + v18 + v19;
    
    /* Final aggregation to prevent elimination */
    int final_result = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7 + 
                      v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + 
                      v17 + v18 + v19 + v20;
    
    return final_result;

label5:
    /* Alternative path with goto */
    key_result = key_result / 2;
    goto label0;

label6:
    key_result = key_result * 3;
    goto label1;

label7:
    fp_key_result = fp_key_result * 2.0;
    goto label2;

label8:
    v18 = key_result & 0xFF;
    goto label3;

label9:
    v19 = key_result * 2;
    goto label4;
}

/* Main function with multiple calls to stress the compiler */
int main(int argc, char** argv) {
    int total = 0;
    
    /* Call stress function multiple times with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Add some volatile operations to prevent optimization */
        volatile int dummy = i;
        if (dummy & 1) {
            /* Use function pointer to create opaque control flow */
            if (volatile_fp) {
                volatile_fp();
            }
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
