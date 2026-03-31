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

__attribute__((noinline)) double external_func2(double x) {
    volatile double y = x * 3.14159;
    return y / 2.0;
}

__attribute__((noinline)) long long external_func3(long long x) {
    volatile long long y = x ^ 0x123456789ABCDEFLL;
    return y * 7;
}

/* Volatile function pointer to prevent inlining */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables of different types to increase register pressure */
    volatile int v1 = seed;
    int v2 = v1 * 2;
    double v3 = v1 * 3.14;
    float v4 = v1 * 1.5f;
    long long v5 = v1 * 1000LL;
    short v6 = v1 & 0xFFFF;
    char v7 = v1 & 0xFF;
    int v8 = v1 + 100;
    double v9 = v3 * 2.0;
    float v10 = v4 * 3.0f;
    long long v11 = v5 * 2LL;
    int v12 = v2 + v8;
    double v13 = v3 + v9;
    float v14 = v4 + v10;
    long long v15 = v5 + v11;
    int v16 = v12 * 2;
    double v17 = v13 * 1.5;
    float v18 = v14 * 2.5f;
    long long v19 = v15 * 3LL;
    
    /* Key intermediate result that will be used conditionally */
    int key_result = v1 + v2 + v8 + v12 + v16;
    
    /* Complex nested conditional structure */
    volatile int control1 = v1 % 7;
    volatile int control2 = v2 % 5;
    volatile int control3 = v6 % 3;
    
    /* Label addresses for goto-based control flow */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4, &&label5 };
    volatile void* volatile_label_ptr = labels[control1 % 5];
    
    /* First conditional block with deeply nested structure */
    if (control1 > 0) {
        if (control2 < 3) {
            switch (control3) {
                case 0:
                    /* Use key_result in inline assembly with many clobbered registers */
                    asm volatile (
                        "mov %0, %%eax\n\t"
                        "add $100, %%eax\n\t"
                        "mov %%eax, %0\n\t"
                        : "+r" (key_result)
                        : 
                        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
                    );
                    break;
                case 1:
                    /* Different mode usage - double */
                    double temp_double = (double)key_result;
                    asm volatile (
                        "fldl %0\n\t"
                        "fadd %%st(0), %%st(0)\n\t"
                        "fstpl %0\n\t"
                        : "+m" (temp_double)
                        :
                        : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory"
                    );
                    key_result = (int)temp_double;
                    break;
                case 2:
                    /* Different mode usage - long long */
                    long long temp_ll = (long long)key_result * 100LL;
                    asm volatile (
                        "movq %0, %%rax\n\t"
                        "imulq $7, %%rax\n\t"
                        "movq %%rax, %0\n\t"
                        : "+r" (temp_ll)
                        :
                        : "rax", "rbx", "rcx", "rdx", "memory"
                    );
                    key_result = (int)(temp_ll / 100LL);
                    break;
            }
            
            /* Call non-inlineable function using key_result */
            key_result = external_func1(key_result);
            
            /* Nested if inside switch case */
            if (key_result % 2 == 0) {
                volatile int inner_control = key_result % 4;
                for (int i = 0; i < inner_control; i++) {
                    key_result += external_func1(i);
                }
            }
        } else {
            /* Alternative path with different register usage */
            double d1 = v3, d2 = v9, d3 = v13, d4 = v17;
            float f1 = v4, f2 = v10, f3 = v14, f4 = v18;
            
            /* Mixed type computations */
            for (int i = 0; i < 4; i++) {
                d1 = d1 * 1.1 + d2;
                d2 = d2 * 1.2 + d3;
                d3 = d3 * 1.3 + d4;
                f1 = f1 * 1.1f + f2;
                f2 = f2 * 1.2f + f3;
                f3 = f3 * 1.3f + f4;
                
                /* Use key_result in floating point context */
                if (i % 2 == 0) {
                    key_result += (int)(d1 + f1);
                }
            }
        }
        
        /* Unpredictable goto using volatile pointer */
        goto *volatile_label_ptr;
    }
    
