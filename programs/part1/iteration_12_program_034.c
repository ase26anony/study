#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
static int func_with_reg_vars(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables that conflict with normal allocation */
    register int x asm("r12") = a + global_seed;
    register int y asm("r13") = b * 2;
    register int z asm("r14") = c ^ d;
    
    int result;
    
    /* Inline assembly with mismatched constraints */
    /* Output is memory, inputs are registers - forces reloads */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        "xorl %3, %0"
        : "=m" (result)      /* Memory output */
        : "r" (x), "r" (y), "r" (z)  /* Register inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi"  /* Clobber many registers */
    );
    
    return result + e + f;
}

/* Function with complex addressing modes and volatile variables */
__attribute__((noinline))
static long func_with_complex_addressing(volatile char *ptr, int idx) {
    volatile long buffer[32];
    volatile short *sptr = (volatile short *)buffer;
    volatile int *iptr = (volatile int *)buffer;
    
    /* Complex address calculation that may need reloads */
    long addr = (long)(ptr + idx * 3);
    
    /* Mixed type operations causing mode changes */
    char c = *ptr;
    short s = (short)c * 256;
    int i = (int)s + idx;
    long l = (long)i * 1000;
    
    /* Inline assembly with memory constraints and immediates */
    /* Forces reloads due to constraint mismatches */
    asm volatile (
        "mov %1, %%rax\n\t"
        "add %2, %%rax\n\t"
        "mov %%rax, %0"
        : "=m" (buffer[idx & 31])  /* Memory output */
        : "r" (l), "i" (0x1234)    /* Register and immediate inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
    );
    
    /* More complex addressing with pointer arithmetic */
    sptr[(addr >> 2) & 15] = (short)l;
    iptr[(addr >> 3) & 7] = (int)l;
    
    return buffer[idx & 31] + (long)sptr[0] + (long)iptr[0];
}

/* Function with mixed data types and mode conversions */
__attribute__((noinline))
static int64_t func_with_mixed_types(volatile int8_t c, volatile int16_t s, 
                                     volatile int32_t i, volatile int64_t l) {
    /* Operations causing implicit mode changes */
    int64_t result = 0;
    volatile int32_t temp[8];
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int j = 0; j < 4; j++) {
        /* Mixed type computations requiring extensions/truncations */
        int8_t c1 = c + j;
        int16_t s1 = s * c1;
        int32_t i1 = i + s1;
        int64_t l1 = l - i1;
        
        /* Inline assembly with alternative constraints */
        /* "r,m" constraint may force reload decisions */
        asm volatile (
            "lea (%1,%2,1), %%rax\n\t"
            "add %3, %%rax"
            : "=r" (temp[j & 7])  /* Register output */
            : "r" (i1), "r" (j), "rm" (l1)  /* Mixed constraints */
            : "rax", "cc"
        );
        
        result += temp[j & 7];
    }
    
    /* More type mixing */
    result = (result << 8) | (c & 0xFF);
    result = (result << 16) | (s & 0xFFFF);
    
    return result;
}

/* Function using unions and bitfields for complex RTL patterns */
__attribute__((noinline))
static unsigned long func_with_unions(int a, int b) {
    union {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } bits;
        unsigned int full;
    } u1, u2;
    
    u1.full = a ^ 0xAAAA;
    u2.full = b ^ 0x5555;
    
    /* Bitfield operations that may generate complex RTL */
    unsigned long combined = ((unsigned long)u1.bits.high << 48) |
                            ((unsigned long)u1.bits.low << 32) |
                            ((unsigned long)u2.bits.high << 16) |
                            u2.bits.low;
    
    /* Inline assembly with multiple outputs */
    unsigned long out1, out2;
    asm volatile (
        "movq %2, %%rax\n\t"
        "rorq $32, %%rax\n\t"
        "movq %%rax, %0\n\t"
        "xorq %3, %%rax\n\t"
        "movq %%rax, %1"
        : "=rm" (out1), "=rm" (out2)  /* Alternative constraints */
        : "rm" (combined), "rm" (0xF0F0F0F0F0F0F0F0UL)
        : "rax", "cc"
    );
    
    return out1 ^ out2;
}

int main(int argc, char *argv[]) {
    /* Use arguments to prevent constant folding */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Many local variables of different types */
    volatile char c1 = 'A' + (global_seed & 0xF);
    volatile short s1 = 1000 + base;
    volatile int i1 = 50000 + base * 2;
    volatile long l1 = 1000000L + base * 1000L;
    volatile int8_t c2 = -128 + (global_seed & 0xFF);
    volatile int16_t s2 = -32768 + base;
    volatile int32_t i2 = 0x7FFFFFFF - base;
    volatile int64_t l2 = 0x7FFFFFFFFFFFFFFFLL - base * 100LL;
    
    /* Pointers with complex derivation */
    volatile char *ptr1 = (volatile char *)&c1;
    volatile int *ptr2 = (volatile int *)&i1;
    volatile long *ptr3 = (volatile long *)&l1;
    
    /* Array with volatile access */
    volatile int arr[64];
    for (volatile int i = 0; i < 64; i++) {
        arr[i] = i * i + base;
    }
    
    /* Call functions repeatedly to increase reload pressure */
    int sum = 0;
    long lsum = 0;
    int64_t llsum = 0;
    
    for (int iter = 0; iter < 10; iter++) {
        /* Mix function calls with different arguments */
        int r1 = func_with_reg_vars(
            arr[iter*2], arr[iter*2+1], 
            i1 + iter, i2 - iter, 
            s1, s2
        );
        
        long r2 = func_with_complex_addressing(
            ptr1 + iter, 
            (iter * 7) & 63
        );
        
        int64_t r3 = func_with_mixed_types(
            c1 + iter, s1 - iter, 
            i1 ^ iter, l1 + iter
        );
        
        unsigned long r4 = func_with_unions(
            arr[iter*3], 
            arr[iter*3+1]
        );
        
        /* Complex computation preventing dead code elimination */
        sum += r1 + (int)r2 + (int)r3 + (int)r4;
        lsum += r2 + (long)r3 + (long)r4;
        llsum += r3 + (int64_t)r4;
        
        /* Modify volatile variables to change constraints */
        c1 += 1;
        s1 -= 2;
        i1 ^= 0x1234;
        l1 += 1000;
        
        /* Pointer arithmetic creating new addressing modes */
        ptr1 = (volatile char *)((uintptr_t)ptr1 + 1);
        ptr2 = (volatile int *)((uintptr_t)ptr2 + 4);
        ptr3 = (volatile long *)((uintptr_t)ptr3 + 8);
    }
    
    /* Final checksum to ensure all code executes */
    unsigned long final_checksum = (unsigned long)sum + 
                                  (unsigned long)lsum + 
                                  (unsigned long)llsum;
    
    printf("Checksum: %lu\n", final_checksum);
    
    return (int)(final_checksum & 0x7FFFFFFF);
}
