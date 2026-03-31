/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
int global_array[100] = {0};
volatile int* volatile volatile_ptr = &global_volatile_int;

/* Bitfield structure to generate SUBREG accesses */
struct bitfield_struct {
    unsigned int small : 8;
    unsigned int medium : 16;
    unsigned int large : 24;
    volatile unsigned int volatile_field : 4;
} bitfield_global;

/* Test 1: Force secondary reloads with restrictive register constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile_int;
    int output;
    
    /* Force use of specific registers with memory operands */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)        /* Output in any register */
        : "m" (input)          /* Input from memory */
        : "%eax", "memory", "cc"
    );
    
    /* Multiple alternative constraints */
    int a = output, b = 100;
    asm volatile (
        "addl %1, %0"
        : "+r" (a)             /* Read-write register operand */
        : "rm" (b)             /* Register or memory */
        : "cc"
    );
    
    /* Fixed register constraint with immediate */
    register int x asm("ebx") = a;
    asm volatile (
        "imull %1, %0"
        : "+a" (x)             /* Must be in eax */
        : "rI" (37)            /* Constant or register */
        : "cc"
    );
    
    global_array[0] = x;
}

/* Test 2: Register-bound variables with conflicting requirements */
void test_register_conflicts(void) {
    /* Bind variables to specific registers */
    register int reg1 asm("esi");
    register int reg2 asm("edi");
    register int reg3 asm("ebx");
    
    reg1 = global_volatile_int;
    reg2 = 100;
    reg3 = 200;
    
    /* Force moves between these fixed registers */
    asm volatile (
        "movl %%esi, %%eax\n\t"
        "addl %%edi, %%eax\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%eax, %%esi"
        : 
        : 
        : "%eax", "%esi", "%edi", "%ebx", "cc"
    );
    
    /* Use the register variable in a context requiring a different register */
    int temp = reg1;
    asm volatile (
        "shrl $2, %0"
        : "+c" (temp)          /* Must be in ecx */
        : 
        : "cc"
    );
    
    global_volatile_int = temp;
}

/* Test 3: Complex addressing modes with pointer arithmetic */
void test_complex_addressing(void) {
    volatile int* ptr = &global_array[50];
    int index = global_volatile_int & 0xF;
    
    /* Memory operand with complex addressing */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "incl %%eax\n\t"
        "movl %%eax, (%1, %2, 4)"
        : 
        : "r" (ptr), "r" (index)
        : "%eax", "memory"
    );
    
    /* Force reloads with displacement */
    int offset = 16;
    asm volatile (
        "movl %c1(%2), %%eax\n\t"
        "addl %c1(%3), %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (global_array[10])
        : "i" (offset), "r" (ptr), "r" (&global_array[20])
        : "%eax", "memory"
    );
}

/* Test 4: Bitfield operations generating SUBREG accesses */
void test_bitfield_operations(void) {
    /* Access different parts of bitfields */
    bitfield_global.small = 0xAB;
    bitfield_global.medium = 0xABCD;
    bitfield_global.large = 0xABCDEF;
    
    /* Operations that may generate SUBREG */
    uint16_t medium_copy = bitfield_global.medium;
    uint8_t small_copy = bitfield_global.small;
    
    /* Use in arithmetic */
    asm volatile (
        "addw %1, %0"
        : "+r" (medium_copy)
        : "rm" (small_copy)
        : "cc"
    );
    
    /* STRICT_LOW_PART-like operation */
    int32_t full_word = 0x12345678;
    int16_t low_part = (int16_t)full_word;
    
    asm volatile (
        "addw $1, %0"
        : "+r" (low_part)
        : 
        : "cc"
    );
    
    bitfield_global.medium = medium_copy + low_part;
}

/* Test 5: Mixed volatile and inline assembly with memory clobbers */
void test_volatile_mix(void) {
    volatile int v1 = 100;
    volatile int v2 = 200;
    int normal1, normal2;
    
    /* Memory barrier to force reloads */
    asm volatile ("" : : : "memory");
    
    /* Volatile variables in assembly */
    asm volatile (
        "movl %2, %%eax\n\t"
        "imull %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %2, %%ebx\n\t"
        "addl %3, %%ebx\n\t"
        "movl %%ebx, %1"
        : "=m" (normal1), "=m" (normal2)
        : "m" (v1), "m" (v2)
        : "%eax", "%ebx", "memory", "cc"
    );
    
    /* Complex constraint with immediate */
    asm volatile (
        "leal (%1, %2, 2), %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (global_volatile_int)
        : "r" (normal1), "i" (8)
        : "%eax", "cc"
    );
}

/* Test 6: Nested inline assembly with output constraints */
void test_nested_constraints(void) {
    int a = 10, b = 20, c = 30, d = 40;
    int result1, result2;
    
    /* Multiple outputs with different constraints */
    asm volatile (
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %4, %%ebx\n\t"
        "subl %5, %%ebx\n\t"
        "movl %%ebx, %1"
        : "=r" (result1), "=r" (result2)
        : "rm" (a), "rm" (b), "rm" (c), "rm" (d)
        : "%eax", "%ebx", "cc"
    );
    
    /* Use results in another constrained operation */
    asm volatile (
        "xorl %%edx, %%edx\n\t"
        "movl %1, %%eax\n\t"
        "divl %2\n\t"
        "movl %%eax, %0"
        : "=a" (global_array[1])   /* Must be in eax */
        : "r" (result1), "r" (result2)
        : "%edx", "cc"
    );
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Run each test multiple times with different values */
    for (int i = 0; i < 3; i++) {
        test_restrictive_constraints();
        checksum += global_array[0];
        
        test_register_conflicts();
        checksum += global_volatile_int;
        
        test_complex_addressing();
        checksum += global_array[50];
        
        test_bitfield_operations();
        checksum += bitfield_global.medium;
        
        test_volatile_mix();
        checksum += global_volatile_int;
        
        test_nested_constraints();
        checksum += global_array[1];
        
        /* Modify globals to change patterns */
        global_volatile_int += i;
        volatile_ptr = &global_array[i * 10];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
