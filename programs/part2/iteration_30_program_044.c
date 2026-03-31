/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile to prevent constant propagation */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Force register clobbering with inline assembly */
#if defined(__x86_64__)
    asm volatile("" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11", 
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11",
        "xmm12", "xmm13", "xmm14", "xmm15",
        "memory");
#elif defined(__aarch64__)
    asm volatile("" : : : 
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
        "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
        "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
        "x24", "x25", "x26", "x27", "x28", "x29", "x30",
        "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
        "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
        "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
        "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
        "memory");
#else
    /* Generic memory clobber */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-inline")))
long caller_function(int param1, int param2) {
    /* Declare many local variables to create register pressure */
    register long a asm("") = param1 * 3;
    register long b asm("") = param2 * 7;
    register long c asm("") = a + b;
    register long d asm("") = b - a;
    register long e asm("") = a * b;
    register long f asm("") = c * d;
    register long g asm("") = e / (param1 + 1);
    register long h asm("") = f ^ e;
    register long i asm("") = g << 2;
    register long j asm("") = h >> 1;
    register long k asm("") = i | j;
    register long l asm("") = k & 0xFFFF;
    
    /* Force values to be in registers before the call */
    asm volatile("" : : : "memory");
    
    /* Complex conditional to create interesting CFG */
    volatile int local_flag = global_flag;
    
    /* This call creates the need for caller-save */
    if (local_flag & 0x1) {
        /* Additional computation before call to create live ranges */
        a = b + c;
        b = c + d;
        c = d + e;
        
        /* Memory barrier to prevent reordering */
        asm volatile("" : : : "memory");
        
        /* The critical call instruction */
        callee_function();
        
        /* Instruction that might be moved by caller-save pass */
        d = e + f;
    } else {
        /* Alternative path without call */
        a = b - c;
        b = c - d;
    }
    
    /* More computations after call, keeping variables live */
    e = f + g;
    f = g + h;
    g = h + i;
    h = i + j;
    i = j + k;
    j = k + l;
    
    /* Complex return value using all variables */
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

/* Another caller to create more optimization opportunities */
__attribute__((noipa, noinline))
long caller_function2(int x, int y) {
    register long v1 = x * 2;
    register long v2 = y * 3;
    register long v3 = v1 + v2;
    register long v4 = v2 - v1;
    register long v5 = v3 * v4;
    
    /* Nested conditional with call */
    if (global_flag > 0) {
        if (x > y) {
            asm volatile("" : : : "memory");
            callee_function();
            v1 = v2 + 1;
        } else {
            v2 = v1 - 1;
        }
        v3 = v4 * 2;
    }
    
    /* Force register spilling */
    asm volatile("" : : : "memory");
    
    return v1 + v2 + v3 + v4 + v5;
}

int main(void) {
    long result1, result2;
    
    /* Vary the global flag to affect optimization decisions */
    global_flag = (rand() % 3) + 1;
    
    /* Call with different parameters to create different patterns */
    result1 = caller_function(global_flag * 10, global_flag * 20);
    
    /* Change flag to potentially trigger different save/restore placement */
    global_flag = (rand() % 2);
    result2 = caller_function2(global_flag * 5, global_flag * 15);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %ld, %ld\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}
