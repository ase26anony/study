/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int g_volatile_int = 12345;
volatile short g_volatile_short = 6789;
int g_normal_int = 42;
short g_normal_short = 24;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} g_bitfield = {0xAAAA, 0x5555};

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input, output;
    
    /* Force input from memory to specific register class */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)
        : "m"(g_volatile_int)
        : "%eax"
    );
    
    /* Multiple alternative constraints with fixed register output */
    asm volatile (
        "addl %1, %0"
        : "+a"(output)          /* EAX only */
        : "rm"(input)           /* Register or memory */
        : "cc"
    );
    
    /* Immediate value to fixed register with memory output */
    asm volatile (
        "movl $999, %%ebx\n\t"
        "movl %%ebx, %0"
        : "=m"(g_volatile_int)
        :
        : "%ebx", "memory"
    );
}

/* Test 2: Register variables with explicit binding */
void test_register_variables(void) {
    /* Bind variables to specific registers */
    register int reg_a asm("ebx");
    register int reg_b asm("ecx");
    register int reg_c asm("edx");
    
    /* Force conflicts between register-bound variables */
    reg_a = g_normal_int;
    reg_b = reg_a + 1;
    
    /* Inline asm that requires different register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(reg_c)
        : "r"(reg_b)
        : "%eax"
    );
    
    /* Use in expression with memory operand */
    g_normal_int = reg_a + reg_c;
}

/* Test 3: SUBREG and partial register access patterns */
void test_subreg_patterns(void) {
    int32_t full_reg;
    int16_t half_reg;
    int8_t byte_reg;
    
    /* Generate SUBREG for truncation */
    full_reg = g_volatile_int;
    half_reg = (int16_t)full_reg;      /* Likely generates SUBREG */
    byte_reg = (int8_t)half_reg;       /* Another SUBREG */
    
    /* Use the truncated values in operations */
    asm volatile (
        "addw %1, %0"
        : "+r"(half_reg)
        : "rm"(g_volatile_short)
        : "cc"
    );
    
    /* Bitfield operations that may generate SUBREG */
    g_bitfield.low16 = half_reg;
    g_bitfield.high16 = byte_reg * 2;
    
    /* Complex expression with partial registers */
    asm volatile (
        "movzwl %1, %%eax\n\t"
        "movsbl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(full_reg)
        : "r"(g_bitfield.low16), "r"(byte_reg)
        : "%eax", "%ebx"
    );
}

/* Test 4: Complex addressing modes and multiple reloads */
void test_complex_addressing(void) {
    int array[10] = {0};
    int index = 3;
    int result;
    
    /* Force base+index*scale addressing with specific register */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(array), "r"(index)
        : "%eax"
    );
    
    /* Memory-to-memory with register constraints */
    asm volatile (
        "pushl %%ebx\n\t"
        "movl %1, %%ebx\n\t"
        "movl %%ebx, %0\n\t"
        "popl %%ebx"
        : "=m"(g_volatile_int)
        : "m"(result)
        : "%ebx", "memory"
    );
    
    /* Multiple constraints that may require secondary reloads */
    asm volatile (
        "imull %1, %0"
        : "+a"(result)          /* EAX only for output */
        : "mr"(g_normal_int)    /* Memory or register */
        : "%edx"
    );
}

/* Test 5: Volatile mixing and optimization barriers */
void test_volatile_mixing(void) {
    volatile int local_volatile;
    int normal_local;
    
    /* Create data flow through volatile */
    local_volatile = g_volatile_int;
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" : : : "memory");
    
    /* Complex operation with volatile input */
    asm volatile (
        "movl %1, %%ecx\n\t"
        "leal (%%ecx, %%ecx, 2), %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(normal_local)
        : "m"(local_volatile)
        : "%eax", "%ecx"
    );
    
    /* Use result in another constrained operation */
    asm volatile (
        "movl %1, %%edx\n\t"
        "negl %%edx\n\t"
        "movl %%edx, %0"
        : "=a"(g_normal_int)    /* Must be EAX */
        : "r"(normal_local)
        : "%edx"
    );
}

/* Test 6: STRICT_LOW_PART patterns via bit operations */
void test_strict_low_part(void) {
    unsigned int value = 0x12345678;
    unsigned short low_part;
    
    /* Operations that might generate STRICT_LOW_PART */
    asm volatile (
        "movw %1, %0\n\t"
        "andw $0xFF, %0"
        : "=r"(low_part)
        : "r"(value)
        : "cc"
    );
    
    /* Use low part in calculation */
    asm volatile (
        "addw $100, %0"
        : "+r"(low_part)
        :
        : "cc"
    );
    
    /* Combine back */
    value = (value & 0xFFFF0000) | low_part;
    g_normal_int = value;
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_restrictive_constraints();
        test_register_variables();
        test_subreg_patterns();
        test_complex_addressing();
        test_volatile_mixing();
        test_strict_low_part();
        
        /* Accumulate some result to prevent dead code elimination */
        result += g_normal_int + g_volatile_int;
    }
    
    printf("Tests completed. Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
