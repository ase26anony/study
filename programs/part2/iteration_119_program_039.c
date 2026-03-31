/* reload_test.c - Test program to trigger secondary reload initialization in GCC's reload pass */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile_int = 42;
int global_int = 100;
static int static_global = 200;

/* Bitfield structure to generate SUBREG RTL patterns */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
} bitfield = {12345, 54321};

/* Function using register variables with explicit binding */
void test_register_variables() {
    /* Bind to specific registers that might conflict with constraints */
    register int r1 asm("ebx");
    register int r2 asm("esi");
    register int r3 asm("edi");
    
    volatile int temp = global_volatile_int;
    
    /* Force reloads by using register variables in conflicting contexts */
    r1 = temp + 1;
    r2 = r1 * 2;
    r3 = r2 - temp;
    
    /* Inline asm with fixed register output and memory input */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0"
        : "+r" (r1)        /* Output in r1 (ebx) */
        : "m" (global_int) /* Memory operand */
        : "%eax", "cc"     /* Clobber eax and flags */
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0"
        : "+r,a" (r2)      /* Output must be in register or specifically eax */
        : "rm,g" (r3)      /* Input can be register/memory or general */
        : "cc"
    );
    
    global_int = r1 + r2 + r3;
}

/* Function using complex addressing modes and constraints */
void test_complex_addressing() {
    int array[4] = {10, 20, 30, 40};
    volatile int* volatile_ptr = array;
    
    /* Force memory operand with specific register class constraint */
    asm volatile (
        "movl (%1), %%ecx\n\t"
        "addl %%ecx, %0"
        : "+a" (array[0])      /* Output must be in eax */
        : "r" (volatile_ptr)   /* Input pointer in any register */
        : "%ecx", "memory", "cc"
    );
    
    /* Immediate value with register constraint */
    int result;
    asm volatile (
        "movl $0x12345678, %%edx\n\t"
        "addl %%edx, %1\n\t"
        "movl %1, %0"
        : "=r" (result)        /* Output in any register */
        : "a" (array[1])       /* Input must be in eax */
        : "%edx", "cc"
    );
    
    /* STRICT_LOW_PART pattern via bitfield */
    struct bitfield_struct local_bitfield;
    local_bitfield.low16 = result & 0xFFFF;
    local_bitfield.high16 = (result >> 16) & 0xFFFF;
    
    /* This may generate SUBREG accesses */
    int combined = (local_bitfield.high16 << 16) | local_bitfield.low16;
    
    /* Complex constraint with memory clobber */
    asm volatile (
        ""
        : "+r" (combined)
        : "r" (local_bitfield.low16), "r" (local_bitfield.high16)
        : "memory"
    );
}

/* Function to force secondary reloads via restrictive constraints */
void test_secondary_reloads() {
    double d1 = 3.14159;
    double d2 = 2.71828;
    double d3;
    
    /* Floating point with fixed register constraints (x87/x86-64 specific) */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "faddp\n\t"
        "fstpl %0"
        : "=m" (d3)
        : "m" (d1), "m" (d2)
        : "st", "st(1)"
    );
    
    /* Integer with multiple alternative constraints */
    int a = 1000, b = 2000, c;
    asm volatile (
        "movl %1, %%eax\n\t"
        "subl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r,m" (c)           /* Output: register or memory */
        : "r,i" (a), "r,i" (b) /* Inputs: register or immediate */
        : "%eax", "cc"
    );
    
    /* Force spill/reload with volatile */
    volatile int v = c;
    asm volatile (
        "addl $1, %0"
        : "+m" (v)
        :
        : "cc"
    );
    
    /* Use the result to prevent dead code elimination */
    global_volatile_int += v;
}

/* Function with mixed types and conversions */
void test_type_conversions() {
    int64_t big = 0x123456789ABCDEF0LL;
    int32_t small1, small2;
    
    /* Truncation that may generate SUBREG */
    small1 = (int32_t)big;
    small2 = (int32_t)(big >> 32);
    
    /* Inline asm with partial register access */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (global_int)
        : "r" (small1), "r" (small2)
        : "%eax", "cc"
    );
    
    /* Byte operations that might need special handling */
    uint8_t bytes[8];
    for (int i = 0; i < 8; i++) {
        bytes[i] = ((uint8_t*)&big)[i];
    }
    
    /* Force reloads with byte operations */
    uint8_t sum = 0;
    for (int i = 0; i < 8; i++) {
        asm volatile (
            "addb %1, %0"
            : "+r" (sum)
            : "rm" (bytes[i])
            : "cc"
        );
    }
    
    static_global += sum;
}

/* Main function that orchestrates all tests */
int main() {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Run each test multiple times to increase chances */
    for (int i = 0; i < 3; i++) {
        test_register_variables();
        test_complex_addressing();
        test_secondary_reloads();
        test_type_conversions();
        
        /* Mix in some direct operations */
        total += global_int;
        total += global_volatile_int;
        total += static_global;
        total += bitfield.low16;
        total += bitfield.high16;
        
        /* Create loop-carried dependencies */
        global_int = (global_int * 13 + 7) & 0xFFFF;
        global_volatile_int ^= total;
    }
    
    printf("Final total: %d\n", total);
    
    /* Return something based on all operations to prevent optimization */
    return (total > 0) ? 0 : 1;
}
