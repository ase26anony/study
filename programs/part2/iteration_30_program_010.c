/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>

/* Global volatile flag to make conditional unpredictable */
volatile int global_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void) {
    /* Use inline assembly to clobber many caller-saved registers */
    /* x86_64 specific clobber list */
#if defined(__x86_64__) || defined(__i386__)
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
#elif defined(__aarch64__)
    /* ARM64 specific clobber list */
    asm volatile (
        "# Clobber caller-saved registers\n\t"
        "mov x0, #0\n\t"
        "mov x1, #0\n\t"
        "mov x2, #0\n\t"
        "mov x3, #0\n\t"
        "mov x4, #0\n\t"
        "mov x5, #0\n\t"
        "mov x6, #0\n\t"
        "mov x7, #0\n\t"
        "mov x8, #0\n\t"
        "mov x9, #0\n\t"
        "mov x10, #0\n\t"
        "mov x11, #0\n\t"
        "mov x12, #0\n\t"
        "mov x13, #0\n\t"
        "mov x14, #0\n\t"
        "mov x15, #0\n\t"
        "mov x16, #0\n\t"
        "mov x17, #0\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8",
          "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "memory"
    );
#else
    /* Generic memory clobber */
    asm volatile ("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-inline")))
int caller_function(int param1, int param2, int param3) {
    /* Declare many local variables to create register pressure */
    register int a = param1 * 2;
    register int b = param2 + 7;
    register int c = param3 - 3;
    register int d = a + b;
    register int e = b * c;
    register int f = c / (param1 ? param1 : 1);
    register int g = d + e + f;
    register int h = a * b * c;
    register int i = param1 + param2 + param3;
    register int j = (param1 << 2) | (param2 & 0xF);
    
    /* Create data dependencies before the call */
    a = b + c;
    b = c * d;
    c = d - e;
    d = e ^ f;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int flag = global_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (flag) {
        /* This call should be in its own basic block */
        callee_function();
    }
    
    /* More computations after the call, using all variables */
    /* Create complex data dependencies to keep variables live */
    e = f + g + (a & 0xFF);
    f = g * h / (i ? i : 1);
    g = h ^ i ^ j;
    h = i + j + a;
    i = j * a + b;
    j = (a << 3) | (b & 0x7);
    
    /* Use all variables in final computation */
    int result = a + b + c + d + e + f + g + h + i + j;
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to prevent tail call optimization */
__attribute__((noipa, noinline))
int use_result(int x) {
    volatile int sink = x;
    return sink;
}

int main(void) {
    /* Initialize with non-zero values */
    int x = 42;
    int y = 17;
    int z = 99;
    
    /* Call the function multiple times with different parameters
       to prevent constant propagation */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        int result = caller_function(x + i, y + i * 2, z - i);
        sum += use_result(result);
        
        /* Modify global flag occasionally */
        if (i % 7 == 0) {
            global_flag = !global_flag;
        }
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
