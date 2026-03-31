/* reload_coverage.c - Test program to exercise GCC's reload pass */
#include <stdio.h>
#include <stdint.h>

/* Global variables to create complex addressing modes */
volatile int global_volatile = 42;
int global_array[100] = {0};
static int static_global = 100;

/* Bitfield structure to generate SUBREG operations */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int b : 10;
    unsigned int c : 15;
    unsigned int d : 2;
} bitfield = {1, 2, 3, 0};

/* Test 1: Force secondary reloads with fixed register constraints */
void test_fixed_registers(void) {
    /* Bind variables to specific registers */
    register int r1 asm("ebx");
    register int r2 asm("ecx");
    register int r3 asm("esi");
    
    int temp1 = global_volatile;
    int temp2 = static_global;
    
    /* Complex inline assembly with conflicting constraints */
    asm volatile (
        /* Force input from memory to fixed register */
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (r1)        /* Output in ebx */
        : "m" (temp1)      /* Input from memory */
        : "%eax", "cc"
    );
    
    /* Multiple alternative constraints */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (r2)        /* Must be in ecx */
        : "rm" (temp2)     /* Register or memory */
        : "cc"
    );
    
    /* Secondary output reload scenario */
    asm volatile (
        "movl %%eax, %0\n\t"
        : "=a" (r3)        /* Output must be in eax, then moved to esi */
        : "0" (global_volatile)  /* Input in same register */
        : "cc"
    );
    
    /* Use the results to prevent dead code elimination */
    global_array[0] = r1 + r2 + r3;
}

/* Test 2: Complex addressing modes with displacement */
void test_complex_addressing(void) {
    int index = global_volatile & 0xF;
    int offset = static_global;
    
    /* Force reload with complex memory addressing */
    asm volatile (
        "movl %c1(,%2,4), %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (global_array[index])
        : "i" (offset * sizeof(int)), "r" (index)
        : "%eax", "cc", "memory"
    );
    
    /* STRICT_LOW_PART pattern through bitfield operations */
    struct bitfield_struct local_bitfield = bitfield;
    local_bitfield.b = (local_bitfield.a + local_bitfield.c) & 0x3FF;
    bitfield = local_bitfield;
}

/* Test 3: Mixed register classes and immediate values */
void test_mixed_constraints(void) {
    register double d1 asm("xmm0");
    register double d2 asm("xmm1");
    volatile double vd = 3.14159;
    
    /* Force reload between different register classes */
    asm volatile (
        "movsd %1, %0\n\t"
        "addsd %2, %0\n\t"
        : "=x" (d1)        /* XMM register constraint */
        : "m" (vd), "x" (d2)
        : 
    );
    
    /* Integer to FP register move that might need secondary reload */
    int int_val = global_volatile;
    asm volatile (
        "cvtsi2sd %1, %0\n\t"
        : "=x" (d2)
        : "r" (int_val)    /* General purpose register */
        : 
    );
    
    /* Use results */
    global_array[1] = (int)(d1 + d2);
}

/* Test 4: SUBREG patterns through type punning */
void test_subreg_patterns(void) {
    int32_t full_word = 0x12345678;
    int16_t half_word;
    int8_t byte_val;
    
    /* These generate SUBREG accesses */
    half_word = (int16_t)full_word;
    byte_val = (int8_t)(full_word >> 16);
    
    /* Force reload of partial registers */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (half_word)
        : "r" (full_word)
        : 
    );
    
    /* Memory to partial register with constraint */
    asm volatile (
        "movb %1, %b0\n\t"
        : "+r" (full_word)
        : "m" (byte_val)
        : 
    );
    
    global_array[2] = full_word + half_word + byte_val;
}

/* Test 5: Multiple reloads in loop */
void test_loop_reloads(void) {
    volatile int accum = 0;
    register int i asm("edi");
    
    for (i = 0; i < 10; i++) {
        int temp = global_array[i];
        
        /* Complex constraint inside loop */
        asm volatile (
            "addl %%eax, %0\n\t"
            : "+rm" (accum)      /* Register or memory */
            : "a" (temp)         /* Must be in eax */
            : "cc"
        );
        
        /* Force spill/reload with memory clobber */
        asm volatile (
            ""
            : "+r" (temp)
            : 
            : "memory"
        );
    }
    
    global_array[3] = accum;
}

/* Test 6: Secondary reloads with pointer constraints */
void test_pointer_reloads(void) {
    int data = 0xABCD;
    int *ptr = &data;
    register int *rptr asm("r12");
    
    /* Force address reload */
    asm volatile (
        "movl (%1), %0\n\t"
        : "=r" (global_array[4])
        : "r" (ptr)
        : "memory"
    );
    
    rptr = ptr;
    
    /* Pointer in fixed register with offset */
    asm volatile (
        "movl 4(%1), %0\n\t"
        : "=r" (global_array[5])
        : "r" (rptr)
        : "memory"
    );
}

/* Main function that runs all tests */
int main(void) {
    int result = 0;
    
    printf("Starting reload coverage tests...\n");
    
    /* Run all tests multiple times to increase coverage chance */
    for (int i = 0; i < 3; i++) {
        test_fixed_registers();
        test_complex_addressing();
        test_mixed_constraints();
        test_subreg_patterns();
        test_loop_reloads();
        test_pointer_reloads();
    }
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < 10; i++) {
        result += global_array[i];
    }
    
    printf("Result: %d\n", result);
    return result;
}
