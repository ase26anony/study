/* test_caller_save.c - Program to trigger specific RTL instruction reordering */
#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create conditional call */
volatile int global_flag = 1;

/* Function that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Clobber many caller-saved registers to force save/restore */
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
        "add %%rcx, %%rax\n\t"
        "add %%rdx, %%rax\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
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
        "add x0, x0, x1\n\t"
        "add x0, x0, x2\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10",
          "x11", "x12", "x13", "x14", "x15", "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with many live registers across call */
__attribute__((noipa, noinline, noclone))
int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed ^ 0x55;
    register int64_t f asm("") = seed | 0xAA;
    register int64_t g asm("") = seed << 2;
    register int64_t h asm("") = seed >> 1;
    register int64_t i asm("") = ~seed;
    register int64_t j asm("") = seed * seed;
    
    /* Create data dependencies before the call */
    a = b * c + d;
    b = c ^ d ^ e;
    c = d + e + f;
    d = e * f - g;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call - creates basic block boundaries */
    volatile int local_flag = global_flag;
    if (local_flag) {
        /* This call will need save/restore for all live registers */
        callee_function();
    }
    
    /* More computations after call - registers must be preserved */
    e = f + g + h;
    f = g * h / i;
    g = h ^ i ^ j;
    h = i + j + a;
    i = j * a - b;
    j = a ^ b ^ c;
    
    /* Complex return value using all variables */
    return a + b * 2 + c * 3 + d * 4 + e * 5 + 
           f * 6 + g * 7 + h * 8 + i * 9 + j * 10;
}

/* Another caller to create different pattern */
__attribute__((noipa, noinline, noclone))
int64_t caller_function2(int seed) {
    int64_t vars[12];
    
    /* Initialize array elements */
    for (int k = 0; k < 12; k++) {
        vars[k] = seed + k * k;
    }
    
    /* Create complex data flow */
    vars[0] = vars[1] * vars[2] + vars[3];
    vars[1] = vars[2] ^ vars[3] ^ vars[4];
    vars[2] = vars[3] + vars[4] + vars[5];
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    /* Nested conditional to create more complex CFG */
    if (global_flag > 0) {
        if (seed % 2) {
            callee_function();
        } else {
            vars[3] = vars[4] * vars[5] - vars[6];
        }
    }
    
    /* Post-call computations */
    vars[4] = vars[5] + vars[6] + vars[7];
    vars[5] = vars[6] * vars[7] / (vars[8] ? vars[8] : 1);
    
    /* Sum all elements */
    int64_t sum = 0;
    for (int k = 0; k < 12; k++) {
        sum += vars[k];
    }
    
    return sum;
}

int main(void) {
    int64_t result1, result2;
    
    /* Vary the global flag to affect code paths */
    for (int iteration = 0; iteration < 10; iteration++) {
        global_flag = iteration % 3;
        
        /* Call both functions to exercise different patterns */
        result1 = caller_function(iteration + 100);
        result2 = caller_function2(iteration + 200);
        
        /* Use results to prevent optimization */
        printf("Iteration %d: results = %lld, %lld\n", 
               iteration, (long long)result1, (long long)result2);
    }
    
    return 0;
}