label1:
    /* Use key_result after conditional block */
    v16 = key_result * 3;
    
    /* More register pressure */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = key_result + i * v16;
    }
    
    /* Compute with all variables to keep them live */
    int sum = 0;
    sum += v1 + v2 + v8 + v12 + v16;
    for (int i = 0; i < 16; i++) {
        sum += array[i];
    }
    
    /* Second complex conditional block */
    volatile int control4 = sum % 11;
    
    if (control4 > 5) {
        /* Use key_result with different mode (short/char) */
        short s_result = (short)(key_result & 0xFFFF);
        char c_result = (char)(key_result & 0xFF);
        
        asm volatile (
            "movw %0, %%ax\n\t"
            "addw $100, %%ax\n\t"
            "movw %%ax, %0\n\t"
            "movb %1, %%al\n\t"
            "subb $50, %%al\n\t"
            "movb %%al, %1\n\t"
            : "+r" (s_result), "+r" (c_result)
            :
            : "ax", "memory"
        );
        
        key_result = s_result + c_result;
        
        /* Call external function with mixed types */
        v3 = external_func2(v3);
        v5 = external_func3(v5);
    } else {
        /* Another path with loop and register pressure */
        long long ll_array[8];
        double d_array[8];
        
        for (int i = 0; i < 8; i++) {
            ll_array[i] = v5 + i * key_result;
            d_array[i] = v3 + i * key_result * 0.1;
            
            /* Inline assembly that clobbers many registers */
            asm volatile (
                "mov %0, %%rax\n\t"
                "mov %1, %%xmm0\n\t"
                "add $1, %%rax\n\t"
                "addsd %%xmm0, %%xmm0\n\t"
                "mov %%rax, %0\n\t"
                "movsd %%xmm0, %1\n\t"
                : "+r" (ll_array[i]), "+r" (d_array[i])
                :
                : "rax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
            );
        }
        
        /* Aggregate results */
        for (int i = 0; i < 8; i++) {
            key_result += (int)(ll_array[i] % 1000) + (int)d_array[i];
        }
    }

label2:
    /* More computations with key_result */
    double final_double = 0.0;
    for (int i = 0; i < 100; i++) {
        final_double += key_result * 0.01 * i;
        
        /* Periodic inline assembly to maintain register pressure */
        if (i % 25 == 0) {
            asm volatile (
                "cpuid\n\t"
                :
                :
                : "eax", "ebx", "ecx", "edx", "memory"
            );
        }
    }
    
label3:
    /* Use all variables in final computation to prevent elimination */
    long long final_ll = v5 + v11 + v15 + v19;
    double final_d = v3 + v9 + v13 + v17 + final_double;
    float final_f = v4 + v10 + v14 + v18;
    int final_int = v1 + v2 + v6 + v7 + v8 + v12 + v16 + key_result;
    
    /* Complex expression mixing all types */
    int result = final_int + (int)final_ll + (int)final_d + (int)final_f;
    
    /* One more conditional with goto */
    if (result % 2 == 0) {
        goto *labels[result % 5];
    }

label4:
    return result;

label5:
    /* Alternative return path */
    return result * 2;
}

/* Main function with multiple calls to increase coverage */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call stress function with different seeds */
    for (int i = 0; i < 100; i++) {
        int seed = i * 12345;
        int result = stress_function(seed);
        total += result;
        
        /* Volatile store to prevent optimization */
        volatile int dummy = result;
        
        /* Periodic complex control flow in main too */
        if (i % 7 == 0) {
            /* Function pointer call */
            void (*funcs[3])(void) = { 
                (void(*)())external_func1,
                (void(*)())external_func2,
                (void(*)())external_func3
            };
            
            volatile int idx = i % 3;
            volatile_func_ptr = (void(*)())funcs[idx];
        }
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different data type sizes */
    {
        char c1 = 10, c2 = 20, c3 = 30;
        short s1 = 100, s2 = 200, s3 = 300;
        int i1 = 1000, i2 = 2000, i3 = 3000;
        long long ll1 = 10000, ll2 = 20000, ll3 = 30000;
        float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
        double d1 = 1.11, d2 = 2.22, d3 = 3.33;
        
        /* Mix all types in computation */
        volatile int mix_result = 
            c1 + c2 + c3 + 
            s1 + s2 + s3 + 
            i1 + i2 + i3 + 
            (int)ll1 + (int)ll2 + (int)ll3 +
            (int)f1 + (int)f2 + (int)f3 +
            (int)d1 + (int)d2 + (int)d3;
            
        printf("Mixed type result: %d\n", mix_result);
    }
    
    return total > 0 ? 0 : 1;
}
