/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 42;
int global_normal = 100;
static int static_global = 200;

/* Bitfield structure to generate SUBREG accesses */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
};

struct bitfield_struct bf = {0};

/* Function using inline assembly with restrictive constraints */
void test_restrictive_constraints(void) {
    int input = global_volatile;
    int output;
    
    /* Force secondary reload: memory -> specific register */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (output)          /* Output in any register */
        : "m" (input)            /* Input must be in memory */
        : "%eax", "memory"
    );
    
    /* Alternative constraints to confuse reload */
    int a = output + 1;
    int b = global_normal;
    
    asm volatile (
        "addl %1, %0"
        : "+r" (a)               /* Read-write register operand */
        : "rm" (b)               /* Register or memory */
        : "cc"
    );
    
    global_normal = a;
}

/* Function using register variables bound to specific registers */
void test_register_variables(void) {
    /* Bind to specific registers that might conflict */
    register int r1 asm("ebx") = global_volatile;
    register int r2 asm("ecx") = static_global;
    
    /* Force move between specific registers */
    int result;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=r" (result)
        : "r" (r1), "r" (r2)
        : "cc"
    );
    
    /* Use result in another asm with different constraints */
    int final;
    asm volatile (
        "imull %1, %0"
        : "=a" (final)           /* Output must be in eax */
        : "r" (result)           /* Input in any register */
        : "cc"
    );
    
    static_global = final;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
void test_subreg_patterns(void) {
    /* Operations that generate partial register accesses */
    int32_t full = global_normal * 2;
    
    /* Explicit truncation - may generate SUBREG */
    int16_t half = (int16_t)full;
    volatile int16_t volatile_half = half;
    
    /* Bitfield operations */
    bf.low16 = (uint16_t)full;
    bf.high16 = (uint16_t)(full >> 16);
    bf.volatile_field = (uint8_t)(full & 0xFF);
    
    /* Complex expression with partial results */
    int32_t combined = (bf.high16 << 16) | bf.low16;
    combined += bf.volatile_field;
    
    /* Inline asm with byte operations */
    uint8_t byte_val;
    asm volatile (
        "movb %1, %b0"           %b0 accesses lower byte
        : "=q" (byte_val)        /* Must be in a, b, c, or d register */
        : "r" (combined)
        : "cc"
    );
    
    global_volatile = byte_val;
}

/* Function with multiple alternative constraints */
void test_multiple_alternatives(void) {
    int x = static_global;
    int y = global_volatile;
    int z;
    
    /* Multiple alternative constraints for input */
    asm volatile (
        "subl %1, %0"
        : "=r" (z)
        : "r,i,m" (x), "0" (y)   /* Three alternatives for x */
        : "cc"
    );
    
    /* Another with output alternatives */
    int w;
    asm volatile (
        "movl %1, %0"
        : "=r,m" (w)             /* Output can be register or memory */
        : "r,m" (z)              /* Input can be register or memory */
        : "cc"
    );
    
    /* Force spill/reload with volatile */
    asm volatile ("" : "+m" (w));
    
    static_global = w;
}

/* Function using floating point to trigger different register classes */
void test_mixed_reg_classes(void) {
    double d = 3.14159;
    int i = global_normal;
    double result;
    
    /* Mix integer and floating point in constraints */
    asm volatile (
        "cvtsi2sd %1, %0"
        : "=x" (result)          /* Must be in SSE register */
        : "r" (i)                /* Integer in general register */
        : "cc"
    );
    
    /* Memory barrier to prevent optimization */
    asm volatile ("" ::: "memory");
    
    /* Use result in integer context */
    int int_result = (int)result;
    global_volatile = int_result;
}

/* Main function that calls all tests */
int main(void) {
    int i, sum = 0;
    
    /* Execute tests multiple times to increase coverage chance */
    for (i = 0; i < 10; i++) {
        test_restrictive_constraints();
        test_register_variables();
        test_subreg_patterns();
        test_multiple_alternatives();
        test_mixed_reg_classes();
        
        /* Accumulate results to prevent dead code elimination */
        sum += global_normal + static_global + global_volatile;
        
        /* Modify globals to create data dependencies */
        global_volatile ^= sum;
        static_global += i;
    }
    
    /* Use sum to prevent optimization */
    asm volatile ("" : "+r" (sum));
    
    printf("Result: %d\n", sum);
    return sum & 0xFF;  /* Return non-zero to indicate execution */
}
