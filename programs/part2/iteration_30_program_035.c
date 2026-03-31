/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void)
{
    /* Clobber caller-saved registers for x86_64 */
#if defined(__x86_64__)
    asm volatile (
        "mov $0x12345678, %%rax\n\t"
        "mov $0x87654321, %%rcx\n\t"
        "mov $0x11111111, %%rdx\n\t"
        "mov $0x22222222, %%rsi\n\t"
        "mov $0x33333333, %%rdi\n\t"
        "mov $0x44444444, %%r8\n\t"
        "mov $0x55555555, %%r9\n\t"
        "mov $0x66666666, %%r10\n\t"
        "mov $0x77777777, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    /* Clobber caller-saved registers for ARM64 */
    asm volatile (
        "mov x0, #0x1234\n\t"
        "mov x1, #0x5678\n\t"
        "mov x2, #0x9abc\n\t"
        "mov x3, #0xdef0\n\t"
        "mov x4, #0x1111\n\t"
        "mov x5, #0x2222\n\t"
        "mov x6, #0x3333\n\t"
        "mov x7, #0x4444\n\t"
        "mov x8, #0x5555\n\t"
        "mov x9, #0x6666\n\t"
        "mov x10, #0x7777\n\t"
        "mov x11, #0x8888\n\t"
        "mov x12, #0x9999\n\t"
        "mov x13, #0xaaaa\n\t"
        "mov x14, #0xbbbb\n\t"
        "mov x15, #0xcccc\n\t"
        "mov x16, #0xdddd\n\t"
        "mov x17, #0xeeee\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline))
long caller_function(int param1, int param2, int param3)
{
    /* Declare many local variables to create register pressure */
    register long a = param1 * 2;
    register long b = param2 + 100;
    register long c = param3 - 50;
    register long d = a * b;
    register long e = b + c;
    register long f = c * param1;
    register long g = d - e;
    register long h = f + g;
    register long i = a + b + c;
    register long j = d * e;
    
    /* Complex computation before call */
    a = b * c + d - e;
    b = f * g / (h + 1);
    c = i - j + a;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* Additional computation right before call */
        d = e * f + 1;
        e = g * h - 2;
        
        /* The call that needs caller-save handling */
        callee_function();
        
        /* Additional computation right after call */
        f = h * i + 3;
        g = j * a - 4;
    } else {
        /* Alternative path without call */
        d = e * f - 1;
        e = g * h + 2;
        f = h * i - 3;
        g = j * a + 4;
    }
    
    /* More computations using all variables */
    h = a + b + c + d + e;
    i = f * g - h;
    j = (a * b) + (c * d) - (e * f);
    
    /* Complex final computation ensuring all variables are live */
    long result = a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to prevent optimization */
__attribute__((noipa, noinline))
void use_result(long result)
{
    volatile long sink = result;
    (void)sink;
}

int main(void)
{
    /* Initialize with different values to prevent constant folding */
    int seed = 42;
    
    /* Call multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        long result = caller_function(seed + i, seed * 2 + i, seed * 3 - i);
        use_result(result);
        
        /* Toggle flag to exercise both paths */
        global_flag = !global_flag;
    }
    
    /* Print something to prevent dead code elimination */
    printf("Final check: %d\n", global_flag);
    
    return 0;
}
