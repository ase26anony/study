/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to force conditional call */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Force register clobbering with inline assembly */
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
        "add %%rax, %%rcx\n\t"
        "add %%rdx, %%rsi\n\t"
        "add %%rdi, %%r8\n\t"
        "add %%r9, %%r10\n\t"
        "add %%r11, %%rax"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
          "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
#elif defined(__aarch64__)
    asm volatile (
        "mov x0, #0x1234\n\t"
        "mov x1, #0x5678\n\t"
        "mov x2, #0x9ABC\n\t"
        "mov x3, #0xDEF0\n\t"
        "mov x4, #0x1111\n\t"
        "mov x5, #0x2222\n\t"
        "mov x6, #0x3333\n\t"
        "mov x7, #0x4444\n\t"
        "mov x8, #0x5555\n\t"
        "mov x9, #0x6666\n\t"
        "mov x10, #0x7777\n\t"
        "mov x11, #0x8888\n\t"
        "mov x12, #0x9999\n\t"
        "mov x13, #0xAAAA\n\t"
        "mov x14, #0xBBBB\n\t"
        "mov x15, #0xCCCC\n\t"
        "add x0, x0, x1\n\t"
        "add x2, x2, x3\n\t"
        "add x4, x4, x5\n\t"
        "add x6, x6, x7\n\t"
        "add x8, x8, x9"
        : /* no outputs */
        : /* no inputs */
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v16", "v17",
          "v18", "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26",
          "v27", "v28", "v29", "v30", "v31", "memory"
    );
#else
    /* Generic clobber for other architectures */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("O2")))
int caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register int a asm("") = param1 * 2;
    register int b asm("") = param2 + 7;
    register int c asm("") = a ^ b;
    register int d asm("") = param1 - param2;
    register int e asm("") = a * b;
    register int f asm("") = c + d;
    register int g asm("") = e ^ f;
    register int h asm("") = param1 * param2;
    register int i asm("") = h << 3;
    register int j asm("") = g ^ i;
    
    /* Create data dependencies before the call */
    a = b + c;
    b = d * e;
    c = f ^ g;
    d = h + i;
    e = j * a;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Conditional call based on volatile flag */
    if (global_flag) {
        /* This call should trigger caller-save register spilling */
        callee_function();
    }
    
    /* More computations after the call - variables are still live */
    f = a + b;
    g = c - d;
    h = e ^ f;
    i = g * h;
    j = i + j;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + param1 + param2;
}

/* Another caller to create different basic block patterns */
__attribute__((noipa, noinline))
int alternate_caller(int x, int y) {
    register int v1 = x * 3;
    register int v2 = y + 11;
    register int v3 = v1 ^ v2;
    register int v4 = x - y;
    register int v5 = v1 * v2;
    register int v6 = v3 + v4;
    register int v7 = v5 ^ v6;
    register int v8 = x * y;
    
    /* Create a loop to generate more complex CFG */
    for (int k = 0; k < 3; k++) {
        v1 = v2 + v3;
        v2 = v4 * v5;
        
        /* Conditional call inside loop */
        if (global_flag & (1 << k)) {
            callee_function();
        }
        
        v3 = v6 ^ v7;
        v4 = v8 + v1;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
}

int main(void) {
    int result1, result2;
    
    /* Vary the global flag to affect code paths */
    global_flag = 1;
    result1 = caller_function(42, 17);
    
    global_flag = 0;
    result2 = alternate_caller(23, 58);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Force compiler to keep both code paths */
    if (result1 > result2) {
        return 0;
    } else {
        return 1;
    }
}
