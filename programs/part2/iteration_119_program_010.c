/* reload_stress.c - Stress GCC's reload pass to cover secondary reload initialization */
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int g1 = 100;
volatile int g2 = 200;
volatile int g3 = 300;
volatile long gl1 = 400;
volatile long gl2 = 500;

/* Bitfield structure to generate SUBREG accesses */
struct bitfields {
    int a : 5;
    int b : 12;
    int c : 15;
    volatile int d : 8;
};

struct bitfields bf = {1, 2, 3, 4};

/* Test function 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers(void) {
    int result;
    int input = g1 + g2;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (result)
        : "m" (g1), "m" (g2)
        : "eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed output */
    register int r1 asm("ebx") = result;
    register int r2 asm("ecx") = g3;
    
    asm volatile (
        "imull %1, %0"
        : "+a" (r1)
        : "rm" (r2)
        : "cc"
    );
    
    g1 = r1;
}

/* Test function 2: Complex addressing modes with mixed constraints */
void test_complex_addressing(void) {
    volatile int arr[10] = {0,1,2,3,4,5,6,7,8,9};
    register long r3 asm("r12");
    register int r4 asm("r13");
    
    /* Force memory operand with specific register class */
    asm volatile (
        "movq %1, %0\n\t"
        "addq $10, %0"
        : "=r" (r3)
        : "m" (gl1)
        : "cc"
    );
    
    /* Multiple constraints that may require secondary reloads */
    asm volatile (
        "movl %1, %0\n\t"
        "subl %2, %0"
        : "=&r" (r4)
        : "g" (arr[5]), "i" (100)
        : "cc"
    );
    
    /* STRICT_LOW_PART pattern via bitfield access */
    bf.a = r4 & 0x1F;
    bf.b = (r4 >> 5) & 0xFFF;
    
    /* Force partial register access */
    int16_t low_part = (int16_t)r4;
    asm volatile (
        "movw %w1, %0"
        : "=m" (arr[0])
        : "r" (low_part)
    );
}

/* Test function 3: Secondary reloads with immediate values */
void test_immediate_reloads(void) {
    int temp1, temp2;
    register int r5 asm("r14") = 0x12345678;
    
    /* Large immediate may require secondary reload on some arches */
    asm volatile (
        "xorl %%eax, %%eax\n\t"
        "addl %1, %%eax"
        : "=a" (temp1)
        : "i" (0x7FFFFFFF)
        : "cc"
    );
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    
    /* Complex constraint combination */
    asm volatile (
        "movl %1, %0\n\t"
        "orl %2, %0"
        : "=r" (temp2)
        : "g" (r5), "i" (0xFF00FF00)
        : "cc"
    );
    
    /* Volatile access to force reloads */
    g2 = temp1 + temp2;
}

/* Test function 4: SUBREG patterns via type punning */
void test_subreg_patterns(void) {
    volatile double d1 = 3.14159;
    volatile float f1 = 2.71828;
    int64_t int64_val;
    int32_t int32_val;
    
    /* Type punning that may generate SUBREG */
    asm volatile (
        "movq %1, %0"
        : "=r" (int64_val)
        : "m" (d1)
    );
    
    /* Access different parts of the value */
    int32_val = (int32_t)(int64_val >> 32);
    
    /* Force register partial access */
    asm volatile (
        "movl %1, %%eax\n\t"
        "shrl $16, %%eax"
        : "=a" (g3)
        : "r" (int32_val)
        : "cc"
    );
    
    /* More bitfield manipulation */
    bf.c = g3 & 0x7FFF;
    bf.d = (g3 >> 15) & 0xFF;
}

/* Test function 5: Multiple output operands with conflicting constraints */
void test_multiple_outputs(void) {
    int out1, out2;
    register int r6 asm("r8") = g1;
    register int r7 asm("r9") = g2;
    
    /* Outputs tied to specific registers */
    asm volatile (
        "movl %2, %%eax\n\t"
        "movl %3, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%eax, %1"
        : "=r" (out1), "=r" (out2)
        : "r" (r6), "r" (r7)
        : "eax", "ebx", "cc"
    );
    
    /* Use the results in memory operations */
    asm volatile (
        "movl %1, %0"
        : "=m" (gl2)
        : "r" (out1 + out2)
    );
}

/* Main function that orchestrates all tests */
int main(void) {
    volatile int counter = 0;
    
    /* Loop to increase reload opportunities */
    for (int i = 0; i < 10; i++) {
        test_fixed_registers();
        counter += g1;
        
        test_complex_addressing();
        counter += bf.a + bf.b;
        
        test_immediate_reloads();
        counter += g2;
        
        test_subreg_patterns();
        counter += g3 + bf.c;
        
        test_multiple_outputs();
        counter += (int)gl2;
        
        /* Memory clobber to force reloads between iterations */
        asm volatile ("" ::: "memory");
    }
    
    /* Final computation to prevent dead code elimination */
    int final_result = counter + bf.d;
    
    /* Use result to prevent optimization */
    asm volatile (
        "movl %0, %%eax"
        :: "r" (final_result)
        : "eax"
    );
    
    return final_result % 256;
}
