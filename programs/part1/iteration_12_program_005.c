#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 12345;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    register int z asm("r14") = 0;
    
    /* Inline assembly with mismatched constraints */
    /* Output is memory, inputs are registers - may require reload */
    volatile int mem_output;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=m" (mem_output)      /* Memory output */
        : "r" (x), "r" (y)       /* Register inputs */
        : "memory", "rax", "rbx", "rcx", "rdx"
    );
    
    /* Another asm with immediate input and register output */
    asm volatile (
        "movl $0x%0, %1\n\t"
        "leal (%1, %2), %0\n\t"
        : "=r" (z), "=r" (x)
        : "i" (100), "r" (y)
        : "cc"
    );
    
    return mem_output + z + x;
}

/* Function with complex memory addressing */
__attribute__((noinline))
static long complex_addressing(volatile char *base, int idx) {
    volatile long result = 0;
    volatile int offset = idx * 7 + 3;
    
    /* Take address of volatile and use in assembly */
    volatile char *ptr1 = base + offset;
    volatile short *ptr2 = (volatile short *)(base + idx * 2);
    
    /* Assembly with memory output and immediate input */
    asm volatile (
        "movb $42, (%0)\n\t"
        "movw $0x1234, (%1)\n\t"
        : 
        : "r" (ptr1), "r" (ptr2)
        : "memory", "rax", "rbx"
    );
    
    /* Complex address calculation that may need reload */
    asm volatile (
        "movzbq (%0, %1), %2\n\t"
        "addq %3, %2\n\t"
        : "=r" (result)
        : "r" (base), "r" (offset), "i" (0x1000)
        : "memory"
    );
    
    return result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static uint64_t mixed_type_operations(volatile char c, volatile short s, 
                                      volatile int i, volatile long l) {
    /* Operations that cause mode changes */
    uint64_t extended;
    
    /* char -> 64-bit with zero extension */
    extended = (uint64_t)c;
    
    /* Inline asm with mismatched operand sizes */
    asm volatile (
        "movzbl %b1, %0\n\t"      /* byte -> long */
        "addw %w2, %w0\n\t"       /* add word */
        "addl %k3, %k0\n\t"       /* add dword */
        "addq %4, %0\n\t"         /* add qword */
        : "=r" (extended)
        : "r" (c), "r" (s), "r" (i), "r" (l)
        : "cc"
    );
    
    /* More complex with memory constraints */
    volatile uint64_t mem_temp;
    asm volatile (
        "movq %1, %0\n\t"
        "rorq $32, %0\n\t"
        : "=m" (mem_temp)
        : "r" (extended)
        : "memory", "rax", "rdx"
    );
    
    return extended + mem_temp;
}

/* Function that creates many local variables for register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    volatile char c1 = 1, c2 = 2, c3 = 3;
    volatile short s1 = 100, s2 = 200, s3 = 300;
    volatile int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000, i5 = 5000;
    volatile long l1 = 10000, l2 = 20000, l3 = 30000;
    volatile int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    
    int result = 0;
    volatile int counter = iterations;
    
    /* Loop with volatile counter prevents optimization */
    while (counter-- > 0) {
        /* Mixed operations causing potential reloads */
        result += c1 + c2 - c3;
        result += s1 * s2 / (s3 + 1);
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (i1)
            : "r" (i2), "r" (i3)
            : "eax", "ebx", "ecx", "edx", "cc"
        );
        
        /* Pointer arithmetic with volatile */
        p1 = p2 + (c1 & 0x3);
        p2 = p3 - (c2 & 0x1);
        
        /* Memory access with complex addressing */
        result += *(p1 + (counter & 0x3));
        result += *(p2 - (counter & 0x1));
        
        /* Mode mixing operation */
        l1 = (long)c1 * (long)s1 * (long)i1;
        result += (int)(l1 & 0xFFFF);
    }
    
    return result;
}

/* Main function that orchestrates everything */
int main(int argc, char *argv[]) {
    int checksum = 0;
    
    /* Initialize with volatile to prevent constant folding */
    volatile int init_val = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Create array with volatile elements */
    volatile char buffer[256];
    for (int i = 0; i < 256; i++) {
        buffer[i] = (char)(i + init_val);
    }
    
    /* Call functions to trigger various reload scenarios */
    
    /* 1. Explicit register variables with constraints */
    checksum += use_explicit_registers(init_val, init_val * 2);
    
    /* 2. Complex addressing modes */
    checksum += complex_addressing(buffer, init_val & 0xFF);
    
    /* 3. Mixed type operations */
    checksum += mixed_type_operations(
        buffer[0], 
        *(volatile short *)&buffer[10],
        init_val,
        (long)init_val * 100
    );
    
    /* 4. High register pressure */
    checksum += high_register_pressure(init_val & 0x3F);
    
    /* Additional stress: multiple calls with different args */
    for (volatile int i = 0; i < 5; i++) {
        checksum += use_explicit_registers(i, i * 3);
        checksum += complex_addressing(buffer, i * 17);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
