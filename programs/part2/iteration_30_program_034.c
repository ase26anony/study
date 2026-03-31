/* test-caller-save.c
 * Designed to trigger specific instruction chain manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-optimize-sibling-calls test.c -o test -fdump-rtl-caller-save
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Callee that clobbers caller-saved registers */
__attribute__((noipa, noinline, noclone))
void callee_function(void) {
    /* Force clobbering of caller-saved registers */
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
    /* Generic memory clobber */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller with high register pressure across conditional call */
__attribute__((noipa, noinline, noclone))
int64_t caller_function(int seed) {
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed / 7;
    register int64_t h asm("") = seed - 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    
    /* Complex computation before call - creates data dependencies */
    a = b * c + d;
    b = c * d - e;
    c = d * e / (f + 1);
    d = e * f + g;
    
    /* Memory barrier to force values to registers */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int local_flag = global_flag;
    
    /* Conditional call - creates basic block structure */
    if (local_flag) {
        /* Additional computation right before call */
        e = f * g + h;
        f = g * h - i;
        
        /* The critical call that will need caller-save handling */
        callee_function();
        
        /* More computation after call - keeps variables live */
        g = h * i + j;
        h = i * j - a;
    } else {
        /* Alternative path with different computation */
        e = f + g + h;
        f = g + h + i;
        g = h + i + j;
        h = i + j + a;
    }
    
    /* Complex post-call computation with all variables */
    i = j * a + b;
    j = a * b - c;
    
    /* Create cross-dependencies to prevent reordering */
    a = a + b + c + d;
    b = b + c + d + e;
    c = c + d + e + f;
    d = d + e + f + g;
    e = e + f + g + h;
    f = f + g + h + i;
    g = g + h + i + j;
    h = h + i + j + a;
    i = i + j + a + b;
    j = j + a + b + c;
    
    /* Final memory barrier */
    asm volatile("" : : : "memory");
    
    /* Return complex expression using all variables */
    return a + b + c + d + e + f + g + h + i + j;
}

/* Another level of indirection to create more optimization opportunities */
__attribute__((noipa, noinline, noclone))
int64_t intermediate_caller(int seed) {
    int64_t x = caller_function(seed);
    int64_t y = caller_function(seed + 100);
    
    /* Create register pressure in this function too */
    register int64_t t1 = x * y;
    register int64_t t2 = x + y;
    register int64_t t3 = x - y;
    register int64_t t4 = x / (y + 1);
    
    asm volatile("" : : : "memory");
    
    if (global_flag > 0) {
        callee_function();
    }
    
    return t1 + t2 + t3 + t4;
}

int main(void) {
    int64_t result = 0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 3; iter++) {
        global_flag = (iter % 2) + 1;  /* Change flag value */
        
        /* Call through intermediate to create more complex CFG */
        result += intermediate_caller(iter * 100 + 12345);
        
        /* Additional computation to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %ld\n", (long)result);
    
    /* Additional test with different seed */
    global_flag = 0;
    result = caller_function(99999);
    printf("Result2: %ld\n", (long)result);
    
    return 0;
}
