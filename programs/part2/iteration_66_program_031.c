/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void clobber_many_regs_1(void) __attribute__((noinline));
extern void clobber_many_regs_2(void) __attribute__((noinline));
extern void clobber_many_regs_3(void) __attribute__((noinline));

/* External function to prevent inlining */
extern int external_func(int x) __attribute__((noinline));

/* Function with extremely high register pressure around calls */
int high_pressure_function(int seed) {
    /* Many local variables that must stay live across calls */
    int a = seed + 1;
    int b = seed + 2;
    int c = seed + 3;
    int d = seed + 4;
    int e = seed + 5;
    int f = seed + 6;
    int g = seed + 7;
    int h = seed + 8;
    int i = seed + 9;
    int j = seed + 10;
    int k = seed + 11;
    int l = seed + 12;
    int m = seed + 13;
    int n = seed + 14;
    int o = seed + 15;
    int p = seed + 16;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a * b - c + d / (e + 1) + f * g - h + i * j;
}

/* Function with control flow variation */
int control_flow_function(int x, int y) {
    /* Many live variables */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = x + y;
    int v4 = x - y;
    int v5 = x * y;
    int v6 = x + 1;
    int v7 = y + 2;
    int v8 = x * 3;
    int v9 = y * 4;
    int v10 = x + y + 1;
    
    if (x > y) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Use variables in true branch */
        v1 = v1 + v2;
        v3 = v3 * v4;
        
        /* Another call */
        external_func(v1);
    } else {
        /* Call in false branch */
        clobber_many_regs_2();
        
        /* Different variable usage pattern */
        v5 = v5 + v6;
        v7 = v7 * v8;
        
        /* Call with arguments */
        external_func(v5);
    }
    
    /* Variables must remain live here */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loop and calls */
int loop_function(int iterations) {
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables inside loop */
        int a = i * 2;
        int b = i * 3;
        int c = i * 4;
        int d = i * 5;
        int e = i * 6;
        int f = i * 7;
        int g = i * 8;
        int h = i * 9;
        
        /* Call that clobbers registers inside loop */
        clobber_many_regs_1();
        
        /* Use variables after call */
        acc += a + b + c + d + e + f + g + h;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
            acc += external_func(i);
        }
    }
    
    return acc;
}

/* Function mixing caller-saved and callee-saved usage */
int mixed_register_function(int x) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("rbx") = x * 2;
    register long r2 asm("r12") = x * 3;
    register long r3 asm("r13") = x * 4;
    
    /* Many caller-saved register variables */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs_3();
    
    /* Use both register types */
    long callee_sum = r1 + r2 + r3;
    int caller_sum = a + b + c + d + e + f + g + h;
    
    /* Another call */
    external_func(caller_sum);
    
    return (int)(callee_sum + caller_sum);
}

/* Function with multiple basic blocks and calls at block boundaries */
int boundary_function(int x, int y, int z) {
    int result = 0;
    
    switch (x % 4) {
        case 0:
            /* Many live variables in case 0 */
            int a0 = y * 1, b0 = y * 2, c0 = y * 3, d0 = y * 4;
            clobber_many_regs_1();
            result = a0 + b0 + c0 + d0;
            break;
            
        case 1:
            /* Different set of variables */
            int a1 = z * 1, b1 = z * 2, c1 = z * 3, d1 = z * 4, e1 = z * 5;
            clobber_many_regs_2();
            result = a1 + b1 + c1 + d1 + e1;
            break;
            
        case 2:
            /* Even more variables */
            int a2 = x * 1, b2 = x * 2, c2 = x * 3, d2 = x * 4;
            int e2 = x * 5, f2 = x * 6, g2 = x * 7;
            clobber_many_regs_3();
            result = a2 + b2 + c2 + d2 + e2 + f2 + g2;
            break;
            
        default:
            /* Complex computation with calls */
            int a3 = x + y, b3 = x + z, c3 = y + z;
            int d3 = x * y, e3 = x * z, f3 = y * z;
            clobber_many_regs_1();
            external_func(a3);
            result = a3 + b3 + c3 + d3 + e3 + f3;
            break;
    }
    
    return result;
}

/* Main function that exercises all patterns */
int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int total = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int iter = argc > 2 ? atoi(argv[2]) : 10;
    int mode = argc > 3 ? atoi(argv[3]) % 4 : 0;
    
    /* Call all test functions */
    total += high_pressure_function(seed);
    total += control_flow_function(seed, seed * 2);
    total += loop_function(iter);
    total += mixed_register_function(seed);
    total += boundary_function(seed, seed + 1, seed + 2);
    
    /* Prevent dead code elimination */
    volatile int output = total;
    
    printf("Result: %d\n", output);
    return output != 0 ? 0 : 1;
}

/* Definitions of clobber functions using inline assembly */
void clobber_many_regs_1(void) {
    /* Clobber many caller-saved registers */
    asm volatile (
#if defined(__x86_64__)
        "mov $0, %%rax\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif defined(__aarch64__)
        "mov x0, #0\n"
        "mov x1, #0\n"
        "mov x2, #0\n"
        "mov x3, #0\n"
        "mov x4, #0\n"
        "mov x5, #0\n"
        "mov x6, #0\n"
        "mov x7, #0\n"
        "mov x8, #0\n"
        "mov x9, #0\n"
        "mov x10, #0\n"
        "mov x11, #0\n"
        "mov x12, #0\n"
        "mov x13, #0\n"
        "mov x14, #0\n"
        "mov x15, #0\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "memory"
#else
        /* Generic clobber */
        : : : "memory"
#endif
    );
}

void clobber_many_regs_2(void) {
    /* Different clobber pattern */
    asm volatile (
#if defined(__x86_64__)
        "xor %%rax, %%rax\n"
        "xor %%rcx, %%rcx\n"
        "xor %%rdx, %%rdx\n"
        "xor %%rsi, %%rsi\n"
        "xor %%rdi, %%rdi\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "memory"
#elif defined(__aarch64__)
        "mov x0, xzr\n"
        "mov x1, xzr\n"
        "mov x2, xzr\n"
        "mov x3, xzr\n"
        "mov x4, xzr\n"
        "mov x5, xzr\n"
        "mov x6, xzr\n"
        "mov x7, xzr\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "memory"
#else
        : : : "memory"
#endif
    );
}

void clobber_many_regs_3(void) {
    /* Yet another clobber pattern */
    asm volatile (
#if defined(__x86_64__)
        "add $1, %%rax\n"
        "add $1, %%rcx\n"
        "add $1, %%rdx\n"
        :
        :
        : "rax", "rcx", "rdx", "memory"
#elif defined(__aarch64__)
        "add x0, x0, #1\n"
        "add x1, x1, #1\n"
        "add x2, x2, #1\n"
        "add x3, x3, #1\n"
        :
        :
        : "x0", "x1", "x2", "x3", "memory"
#else
        : : : "memory"
#endif
    );
}

/* Simple external function implementation */
int external_func(int x) {
    return x * 2 + 1;
}
