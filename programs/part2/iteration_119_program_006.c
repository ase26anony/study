/* Test program to trigger secondary reload initialization in GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};
int *global_ptr = &global_array[0];

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int b : 10;
    unsigned int c : 15;
    unsigned int d : 2;
} bitfield;

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile;
    int output;
    
    /* Force secondary reload: memory -> specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints */
    int in1 = global_volatile + 1;
    int in2 = global_volatile + 2;
    
    asm volatile (
        "addl %2, %0\n\t"
        "subl %3, %0"
        : "+&a" (output)
        : "0" (output), "rm" (in1), "rm" (in2)
        : "cc"
    );
    
    global_volatile = output;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx");
    register int r2 asm("edi");
    register int r3 asm("esi");
    
    r1 = global_volatile;
    r2 = r1 * 2;
    r3 = r2 + 100;
    
    /* Force move between fixed registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (r3)
        : "r" (r1)
        : "%eax", "cc"
    );
    
    /* Complex addressing with fixed register */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "imull %%eax, %0"
        : "+r" (r2)
        : "r" (global_ptr)
        : "%eax", "memory", "cc"
    );
    
    global_volatile = r3 + r2;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Bitfield operations generate SUBREG */
    bitfield.a = global_volatile & 0x1F;
    bitfield.b = (global_volatile >> 5) & 0x3FF;
    bitfield.c = (global_volatile >> 15) & 0x7FFF;
    
    /* Explicit truncation */
    int32_t large_val = global_volatile * 1000;
    int16_t truncated = (int16_t)large_val;
    int8_t small_truncated = (int8_t)truncated;
    
    /* Use truncated values in operations requiring reloads */
    asm volatile (
        "movswl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (global_volatile)
        : "r" (truncated)
        : "%eax", "cc"
    );
    
    /* Memory operation with partial register */
    asm volatile (
        "movb %1, (%0)"
        :
        : "r" (&small_truncated), "r" ((uint8_t)(global_volatile & 0xFF))
        : "memory"
    );
}

/* Function with complex memory addressing modes */
void test_complex_addressing(void) {
    int index = global_volatile % 50;
    int scale = 2;
    
    /* Complex addressing that might need secondary reload */
    asm volatile (
        "movl (%1, %2, %c3), %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (global_volatile)
        : "r" (global_array), "r" (index), "i" (sizeof(int))
        : "%eax", "memory", "cc"
    );
    
    /* Immediate value with restrictive output */
    int result;
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "xorl %%eax, %0"
        : "=a" (result)
        : "0" (global_volatile)
        : "cc"
    );
    
    global_volatile = result;
}

/* Function mixing volatile and inline assembly */
void test_volatile_mix(void) {
    volatile int v1 = global_volatile;
    volatile int v2 = v1 * 2;
    
    /* Memory clobber to force conservative codegen */
    asm volatile (
        ""
        : "+m" (v1), "+m" (v2)
        :
        : "memory"
    );
    
    /* Complex operation with volatile operands */
    int temp;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (temp)
        : "m" (v1), "m" (v2)
        : "%eax", "cc"
    );
    
    /* Use in restrictive context */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "subl %%ebx, %0"
        : "+a" (global_volatile)
        : "r" (temp)
        : "%ebx", "cc"
    );
}

/* Main function orchestrating all tests */
int main(void) {
    int i;
    
    /* Initialize global data */
    for (i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run test functions multiple times with different values */
    for (i = 0; i < 10; i++) {
        global_volatile = i * 7;
        
        test_restrictive_constraints();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_mix();
        
        /* Create data dependencies between iterations */
        global_ptr = &global_array[global_volatile % 90];
    }
    
    /* Final computation to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < 100; i++) {
        sum += global_array[i];
    }
    sum += global_volatile;
    
    printf("Result: %d\n", sum);
    return sum & 0xFF;
}
