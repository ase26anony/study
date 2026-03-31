/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void complex_addressing(int idx, volatile int* results) {
    volatile char buffer[256];
    volatile short shorts[128];
    volatile long longs[64];
    
    /* Complex address calculation that may need reloads */
    char* ptr1 = (char*)&buffer[idx % 256];
    short* ptr2 = &shorts[(idx * 7) % 128];
    long* ptr3 = &longs[(idx * 3) % 64];
    
    /* Inline asm with memory constraints and clobbers */
    asm volatile (
        "movb %1, %%al\n\t"
        "movw %2, %%bx\n\t"
        "addw %%bx, %%ax\n\t"
        "movl %3, %%ecx\n\t"
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0"
        : "=m" (*ptr3)          /* Output to memory */
        : "m" (*ptr1),          /* Input from memory */
          "m" (*ptr2),
          "i" (g_volatile_seed) /* Immediate input */
        : "rax", "rbx", "rcx", "memory"
    );
    
    results[0] = *ptr3;
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static void register_conflict(int a, int b, int* res) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    
    /* Inline asm with alternative constraints that may force reloads */
    asm volatile (
        "addl %[x], %[y]\n\t"
        "imull %[imm], %[y]\n\t"
        "movl %[y], %[res]"
        : [res] "=rm" (*res)    /* Output: register OR memory */
        : [x] "rm" (x),         /* Input: register OR memory */
          [y] "0" (y),          /* Same as output constraint */
          [imm] "i" (37)        /* Immediate */
        : "r12", "r13", "cc", "memory"
    );
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static void mixed_type_ops(volatile char c, volatile short s, 
                          volatile long l, int64_t* results) {
    volatile int temp;
    
    /* Operations causing mode changes */
    long l1 = (long)c;          /* char -> long (zero/sign extend) */
    long l2 = (long)s;          /* short -> long */
    int64_t ll1 = (int64_t)l;   /* long -> int64_t */
    
    /* Inline asm with mismatched operand sizes */
    asm volatile (
        "movsbl %[char], %%eax\n\t"     /* Sign extend byte */
        "movswl %[short], %%ebx\n\t"    /* Sign extend word */
        "addl %%ebx, %%eax\n\t"
        "movslq %%eax, %%rax\n\t"       /* Extend to 64-bit */
        "addq %[long64], %%rax\n\t"
        "movq %%rax, %[out]"
        : [out] "=rm" (*results)
        : [char] "rm" (c),
          [short] "rm" (s),
          [long64] "rm" (ll1)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    /* Additional operations to create more reload opportunities */
    for (volatile int i = 0; i < 5; i++) {
        asm volatile (
            "movl %[in], %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%eax\n\t"
            "addl %[idx], %%eax\n\t"
            "movl %%eax, %[out]"
            : [out] "=rm" (temp)
            : [in] "rm" (l1),
              [idx] "r" (i)
            : "rax", "cc"
        );
        results[i % 3] += temp;
    }
}

/* Function with pointer arithmetic and memory constraints */
__attribute__((noinline))
static void pointer_arithmetic(int offset, volatile int* base, int* result) {
    volatile int* ptr = base + offset;
    volatile int* ptr2 = ptr + g_volatile_seed;
    
    /* Multiple alternative constraints that may force reloads */
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl (%[ptr2]), %%eax\n\t"
        "movl %%eax, (%[dest])"
        : 
        : [ptr] "r" (ptr),      /* Register constraint */
          [ptr2] "rm" (ptr2),   /* Register OR memory */
          [dest] "r" (result)   /* Register constraint */
        : "rax", "memory", "cc"
    );
    
    /* Additional asm with immediate to memory */
    asm volatile (
        "movl $0x12345678, %[mem]"
        : [mem] "=m" (*ptr)
        :
        : "memory"
    );
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) {
    /* Many local variables of different types */
    volatile char c1 = argc + 1;
    volatile char c2 = argc + 2;
    volatile short s1 = argc * 3;
    volatile short s2 = argc * 4;
    volatile int i1 = argc * 10;
    volatile int i2 = argc * 20;
    volatile long l1 = (long)argc * 100;
    volatile long l2 = (long)argc * 200;
    volatile int* ptr1 = (int*)&i1;
    volatile int* ptr2 = (int*)&i2;
    
    int results[10] = {0};
    int64_t mixed_results[5] = {0};
    
    /* Call functions multiple times with different arguments */
    for (volatile int i = 0; i < 8; i++) {
        complex_addressing(i + argc, results);
        register_conflict(i1 + i, i2 - i, &results[i % 10]);
        mixed_type_ops(c1 + i, s1 + i, l1 + i, mixed_results);
        pointer_arithmetic(i * 2, ptr1, &results[(i + 2) % 10]);
        
        /* Additional operations to increase register pressure */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "movl %[d], %%edx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "addl %%edx, %%eax\n\t"
            "movl %%eax, %[res]"
            : [res] "=rm" (results[i % 10])
            : [a] "rm" (i1),
              [b] "rm" (i2),
              [c] "rm" (results[0]),
              [d] "i" (g_volatile_seed)
            : "rax", "rbx", "rcx", "rdx", "cc"
        );
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int j = 0; j < 10; j++) {
        checksum ^= results[j];
        checksum = (checksum << 3) | (checksum >> 29);
    }
    
    for (int j = 0; j < 5; j++) {
        checksum ^= (mixed_results[j] & 0xFFFFFFFF);
        checksum ^= (mixed_results[j] >> 32);
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
