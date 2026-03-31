/* early-remat-trigger.c
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing early-remat-trigger.c -o test
 * For 32-bit: gcc -O2 -m32 -march=i686 -fno-dse -fearly-remat -fno-inline early-remat-trigger.c -o test32
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable functions to create barriers */
__attribute__((noinline)) void use_registers(void) {
    /* Empty but prevents inlining */
    asm volatile("" ::: "memory");
}

__attribute__((noinline)) int external_func(int x) {
    volatile static int counter = 0;
    counter += x;
    return counter;
}

/* Volatile function pointer to prevent optimization */
typedef void (*jump_func_t)(void);
volatile jump_func_t volatile_jump = NULL;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    short v6 = seed * 6;
    char v7 = seed * 7;
    int *v8 = &v1;
    float v9 = seed * 9.0f;
    double v10 = seed * 10.0;
    int v11 = seed * 11;
    long long v12 = seed * 12LL;
    float v13 = seed * 13.0f;
    double v14 = seed * 14.0;
    int v15 = seed * 15;
    int v16 = seed * 16;
    int v17 = seed * 17;
    int v18 = seed * 18;
    int v19 = seed * 19;
    int v20 = seed * 20;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex computation with mixed types */
    key_result = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7;
    
    /* Store label addresses for goto */
    void *label1 = &&L1, *label2 = &&L2, *label3 = &&L3;
    volatile_jump = (jump_func_t)label1;
    
    /* Deeply nested conditional structure */
    if (v1 > 0) {
        if (v2 % 3 == 0) {
            switch (v7 % 4) {
                case 0:
                    /* Use key_result in inline asm with many clobbered registers */
                    asm volatile (
                        "mov %0, %%eax\n\t"
                        "add $1, %%eax\n\t"
                        "mov %%eax, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    break;
                case 1:
                    /* Different mode usage - double */
                    double temp_d = (double)key_result;
                    asm volatile (
                        "fldl %0\n\t"
                        "fadd %%st(0), %%st(0)\n\t"
                        "fstpl %0\n\t"
                        : "+m" (temp_d)
                        :
                        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
                    );
                    key_result = (int)temp_d;
                    break;
                case 2:
                    /* Float mode usage */
                    float temp_f = (float)key_result;
                    asm volatile (
                        "flds %0\n\t"
                        "fmul %%st(0), %%st(0)\n\t"
                        "fstps %0\n\t"
                        : "+m" (temp_f)
                        :
                        : "st", "st(1)", "memory"
                    );
                    key_result = (int)temp_f;
                    break;
                default:
                    /* Long long mode usage */
                    long long temp_ll = key_result;
                    asm volatile (
                        "addq $0x100, %0\n\t"
                        : "+r" (temp_ll)
                        :
                        : "cc", "memory"
                    );
                    key_result = (int)temp_ll;
            }
            
            /* Call non-inlineable function that uses the register */
            key_result = external_func(key_result);
            
            /* Opaque jump using volatile function pointer */
            if (v3 & 0x1) {
                goto *volatile_jump;
            }
        } else if (v4 > 100.0f) {
            /* Another conditional path */
            for (int i = 0; i < 5; i++) {
                key_result += v6 * i;
                if (i == 3) {
                    /* Use different register modes again */
                    short temp_s = key_result;
                    asm volatile (
                        "addw $0x10, %0\n\t"
                        : "+r" (temp_s)
                        :
                        : "cc"
                    );
                    key_result = temp_s;
                }
            }
        }
    }
    
L1:
    /* Use key_result after conditional block */
    v15 += key_result;
    
    /* More complex control flow with goto */
    if (v5 < 50.0) {
        volatile_jump = (jump_func_t)label2;
        goto *volatile_jump;
    }
    
    /* Additional conditional use with different mode */
    double key_double = (double)key_result;
    if (v9 > 0.0f) {
        asm volatile (
            "fldl %1\n\t"
            "fsqrt\n\t"
            "fstpl %0\n\t"
            : "=m" (key_double)
            : "m" (key_double)
            : "st", "st(1)", "memory"
        );
        key_result = (int)key_double;
    }
    
L2:
    /* Mix with other variables to keep them live */
    v16 = key_result + v17;
    
    /* Another nested conditional */
    if (v8 != NULL) {
        switch (*v8 % 5) {
            case 0:
                key_result *= 2;
                break;
            case 1:
                /* Char mode usage */
                char temp_c = key_result;
                asm volatile (
                    "addb $0x5, %0\n\t"
                    : "+r" (temp_c)
                    :
                    : "cc"
                );
                key_result = temp_c;
                break;
            case 2:
                /* Inline asm that clobbers many registers */
                asm volatile (
                    "push %%eax\n\t"
                    "push %%ebx\n\t"
                    "push %%ecx\n\t"
                    "mov %0, %%eax\n\t"
                    "imul $7, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    "pop %%ecx\n\t"
                    "pop %%ebx\n\t"
                    "pop %%eax\n\t"
                    : "+r" (key_result)
                    :
                    : "cc", "memory"
                );
                break;
            default:
                key_result = key_result >> 2;
        }
        
        /* Opaque jump again */
        if (v10 > 100.0) {
            volatile_jump = (jump_func_t)label3;
            goto *volatile_jump;
        }
    }
    
L3:
    /* Final use of key_result with other live variables */
    int final_result = key_result + v2 + v3 + v4 + v5 + v6 + 
                      v9 + v10 + v11 + v12 + v13 + v14 + 
                      v15 + v16 + v17 + v18 + v19 + v20;
    
    /* Prevent dead code elimination */
    use_registers();
    
    return final_result;
}

int main(int argc, char **argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int total = 0;
    for (int i = 0; i < 10; i++) {
        total += stress_function(seed + i);
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}
