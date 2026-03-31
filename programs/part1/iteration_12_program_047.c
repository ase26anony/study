/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_explicit_registers(int arg1, int arg2) {
    /* Explicit register variables with potential conflicts */
    register int x asm("r12") = arg1 + g_volatile_seed;
    register int y asm("r13") = arg2 - g_volatile_seed;
    register int z asm("r14");
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[input], %[output]\n\t"
        "subq %[imm], %[output]"
        : [output] "=r,m" (z)          /* Output: register OR memory */
        : [input] "r,m" (x),           /* Input: register OR memory */
          [imm] "i" (5)                /* Immediate constraint */
        : "r12", "r13", "r14", "cc"   /* Clobber explicit registers */
    );
    
    /* Use the result to prevent elimination */
    g_volatile_seed += z;
}

/* Function with memory constraints and complex addressing */
__attribute__((noinline))
static void memory_constraints_ops(volatile int* ptr, long offset) {
    int temp1, temp2;
    volatile int local_volatile = *ptr;
    
    /* Take address of volatile with offset */
    int* addr = (int*)((char*)ptr + offset);
    
    /* Assembly with memory output and register input */
    asm volatile (
        "movl %[in], %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (*addr)           /* Memory output constraint */
        : [in] "r,i" (local_volatile)  /* Register OR immediate input */
        : "rax", "cc", "memory"
    );
    
    /* Another asm with multiple alternatives */
    asm volatile (
        "testl %1, %1\n\t"
        "cmovnel %1, %0"
        : "+r,m" (temp1)
        : "r,m,i" (temp2)
        : "cc"
    );
}

/* Function with mixed types causing mode changes */
__attribute__((noinline))
static void mixed_type_operations(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations causing implicit mode changes */
    long result = (long)vc + (vs << 8) + (vi * 256L) + vl;
    
    /* Use bit-fields to create subreg operations */
    struct {
        unsigned int low : 8;
        unsigned int high : 24;
    } bits;
    
    bits.low = vc;
    bits.high = vi & 0xFFFFFF;
    
    /* Union for type punning */
    union {
        uint32_t u32;
        uint16_t u16[2];
        uint8_t u8[4];
    } converter;
    
    converter.u32 = vi;
    vs = converter.u16[0] + converter.u8[1];
    
    /* Loop with volatile counter causing reloads */
    volatile int counter = 10;
    while (counter-- > 0) {
        /* Mixed operations in loop */
        result += (vc * counter) - (vs / (counter + 1));
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            ""
            : 
            : "r" (result)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13",
              "r14", "r15", "cc", "memory"
        );
    }
    
    g_volatile_seed ^= (int)result;
}

/* Function with pointer arithmetic and complex addressing */
__attribute__((noinline))
static void complex_addressing(int* base, volatile int index) {
    /* Array with volatile index prevents optimization */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * g_volatile_seed;
    }
    
    /* Complex address calculation */
    int* ptr1 = &array[index % 100];
    int* ptr2 = &array[(index + g_volatile_seed) % 100];
    
    /* Assembly with base+index addressing */
    asm volatile (
        "movl (%[base], %[index], 4), %%eax\n\t"
        "addl %%eax, %[sum]"
        : [sum] "+r,m" (*ptr1)
        : [base] "r" (array),
          [index] "r" (index % 100)
        : "rax", "cc"
    );
    
    /* More complex addressing with scale */
    asm volatile (
        "imull $3, %[idx], %%ecx\n\t"
        "movl (%[arr], %%rcx), %%edx\n\t"
        "subl %%edx, %[diff]"
        : [diff] "+r,m" (*ptr2)
        : [arr] "r" (array),
          [idx] "r" ((index + 1) % 100)
        : "rcx", "rdx", "cc"
    );
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) {
    /* Many local variables of different types */
    char c1 = argc > 1 ? argv[1][0] : 'A';
    short s1 = argc * 100;
    int i1 = argc + g_volatile_seed;
    long l1 = (long)argc << 32;
    
    volatile int vi1 = i1;
    volatile int vi2 = i1 * 2;
    volatile int vi3 = i1 / 2;
    
    int* ptr1 = &vi1;
    int* ptr2 = &vi2;
    int* ptr3 = &vi3;
    
    /* Call functions repeatedly with different args */
    for (int i = 0; i < 10; i++) {
        use_explicit_registers(i1 + i, i1 - i);
        memory_constraints_ops(ptr1, i * sizeof(int));
        mixed_type_operations(c1 + i, s1 + i, i1 + i, l1 + i);
        complex_addressing(ptr2, vi3 + i);
        
        /* Additional pressure with many live variables */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %2"
            : "=m" (vi1)
            : "r,i" (vi2), "m" (vi3)
            : "rax", "cc"
        );
    }
    
    /* Compute checksum to prevent elimination */
    unsigned long checksum = 0;
    checksum += (unsigned long)c1;
    checksum += (unsigned long)s1;
    checksum += (unsigned long)i1;
    checksum += l1;
    checksum += (unsigned long)vi1;
    checksum += (unsigned long)vi2;
    checksum += (unsigned long)vi3;
    checksum += (unsigned long)g_volatile_seed;
    
    /* Use all variables one more time */
    asm volatile (
        "xorq %%rax, %%rax\n\t"
        "add %[c1], %%al\n\t"
        "add %[s1], %%ax\n\t"
        "add %[i1], %%eax\n\t"
        "add %[l1], %%rax"
        : 
        : [c1] "r" ((int)c1),
          [s1] "r" ((int)s1),
          [i1] "r" (i1),
          [l1] "r" (l1)
        : "rax", "cc"
    );
    
    printf("Checksum: %lu\n", checksum);
    printf("Final seed: %d\n", g_volatile_seed);
    
    return (int)(checksum % 256);
}
