/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 12345;
int global_normal = 67890;
static int static_global = 54321;

/* Bit-field structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
};

struct bitfield_struct bf = {0xAAAA, 0x5555, 0x77};

/* Test function 1: Force secondary reloads via restrictive register constraints */
__attribute__((noinline))
static int test_restrictive_constraints(void) {
    int result = 0;
    int input = global_volatile + 1;
    int output;
    
    /* Force use of specific register classes with memory operands */
    asm volatile (
        /* Try to force eax/rax register for output with memory input */
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(output)          /* Output in any register */
        : "m"(input)            /* Input from memory */
        : "%eax", "memory"
    );
    
    /* Multiple alternative constraints to confuse reload */
    int a = output;
    int b = global_normal;
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0"
        : "+rm"(a)              /* Register or memory */
        : "rm"(b), "i"(42)      /* Register/memory and immediate */
        : "cc"
    );
    
    result += a;
    
    /* Fixed register constraint with complex expression */
    register int x asm("ebx") = global_volatile;
    register int y asm("ecx") = static_global;
    
    asm volatile (
        "imull %%ecx, %%ebx\n\t"
        "addl %%ebx, %0"
        : "+r"(result)
        : "r"(x), "r"(y)
        : "%ebx", "%ecx", "cc"
    );
    
    return result;
}

/* Test function 2: SUBREG and partial register access patterns */
__attribute__((noinline))
static int test_subreg_patterns(void) {
    int result = 0;
    
    /* Operations that generate SUBREG RTL */
    int32_t full_word = global_normal * 2;
    
    /* Explicit truncation to 16-bit */
    int16_t half_word = (int16_t)full_word;
    result += half_word;  /* May generate SUBREG */
    
    /* Bit-field access */
    bf.low16 = result & 0xFFFF;
    bf.high16 = (result >> 16) & 0xFFFF;
    bf.volatile_field = result & 0xFF;
    
    /* Access bit-fields in ways that might need reloads */
    result += bf.low16;
    result -= bf.high16;
    result ^= bf.volatile_field;
    
    /* Mixed-size operations */
    int64_t big_val = (int64_t)full_word * (int64_t)half_word;
    result += (int)(big_val >> 16);  /* More SUBREG possibilities */
    
    return result;
}

/* Test function 3: Complex addressing modes with volatile */
__attribute__((noinline))
static int test_complex_addressing(void) {
    volatile int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * i + global_volatile;
    }
    
    int sum = 0;
    
    /* Force memory operands with specific register constraints */
    for (int i = 0; i < 10; i++) {
        int temp;
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%eax, %0"
            : "+r"(sum)
            : "m"(arr[i])
            : "%eax", "memory"
        );
    }
    
    /* Double volatile indirection */
    volatile int* volatile volatile_ptr = &arr[5];
    sum += *volatile_ptr;
    
    return sum;
}

/* Test function 4: Multiple reload candidates with register variables */
__attribute__((noinline))
static int test_register_variables(void) {
    /* Bind to specific registers */
    register int r1 asm("esi");
    register int r2 asm("edi");
    register int r3 asm("ebx");
    
    r1 = global_volatile;
    r2 = static_global;
    r3 = global_normal;
    
    int result = 0;
    
    /* Force conflicts between fixed registers */
    asm volatile (
        "movl %%esi, %%eax\n\t"
        "addl %%edi, %%eax\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm"(result)
        :
        : "%eax", "%esi", "%edi", "%ebx", "cc"
    );
    
    /* Now try to use the same variables differently */
    asm volatile (
        "xchgl %%ebx, %%esi\n\t"
        "subl %%edi, %%esi\n\t"
        "addl %%esi, %0"
        : "+r"(result)
        :
        : "%esi", "%edi", "%ebx", "cc"
    );
    
    return result;
}

/* Test function 5: Immediate values with restrictive constraints */
__attribute__((noinline))
static int test_immediate_constraints(void) {
    int result = 0;
    
    /* Large immediate that might need reloading */
    const int large_imm = 0x12345678;
    
    /* Try to force immediate into specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r"(result)
        : "i"(large_imm)
        : "%eax"
    );
    
    /* Multiple constraints for same operand */
    int a = 100, b = 200;
    asm volatile (
        "addl %2, %0\n\t"
        "subl %1, %0"
        : "+r,m"(result)
        : "r,i"(a), "r,i"(b)
        : "cc"
    );
    
    return result;
}

/* Main function that combines all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Run each test multiple times to increase reload opportunities */
    for (int i = 0; i < 3; i++) {
        total += test_restrictive_constraints();
        total += test_subreg_patterns();
        total += test_complex_addressing();
        total += test_register_variables();
        total += test_immediate_constraints();
        
        /* Modify globals to change data flow */
        global_volatile ^= total;
        static_global += i;
    }
    
    /* Final complex expression with inline asm */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=rm"(total)
        : "r"(total), "i"(42)
        : "%eax"
    );
    
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
