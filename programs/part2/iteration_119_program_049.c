/* reload_coverage.c - Program to stress GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};
register int reg_var asm("ebx");

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low : 8;
    unsigned int high : 24;
} bitfield;

/* Test function 1: Complex addressing modes with inline assembly */
void test_complex_addressing(void) {
    int local_var = 123;
    int output1, output2;
    
    /* Force secondary reload by requiring specific register class */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=r" (output1)
        : "m" (global_volatile)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0"
        : "+r" (output2)
        : "rm" (local_var)
        : "cc"
    );
    
    /* Mix register-bound variable with memory constraint */
    register int temp asm("ecx") = 456;
    asm volatile (
        "addl %1, %0"
        : "+r" (temp)
        : "m" (global_array[50])
        : "cc"
    );
    
    global_volatile = output1 + output2 + temp;
}

/* Test function 2: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield operations generate SUBREG */
    bitfield.low = 0xAB;
    bitfield.high = 0xCDEF01;
    
    /* Explicit truncation */
    int32_t large_val = 0x12345678;
    int16_t truncated = (int16_t)large_val;
    
    /* Use truncated value in way that requires reload */
    volatile int16_t volatile_short = truncated;
    asm volatile (
        "movw %1, %%ax\n\t"
        "cwtl\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[10])
        : "r" (volatile_short)
        : "%eax"
    );
    
    /* STRICT_LOW_PART pattern via inline assembly */
    int result;
    asm volatile (
        "addl %1, %0"
        : "=@ccc" (result)
        : "r" (truncated), "0" (bitfield.low)
        : "cc"
    );
}

/* Test function 3: Register conflicts and secondary reloads */
void test_register_conflicts(void) {
    /* Bind multiple variables to specific registers */
    register int a asm("eax");
    register int b asm("ebx");
    register int c asm("ecx");
    
    a = 100;
    b = 200;
    c = 300;
    
    /* Force reload by requiring different register for operation */
    asm volatile (
        "xchgl %%ebx, %%ecx\n\t"
        "addl %%eax, %%ecx\n\t"
        "movl %%ecx, %0"
        : "=m" (global_array[20])
        : 
        : "%eax", "%ebx", "%ecx", "cc"
    );
    
    /* Complex constraint with immediate */
    int d = 400;
    asm volatile (
        "leal (%1, %2), %0"
        : "=r" (d)
        : "r" (a), "ir" (12345)
    );
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
}

/* Test function 4: Mixed constraints and volatile operations */
void test_mixed_constraints(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3;
    int r1, r2, r3;
    
    /* Multiple output constraints */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1"
        : "=&r" (r1), "=r" (r2)
        : "m" (v1), "m" (v2)
        : "memory"
    );
    
    /* Input/output with different classes */
    asm volatile (
        "addl %2, %0"
        : "+r" (r3)
        : "r" (r1), "m" (global_volatile)
        : "cc"
    );
    
    /* Force spill/reload with large expression */
    for (int i = 0; i < 10; i++) {
        asm volatile (
            "addl $1, %0"
            : "+m" (global_array[i])
        );
    }
}

/* Test function 5: Architecture-specific constraints */
#ifdef __x86_64__
void test_x86_64_specific(void) {
    uint64_t large_val = 0x123456789ABCDEF0ULL;
    uint32_t low_part, high_part;
    
    /* Force 64-bit operations with 32-bit constraints */
    asm volatile (
        "movq %1, %%rax\n\t"
        "movl %%eax, %0\n\t"
        "shrl $32, %%rax\n\t"
        "movl %%eax, %2"
        : "=r" (low_part), "=r" (high_part)
        : "m" (large_val)
        : "%rax", "cc"
    );
    
    /* SSE register constraints */
    double d1 = 3.14, d2 = 2.71;
    asm volatile (
        "addsd %1, %0"
        : "+x" (d1)
        : "x" (d2)
    );
}
#endif

/* Main function that orchestrates all tests */
int main(void) {
    int result = 0;
    
    /* Initialize global data */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    reg_var = 999;
    
    /* Run all test functions multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_complex_addressing();
        test_subreg_patterns();
        test_register_conflicts();
        test_mixed_constraints();
        
        #ifdef __x86_64__
        test_x86_64_specific();
        #endif
        
        /* Accumulate some result to prevent dead code elimination */
        result += global_array[iteration * 10] + global_volatile + reg_var;
    }
    
    /* Final computation using all generated values */
    for (int i = 0; i < 50; i++) {
        result += global_array[i];
    }
    
    /* Use result to prevent optimization */
    asm volatile ("" : "+r" (result));
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}
