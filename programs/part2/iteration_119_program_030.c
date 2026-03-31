/* Test program to trigger secondary reload initialization in GCC's reload pass */
#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile long global_long = 100;
volatile int* global_ptr = &global_var;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int full32;
} bitfield = {0xAAAA, 0x5555, 0xDEADBEEF};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_registers(void) {
    int input = 1234;
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
    register int x asm("ebx") = output;
    int y = 5678;
    
    asm volatile (
        "addl %1, %0"
        : "+r" (x)
        : "rm" (y)
        : "cc"
    );
    
    /* Complex constraint: immediate value -> specific register */
    asm volatile (
        "movl $999, %%ecx"
        :: 
        : "%ecx"
    );
    
    global_var = x;
}

/* Function using register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind variables to specific hardware registers */
    register int a asm("esi") = 100;
    register int b asm("edi") = 200;
    volatile int c = 300;
    
    /* Force reload by using variable in conflicting context */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (a)
        : "r" (b)
        : "%eax", "cc"
    );
    
    /* Memory clobber to prevent optimization */
    asm volatile ("" ::: "memory");
    
    /* Use all register-bound variables in computation */
    int result = a + b + c;
    
    /* Inline asm with output in different register than input */
    asm volatile (
        "movl %1, %%edx\n\t"
        "imull %%edx, %0"
        : "+r" (result)
        : "r" (global_var)
        : "%edx", "cc"
    );
    
    global_long = result;
}

/* Function to generate SUBREG and partial register accesses */
void test_subreg_patterns(void) {
    /* Bitfield access generates SUBREG */
    uint16_t low_part = bitfield.low16;
    uint16_t high_part = bitfield.high16;
    
    /* Explicit truncation */
    int32_t large_val = 0x12345678;
    int16_t truncated = (int16_t)large_val;
    
    /* Use in arithmetic to force register allocation */
    volatile int sum = low_part + high_part + truncated;
    
    /* Inline asm with byte-wise operations (partial register access) */
    uint32_t value = 0xAABBCCDD;
    uint8_t byte1, byte2;
    
    asm volatile (
        "movb %h1, %0\n\t"
        "movb %b1, %2"
        : "=r" (byte1), "=r" (byte2)
        : "0" (value), "1" (value)
        : "cc"
    );
    
    /* Force memory operand with specific register class */
    asm volatile (
        "movl %1, %%eax\n\t"
        "andl $0xFFFF, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (bitfield.full32)
        : "m" (value)
        : "%eax", "cc", "memory"
    );
}

/* Function with complex addressing modes and multiple operands */
void test_complex_addressing(void) {
    volatile int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int index = 5;
    int result;
    
    /* Complex memory addressing with index */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (array), "r" (index)
        : "%eax", "memory"
    );
    
    /* Multiple output operands with different constraints */
    int out1, out2;
    asm volatile (
        "movl $1, %0\n\t"
        "movl $2, %1"
        : "=a" (out1), "=r" (out2)
        :
        : "cc"
    );
    
    /* Use both outputs in another asm with memory operand */
    asm volatile (
        "addl %2, %0\n\t"
        "subl %2, %1"
        : "+r" (out1), "+r" (out2)
        : "rm" (global_var)
        : "cc"
    );
    
    global_ptr = &array[result % 10];
}

/* Function mixing all techniques with loops to increase complexity */
void test_mixed_reloads(void) {
    volatile int counters[3] = {0};
    register int r1 asm("ebx") = 1;
    register int r2 asm("ecx") = 2;
    
    for (int i = 0; i < 10; i++) {
        /* Vary constraints in loop */
        int temp;
        
        if (i % 2 == 0) {
            asm volatile (
                "movl %%ebx, %%eax\n\t"
                "addl %%ecx, %%eax\n\t"
                "movl %%eax, %0"
                : "=rm" (temp)
                :
                : "%eax", "cc"
            );
        } else {
            asm volatile (
                "imull %%ecx, %%ebx\n\t"
                "movl %%ebx, %0"
                : "=r" (temp)
                :
                : "%ebx", "cc"
            );
        }
        
        /* Access bitfield in loop */
        bitfield.low16 = (bitfield.low16 + temp) & 0xFFFF;
        
        /* Complex addressing with global pointer */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0"
            : "+r" (counters[i % 3])
            : "r" (global_ptr)
            : "%eax", "memory", "cc"
        );
        
        /* Memory barrier */
        asm volatile ("" ::: "memory");
    }
    
    /* Final computation using all techniques */
    int final_result = 0;
    for (int i = 0; i < 3; i++) {
        final_result += counters[i];
    }
    
    /* Force secondary reload with immediate and specific register */
    asm volatile (
        "movl $0xFFFFFFFF, %%edx\n\t"
        "andl %1, %%edx\n\t"
        "movl %%edx, %0"
        : "=a" (final_result)
        : "r" (final_result)
        : "%edx", "cc"
    );
    
    global_var = final_result;
}

int main(void) {
    int total = 0;
    
    /* Call all test functions multiple times with different parameters */
    for (int i = 0; i < 3; i++) {
        test_restrictive_registers();
        total += global_var;
        
        test_register_conflicts();
        total += (int)global_long;
        
        test_subreg_patterns();
        total += bitfield.full32;
        
        test_complex_addressing();
        total += *global_ptr;
        
        test_mixed_reloads();
        total += global_var;
        
        /* Vary bitfield to create different SUBREG patterns */
        bitfield.low16 = (bitfield.low16 * 3) & 0xFFFF;
        bitfield.high16 = (bitfield.high16 / 2) & 0xFFFF;
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
