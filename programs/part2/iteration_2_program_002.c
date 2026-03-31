/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */

#include <stdint.h>
#include <stdlib.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) 
int external_func(int x, int y) {
    volatile int dummy = x + y;
    return dummy;
}

/* Volatile function pointer to prevent optimization */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_fp = external_func;

/* Main stress function with complex control flow */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    int v2 = seed * 2;
    long long v3 = seed * 3LL;
    float v4 = seed * 4.0f;
    double v5 = seed * 5.0;
    short v6 = seed * 6;
    char v7 = seed * 7;
    int v8 = seed * 8;
    float v9 = seed * 9.0f;
    double v10 = seed * 10.0;
    int v11 = seed * 11;
    long long v12 = seed * 12LL;
    int v13 = seed * 13;
    float v14 = seed * 14.0f;
    double v15 = seed * 15.0;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = 0;
    
    /* Complex conditional computation with mixed types */
    if (v1 > 100) {
        key_result = v2 + (int)v3;
        v4 = v4 * 2.0f + (float)key_result;
        
        /* Nested if-else chain */
        if (v6 < 50) {
            key_result += v7 * 2;
            v5 = v5 / 2.0 + (double)key_result;
            
            /* Switch inside nested if */
            switch (v8 % 4) {
                case 0:
                    key_result = key_result * 3 + v11;
                    /* Use inline assembly that clobbers many registers */
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
                    key_result = key_result / 2 + v13;
                    break;
                case 2:
                    key_result = key_result + (int)v14 + (int)v15;
                    break;
                default:
                    key_result = key_result - v12;
                    break;
            }
        } else if (v6 > 100) {
            /* Another path with different computation */
            key_result = key_result * v7;
            v9 = v9 + (float)key_result;
            
            /* Call through volatile function pointer */
            int tmp = volatile_fp(key_result, v2);
            key_result += tmp;
        } else {
            /* Default path with loop */
            for (int i = 0; i < v7; i++) {
                key_result += i * v2;
                if (i % 3 == 0) {
                    key_result -= v3;
                }
            }
        }
        
        /* Use key_result in another computation */
        v10 = v10 * (double)key_result;
    } else if (v1 < -50) {
        /* Alternative branch with different data types */
        key_result = (int)v4 + (int)v5;
        
        /* Deeply nested conditionals */
        if (v11 != 0) {
            if (v12 > 1000LL) {
                key_result = key_result * 2 + (int)v9;
                
                /* Use key_result in inline assembly with different mode */
                asm volatile (
                    "mov %0, %%eax\n\t"
                    "imul $7, %%eax\n\t"
                    "mov %%eax, %0\n\t"
                    : "+r" (key_result)
                    : 
                    : "eax", "ebx", "ecx", "edx", "memory"
                );
            }
        }
    } else {
        /* Third branch with switch statement */
        switch (v1 % 5) {
            case 0:
                key_result = v2 * v8;
                break;
            case 1:
                key_result = (int)v3 + v11;
                break;
            case 2:
                key_result = (int)v4 * 2;
                break;
            case 3:
                key_result = (int)v5 / 3;
                break;
            case 4:
                key_result = v6 * v7;
                break;
        }
    }
    
    /* Use goto with computed labels to create irreducible control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile int label_index = key_result % 4;
    
    /* Force the compiler to keep all variables live */
    int sum = v1 + v2 + (int)v3 + (int)v4 + (int)v5 + v6 + v7 + v8 + 
              (int)v9 + (int)v10 + v11 + (int)v12 + v13 + (int)v14 + (int)v15;
    
    /* Conditional jump based on volatile variable */
    if (v1 > 0) {
        goto *labels[label_index];
    }
    
label1:
    /* Use key_result after conditional block */
    sum += key_result * 2;
    key_result = external_func(key_result, sum);
    goto end;
    
label2:
    sum += key_result / 2;
    /* Another inline assembly use with different mode (long long) */
    long long ll_tmp = (long long)key_result * v3;
    asm volatile (
        "movq %0, %%rax\n\t"
        "addq $100, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "+r" (ll_tmp)
        : 
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    key_result += (int)ll_tmp;
    goto end;
    
label3:
    sum += key_result + v2;
    /* Float/double operations to generate different modes */
    float f_tmp = (float)key_result * v4;
    double d_tmp = (double)key_result * v5;
    sum += (int)f_tmp + (int)d_tmp;
    goto end;
    
label4:
    sum += key_result - v8;
    /* Mixed size operations */
    short s_tmp = (short)key_result + v6;
    char c_tmp = (char)key_result * v7;
    sum += s_tmp + c_tmp;
    /* fall through */
    
end:
    /* Final computation using key_result */
    int final_result = sum + key_result;
    
    /* More register pressure with array operations */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = final_result * i;
        if (i % 2 == 0) {
            array[i] += v1;
        } else {
            array[i] += v2;
        }
    }
    
    /* Aggregate array values */
    for (int i = 0; i < 16; i++) {
        final_result += array[i];
    }
    
    return final_result;
}

/* Main function with different compilation paths */
int main(int argc, char** argv) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Call stress function multiple times with different seeds */
    int result = 0;
    for (int i = 0; i < 10; i++) {
        result += stress_function(seed + i);
        
        /* Alternate between different control flow patterns */
        if (i % 3 == 0) {
            seed = result % 100;
        }
    }
    
    return result % 256;
}
