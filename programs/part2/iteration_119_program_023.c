/* reload_stress_test.c
 * Designed to trigger GCC's reload pass initialization logic
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g reload_stress_test.c -o reload_test
 * Also try: gcc -O2 -finline-small-functions -fno-schedule-insns reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 42;
int global_normal = 100;
static int static_global = 200;

/* Bit-field structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int partial : 16;
    int small : 8;
} bitfield = {0};

/* Test function 1: Complex inline assembly with restrictive constraints */
void test_secondary_reloads(void) {
    int input, output;
    int temp1, temp2;
    
    /* Force secondary reloads with fixed register constraints */
    register int reg_eax asm("eax");
    register int reg_ebx asm("ebx");
    register int reg_ecx asm("ecx");
    
    /* Use volatile to prevent optimization */
    volatile int mem_var = global_volatile;
    
    /* Complex inline assembly with multiple alternative constraints */
    asm volatile (
        /* Move from memory to fixed register - may need secondary reload */
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "=r" (output)
        : "m" (mem_var), "0" (global_normal)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with conflicting requirements */
    asm volatile (
        "imull %1, %0"
        : "+r,a" (reg_eax)
        : "rm,i" (global_normal)
        : "cc"
    );
    
    /* Force reload for output in specific register with memory input */
    asm volatile (
        "movl %1, %%ebx\n\t"
        "leal (%%ebx, %2), %%ecx"
        : "=c" (reg_ecx)
        : "m" (global_volatile), "r" (reg_eax)
        : "%ebx"
    );
}

/* Test function 2: Bit-field operations for SUBREG generation */
void test_subreg_reloads(void) {
    /* Operations on bit-fields generate SUBREG RTL */
    bitfield.partial = (int16_t)global_volatile;
    bitfield.small = (int8_t)(global_normal + bitfield.partial);
    
    /* Complex expression with bit-field */
    int result = bitfield.full + (bitfield.partial << 3);
    
    /* Inline assembly using bit-field values */
    asm volatile (
        "addw %1, %0"
        : "+r" (bitfield.partial)
        : "rm" (result)
        : "cc"
    );
}

/* Test function 3: Register variables with explicit binding conflicts */
void test_register_conflicts(void) {
    /* Declare register variables bound to specific registers */
    register int fixed_reg1 asm("edi");
    register int fixed_reg2 asm("esi");
    
    /* Initialize with complex expressions */
    fixed_reg1 = global_volatile * 2;
    fixed_reg2 = static_global + 50;
    
    /* Create conflict: use both in an operation requiring different registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        "movl %0, %%ebx"
        : "+r" (fixed_reg1)
        : "r" (fixed_reg2)
        : "%eax", "%ebx", "cc"
    );
    
    /* More conflicts with memory operands */
    volatile int stack_var = fixed_reg1;
    asm volatile (
        "movl %1, %%edx\n\t"
        "subl %%edx, %0"
        : "+r" (fixed_reg2)
        : "m" (stack_var)
        : "%edx", "cc"
    );
}

/* Test function 4: Complex addressing modes and immediate values */
void test_complex_addressing(void) {
    int array[10] = {0};
    volatile int index = 3;
    
    /* Force complex addressing mode reloads */
    asm volatile (
        "movl (%1, %2, 4), %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (global_normal)
        : "r" (array), "r" (index)
        : "%eax", "cc", "memory"
    );
    
    /* Immediate value with restrictive output constraint */
    asm volatile (
        "movl $0x12345678, %0"
        : "=a" (array[0])
        :
        : "cc"
    );
    
    /* Multiple memory constraints */
    asm volatile (
        "movl %1, %0"
        : "=rm" (array[5])
        : "rm" (array[0])
        : "cc"
    );
}

/* Test function 5: Mixed types and truncation operations */
void test_mixed_types(void) {
    int32_t int32_val = global_volatile;
    int16_t int16_val;
    int8_t int8_val;
    
    /* Truncation operations that may need reloads */
    int16_val = (int16_t)int32_val;
    int8_val = (int8_t)(int16_val + 100);
    
    /* Use truncated values in operations requiring full registers */
    asm volatile (
        "movsx %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (global_normal)
        : "r" (int16_val)
        : "%eax", "cc"
    );
    
    /* More complex type mixing */
    uint64_t big_val = (uint64_t)global_normal * 1000;
    uint32_t part = (uint32_t)(big_val >> 16);
    
    asm volatile (
        "addl %1, %0"
        : "+r" (part)
        : "rm" (int32_val)
        : "cc"
    );
}

/* Main function that orchestrates all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Execute all test functions multiple times */
    for (int i = 0; i < 3; i++) {
        test_secondary_reloads();
        test_subreg_reloads();
        test_register_conflicts();
        test_complex_addressing();
        test_mixed_types();
        
        /* Accumulate results to prevent dead code elimination */
        total += global_normal + static_global + bitfield.full;
    }
    
    printf("Tests completed. Result: %d\n", total);
    
    /* Final complex inline assembly to ensure reload opportunities */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm" (total)
        : "rm" (global_volatile), "rm" (static_global)
        : "%eax", "%ebx", "cc"
    );
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
