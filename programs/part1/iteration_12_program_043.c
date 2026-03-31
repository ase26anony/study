/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile int* base) {
    /* Force non-constant address calculation */
    volatile int offset = g_volatile_seed;
    int* ptr = (int*)base + idx + offset;
    
    /* Inline asm with memory constraint and clobbered registers */
    int result;
    asm volatile (
        "movl (%1), %0\n\t"
        "addl $0x7F, %0\n\t"
        : "=r" (result)
        : "r" (ptr)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    
    return result;
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static int register_conflict(int a, int b) {
    /* Explicit register variables that may conflict */
    register int x asm("r12") = a;
    register int y asm("r13") = b;
    register int z asm("r14");
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %1, %0\n\t"
        "movl %0, %2\n\t"
        : "+r" (x), "=r" (y)
        : "m" (z), "0" (x), "1" (y)
        : "cc", "r15"
    );
    
    /* Force mode change with different sized operations */
    char c = (char)x;
    short s = (short)y;
    long l = (long)c + (long)s;
    
    return (int)l;
}

/* Function with mixed types and mode conversions */
__attribute__((noinline))
static long mixed_type_ops(volatile char* chars, volatile short* shorts) {
    long total = 0;
    volatile int i;
    
    /* Loop with volatile counter to prevent optimization */
    for (i = 0; i < 8; i++) {
        /* Mixed type operations requiring mode changes */
        char c = chars[i];
        short s = shorts[i];
        
        /* Operations that may require zero/sign extension */
        long temp = (long)c * (long)s;
        
        /* Inline asm with immediate constraints and memory output */
        asm volatile (
            "imulq %1, %0\n\t"
            "addq %0, %2\n\t"
            : "+r" (temp)
            : "i" (0x1001), "m" (total)
            : "rax", "rdx", "cc"
        );
        
        total += temp;
    }
    
    return total;
}

/* Function using pointer arithmetic and complex constraints */
__attribute__((noinline))
static void pointer_arithmetic(volatile int* arr, int size) {
    volatile int* volatile_ptr = arr;
    
    for (volatile int i = 0; i < size; i++) {
        /* Complex address calculation */
        int* ptr = (int*)volatile_ptr + i + g_volatile_seed;
        
        /* Inline asm with alternative constraints */
        int val = i * 0xABCD;
        asm volatile (
            "movl %1, (%0)\n\t"
            : 
            : "r" (ptr), "ri" (val)
            : "memory", "rax", "rbx"
        );
    }
}

/* Function with bitfields and unions causing subreg operations */
__attribute__((noinline))
static int bitfield_union_ops(void) {
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bits;
        uint32_t full;
    } data;
    
    volatile int* vptr = &g_volatile_seed;
    data.full = *vptr;
    
    /* Operations on bitfields requiring extraction/insertion */
    data.bits.a = (data.bits.b + data.bits.c) & 0xF;
    data.bits.d = data.bits.a | data.bits.b;
    
    /* Inline asm using the union with register constraints */
    uint32_t result;
    asm volatile (
        "movl %1, %0\n\t"
        "rorl $8, %0\n\t"
        : "=r" (result)
        : "m" (data.full)
        : "cc"
    );
    
    return (int)result;
}

/* Main function creating maximum register pressure */
int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    
    /* Many local variables of different types */
    char char_array[16];
    short short_array[16];
    int int_array[32];
    long long_array[16];
    volatile int volatile_vars[8];
    
    /* Initialize arrays with non-constant values */
    for (volatile int i = 0; i < 16; i++) {
        char_array[i] = (char)(seed + i);
        short_array[i] = (short)(seed * i);
        int_array[i] = seed ^ (i * 0x1234);
        long_array[i] = (long)seed << (i & 0xF);
        if (i < 8) volatile_vars[i] = seed + i * 0x100;
    }
    
    /* Call functions to create various reload scenarios */
    int sum = 0;
    
    /* 1. Complex addressing with memory constraints */
    sum += complex_addressing(seed & 7, volatile_vars);
    
    /* 2. Register conflict scenarios */
    sum += register_conflict(seed, seed * 2);
    
    /* 3. Mixed type operations */
    sum += (int)mixed_type_ops(char_array, short_array);
    
    /* 4. Pointer arithmetic with complex constraints */
    pointer_arithmetic(int_array, 8);
    
    /* 5. Bitfield and union operations */
    sum += bitfield_union_ops();
    
    /* Additional inline asm blocks to clobber registers */
    asm volatile (
        "movq %%rax, %%rbx\n\t"
        "movq %%rcx, %%rdx\n\t"
        "movq %%rsi, %%rdi\n\t"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "cc"
    );
    
    /* Compute checksum from all modified data */
    long checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += char_array[i];
        checksum += short_array[i];
        checksum += int_array[i];
        checksum += long_array[i];
    }
    for (int i = 0; i < 8; i++) {
        checksum += volatile_vars[i];
    }
    checksum += sum;
    
    /* Prevent dead code elimination */
    volatile long final_result = checksum;
    printf("Result: %ld\n", (long)final_result);
    
    return (int)(checksum & 0x7FFFFFFF);
}
