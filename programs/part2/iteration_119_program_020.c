/* reload_stress_test.c
 * Designed to stress GCC's reload pass and trigger initialization
 * of secondary reload fields in push_reload().
 */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile_int = 42;
int global_array[100] = {0};
volatile int16_t global_volatile_short = 100;

/* Bitfield structure to generate SUBREG RTL */
struct bitfield_struct {
    unsigned int low_bits : 8;
    unsigned int high_bits : 24;
    volatile unsigned int volatile_field : 16;
};

struct bitfield_struct bitfield_global = {0};

/* Function using register variables with explicit binding */
int test_register_binding(void) {
    /* Bind to specific registers - these may conflict with constraints */
    register int reg_eax asm("eax");
    register int reg_ebx asm("ebx");
    register int reg_ecx asm("ecx");
    
    int result = 0;
    volatile int mem_var = 1234;
    
    /* Force reloads by using register variables in conflicting constraints */
    asm volatile (
        /* Move from memory to specific register - may need secondary reload */
        "movl %1, %0\n\t"
        : "=a"(reg_eax)        /* Output must be in eax */
        : "m"(mem_var)         /* Input from memory */
        : /* No clobbers */
    );
    
    /* Multiple alternative constraints with register pressure */
    asm volatile (
        "addl %1, %0\n\t"
        : "+r"(reg_ebx)        /* Read-write in any register */
        : "rm"(global_volatile_int)  /* Register or memory */
        : "cc"
    );
    
    /* Complex constraint with immediate and memory */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(reg_ecx)
        : "rmi"(global_volatile_int)  /* Register, memory, or immediate */
        : "cc"
    );
    
    /* Use all register variables to prevent optimization */
    result = reg_eax + reg_ebx + reg_ecx;
    
    return result;
}

/* Function using complex addressing modes */
int test_complex_addressing(void) {
    int result = 0;
    volatile int* volatile_ptr = &global_volatile_int;
    
    /* Force memory operand with displacement */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(result)
        : "m"(global_array[50])  /* Memory with index */
        : /* No clobbers */
    );
    
    /* Multiple memory operands with different constraints */
    int temp1, temp2;
    asm volatile (
        "movl (%1), %0\n\t"
        "addl (%2), %0\n\t"
        : "=&r"(temp1)          /* Early clobber to force separate reg */
        : "r"(&global_array[10]), "r"(&global_array[20])
        : "memory"
    );
    
    /* Volatile pointer dereference in asm */
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r"(temp2)
        : "r"(volatile_ptr)
        : "memory"
    );
    
    result += temp1 + temp2;
    
    return result;
}

/* Function to generate SUBREG/STRICT_LOW_PART patterns */
int test_subreg_patterns(void) {
    int32_t full_int = 0x12345678;
    int16_t half_int;
    int8_t byte_val;
    
    /* Truncation that may generate SUBREG */
    half_int = (int16_t)full_int;
    
    /* Bitfield access - likely generates SUBREG */
    bitfield_global.low_bits = 0xAB;
    bitfield_global.high_bits = 0xCDEF01;
    bitfield_global.volatile_field = half_int;
    
    /* Use bitfield in asm with constraint */
    unsigned int extracted;
    asm volatile (
        "movzbl %1, %0\n\t"      /* Zero extend byte */
        : "=r"(extracted)
        : "m"(bitfield_global.low_bits)
        : /* No clobbers */
    );
    
    /* Masking operation for STRICT_LOW_PART */
    byte_val = full_int & 0xFF;
    
    /* Inline asm that might use strict_low_part */
    asm volatile (
        "andb $0xF0, %b0\n\t"    /* Modify low byte of register */
        : "+r"(byte_val)
        : /* No inputs */
        : "cc"
    );
    
    return extracted + byte_val + bitfield_global.volatile_field;
}

/* Function with multiple alternative constraints */
int test_multiple_alternatives(void) {
    int a = 100, b = 200, c = 300;
    int result1, result2;
    
    /* Asm with multiple alternative constraints */
    asm volatile (
        "movl %1, %0\n\t"
        : "=r,a,r"(result1)      /* Multiple alternatives */
        : "g,m,i"(a)             /* General, memory, or immediate */
        : /* No clobbers */
    );
    
    /* Complex asm with input/output in same register but different constraints */
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        : "=r"(result2)
        : "r"(b), "r"(c)
        : /* No clobbers */
    );
    
    /* Force spill/reload with many operands */
    asm volatile (
        "imull %2, %1\n\t"
        "addl %1, %0\n\t"
        : "+r"(result1), "+&r"(result2)
        : "rm"(global_volatile_int)
        : "cc"
    );
    
    return result1 + result2;
}

/* Function using specific x86 register constraints */
int test_x86_specific_constraints(void) {
    int input = 999;
    int output;
    
    /* Force use of specific registers that might need secondary reloads */
    
    /* 'a' constraint for eax */
    asm volatile (
        "xchgl %%eax, %1\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(output)
        : "a"(input)            /* Input in eax */
        : "%eax"
    );
    
    /* 'b' constraint for ebx */
    register int ebx_var asm("ebx") = 555;
    asm volatile (
        "addl %%ebx, %0\n\t"
        : "+r"(output)
        : "b"(ebx_var)          /* Input in ebx */
        : "cc"
    );
    
    /* 'c' constraint for ecx with memory operand */
    volatile int count = 10;
    asm volatile (
        "movl %1, %%ecx\n\t"
        "1:\n\t"
        "decl %%ecx\n\t"
        "jnz 1b\n\t"
        : /* No outputs */
        : "c"(count)            /* Input in ecx */
        : "cc"
    );
    
    return output;
}

/* Main function that calls all tests */
int main(void) {
    int total = 0;
    
    printf("Starting reload stress test...\n");
    
    /* Call each test multiple times with different values */
    for (int i = 0; i < 10; i++) {
        global_volatile_int = i * 100;
        global_array[i] = i * 50;
        
        total += test_register_binding();
        total += test_complex_addressing();
        total += test_subreg_patterns();
        total += test_multiple_alternatives();
        total += test_x86_specific_constraints();
        
        /* Create register pressure with many local variables */
        {
            int a = total, b = total * 2, c = total * 3;
            int d = total / 2, e = total / 3, f = total / 4;
            
            /* Use all variables in volatile asm to prevent optimization */
            asm volatile (
                "addl %1, %0\n\t"
                "addl %2, %0\n\t"
                "addl %3, %0\n\t"
                "addl %4, %0\n\t"
                "addl %5, %0\n\t"
                : "+r"(total)
                : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e)
                : "cc"
            );
        }
    }
    
    printf("Final result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
