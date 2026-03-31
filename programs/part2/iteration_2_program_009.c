/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing -m32 -march=i686 -fno-dse -o test test.c */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with unknown target */
typedef int (*func_ptr_t)(int);
volatile func_ptr_t volatile_func_ptr = external_func;

/* Stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    float v3 = seed * 3.0f;
    double v4 = seed * 4.0;
    short v5 = seed * 5;
    char v6 = seed * 6;
    long long v7 = seed * 7LL;
    int *v8 = &v1;
    float v9 = seed * 9.0f;
    double v10 = seed * 10.0;
    int v11 = seed * 11;
    int v12 = seed * 12;
    float v13 = seed * 13.0f;
    double v14 = seed * 14.0;
    short v15 = seed * 15;
    char v16 = seed * 16;
    long long v17 = seed * 17LL;
    int v18 = seed * 18;
    float v19 = seed * 19.0f;
    double v20 = seed * 20.0;
    
    /* Key intermediate result - this is the register we want to stress */
    int key_result = v1 + v2 + (int)v3 + (int)v4 + v5 + v6 + (int)v7;
    
    /* Complex nested conditional structure */
    volatile int control = seed % 7;
    
    /* Label addresses for irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[control % 5];
    
    /* First conditional use of key_result */
    if (control > 0) {
        if (control < 3) {
            switch (control) {
                case 1:
                    /* Use key_result in inline assembly with many clobbers */
                    asm volatile (
                        "add %[val], %[val], #1\n\t"
                        : [val] "+r" (key_result)
                        : 
                        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                          "memory", "cc"
                    );
                    break;
                case 2:
                    /* Different mode usage - short */
                    short temp_short = (short)key_result;
                    asm volatile (
                        "add %0, %0, #1"
                        : "+r" (temp_short)
                        :
                        : "memory"
                    );
                    key_result = temp_short;
                    break;
                default:
                    break;
            }
            
            /* Call non-inlineable function using key_result */
            key_result = volatile_func_ptr(key_result);
        } else {
            /* Float mode usage */
            float temp_float = (float)key_result;
            asm volatile (
                "fadds %0, %0, %1"
                : "+w" (temp_float)
                : "w" (v3)
                : "memory"
            );
            key_result = (int)temp_float;
        }
        
        /* Deep nesting */
        if (control == 4) {
            volatile int inner_control = seed % 3;
            for (int i = 0; i < inner_control; i++) {
                /* Double mode usage */
                double temp_double = (double)key_result;
                asm volatile (
                    "faddd %0, %0, %1"
                    : "+w" (temp_double)
                    : "w" (v4)
                    : "memory"
                );
                key_result = (int)temp_double;
                
                /* Unpredictable goto */
                if (i == inner_control - 1) {
                    goto *volatile_label_ptr;
                }
            }
        }
    }
    
label1:
    /* Use key_result again after conditional block */
    int result1 = key_result + v11 + v12;
    
label2:
    /* More mixed mode operations */
    long long temp_ll = (long long)key_result * v7;
    asm volatile ("" : "+r" (temp_ll) : : "memory");
    
label3:
    /* Another conditional path */
    if (control % 2 == 0) {
        /* Char mode usage */
        char temp_char = (char)key_result;
        asm volatile (
            "add %0, %0, #1"
            : "+r" (temp_char)
            :
            : "memory"
        );
        key_result = temp_char;
    }
    
label4:
    /* Use all variables to keep them live */
    v18 = key_result + (int)v19 + (int)v20;
    
label5:
    /* Final computation using key_result */
    int final_result = key_result 
                     + v2 + (int)v3 + (int)v4 
                     + v5 + v6 + (int)v7
                     + v11 + v12 + (int)v13 + (int)v14
                     + v15 + v16 + (int)v17 + v18;
    
    /* More complex control flow with computed goto */
    static void* jump_table[] = { &&end, &&loop1, &&loop2 };
    void* target = jump_table[control % 3];
    goto *target;
    
loop1:
    for (int i = 0; i < 3; i++) {
        key_result += i;
        /* Force spill/reload with inline asm clobbering */
        asm volatile (
            "mov r0, %0\n\t"
            "add r0, r0, #1\n\t"
            "mov %0, r0\n\t"
            : "+r" (key_result)
            :
            : "r0", "r1", "r2", "memory"
        );
    }
    goto end;
    
loop2:
    {
        /* Different data type chain */
        float chain = (float)key_result;
        double chain2 = (double)chain;
        int chain3 = (int)chain2;
        key_result = chain3 + v1;
    }
    
end:
    return final_result + key_result;
}

/* Main function with varying inputs */
int main() {
    int total = 0;
    
    /* Call stress function with different seeds */
    for (int i = 0; i < 100; i++) {
        total += stress_function(i);
        
        /* Alternate function pointer target */
        if (i % 7 == 0) {
            volatile_func_ptr = &external_func;
        }
    }
    
    /* Use result to prevent optimization */
    volatile int result = total;
    return result;
}
