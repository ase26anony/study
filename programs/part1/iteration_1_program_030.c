#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int result = a + b + c + d + e + f + g + h + i + j;
    /* Use inline assembly to clobber registers */
    __asm__ volatile ("" : : : "eax", "ebx", "ecx", "edx");
    return result;
}

/* Function with pointer arguments and float operations */
float __attribute__((noinline))
process_floats(float f1, float f2, float f3, float* out1, float* out2) {
    volatile float temp = f1 * f2 + f3;
    *out1 = temp;
    *out2 = temp / 2.0f;
    /* Clobber floating point registers */
    __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2");
    return temp;
}

/* Function that takes mixed types and returns pointer */
static void* __attribute__((noinline))
complex_operation(int a, float b, double c, long d) {
    static char buffer[256];
    volatile double result = (double)a + (double)b + c + (double)d;
    
    /* Use alloca to affect frame pointer */
    char* dynamic = (char*)alloca(64);
    for (int i = 0; i < 64; i++) {
        dynamic[i] = (char)(result + i);
    }
    
    /* More register clobbering */
    __asm__ volatile ("" : : : "r10", "r11", "r12", "r13");
    
    snprintf(buffer, sizeof(buffer), "%.6f", result);
    return buffer;
}

/* Function with variable arguments (simulated) */
int __attribute__((noinline))
multi_arg_func(int a1, int a2, int a3, int a4, int a5, 
               int a6, int a7, int a8, int a9, int a10,
               float f1, float f2, double d1) {
    volatile int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    volatile float fsum = f1 + f2;
    volatile double total = (double)sum + (double)fsum + d1;
    
    /* Force register pressure with inline assembly */
    __asm__ volatile (
        "movl %%eax, %%ebx\n\t"
        "addl %%ecx, %%edx\n\t"
        : : : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    return (int)total;
}

/* Recursive function to create more call sites */
static int __attribute__((noinline, noipa))
recursive_helper(int n, int* counter) {
    volatile int local = n * 2;
    (*counter)++;
    
    if (n <= 1) {
        return local;
    }
    
    /* Create register pressure before recursive call */
    volatile int a = local + 1;
    volatile int b = local * 3;
    volatile int c = a ^ b;
    
    /* Inline assembly between computations */
    __asm__ volatile ("" : : : "rax", "rbx", "rcx");
    
    return recursive_helper(n - 1, counter) + c;
}

int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.5f, f2 = 2.5f, f3 = 3.5f, f4 = 4.5f;
    volatile double d1 = 10.5, d2 = 20.5;
    volatile long l1 = 100, l2 = 200;
    volatile int* ptr1 = &v1;
    volatile float* ptr2 = &f1;
    
    int checksum = 0;
    int call_counter = 0;
    
    /* Take addresses to affect frame pointer optimization */
    int* addr1 = &v1;
    float* addr2 = &f1;
    (void)addr1; (void)addr2;  /* Prevent unused warning */
    
    /* Complex control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + v1;
        f1 = f1 * 1.1f + (float)iteration;
        
        /* Inline assembly to clobber call-clobbered registers */
        __asm__ volatile (
            "movq $0x12345678, %%rax\n\t"
            "addq $1, %%rax\n\t"
            : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
        );
        
        /* Function call with many arguments - will need caller-save */
        int sum1 = compute_sum(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        checksum += sum1;
        
        /* Second basic block */
        if (iteration % 2 == 0) {
            float out1, out2;
            float float_result = process_floats(f1, f2, f3, &out1, &out2);
            checksum += (int)float_result;
            
            /* More computations between calls */
            v3 = v3 ^ v4;
            v4 = v4 | v5;
            
            /* Another inline assembly block */
            __asm__ volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4");
            
            /* Function returning pointer */
            void* str_result = complex_operation(v1, f1, d1, l1);
            checksum += (int)((uintptr_t)str_result & 0xFF);
        } else {
            /* Alternative path with different calls */
            int multi_result = multi_arg_func(
                v1, v2, v3, v4, v5, v6, v7, v8, v9, v10,
                f1, f2, d1
            );
            checksum += multi_result;
            
            /* Use alloca in loop to affect stack frame */
            char* temp_buf = (char*)alloca(32);
            for (int i = 0; i < 32; i++) {
                temp_buf[i] = (char)(checksum + i);
            }
        }
        
        /* Recursive call to create more call depth */
        int rec_result = recursive_helper(3, &call_counter);
        checksum += rec_result;
        
        /* Final computations in the loop */
        d1 = d1 * 1.01;
        d2 = d2 / 1.01;
        l1 = l1 + iteration;
        l2 = l2 - iteration;
        
        /* More register clobbering */
        __asm__ volatile (
            "movl %%eax, %%ebx\n\t"
            "movl %%ecx, %%edx\n\t"
            : : : "eax", "ebx", "ecx", "edx", "r8", "r9", "r10"
        );
    }
    
    /* Final output to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    printf("Total calls made: %d\n", call_counter);
    
    return checksum != 0 ? 0 : 1;
}
