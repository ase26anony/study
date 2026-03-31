/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};
volatile int* volatile volatile_ptr = &global_volatile;

/* Bitfield structure for SUBREG generation */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global;

/* Test 1: Complex addressing modes with inline assembly */
void test_complex_addressing(void) {
    int local_var = 123;
    volatile int vol_local = 456;
    int result;
    
    /* Force secondary reload by requiring specific register class */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=r" (result)
        : "m" (global_volatile), "0" (local_var)
        : "eax", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0"
        : "+r,a" (result)
        : "rm,i" (vol_local)
        : "cc"
    );
    
    /* Memory operand with specific register output */
    register int reg_bound asm("ebx") = result;
    asm volatile (
        "movl %1, %%ecx\n\t"
        "addl %%ecx, %%ebx"
        : "+b" (reg_bound)
        : "m" (global_array[10])
        : "ecx", "cc"
    );
    
    global_volatile = reg_bound;
}

/* Test 2: Register-bound variables with conflicting constraints */
void test_register_conflicts(void) {
    /* Bind to specific registers */
    register int r1 asm("edi");
    register int r2 asm("esi");
    register int r3 asm("ebx");
    
    r1 = global_volatile;
    r2 = 1000;
    
    /* Force reload by using conflicting constraints */
    asm volatile (
        "addl %%esi, %%edi\n\t"
        "movl %%edi, %%ebx"
        : "=b" (r3), "+D" (r1)
        : "S" (r2)
        : "cc"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "leal (%1, %2, 4), %0"
        : "=r" (r1)
        : "r" (r3), "r" (r2)
    );
    
    volatile_ptr = &global_array[r1 & 63];
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    int32_t full_reg = 0x12345678;
    volatile int16_t half_vol;
    
    /* Generate SUBREG through truncation */
    int16_t truncated = (int16_t)full_reg;
    half_vol = truncated;
    
    /* Bitfield operations that generate SUBREG */
    bitfield_global.low16 = truncated;
    bitfield_global.high16 = (full_reg >> 16) & 0xFFFF;
    bitfield_global.volatile_field = (truncated >> 4) & 0xFF;
    
    /* Mixed-size operations */
    int32_t extended = (int32_t)half_vol;
    extended = extended * 2;
    
    /* Inline asm with size modifiers */
    asm volatile (
        "movw %w1, %w0"
        : "=r" (extended)
        : "r" (truncated)
    );
    
    global_array[0] = extended;
}

/* Test 4: Memory barriers and volatile forcing reloads */
void test_memory_barriers(void) {
    volatile int barrier_var = 0;
    int temp1, temp2, temp3;
    
    /* Memory clobber forces reloads */
    asm volatile (
        "movl %2, %%eax\n\t"
        "addl %%eax, %1\n\t"
        "movl %1, %0"
        : "=rm" (temp1), "+rm" (temp2)
        : "rm" (global_volatile)
        : "eax", "cc", "memory"
    );
    
    /* Compiler barrier */
    asm volatile ("" : : : "memory");
    
    /* Multiple volatile accesses */
    barrier_var = temp1;
    asm volatile ("" : "+m" (barrier_var));
    
    /* Complex addressing with volatile */
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r" (temp3)
        : "r" (global_array), "r" (barrier_var)
        : "memory"
    );
    
    /* Force spill/reload with many operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %3, %%ecx\n\t"
        "subl %%ecx, %0"
        : "=rm" (temp1)
        : "rm" (temp2), "rm" (temp3), "rm" (barrier_var)
        : "eax", "ebx", "ecx", "cc", "memory"
    );
}

/* Test 5: Loop with complex constraints to stress reload */
void test_loop_reloads(void) {
    int i;
    volatile int accum = 0;
    register int loop_reg asm("esi") = 0;
    
    for (i = 0; i < 100; i++) {
        int idx = i & 31;
        
        /* Varying constraints in loop */
        asm volatile (
            "addl %1, %0"
            : "+r" (loop_reg)
            : "rm" (global_array[idx])
            : "cc"
        );
        
        /* Alternate between register and memory constraints */
        if (i & 1) {
            asm volatile (
                "subl %1, %0"
                : "+r" (loop_reg)
                : "i" (1)
                : "cc"
            );
        } else {
            asm volatile (
                "addl $2, %0"
                : "+r" (loop_reg)
                :: "cc"
            );
        }
        
        /* Force memory store occasionally */
        if ((i % 10) == 0) {
            asm volatile (
                "movl %1, %0"
                : "=m" (global_array[idx])
                : "r" (loop_reg)
            );
        }
    }
    
    accum = loop_reg;
    global_volatile = accum;
}

/* Main function that runs all tests */
int main(void) {
    int final_result = 0;
    
    /* Initialize globals */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 3;
    }
    
    /* Run all tests multiple times */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_complex_addressing();
        test_register_conflicts();
        test_subreg_patterns();
        test_memory_barriers();
        test_loop_reloads();
        
        /* Mix results to prevent dead code elimination */
        final_result += global_volatile;
        final_result ^= global_array[iteration * 10];
        final_result += (int)bitfield_global.low16;
    }
    
    /* Final computation using all techniques */
    register int final_reg asm("eax") = final_result;
    asm volatile (
        "movl %1, %%ecx\n\t"
        "leal (%%ecx, %0, 2), %0"
        : "+a" (final_reg)
        : "rm" (global_volatile)
        : "ecx"
    );
    
    /* Ensure value is used */
    printf("Final result: %d\n", final_reg);
    
    return final_reg & 255;  /* Return non-zero to indicate execution */
}
