/* reload_stress_test.c
 * Designed to trigger secondary reload initialization in GCC's reload pass
 * Compile with: gcc -O1 -fno-omit-frame-pointer -g -c reload_stress_test.c
 * Or for more stress: gcc -O2 -finline-small-functions -fno-schedule-insns -fno-schedule-insns2 -c reload_stress_test.c
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex data flows */
volatile int global_volatile = 1234;
int global_normal = 5678;
static int static_global = 9012;

/* Bit-field structure to generate SUBREG RTL */
struct bitfield_struct {
    int full : 32;
    int part : 16;
    int small : 8;
} bitfield = {0};

/* Function using register variables with explicit binding */
int test_register_vars(void) {
    /* Bind to specific registers - will conflict with constraints */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("edx");
    
    int result = 0;
    
    /* Force reloads by using register variables in conflicting contexts */
    r1 = global_volatile;
    r2 = global_normal;
    
    /* Inline asm with fixed register output, memory input */
    /* This often requires secondary reloads */
    asm volatile (
        "movl %1, %0\n\t"
        : "=a"(result)        /* Fixed to eax */
        : "m"(r1)             /* Memory operand from ebx-bound variable */
        : "cc"
    );
    
    /* Another asm with multiple alternatives and conflicts */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(r3)            /* edx - but might need reload */
        : "rm"(r2)            /* ecx-bound or memory */
        : "cc"
    );
    
    return result + r3;
}

/* Function using complex addressing modes and constraints */
int test_complex_constraints(int x, int y) {
    int result;
    volatile int temp;
    
    /* Force memory operand with restrictive register class */
    /* May require secondary reload to get from memory to specific register */
    asm volatile (
        "imull %1, %0\n\t"
        : "=a"(result)        /* Must be in eax */
        : "rm"(x),            /* Register or memory */
          "0"(y)              /* Same as output 0 (eax) */
        : "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "movl %%eax, %0\n\t"
        "movl %1, %%eax\n\t"
        : "=rm"(temp)         /* Output to memory or register */
        : "g"(global_volatile) /* General operand - could be anything */
        : "eax", "cc"
    );
    
    /* STRICT_LOW_PART pattern via bit-field */
    bitfield.part = (int16_t)result;  /* Truncation to 16-bit */
    
    return bitfield.part + temp;
}

/* Function with SUBREG patterns via type punning */
int test_subreg_patterns(void) {
    int32_t full = 0x12345678;
    int16_t half;
    int8_t quarter;
    
    /* These generate SUBREG accesses */
    half = (int16_t)full;                     /* Truncation */
    quarter = (int8_t)(full >> 16);           /* Different part */
    
    /* Use in inline asm with constraints */
    int result;
    asm volatile (
        "movswl %1, %0\n\t"    /* Sign extend word to long */
        : "=r"(result)
        : "m"(half)            /* Memory operand from 16-bit value */
        : "cc"
    );
    
    /* Another with byte access */
    asm volatile (
        "movsbl %1, %0\n\t"
        : "=a"(result)         /* Fixed register constraint */
        : "m"(quarter)
        : "cc"
    );
    
    return result + full;
}

/* Function mixing everything with loops to create complex flow */
int test_mixed_reloads(int iterations) {
    register int accum asm("esi") = 0;
    volatile int barrier;
    
    for (int i = 0; i < iterations; i++) {
        int temp;
        
        /* Memory barrier to prevent optimization */
        asm volatile ("" : "=m"(barrier) : : "memory");
        
        /* Complex asm with multiple constraints */
        asm volatile (
            "leal (%1, %2, 2), %0\n\t"
            : "=r"(temp)
            : "r"(i), "rm"(accum)  /* accum is esi-bound, causing potential conflict */
            : "cc"
        );
        
        /* Fixed register constraint with memory operand */
        asm volatile (
            "xchgl %%eax, %0\n\t"
            : "=m"(global_volatile), "+a"(temp)
            :
            : "cc"
        );
        
        accum += temp + bitfield.small;
        
        /* Access bit-field to generate SUBREG */
        bitfield.small = (accum & 0xFF);
    }
    
    return accum;
}

/* Main function that calls all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Test 1: Register variables with explicit binding */
    total += test_register_vars();
    
    /* Test 2: Complex constraints and addressing */
    total += test_complex_constraints(100, 200);
    
    /* Test 3: SUBREG patterns */
    total += test_subreg_patterns();
    
    /* Test 4: Mixed patterns with loops */
    total += test_mixed_reloads(10);
    
    /* Additional inline asm to force more reloads */
    {
        int a = 42, b = 99, c;
        
        /* Multiple alternative constraints with immediate */
        asm volatile (
            "cmpl %1, %2\n\t"
            "setg %0\n\t"
            : "=q"(c)          /* Byte-addressable register (a, b, c, d) */
            : "r"(a), "rm"(b)  /* Register or memory */
            : "cc"
        );
        
        total += c;
        
        /* Memory operand with specific register class */
        asm volatile (
            "movl %1, %%eax\n\t"
            "shrl $2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=m"(global_normal)
            : "m"(total)
            : "eax", "cc"
        );
    }
    
    printf("Test complete. Result: %d\n", total);
    
    /* Return something based on all operations to prevent dead code elimination */
    return (total > 0) ? 0 : 1;
}
