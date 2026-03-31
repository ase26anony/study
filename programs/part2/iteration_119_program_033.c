/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789L;
int g_normal_array[100] = {0};

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} g_bitfield;

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = g_volatile_int;
    int output;
    
    /* Force 'eax' register constraint with memory input */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)
        : "m" (input)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    long long_val = g_volatile_long;
    long result;
    asm volatile (
        "add %1, %0"
        : "+a" (result)      /* 'a' constraint for eax */
        : "rm" (long_val)    /* register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate value */
    asm volatile (
        "imul $0x%0, %1, %2"
        : "=r" (output)
        : "r" (input), "N" (37)  /* 'N' for unsigned 8-bit constant */
        : "cc"
    );
}

/* Test 2: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind to specific registers */
    register int reg_eax asm("eax");
    register int reg_ebx asm("ebx");
    register int reg_ecx asm("ecx");
    
    /* Force conflicts between register-bound variables */
    reg_eax = g_volatile_int;
    reg_ebx = reg_eax + 1;
    
    /* Inline asm that requires different registers */
    asm volatile (
        "xchg %0, %1"
        : "+r" (reg_eax), "+r" (reg_ebx)
        :
        : "cc"
    );
    
    /* Use in expression that forces reload */
    reg_ecx = (reg_eax << 4) | (reg_ebx & 0xF);
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    /* Bitfield operations that generate SUBREG */
    g_bitfield.low16 = g_volatile_int & 0xFFFF;
    g_bitfield.high16 = (g_volatile_int >> 16) & 0xFFFF;
    g_bitfield.volatile_field = g_volatile_int & 0xFF;
    
    /* Explicit truncation */
    int32_t full_int = g_volatile_int * 100;
    int16_t half_int = (int16_t)full_int;
    int8_t byte_val = (int8_t)(full_int >> 8);
    
    /* Use truncated values in complex expressions */
    volatile int16_t v_half = half_int;
    asm volatile (
        "movw %w1, %w0"
        : "=r" (full_int)
        : "r" (v_half)
    );
    
    /* STRICT_LOW_PART pattern via masking */
    int masked = full_int & 0x0000FFFF;
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r" (masked)
        :
        : "cc"
    );
}

/* Test 4: Complex addressing modes with arrays */
void test_complex_addressing(void) {
    int index = g_volatile_int % 100;
    int offset = g_volatile_int % 50;
    
    /* Base + index * scale addressing */
    int value1 = g_normal_array[index * 2];
    int value2 = g_normal_array[offset + index];
    
    /* Force memory operand with restrictive register */
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r" (value1)
        : "r" (g_normal_array), "r" (index)
        : "memory"
    );
    
    /* Displacement addressing with fixed register */
    asm volatile (
        "leal 0x1000(%1), %0"
        : "=a" (value2)
        : "r" (&g_normal_array[50])
        :
    );
}

/* Test 5: Multiple reloads in loops */
void test_loop_reloads(void) {
    volatile int accum = 0;
    register int i asm("ebx");
    
    for (i = 0; i < 10; i++) {
        int temp;
        
        /* Each iteration may need different reloads */
        asm volatile (
            "movl %%ebx, %%eax\n\t"
            "imull %1, %%eax\n\t"
            "movl %%eax, %0"
            : "=rm" (temp)
            : "rm" (g_volatile_int)
            : "%eax", "cc"
        );
        
        accum += temp;
        
        /* Memory clobber to force reloads */
        asm volatile ("" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    g_normal_array[0] = accum;
}

/* Test 6: Mixed size operations forcing mode changes */
void test_mixed_sizes(void) {
    char char_val = g_volatile_int & 0xFF;
    short short_val = g_volatile_int & 0xFFFF;
    int int_val = g_volatile_int;
    long long long_val = (long long)g_volatile_int * 1000LL;
    
    /* Operations mixing different sizes */
    asm volatile (
        "movsbl %1, %0\n\t"
        "addw %2, %w0\n\t"
        "addl %3, %0"
        : "+r" (int_val)
        : "r" (char_val), "r" (short_val), "rm" (g_volatile_int)
        : "cc"
    );
    
    /* 64-bit operation on 32-bit target (if applicable) */
    asm volatile (
        "addl %%eax, %%edx\n\t"
        "adcl $0, %%edx"
        : "+d" (long_val)
        : "a" (int_val), "0" (long_val)
        : "cc"
    );
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times with different values */
    for (int iteration = 0; iteration < 3; iteration++) {
        g_volatile_int = 1000 + iteration * 500;
        g_volatile_long = 5000L + iteration * 1000L;
        
        test_restrictive_constraints();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_loop_reloads();
        test_mixed_sizes();
        
        result += g_normal_array[0] + g_volatile_int;
        
        /* Compiler barrier */
        asm volatile ("" : : : "memory");
    }
    
    printf("Tests completed. Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
