/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789L;
int g_normal_int = 100;
long g_normal_long = 999999999L;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} g_bitfield = {0xAAAA, 0x5555, 0xFF};

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int result1, result2;
    int input1 = g_volatile_int;
    long input2 = g_volatile_long;
    
    /* Force use of specific registers with memory operands */
    /* This may require secondary reloads to move memory to restricted registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=r"(result1)
        : "m"(input1)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed register output */
    asm volatile (
        "imul %2, %0"
        : "=a"(result2)
        : "0"(result1), "rm"(input2)
        : "cc", "edx"
    );
    
    /* Complex constraint with immediate and memory */
    int temp = g_normal_int;
    asm volatile (
        "leal 100(%1, %0), %0"
        : "+r"(temp)
        : "rm"(g_volatile_int)
        : "cc"
    );
    
    g_normal_int = temp + result1 + result2;
}

/* Test 2: Register-bound variables causing conflicts */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int reg_a asm("ebx");
    register int reg_b asm("ecx");
    register int reg_c asm("esi");
    
    /* Initialize with volatile reads to prevent optimization */
    reg_a = g_volatile_int;
    reg_b = g_normal_int;
    
    /* Force conflict: use register-bound variable in asm requiring different reg */
    asm volatile (
        "xchgl %%ebx, %%ecx\n\t"
        "addl %%esi, %%ebx"
        : "+&b"(reg_a), "+&c"(reg_b)
        : "S"(reg_c)
        : "cc"
    );
    
    /* More conflicts with memory constraints */
    int memory_var = g_volatile_long;
    asm volatile (
        "movl %%ebx, %0\n\t"
        "movl %1, %%ebx"
        : "=m"(memory_var)
        : "r"(reg_a)
        : "%ebx"
    );
    
    g_normal_int += reg_a + reg_b;
}

/* Test 3: Bitfield operations generating SUBREG RTL */
void test_bitfield_operations(void) {
    /* Access bitfields - may generate SUBREG in RTL */
    unsigned int low_part = g_bitfield.low16;
    unsigned int high_part = g_bitfield.high16;
    volatile unsigned int volatile_part = g_bitfield.volatile_field;
    
    /* Operations that might require partial register accesses */
    g_bitfield.low16 = (low_part ^ 0xFFFF) & 0x7FFF;
    g_bitfield.high16 = high_part | volatile_part;
    
    /* Explicit truncation to smaller types */
    int32_t full_int = g_volatile_int * 2;
    int16_t half_int = (int16_t)full_int;  /* May generate SUBREG */
    int8_t quarter_int = (int8_t)half_int; /* Another SUBREG */
    
    /* Use truncated values in asm with constraints */
    asm volatile (
        "movswl %1, %0"
        : "=r"(full_int)
        : "r"(half_int)
    );
    
    /* STRICT_LOW_PART-like operation through masking */
    full_int = (full_int & ~0xFF) | (quarter_int & 0xFF);
    
    g_normal_int += full_int + half_int;
}

/* Test 4: Complex addressing modes and multiple reloads */
void test_complex_addressing(void) {
    int array[100];
    static int static_array[50];
    volatile int* volatile_ptr = &g_volatile_int;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array[i] = i * g_volatile_int;
    }
    
    /* Complex addressing in asm with multiple constraints */
    int index = g_normal_int % 100;
    int result;
    
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r"(result)
        : "r"(array), "r"(index)
        : "memory"
    );
    
    /* Multiple alternative constraints */
    int temp1, temp2;
    asm volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        "movl %0, %1"
        : "=&a"(temp1), "=rm"(temp2)
        : "rm"(result), "ri"(g_volatile_int)
        : "cc"
    );
    
    /* Pointer arithmetic with volatile */
    int* ptr = &array[50];
    asm volatile (
        ""
        : "+r"(ptr)
        : "r"(volatile_ptr)
        : "memory"
    );
    
    g_normal_int = result + temp1 + temp2;
}

/* Test 5: Mixed operations to create complex reload scenarios */
void test_mixed_operations(void) {
    double double_var = 3.14159;
    volatile double volatile_double = 2.71828;
    long double long_double_var = 1.41421356L;
    
    /* Mix floating point and integer - may require different register classes */
    int int_from_double;
    asm volatile (
        "fldl %1\n\t"
        "fistpl %0"
        : "=m"(int_from_double)
        : "m"(double_var)
        : "st", "st(1)", "memory"
    );
    
    /* Force spill/reload with volatile */
    volatile_double = double_var * 2.0;
    double_var = volatile_double / 3.0;
    
    /* Integer operations with the result */
    register int reg_int asm("edi");
    reg_int = int_from_double;
    
    asm volatile (
        "imull %1, %0"
        : "+r"(reg_int)
        : "rm"(g_normal_int)
        : "cc", "%edx"
    );
    
    /* Memory barrier to force all pending operations */
    asm volatile ("" ::: "memory");
    
    g_normal_int = reg_int;
}

int main(void) {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Run all tests multiple times to increase coverage chances */
    for (int i = 0; i < 10; i++) {
        test_restrictive_constraints();
        test_register_conflicts();
        test_bitfield_operations();
        test_complex_addressing();
        test_mixed_operations();
        
        total += g_normal_int;
        g_volatile_int++;  /* Change volatile to affect next iteration */
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final total: %d\n", total);
    printf("Bitfield: low=0x%04X, high=0x%04X\n", 
           g_bitfield.low16, g_bitfield.high16);
    
    return total % 256;  /* Return non-zero to indicate execution */
}
