/* reload_test.c - Test program to trigger GCC reload pass initialization */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[100] = {0};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} volatile bitfield;

/* Test 1: Force secondary reloads via restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_var;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int r1 asm("ebx") = output;
    int r2;
    
    asm volatile (
        "movl %2, %0\n\t"
        "addl %1, %0"
        : "=&a" (r2), "+&r" (r1)
        : "rm" (global_var)
        : "cc"
    );
    
    global_var = r2;
}

/* Test 2: Complex addressing modes with register binding conflicts */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int a asm("esi") = 100;
    register int b asm("edi") = 200;
    register int c asm("ebx");
    
    /* Force reloads by using conflicting constraints */
    asm volatile (
        "imull %1, %0\n\t"
        "addl %2, %0"
        : "+&r" (a), "+&r" (b)
        : "rm" (global_array[50])
        : "cc"
    );
    
    /* Use the bound register in another asm with different constraint */
    asm volatile (
        "movl %%ebx, %0\n\t"
        "leal (%1, %2, 4), %%ebx"
        : "=r" (c)
        : "r" (a), "r" (b)
        : "%ebx"
    );
    
    global_array[0] = c;
}

/* Test 3: SUBREG generation via bitfield operations */
void test_subreg_patterns(void) {
    /* Access bitfields - generates SUBREG RTL */
    bitfield.low16 = 0xABCD;
    bitfield.high16 = 0x1234;
    
    uint32_t full_word;
    uint16_t half_word;
    
    /* Force partial register access */
    asm volatile (
        "movzwl %1, %0"
        : "=r" (full_word)
        : "m" (bitfield.low16)
    );
    
    /* Explicit truncation */
    half_word = (uint16_t)full_word;
    
    /* Use in complex expression requiring reload */
    asm volatile (
        "addw %1, %0"
        : "+r" (half_word)
        : "rm" (bitfield.high16)
        : "cc"
    );
    
    global_var = half_word;
}

/* Test 4: STRICT_LOW_PART and memory clobbers */
void test_strict_low_part(void) {
    volatile uint32_t x = 0xFFFFFFFF;
    uint16_t y;
    
    /* Pattern that may generate STRICT_LOW_PART */
    asm volatile (
        "movw %1, %0\n\t"
        "andw $0x7FFF, %0"
        : "=r" (y)
        : "m" (x)
        : "cc"
    );
    
    /* Memory clobber to force conservative reloads */
    asm volatile (
        "lock; addl $1, %0"
        : "+m" (global_var)
        :
        : "cc", "memory"
    );
    
    /* Use the result with restrictive constraint */
    register uint16_t z asm("ax") = y;
    
    asm volatile (
        "incw %0"
        : "+r" (z)
        :
        : "cc"
    );
    
    global_array[1] = z;
}

/* Test 5: Complex inline asm with multiple outputs and earlyclobber */
void test_complex_asm(void) {
    int in1 = global_var;
    int in2 = global_array[10];
    int out1, out2;
    
    /* Earlyclobber constraints force separate registers */
    asm volatile (
        "movl %2, %0\n\t"
        "imull %3, %0\n\t"
        "movl %0, %1\n\t"
        "addl $100, %1"
        : "=&a" (out1), "=&r" (out2)
        : "rm" (in1), "rm" (in2)
        : "cc"
    );
    
    /* Nested asm use */
    asm volatile (
        "addl %1, %0"
        : "+r" (out1)
        : "r" (out2)
        : "cc"
    );
    
    global_var = out1;
}

/* Test 6: Mixed size operations forcing mode changes */
void test_mixed_sizes(void) {
    int8_t byte_var = 127;
    int16_t short_var = -1000;
    int32_t int_var = 1000000;
    int64_t long_var;
    
    /* Mixed size operations requiring mode conversions */
    asm volatile (
        "movsbl %1, %%eax\n\t"
        "movswl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cltd\n\t"
        "idivl %3\n\t"
        "movl %%eax, %0"
        : "=r" (int_var)
        : "m" (byte_var), "m" (short_var), "rm" (global_var)
        : "%eax", "%ebx", "%edx", "cc"
    );
    
    /* 64-bit operation on 32-bit target (may require special handling) */
    long_var = (int64_t)int_var * global_var;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "mull %2\n\t"
        "movl %%eax, %0"
        : "=rm" (global_array[2])
        : "r" ((int)long_var), "r" (global_var)
        : "%eax", "%edx", "cc"
    );
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_strict_low_part();
        test_complex_asm();
        test_mixed_sizes();
        
        /* Use results to prevent dead code elimination */
        result += global_var + global_array[0] + global_array[1] + global_array[2];
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
