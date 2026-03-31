/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int* create_complex_address(volatile int* base, int offset) {
    return (int*)((char*)base + offset * sizeof(int) * 2);
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + global_seed;
    register int y asm("r13") = b - global_seed;
    register int z asm("r14") = 0;
    
    /* Inline asm with mismatched constraints to force reloads */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "movl %[y], %[z]\n\t"
        : [z] "=r" (z)          /* Output in register */
        : [x] "rm" (x),         /* Input can be register or memory (creates alternatives) */
          [y] "0" (y)           /* Input in same register as output 0 */
        : "cc", "r12", "r13", "r14"  /* Clobber explicit registers */
    );
    
    /* More complex asm with memory output and register input */
    volatile int mem_result;
    asm volatile (
        "movl %[input], %%eax\n\t"
        "leal (%%eax, %%eax, 2), %%eax\n\t"
        "movl %%eax, %[output]\n\t"
        : [output] "=m" (mem_result)  /* Memory output */
        : [input] "r" (z)             /* Register input */
        : "eax", "memory"             /* Clobber eax and memory */
    );
    
    return z + mem_result;
}

/* Function with volatile addresses and complex constraints */
__attribute__((noinline))
static void use_volatile_addresses(volatile int* arr, int size) {
    volatile int temp;
    volatile int* volatile_ptr = &temp;
    
    /* Take address of volatile and use in asm */
    for (volatile int i = 0; i < size; i = i + 1) {
        int idx = i + global_seed;
        
        /* Complex addressing mode that may need reloads */
        asm volatile (
            "movl %[idx], %%ecx\n\t"
            "movl %[arr], %%ebx\n\t"
            "movl (%%ebx, %%ecx, 4), %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %[out]\n\t"
            : [out] "=m" (arr[idx % size])  /* Memory output with complex addressing */
            : [arr] "r" (arr),              /* Base register */
              [idx] "rm" (idx)              /* Index can be reg or mem */
            : "eax", "ebx", "ecx", "memory", "cc"
        );
    }
    
    /* Multiple output operands with different constraints */
    int out1, out2;
    asm volatile (
        "movl $100, %0\n\t"
        "movl $200, %1\n\t"
        : "=r" (out1), "=m" (out2)  /* Mixed register and memory outputs */
        :
        : "memory"
    );
    
    *volatile_ptr = out1 + out2;
}

/* Function mixing data types to create mode changes */
__attribute__((noinline))
static long mix_data_types(char c, short s, int i, long l) {
    /* Operations that change machine modes */
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Mix types in calculations - creates subreg/zero_extend in RTL */
    long result = 0;
    
    /* char in 64-bit operation */
    result += (long)vc * 256L;
    
    /* short in 32-bit operation with extension */
    result += (long)(vi + (int)vs);
    
    /* Pointer arithmetic with different types */
    volatile char* char_ptr = (volatile char*)&vi;
    result += (long)(char_ptr[1] << 8);
    
    /* Union to force type punning */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    u.i = vi;
    result += u.s[0] * 100L + u.c[1];
    
    /* Bit-field operations */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
    } bits = {vc & 0xF, (unsigned int)vs & 0xFF, (unsigned int)vi & 0xFFF};
    
    result += bits.a + bits.b * 10L + bits.c * 100L;
    
    return result + vl;
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3, c4 = 4;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    long l1 = 1000, l2 = 2000, l3 = 3000, l4 = 4000;
    volatile int vi1 = 500, vi2 = 600;
    
    /* Take addresses to force spill/reload */
    int* p1 = &i1;
    int* p2 = &i2;
    int* p3 = &i3;
    int* p4 = &i4;
    
    /* Complex loop with mixed operations */
    volatile int counter = 0;
    long total = 0;
    
    for (counter = 0; counter < iterations; counter = counter + 1) {
        /* Mixed-type calculations */
        i1 = (int)c1 + (int)s1 + vi1;
        i2 = (int)c2 * (int)s2 - vi2;
        i3 = i1 ^ i2;
        i4 = i1 | i2;
        
        /* Pointer arithmetic */
        total += *p1 + *p2 + *p3 + *p4;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "movl %[i1], %%eax\n\t"
            "addl %[i2], %%eax\n\t"
            "movl %%eax, %[i3]\n\t"
            : [i3] "=rm" (i3)      /* Output can be reg or mem */
            : [i1] "rm" (i1),      /* Input can be reg or mem */
              [i2] "rm" (i2)       /* Input can be reg or mem */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc"
        );
        
        /* Rotate variables */
        char tc = c1;
        c1 = c2; c2 = c3; c3 = c4; c4 = tc;
        s1 += s2; s2 += s3; s3 += s4; s4 = s1;
    }
    
    return (int)(total % 1000000);
}

int main(int argc, char* argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    global_seed = seed;
    
    /* Array with volatile elements for complex addressing */
    volatile int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * seed;
    }
    
    /* Call functions to trigger various reload scenarios */
    int result1 = use_explicit_registers(seed, seed * 2);
    
    use_volatile_addresses((volatile int*)array, 100);
    
    long result2 = mix_data_types(
        (char)(seed & 0xFF),
        (short)(seed & 0xFFFF),
        seed,
        (long)seed * 1000L
    );
    
    int result3 = high_register_pressure(10 + (seed % 5));
    
    /* Compute checksum to prevent elimination */
    long checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum += array[i];
    }
    checksum += result1 + result2 + result3;
    
    /* Use checksum to prevent dead code elimination */
    if (checksum != 0) {
        printf("Result: %ld (seed=%d)\n", checksum, seed);
    }
    
    return (int)(checksum & 0x7FFFFFFF);
}
