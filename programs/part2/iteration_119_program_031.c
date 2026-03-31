/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};

/* Bit-field structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
};

/* Global bitfield */
struct bitfield_struct global_bitfield;

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = global_volatile;
    int output;
    
    /* Force use of specific register class with memory operand */
    /* May require secondary reload to move from memory to eax */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    int in2 = output + 1;
    asm volatile (
        "addl %1, %0"
        : "+a" (output)      /* eax only */
        : "rm" (in2)         /* register or memory */
        : "cc"
    );
    
    global_volatile = output;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    
    r1 = global_volatile;
    r2 = r1 * 2;
    
    /* Force move between specific registers */
    asm volatile (
        "xchgl %%ebx, %%ecx"
        : "+r" (r1), "+r" (r2)
    );
    
    /* Use in expression requiring different register */
    int result;
    asm volatile (
        "imull %1, %2\n\t"
        "movl %2, %0"
        : "=r" (result)
        : "r" (r1), "a" (r2)  /* r2 must be in eax for imul */
        : "cc"
    );
    
    global_array[0] = result;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Operations that generate partial register accesses */
    int32_t full_word = global_volatile * 100;
    
    /* Explicit truncation - may generate SUBREG */
    int16_t half_word = (int16_t)full_word;
    
    /* Use in arithmetic forcing promotion back to int */
    int32_t expanded = half_word * 2;
    
    /* Bitfield operations */
    global_bitfield.low16 = half_word;
    global_bitfield.high16 = expanded >> 16;
    
    /* Volatile bitfield access */
    global_bitfield.volatile_field = (expanded & 0xFF);
    
    /* Complex expression with partial results */
    asm volatile (
        "movzwl %1, %0\n\t"    /* zero-extend 16-bit to 32-bit */
        "addl $1, %0"
        : "=r" (expanded)
        : "m" (half_word)
    );
    
    global_volatile = expanded;
}

/* Function with complex addressing modes */
void test_complex_addressing(void) {
    int index = global_volatile & 0xF;
    
    /* Memory operand with complex address calculation */
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r" (index)
        : "r" (global_array), "r" (index)
    );
    
    /* Force secondary reload with immediate + memory */
    asm volatile (
        "addl $0x12345678, %0"
        : "+r" (index)
        : 
        : "cc"
    );
    
    /* Store with complex addressing */
    asm volatile (
        "movl %1, (%0, %2, 4)"
        : 
        : "r" (global_array), "r" (index), "r" (index)
        : "memory"
    );
}

/* Function mixing all techniques */
void test_mixed_reloads(void) {
    volatile int local_volatile = 99;
    register int reg_var asm("esi");
    
    /* Start with volatile */
    reg_var = local_volatile;
    
    /* Complex inline asm with multiple constraints */
    int temp;
    asm volatile (
        "movl %%esi, %%eax\n\t"
        "leal (%%eax, %%eax, 2), %0\n\t"
        "addl %1, %0"
        : "=&r" (temp)        /* earlyclobber */
        : "ri" (global_volatile)  /* register or immediate */
        : "%eax", "%esi", "cc"
    );
    
    /* Use in memory operation */
    asm volatile (
        "lock xaddl %0, %1"
        : "+r" (temp), "+m" (global_array[10])
    );
    
    /* Generate SUBREG through bit manipulation */
    uint64_t large_val = (uint64_t)temp * 0x100000000ULL;
    uint32_t low_part = (uint32_t)large_val;  /* truncation */
    
    /* Force reload of partial result */
    asm volatile (
        "bsrl %1, %0"
        : "=r" (temp)
        : "r" (low_part)
        : "cc"
    );
    
    local_volatile = temp;
}

/* Main function that calls all tests */
int main(void) {
    int i, sum = 0;
    
    /* Initialize globals */
    for (i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Run tests multiple times to increase coverage chance */
    for (i = 0; i < 10; i++) {
        test_restrictive_registers();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_mixed_reloads();
        
        /* Use results to prevent dead code elimination */
        sum += global_volatile + global_array[i] + global_bitfield.low16;
    }
    
    printf("Result: %d\n", sum);
    return sum & 0xFF;  /* Return non-zero to ensure execution */
}
