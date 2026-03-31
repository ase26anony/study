/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[100] = {0};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int full32;
};

volatile struct bitfield_struct bf = {0};

/* Function using inline assembly with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_var;
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
        : "+a" (output)      /* eax only */
        : "rm" (in2)         /* register or memory */
        : "cc"
    );
    
    global_var = output;
}

/* Function using register variables bound to specific registers */
void test_register_binding(void) {
    /* Bind variables to specific registers */
    register int x asm("ebx");
    register int y asm("ecx");
    register int z asm("edx");
    
    x = global_var;
    y = x * 2;
    
    /* Force conflict: ebx-bound variable used in asm requiring different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (z)           /* edx-bound, but compiler may need to reload */
        : "r" (x)            /* ebx-bound */
        : "%eax", "cc"
    );
    
    /* Complex addressing with displacement */
    asm volatile (
        "movl %c1(%%ebx), %0"
        : "=r" (y)
        : "i" (16), "b" (&global_array[0])
        : "memory"
    );
    
    global_array[0] = z + y;
}

/* Function to generate SUBREG and partial register accesses */
void test_subreg_patterns(void) {
    /* Bitfield access generates SUBREG */
    bf.low16 = 0xABCD;
    bf.high16 = 0x1234;
    
    /* Explicit truncation */
    int32_t large_val = 0x12345678;
    int16_t truncated = (int16_t)large_val;
    
    /* Use truncated value in way that requires reload */
    volatile int16_t* ptr = &truncated;
    int result;
    
    asm volatile (
        "movswl %1, %0"
        : "=r" (result)
        : "m" (*ptr)
        : "memory"
    );
    
    /* STRICT_LOW_PART pattern via masking */
    int masked = result & 0xFFFF;
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r" (masked)
        :
        : "cc"
    );
    
    bf.full32 = masked;
}

/* Function with complex memory addressing modes */
void test_complex_addressing(void) {
    volatile int* ptr = &global_var;
    int index = 10;
    int scale = 4;
    int result;
    
    /* Complex addressing mode that may require secondary reload */
    asm volatile (
        "movl (%1, %2, %c3), %0"
        : "=r" (result)
        : "r" (ptr), "r" (index), "i" (scale)
        : "memory"
    );
    
    /* Immediate value with restrictive output */
    asm volatile (
        "imull %1, %0"
        : "+a" (result)      /* eax only */
        : "i" (37)           /* immediate */
        : "cc"
    );
    
    global_array[index] = result;
}

/* Function mixing volatile and optimization barriers */
void test_volatile_barriers(void) {
    volatile int v1 = 100;
    volatile int v2 = 200;
    int temp;
    
    /* Memory clobber forces conservative treatment */
    asm volatile (
        ""
        : "+m" (v1), "+m" (v2)
        :
        : "memory"
    );
    
    /* Complex expression with volatile operands */
    temp = v1 + v2;
    
    /* Force reload of computed value into restrictive register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "leal (%%eax, %%eax, 2), %0"
        : "=r" (v1)
        : "r" (temp)
        : "%eax"
    );
    
    /* Nested asm with multiple constraints */
    asm volatile (
        "push %%eax\n\t"
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        "pop %%eax"
        : "+m" (v2)
        : "ir" (50)          /* immediate or register */
        : "cc", "memory"
    );
}

/* Main function orchestrating all tests */
int main(void) {
    int sum = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        global_var = i * 100;
        
        test_restrictive_constraints();
        test_register_binding();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_barriers();
        
        /* Accumulate results to prevent dead code elimination */
        sum += global_var + global_array[i] + bf.full32;
    }
    
    printf("Test completed. Checksum: %d\n", sum);
    
    /* Use result to prevent optimization */
    asm volatile ("" : : "r" (sum));
    
    return sum != 0 ? 0 : 1;
}
