/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;
volatile void *g_volatile_ptr = NULL;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int use_complex_address(volatile int *base, int idx1, int idx2) {
    volatile int temp = 0;
    int result;
    
    /* Complex addressing that may need base+index reloads */
    asm volatile (
        "movl (%[base], %[idx1], 4), %%eax\n\t"
        "addl %[idx2], %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=r" (result)
        : [base] "r" (base), [idx1] "r" (idx1), [idx2] "i" (idx2)
        : "eax", "memory", "cc"
    );
    
    /* Force memory operand with mismatched constraint */
    asm volatile (
        "movl %[val], (%[addr])\n\t"
        : 
        : [val] "ri" (idx1 + idx2), [addr] "r" (&temp)
        : "memory"
    );
    
    return result + temp;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static long mix_types_and_registers(char c, short s, int i, long l) {
    register long reg1 asm("r12") = l;
    register int reg2 asm("r13") = i;
    register short reg3 asm("r14") = s;
    register char reg4 asm("r15") = c;
    
    long result;
    
    /* Inline asm with multiple alternative constraints that may force reloads */
    asm volatile (
        "add %[c], %[s]\n\t"
        "movswl %w[s], %k[i]\n\t"
        "add %k[i], %k[reg2]\n\t"
        "movslq %k[reg2], %[result]\n\t"
        "add %[reg1], %[result]\n\t"
        : [result] "=&r" (result), [s] "+&r" (reg3), [i] "+&r" (reg2)
        : [c] "ri" (c), [reg1] "r" (reg1), [reg4] "r" (reg4)
        : "cc"
    );
    
    /* Clobber many registers to force spills/reloads */
    asm volatile (
        ""
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "memory"
    );
    
    return result;
}

/* Function with memory output operands and immediate inputs */
__attribute__((noinline))
static void memory_ops_with_immediates(volatile int *arr, int size) {
    volatile int local_arr[10];
    volatile int *ptr = arr;
    
    for (volatile int i = 0; i < size && i < 10; i = i + 1) {
        /* Memory output with immediate input - may need reload */
        asm volatile (
            "movl %[imm], (%[mem])\n\t"
            :
            : [imm] "i" (i * 17 + 3), [mem] "r" (&local_arr[i])
            : "memory"
        );
        
        /* Multiple constraints that could mismatch */
        asm volatile (
            "addl %%eax, (%[dest])\n\t"
            :
            : "a" (g_volatile_seed), [dest] "rm" (ptr + i)
            : "memory", "cc"
        );
    }
    
    /* Pointer arithmetic that creates complex addresses */
    int offset = g_volatile_seed & 7;
    asm volatile (
        "movl (%[base], %[offset], 4), %%eax\n\t"
        "addl $0x12345678, %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=rm" (local_arr[0])
        : [base] "r" (arr), [offset] "r" (offset)
        : "eax", "memory", "cc"
    );
}

/* Function with mixed operations causing mode changes */
__attribute__((noinline))
static uint64_t mixed_mode_operations(void) {
    volatile char c = g_volatile_seed;
    volatile short s = g_volatile_seed * 2;
    volatile int i = g_volatile_seed * 3;
    volatile long l = g_volatile_seed * 5;
    
    uint64_t result = 0;
    
    /* Operations that change modes - may require zero/sign extend */
    result = (uint64_t)c;           /* char to 64-bit */
    result += (uint64_t)(s << 3);   /* short shifted then extended */
    result += (uint64_t)(i * i);    /* int multiplication */
    result += l;                    /* long addition */
    
    /* Use bit-fields and unions for additional mode complexity */
    union {
        struct {
            unsigned short a : 4;
            unsigned short b : 12;
        } bits;
        unsigned short word;
    } u;
    
    u.word = s;
    result += u.bits.a * 100 + u.bits.b;
    
    /* Cast between pointer and integer types */
    volatile int *ptr = &i;
    uintptr_t int_ptr = (uintptr_t)ptr;
    result ^= int_ptr;
    
    return result;
}

/* Main function that creates maximum register pressure */
int main(int argc, char *argv[]) {
    /* Initialize with volatile to prevent optimization */
    volatile int base_seed = g_volatile_seed;
    if (argc > 1) {
        base_seed = atoi(argv[1]);
    }
    
    /* Many local variables of different types */
    char c1 = base_seed & 0xFF;
    char c2 = (base_seed >> 8) & 0xFF;
    short s1 = base_seed * 3;
    short s2 = base_seed * 7;
    int i1 = base_seed * 11;
    int i2 = base_seed * 13;
    long l1 = base_seed * 17;
    long l2 = base_seed * 19;
    
    volatile int arr[20];
    for (int j = 0; j < 20; j++) {
        arr[j] = base_seed + j * 5;
    }
    
    uint64_t checksum = 0;
    
    /* Call functions repeatedly with different arguments */
    for (volatile int iter = 0; iter < 5; iter++) {
        /* Force many different operations that may need reloads */
        checksum += mix_types_and_registers(c1 + iter, s1, i1, l1);
        checksum += mix_types_and_registers(c2 - iter, s2, i2, l2);
        
        memory_ops_with_immediates(arr, 10 + iter);
        
        checksum += use_complex_address(arr, iter, iter * 2);
        checksum += use_complex_address(arr + 5, iter * 3, iter * 4);
        
        checksum ^= mixed_mode_operations();
        
        /* Modify variables to prevent loop invariant removal */
        c1 += iter;
        s1 -= iter;
        i1 ^= iter;
        l1 += checksum & 0xFFFF;
    }
    
    /* Additional stress with inline assembly blocks */
    register int r1 asm("rbx") = i1;
    register int r2 asm("rsi") = i2;
    
    asm volatile (
        "movl %[r1], %%eax\n\t"
        "imull %[r2], %%eax\n\t"
        "addl %%eax, %[sum]\n\t"
        "movl $0x1234, %%ebx\n\t"
        "addl %%ebx, %[sum]\n\t"
        : [sum] "+r" (i1)
        : [r1] "rm" (r1), [r2] "rm" (r2)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    checksum += i1 + i2 + l1 + l2 + s1 + s2 + c1 + c2;
    
    /* Use all variables in final output to prevent dead code elimination */
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
