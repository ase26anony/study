/* reload_test.c
 * Test program to trigger GCC's reload pass initialization block
 * Specifically targets lines 1381-1399 in reload.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
int global_array[100] = {0};
volatile int *volatile_ptr = &global_volatile_int;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global;

/* Test 1: Force secondary reloads via restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = 100;
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
    
    /* Multiple alternative constraints with fixed register */
    register int r1 asm("ebx") = 50;
    asm volatile (
        "addl %1, %0"
        : "+a" (output)
        : "rm" (r1)
        : "cc"
    );
    
    /* Complex constraint with immediate and memory */
    asm volatile (
        "imull %1, %0"
        : "+a" (output)
        : "rmi" (global_volatile_int)
        : "cc"
    );
    
    global_array[0] = output;
}

/* Test 2: Register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int reg_a asm("eax");
    register int reg_b asm("ebx");
    register int reg_c asm("ecx");
    
    reg_a = 10;
    reg_b = 20;
    reg_c = 30;
    
    /* Force moves between these registers through memory */
    volatile int temp;
    
    /* This should require reloads */
    asm volatile (
        "movl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax"
        : "=a" (reg_a)
        : "b" (reg_b), "c" (reg_c)
        : "cc"
    );
    
    /* Use the result in a memory operation */
    asm volatile (
        "movl %%eax, %0"
        : "=m" (temp)
        : "a" (reg_a)
    );
    
    /* Complex addressing mode with index */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[reg_a % 10])
        : "m" (temp)
        : "%eax"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield operations generate SUBREG */
    bitfield_global.low16 = 0xABCD;
    bitfield_global.high16 = 0x1234;
    
    /* Access through pointer with type punning */
    uint32_t full_word = *(uint32_t*)&bitfield_global;
    
    /* Truncation operations */
    int32_t large_val = 0x12345678;
    int16_t truncated = (int16_t)large_val;
    
    /* Use in arithmetic forcing register partial access */
    asm volatile (
        "movswl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[1])
        : "r" (truncated)
        : "%eax", "cc"
    );
    
    /* STRICT_LOW_PART pattern through masking */
    int masked = full_word & 0xFFFF;
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r" (masked)
        :: "cc"
    );
    
    global_array[2] = masked;
}

/* Test 4: Complex addressing modes with multiple memory references */
void test_complex_addressing(void) {
    int index = global_volatile_int;
    int scale = 4;
    
    /* Force base+index*scale addressing */
    asm volatile (
        "movl (%1, %2, %3), %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[10])
        : "r" (global_array), "r" (index), "i" (scale)
        : "%eax", "memory"
    );
    
    /* Multiple memory constraints in one asm */
    int src = 999;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[20])
        : "m" (src)
        : "%eax", "memory"
    );
}

/* Test 5: Volatile and optimization barrier combinations */
void test_volatile_barriers(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3;
    int result;
    
    /* Memory clobber forces conservative treatment */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=rm" (result)
        : "rm" (v1), "rm" (v2)
        : "%eax", "cc", "memory"
    );
    
    /* Chain of volatile operations */
    asm volatile ("" : "+m" (v3));
    v3 = result;
    asm volatile ("" : : "m" (v3));
    
    /* Use volatile pointer with offset */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl $10, %%eax\n\t"
        "movl %%eax, (%1)"
        : 
        : "r" (volatile_ptr)
        : "%eax", "memory", "cc"
    );
}

/* Test 6: Mixed register classes and spill scenarios */
void test_mixed_register_classes(void) {
    double d1 = 3.14, d2 = 2.71;
    int i1, i2;
    
    /* Mix float and integer registers */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fistpl %0"
        : "=m" (i1)
        : "m" (d1), "m" (d2)
        : "st", "st(1)", "memory"
    );
    
    /* Force x87 register pressure */
    asm volatile (
        "fld1\n\t"
        "fldln2\n\t"
        "fldlg2\n\t"
        : : : "st", "st(1)", "st(2)"
    );
    
    /* Now use general registers with memory constraint */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (i2)
        : "m" (i1)
        : "%eax", "cc"
    );
}

/* Test 7: Loop with varying addressing modes */
void test_loop_reloads(void) {
    int i, sum = 0;
    
    for (i = 0; i < 10; i++) {
        volatile int* addr = &global_array[i];
        
        /* Varying constraints in loop */
        asm volatile (
            "movl (%1), %%eax\n\t"
            "addl %%eax, %0"
            : "+r" (sum)
            : "r" (addr)
            : "%eax", "cc", "memory"
        );
        
        /* Force different register allocation each iteration */
        if (i & 1) {
            asm volatile (
                "movl %1, %%ebx\n\t"
                "addl %%ebx, %0"
                : "+r" (sum)
                : "r" (i)
                : "%ebx", "cc"
            );
        }
    }
    
    global_array[99] = sum;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_barriers();
        test_mixed_register_classes();
        test_loop_reloads();
        
        /* Accumulate results to prevent dead code elimination */
        total += global_array[0] + global_array[99];
    }
    
    printf("Tests completed. Result: %d\n", total);
    
    /* Return non-zero to ensure all code paths are considered */
    return total != 0 ? 0 : 1;
}
