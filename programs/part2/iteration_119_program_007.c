/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to prevent optimizations */
volatile int global_volatile = 12345;
int global_normal = 67890;

/* Bitfield structure to generate SUBREG accesses */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
};

struct bitfield_struct bitfield_global = {0};

/* Function using register variables with explicit binding */
void test_register_binding() {
    /* Bind to specific registers that might conflict with constraints */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    volatile int temp = global_volatile;
    
    /* Force conflicts between register-bound variables and constraints */
    asm volatile (
        /* Output in ebx, input in memory - may need secondary reload */
        "movl %1, %%ebx\n\t"
        "addl $1, %%ebx\n\t"
        "movl %%ebx, %0"
        : "=m"(temp)
        : "m"(global_normal)
        : "ebx", "memory"
    );
    
    /* Multiple alternative constraints with register binding conflict */
    asm volatile (
        "imull %1, %0"
        : "+r"(r1), "+r"(r2)
        : "rm"(r3), "rm"(temp)
        : "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    global_volatile = r1 + r2;
}

/* Function using complex addressing modes and constraints */
void test_complex_constraints() {
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int result;
    
    /* Force memory operand with restrictive register class */
    asm volatile (
        /* 'a' constraint for eax register with memory input */
        "movl %1, %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%eax\n\t"
        "movl %%eax, %0"
        : "=m"(result)
        : "m"(array[5])
        : "eax", "memory"
    );
    
    /* Multiple constraints that might require secondary reloads */
    int x = global_normal;
    asm volatile (
        /* 'r' constraint for general register, but complex addressing */
        "addl %%ecx, %0"
        : "+r"(x)
        : "c"(result), "m"(array[2])
        : "cc"
    );
    
    /* Alternative constraints with immediate */
    asm volatile (
        "subl %1, %0"
        : "+rm"(x)
        : "ri"(100)
        : "cc"
    );
    
    global_normal = x;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns() {
    /* Operations that generate partial register accesses */
    int32_t full = global_volatile;
    int16_t half;
    
    /* Explicit truncation - may generate SUBREG */
    half = (int16_t)full;
    
    /* Use in inline asm with constraints */
    asm volatile (
        "movw %1, %0"
        : "=r"(half)
        : "r"(full)
        : "memory"
    );
    
    /* Bitfield operations */
    bitfield_global.low16 = half;
    bitfield_global.high16 = (uint16_t)(full >> 16);
    
    /* Volatile bitfield access */
    volatile uint8_t v = bitfield_global.volatile_field;
    bitfield_global.volatile_field = v + 1;
    
    /* Masking operation that might use STRICT_LOW_PART */
    int masked = full & 0xFFFF;
    asm volatile (
        "andl $0xFF, %0"
        : "+r"(masked)
        :
        : "cc"
    );
}

/* Function with mixed constraints and memory clobbers */
void test_mixed_operands() {
    volatile int v1, v2, v3;
    int r1, r2;
    
    /* Complex inline asm with multiple constraints */
    asm volatile (
        /* Output in specific register, inputs mixed */
        "movl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "movl %4, %1"
        : "=rm"(v1), "=r"(r1)
        : "g"(global_volatile), "i"(100), "m"(global_normal)
        : "eax", "memory", "cc"
    );
    
    /* Force spill/reload with memory clobber */
    asm volatile ("" : : : "memory");
    
    /* More complex constraints */
    asm volatile (
        "imull %1, %0"
        : "+r"(r1), "+r"(r2)
        : "rm"(v1), "rm"(v2)
        : "cc"
    );
    
    /* Use results */
    v3 = r1 + r2;
}

/* Main function orchestrating all tests */
int main() {
    int i;
    
    printf("Starting reload stress test...\n");
    
    /* Run tests multiple times to increase chances */
    for (i = 0; i < 10; i++) {
        test_register_binding();
        test_complex_constraints();
        test_subreg_patterns();
        test_mixed_operands();
        
        /* Modify globals to create data dependencies */
        global_volatile += i;
        global_normal ^= i;
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : : : "memory");
    }
    
    /* Final computation using all modified values */
    int result = global_volatile + global_normal + 
                 bitfield_global.low16 + bitfield_global.high16;
    
    printf("Test completed. Result: %d\n", result);
    
    /* Return non-zero to ensure code isn't eliminated */
    return result != 0 ? 0 : 1;
}
