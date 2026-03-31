/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
int global_array[100] = {0};
volatile int16_t global_volatile_short = 100;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low_part : 8;
    unsigned int high_part : 8;
    unsigned int pad : 16;
} volatile bitfield_var;

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = global_volatile_int;
    int output;
    
    /* Force secondary reload: memory -> specific register (eax) */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints with fixed register output */
    int in2 = output + 1;
    asm volatile (
        "addl %1, %0"
        : "+a" (output)  /* Fixed to eax */
        : "rm" (in2)     /* Register or memory */
        : "cc"
    );
    
    global_volatile_int = output;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx") = global_volatile_int;
    register int r2 asm("ecx") = r1 + 1;
    
    /* Force move between specific registers through memory */
    int temp;
    asm volatile (
        "movl %%ebx, %0\n\t"
        "movl %0, %%ecx"
        : "=m" (temp)
        : 
        : "ebx", "ecx", "memory"
    );
    
    /* Use in expression requiring different register */
    asm volatile (
        "imull %%ecx, %%ebx"
        : "+b" (r1)
        : "c" (r2)
        : "cc"
    );
    
    global_volatile_int = r1;
}

/* Function to generate SUBREG and STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Access bitfield - generates SUBREG */
    bitfield_var.low_part = global_volatile_int & 0xFF;
    
    /* Explicit truncation */
    int32_t wide_val = global_volatile_int * 1000;
    int16_t narrow_val = (int16_t)wide_val;  /* May generate SUBREG */
    
    /* Use truncated value in complex expression */
    asm volatile (
        "addw %1, %0"
        : "+r" (narrow_val)
        : "rm" (global_volatile_short)
        : "cc"
    );
    
    global_volatile_short = narrow_val;
}

/* Function with complex addressing modes and memory operands */
void test_complex_addressing(void) {
    int index = global_volatile_int & 0xF;
    
    /* Memory operand with displacement */
    asm volatile (
        "addl $1, %0"
        : "+m" (global_array[index + 10])
        : 
        : "cc", "memory"
    );
    
    /* Multiple memory references in one asm */
    int temp;
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl %%eax, %0"
        : "+m" (global_array[index])
        : "r" (global_array), "r" (index)
        : "%eax", "cc", "memory"
    );
}

/* Function using inline asm with output in specific register class */
#ifdef __x86_64__
void test_64bit_constraints(void) {
    uint64_t large_val = 0x123456789ABCDEF0ULL;
    uint64_t result;
    
    /* Force use of 64-bit register with memory operand */
    asm volatile (
        "movq %1, %%rax\n\t"
        "rorq $32, %%rax\n\t"
        "movq %%rax, %0"
        : "=r" (result)
        : "m" (large_val)
        : "%rax", "cc"
    );
    
    /* Mix 32-bit and 64-bit operations */
    uint32_t low_part = result & 0xFFFFFFFF;
    asm volatile (
        "bswapl %0"
        : "+r" (low_part)
        : 
        : "cc"
    );
}
#endif

/* Function with volatile and optimization barriers */
void test_volatile_barriers(void) {
    volatile int barrier1 = global_volatile_int;
    volatile int barrier2;
    
    /* Memory clobber to force reloads */
    asm volatile (
        ""
        : 
        : "m" (barrier1)
        : "memory"
    );
    
    /* Complex expression split across multiple asm statements */
    asm volatile (
        "movl %1, %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%eax"
        : "=a" (barrier2)
        : "m" (barrier1)
        : "cc"
    );
    
    /* Another barrier */
    asm volatile (
        "movl %1, %0"
        : "=rm" (global_volatile_int)
        : "r" (barrier2)
        : "cc", "memory"
    );
}

/* Main function that calls all tests */
int main(void) {
    int i, sum = 0;
    
    /* Run tests multiple times to increase coverage chance */
    for (i = 0; i < 10; i++) {
        test_restrictive_registers();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_barriers();
        
        #ifdef __x86_64__
        test_64bit_constraints();
        #endif
        
        /* Use results to prevent dead code elimination */
        sum += global_volatile_int + global_volatile_short + global_array[i];
    }
    
    /* Final computation using inline asm with multiple constraints */
    int final_result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (final_result)
        : "r" (sum), "m" (global_volatile_int)
        : "%eax", "cc", "memory"
    );
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;  /* Return non-zero to indicate execution */
}
