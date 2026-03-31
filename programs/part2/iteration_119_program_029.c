/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var = 42;
volatile int global_array[4] = {1, 2, 3, 4};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int part : 16;
    int small : 8;
} bf;

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input, output;
    
    /* Force input from memory to specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(global_var)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int x asm("ebx") = input;
    asm volatile (
        "addl %1, %0"
        : "+a"(output)      /* eax only */
        : "rm"(x)           /* register or memory */
        : "cc"
    );
    
    /* Immediate value to fixed register with memory output */
    asm volatile (
        "movl $100, %%ecx\n\t"
        "addl %%ecx, %0"
        : "+m"(global_array[1])
        : 
        : "%ecx", "memory"
    );
}

/* Test 2: Complex addressing modes with register binding */
void test_complex_addressing(void) {
    /* Bind variables to specific registers */
    register int a asm("esi");
    register int b asm("edi");
    
    a = global_var;
    b = 100;
    
    /* Force reloads by using both in constraints */
    int result;
    asm volatile (
        "imull %1, %2\n\t"
        "addl %2, %0"
        : "=r"(result)
        : "r"(a), "r"(b)
        : "cc"
    );
    
    /* Memory operand with offset */
    asm volatile (
        "movl 4(%1), %0"
        : "=r"(result)
        : "r"(global_array)
        : "memory"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield accesses generate SUBREG */
    bf.full = 0x12345678;
    int16_t partial = bf.part;  /* This may generate SUBREG */
    
    /* Explicit truncation */
    int32_t large = 0x98765432;
    int16_t truncated = (int16_t)large;
    
    /* Use truncated values in operations requiring reloads */
    asm volatile (
        "addw %1, %0"
        : "+r"(partial)
        : "rm"(truncated)
        : "cc"
    );
    
    /* STRICT_LOW_PART pattern via inline asm */
    asm volatile (
        "addw %1, %0"
        : "+r"(partial)
        : "rm"(truncated)
        : "cc"
    );
}

/* Test 4: Mixed volatile and register variables */
void test_mixed_volatile(void) {
    volatile int v1 = 10;
    volatile int v2 = 20;
    register int r1 asm("edx");
    register int r2 asm("ecx");
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Complex constraints with volatile */
    int temp;
    asm volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %1, %%eax"
        : "=rm"(temp), "=a"(r1)
        : "m"(v1), "r"(v2)
        : "%eax"
    );
    
    /* Multiple output constraints */
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1"
        : "=r"(r1), "=r"(r2)
        : "m"(global_var), "i"(100)
        : 
    );
}

/* Test 5: Nested inline assembly with clobbers */
void test_nested_assembly(void) {
    int a = 1, b = 2, c = 3, d = 4;
    
    /* Multiple inline asm statements forcing intermediate reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax"
        : "=a"(a)
        : "rm"(b), "rm"(c)
        : "cc"
    );
    
    asm volatile (
        "xchgl %0, %1"
        : "+r"(a), "+r"(d)
        : 
        : "cc"
    );
    
    /* Complex constraints with earlyclobber */
    asm volatile (
        "leal (%1, %2), %0"
        : "=&r"(c)      /* earlyclobber */
        : "r"(a), "r"(d)
        : 
    );
}

/* Test 6: Force secondary reloads for floating point */
void test_float_reloads(void) {
    volatile double d1 = 3.14;
    volatile double d2 = 2.71;
    double result;
    
    /* x87 floating point with memory operands */
    asm volatile (
        "fldl %1\n\t"
        "faddl %2\n\t"
        "fstpl %0"
        : "=m"(result)
        : "m"(d1), "m"(d2)
        : "st", "st(1)"
    );
    
    /* MMX/SSE constraints */
    float f1 = 1.0, f2 = 2.0;
    asm volatile (
        "addss %1, %0"
        : "+x"(f1)
        : "xm"(f2)
        : 
    );
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    /* Run all tests multiple times to increase reload opportunities */
    for (int i = 0; i < 3; i++) {
        test_restrictive_constraints();
        test_complex_addressing();
        test_subreg_patterns();
        test_mixed_volatile();
        test_nested_assembly();
        test_float_reloads();
        
        checksum += global_var + global_array[i % 4];
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Final inline asm with complex constraints */
    register int final asm("eax") = checksum;
    asm volatile (
        "movl %%eax, %%ecx\n\t"
        "shrl $1, %%ecx\n\t"
        "addl %%ecx, %0"
        : "+m"(global_var)
        : "a"(final)
        : "%ecx", "cc", "memory"
    );
    
    return global_var > 0 ? 0 : 1;
}
