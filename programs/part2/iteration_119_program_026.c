/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload.cc
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 12345;
int global_normal = 67890;
static int static_global = 54321;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global = {0xAAAA, 0x5555, 0x77};

/* Test function 1: Force secondary reloads with fixed register constraints */
int test_fixed_registers(void) {
    int result = 0;
    
    /* Use register variables bound to specific hardware registers */
    register int r1 asm("ebx") = global_volatile;
    register int r2 asm("esi") = global_normal;
    
    /* Inline assembly with fixed output register constraints */
    asm volatile (
        /* Force eax constraint with memory input */
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (result)
        : "m" (global_volatile)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r,a" (result)
        : "rm,i" (r1)
        : "cc"
    );
    
    /* Conflict: r2 is bound to esi but we need it in edi */
    asm volatile (
        "movl %1, %%edi\n\t"
        "subl %%edi, %0\n\t"
        : "+r" (result)
        : "r" (r2)
        : "%edi", "cc"
    );
    
    return result;
}

/* Test function 2: Complex addressing modes and memory operands */
int test_complex_addressing(void) {
    int array[100];
    int sum = 0;
    
    /* Initialize array with volatile to prevent optimization */
    for (int i = 0; i < 100; i++) {
        array[i] = global_volatile + i;
    }
    
    /* Memory operand with displacement that might need secondary reload */
    asm volatile (
        "movl 16(%1), %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (sum)
        : "r" (&array[0])
        : "%eax"
    );
    
    /* Multiple memory references in one asm */
    asm volatile (
        "movl (%1), %%ecx\n\t"
        "movl 32(%2), %%edx\n\t"
        "addl %%ecx, %%edx\n\t"
        "movl %%edx, %0\n\t"
        : "=r" (sum)
        : "r" (&global_normal), "r" (array)
        : "%ecx", "%edx", "memory"
    );
    
    return sum;
}

/* Test function 3: Bitfield operations for SUBREG generation */
int test_bitfield_operations(void) {
    struct bitfield_struct local_bitfield = {0x1234, 0x5678, 0x9A};
    int result = 0;
    
    /* Access bitfields - these generate SUBREG in RTL */
    result = local_bitfield.low16;
    result += bitfield_global.high16;
    
    /* Volatile bitfield access */
    result += bitfield_global.volatile_field;
    
    /* Cast to different sizes to force partial register accesses */
    int16_t short_val = (int16_t)result;
    int8_t byte_val = (int8_t)short_val;
    
    /* Use these in inline assembly with register constraints */
    asm volatile (
        "movsx %1, %0\n\t"
        : "=r" (result)
        : "r" (byte_val)
    );
    
    return result;
}

/* Test function 4: Mixed constraints and immediate values */
int test_mixed_constraints(void) {
    int a = global_volatile;
    int b = static_global;
    int c = 0;
    
    /* Complex constraint with immediate and register alternatives */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        : "=r" (c)
        : "r" (a), "r,i" (b)
    );
    
    /* Multiple outputs with different constraints */
    int d, e;
    asm volatile (
        "movl %2, %0\n\t"
        "movl %3, %1\n\t"
        : "=a" (d), "=r" (e)
        : "m" (global_volatile), "r" (a)
        : "cc"
    );
    
    /* STRICT_LOW_PART-like operation through masking */
    e = (e & 0xFFFF) + d;
    
    return c + d + e;
}

/* Test function 5: Nested inline assembly and volatile propagation */
int test_volatile_propagation(void) {
    volatile int v1 = 100;
    volatile int v2 = 200;
    register int r3 asm("ecx") = 300;
    int result = 0;
    
    /* Memory barrier to force reloads */
    asm volatile ("" : : : "memory");
    
    /* Complex chain of operations */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "g" (v1), "r" (v2)
        : "%eax", "cc"
    );
    
    /* Use the register variable in conflicting context */
    asm volatile (
        "xchgl %0, %1\n\t"
        : "+r" (result), "+r" (r3)
    );
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
    
    return result + r3;
}

/* Main function that calls all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Call each test multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        total += test_fixed_registers();
        total += test_complex_addressing();
        total += test_bitfield_operations();
        total += test_mixed_constraints();
        total += test_volatile_propagation();
        
        /* Modify globals to create different code paths */
        global_volatile += i;
        static_global -= i;
        bitfield_global.volatile_field ^= i;
    }
    
    printf("Test completed. Result: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    return total % 256;
}
