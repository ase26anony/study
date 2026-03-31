/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload() function.
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_var1 = 123;
volatile int global_var2 = 456;
volatile long long global_ll = 789LL;

/* Bit-field structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int b : 11;
    unsigned int c : 16;
} volatile bitfield = {1, 2, 3};

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers() {
    int result;
    int input = 42;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=r" (result)
        : "m" (input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed output */
    register int x asm("ebx") = 100;
    asm volatile (
        "imull %1, %0"
        : "+a" (result)
        : "rm" (x)
        : "cc"
    );
}

/* Test 2: Complex addressing modes with register binding */
void test_complex_addressing() {
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    
    reg1 = global_var1;
    reg2 = global_var2;
    
    /* Force reload between differently bound registers */
    int temp;
    asm volatile (
        "movl %%esi, %%ecx\n\t"
        "addl %%edi, %%ecx\n\t"
        "movl %%ecx, %0"
        : "=r" (temp)
        :
        : "%ecx", "%esi", "%edi", "cc"
    );
    
    /* Memory operand with specific register class requirement */
    asm volatile (
        "movq %1, %%mm0\n\t"
        "movq %%mm0, %0"
        : "=m" (global_ll)
        : "m" (global_ll)
        : "%mm0"
    );
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns() {
    /* Bit-field accesses generate SUBREG */
    unsigned int val = bitfield.a + bitfield.b + bitfield.c;
    
    /* Explicit truncation */
    int32_t large = 0x12345678;
    int16_t small = (int16_t)large;
    
    /* Use the truncated value in a way that requires reload */
    asm volatile (
        "addw %1, %0"
        : "+r" (small)
        : "rm" (val)
        : "cc"
    );
    
    /* STRICT_LOW_PART-like pattern */
    int32_t combined;
    asm volatile (
        "movw %1, %0\n\t"
        "shrl $16, %0"
        : "=&r" (combined)
        : "r" (small)
        : "cc"
    );
}

/* Test 4: Multiple reloads with volatile and barriers */
void test_volatile_reloads() {
    volatile int v1 = 100;
    volatile int v2 = 200;
    int result;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Complex constraint with multiple alternatives */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (result)
        : "g" (v1), "g" (v2)
        : "%eax", "cc", "memory"
    );
    
    /* Nested inline asm to create dependency chains */
    int temp = result;
    asm volatile (
        "1:\n\t"
        "decl %0\n\t"
        "jnz 1b"
        : "+r" (temp)
        :
        : "cc"
    );
}

/* Test 5: 64-bit operations on 32-bit targets (if applicable) */
void test_64bit_reloads() {
    long long ll1 = global_ll;
    long long ll2 = 1000LL;
    long long result;
    
    /* 64-bit operations often require multiple registers */
    asm volatile (
        "addq %1, %0"
        : "+r" (ll1)
        : "rm" (ll2)
        : "cc"
    );
    
    result = ll1;
    
    /* Mix with 32-bit accesses */
    int low_part = (int)result;
    asm volatile (
        "imull %1, %0"
        : "+a" (low_part)
        : "rm" (global_var1)
        : "cc"
    );
}

/* Test 6: Function calling with register arguments */
#ifdef __x86_64__
void test_register_arguments(int a asm("rdi"), int b asm("rsi")) {
    /* Force specific register usage for arguments */
    int result;
    asm volatile (
        "leal (%1, %2), %0"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Use result in another constrained operation */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "shll %%cl, %0"
        : "+r" (result)
        : "r" (a)
        : "%ecx", "cc"
    );
}
#endif

/* Main function that runs all tests */
int main() {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        total += global_var1;
        
        test_complex_addressing();
        total += global_var2;
        
        test_subreg_patterns();
        total += i;
        
        test_volatile_reloads();
        total += 1;
        
        test_64bit_reloads();
        total += (int)global_ll;
        
        #ifdef __x86_64__
        test_register_arguments(i, i * 2);
        total += i * 3;
        #endif
        
        /* Modify globals to create different addressing modes */
        global_var1 += i;
        global_var2 -= i;
        global_ll *= (i + 1);
    }
    
    printf("Tests completed. Result: %d\n", total);
    return total;
}
