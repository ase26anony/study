/* reload_stress.c - Stress test for GCC's reload pass */
#include <stdint.h>
#include <stdio.h>

/* Global variables to force memory operations */
volatile int global_volatile = 42;
int global_normal = 100;
volatile int16_t global_16bit = 32767;

/* Bitfield structure to generate SUBREG accesses */
struct bitfield_struct {
    unsigned int low16 : 16;
    unsigned int high16 : 16;
    volatile unsigned int volatile_field : 8;
} bitfield_global;

/* Test 1: Fixed register constraints with memory operands */
void test_fixed_registers(void) {
    int result;
    int input = global_volatile;
    
    /* Force eax register with memory input - may need secondary reload */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $1, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "m"(input)
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints with fixed register output */
    register int x asm("ebx") = result;
    asm volatile (
        "imull %1, %0"
        : "+a"(x)          /* eax only */
        : "rm"(global_normal)  /* register or memory */
        : "cc", "edx"
    );
    
    global_normal = x;
}

/* Test 2: Complex addressing modes with register binding */
void test_complex_addressing(void) {
    /* Bind variables to specific registers */
    register int a asm("esi");
    register int b asm("edi");
    register int c asm("ebx");
    
    a = global_volatile;
    b = global_normal;
    
    /* Force reloads by using conflicting constraints */
    asm volatile (
        "leal (%1, %2, 2), %0"
        : "=r"(c)
        : "r"(a), "r"(b)
        : /* empty clobber */
    );
    
    /* Memory operand with specific register class */
    int temp;
    asm volatile (
        "movl %%ebx, %0\n\t"
        "shrl $4, %0"
        : "=a"(temp)      /* Must be in eax */
        : /* no inputs */
        : "cc"
    );
    
    /* Use the result in another asm with different constraint */
    asm volatile (
        "addl %%eax, %0"
        : "+m"(global_volatile)
        : /* no explicit inputs, eax is implicit */
        : "cc"
    );
}

/* Test 3: SUBREG and partial register accesses */
void test_subreg_patterns(void) {
    /* Bitfield access generates SUBREG */
    bitfield_global.low16 = (uint16_t)global_volatile;
    bitfield_global.high16 = (uint16_t)(global_volatile >> 16);
    
    /* Explicit truncation */
    int32_t large = 0x12345678;
    int16_t truncated;
    
    /* This should generate SUBREG */
    truncated = (int16_t)large;
    
    /* Use truncated value in inline asm */
    asm volatile (
        "movw %w1, %0"    /* %w1 means 16-bit version of register */
        : "=m"(global_16bit)
        : "r"(truncated)
    );
    
    /* STRICT_LOW_PART pattern through masking */
    int masked = global_normal & 0xFFFF;
    asm volatile (
        "andl $0xFFFF, %0"
        : "+r"(masked)
        : /* no inputs */
        : "cc"
    );
    
    global_normal = masked;
}

/* Test 4: Secondary reloads via restrictive constraints */
void test_secondary_reloads(void) {
    double dbl_input = 3.14159;
    double dbl_output;
    
    /* x87 floating point constraints - often need secondary reloads */
    asm volatile (
        "fldl %1\n\t"
        "fstpl %0"
        : "=m"(dbl_output)
        : "m"(dbl_input)
        : "st", "st(1)"
    );
    
    /* MMX/SSE constraints mixed with general registers */
    int mmx_var;
    asm volatile (
        "movd %1, %%mm0\n\t"
        "movd %%mm0, %0"
        : "=r"(mmx_var)
        : "m"(global_normal)
        : "mm0"
    );
    
    /* Multiple alternative constraints that conflict */
    int a = 10, b = 20, c;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0"
        : "=&a"(c)        /* Early clobber + eax constraint */
        : "rm"(a), "rm"(b)
        : "cc"
    );
}

/* Test 5: Volatile and optimization barriers */
void test_volatile_barriers(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3;
    register int r1 asm("ecx");
    register int r2 asm("edx");
    
    /* Memory barrier to force reloads */
    asm volatile ("" : : : "memory");
    
    /* Complex sequence with volatile operands */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "imull %3, %0"
        : "=c"(r1)
        : "m"(v1), "m"(v2), "m"(v3)
        : "cc", "eax", "edx"
    );
    
    /* Use the result with different constraint */
    asm volatile (
        "movl %%ecx, %%edx\n\t"
        "shll $2, %%edx"
        : "=d"(r2)
        : /* ecx is already set */
        : "cc"
    );
    
    /* Final store with memory clobber */
    asm volatile (
        "movl %%edx, %0"
        : "=m"(global_volatile)
        : /* edx is already set */
        : "memory"
    );
}

/* Test 6: Loop with accumulating reloads */
void test_loop_reloads(int iterations) {
    register int accum asm("ebx") = 0;
    volatile int counter = iterations;
    
    while (counter > 0) {
        int temp;
        
        /* Mix of constraints in loop body */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %0"
            : "=r"(temp)
            : "m"(global_normal)
            : "%eax", "cc"
        );
        
        accum = temp;
        
        /* Force memory store each iteration */
        asm volatile (
            "movl %%ebx, %0"
            : "=m"(global_volatile)
            : /* ebx is already set */
            : /* empty */
        );
        
        counter--;
    }
    
    global_normal = accum;
}

int main(void) {
    int total = 0;
    
    printf("Starting reload stress tests...\n");
    
    /* Execute all tests multiple times */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        total += global_normal;
        
        test_complex_addressing();
        total += global_volatile;
        
        test_subreg_patterns();
        total += bitfield_global.low16;
        
        test_secondary_reloads();
        total += global_normal % 100;
        
        test_volatile_barriers();
        total += global_volatile;
        
        test_loop_reloads(5);
        total += global_normal;
    }
    
    printf("Total (checksum): %d\n", total);
    return total & 0xFF;  /* Return non-zero to prevent dead code elimination */
}
