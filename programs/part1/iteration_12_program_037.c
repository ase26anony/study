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
    volatile int offset = g_volatile_seed & 0xF;
    int* ptr = (int*)base + offset + idx;
    
    /* Inline asm with memory constraint and clobbered registers */
    int result;
    asm volatile (
        "movl (%[ptr]), %[res]\n\t"
        "addl $0x7F, %[res]\n\t"
        : [res] "=r" (result)
        : [ptr] "r" (ptr)
        : "memory", "cc"
    );
    
    return result;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static void register_conflict(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    
    /* Inline asm with multiple alternative constraints */
    int sum;
    asm volatile (
        "addl %[x], %[y]\n\t"
        "movl %[y], %[sum]\n\t"
        : [sum] "=r,m" (sum)
        : [x] "r,m" (x), [y] "r,m" (y)
        : "cc"
    );
    
    /* Use the result to prevent elimination */
    g_volatile_seed ^= sum;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long long mixed_type_ops(char c, short s, int i, long long ll) {
    /* Operations that cause mode changes */
    char c2 = c + (g_volatile_seed & 0xFF);
    short s2 = s * c2;
    int i2 = i + s2;
    long long ll2 = ll + i2;
    
    /* Bitfield operations */
    struct {
        unsigned int a : 5;
        unsigned int b : 11;
        unsigned int c : 16;
    } bits = {c2, s2 & 0x7FF, i2 & 0xFFFF};
    
    /* Union for type punning */
    union {
        int ival;
        float fval;
        void* pval;
    } u;
    
    u.ival = i2;
    ll2 += (long long)u.pval;
    
    /* Inline asm with mismatched operand sizes */
    long long result;
    asm volatile (
        "movsbl %[c2], %%eax\n\t"
        "movswl %[s2], %%ebx\n\t"
        "addl %%eax, %%ebx\n\t"
        "movslq %%ebx, %%rax\n\t"
        "addq %[ll2], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "=r" (result)
        : [c2] "r" (c2), [s2] "r" (s2), [ll2] "r" (ll2)
        : "rax", "rbx", "cc"
    );
    
    return result + bits.a + bits.b + bits.c;
}

/* Function with memory output constraints and immediate inputs */
__attribute__((noinline))
static void memory_constraints(volatile int* mem, int val) {
    /* Multiple asm statements with memory outputs */
    asm volatile (
        "movl %[val], (%[mem])\n\t"
        : 
        : [mem] "r" (mem), [val] "i" (0x12345678)
        : "memory"
    );
    
    /* Complex addressing with index */
    volatile int* mem2 = mem + (g_volatile_seed & 3);
    asm volatile (
        "addl $1, (%[mem2])\n\t"
        : 
        : [mem2] "r" (mem2)
        : "memory", "cc"
    );
}

/* Function that creates many local variables to increase register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3, c4 = 4;
    short s1 = 100, s2 = 200, s3 = 300, s4 = 400;
    int i1 = 1000, i2 = 2000, i3 = 3000, i4 = 4000;
    long long ll1 = 10000, ll2 = 20000, ll3 = 30000, ll4 = 40000;
    void* p1 = &c1, *p2 = &s1, *p3 = &i1, *p4 = &ll1;
    
    volatile int counter = iterations;
    int sum = 0;
    
    /* Loop with volatile counter to prevent optimization */
    while (counter-- > 0) {
        /* Mixed operations causing mode conversions */
        i1 = c1 + s1;
        i2 = c2 * s2;
        i3 = c3 - s3;
        i4 = c4 / (s4 ? s4 : 1);
        
        /* Pointer arithmetic */
        p1 = (char*)p1 + 1;
        p2 = (short*)p2 + 1;
        p3 = (int*)p3 + 1;
        p4 = (long long*)p4 + 1;
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            "mov %[i1], %%eax\n\t"
            "add %[i2], %%eax\n\t"
            "mov %%eax, %[sum]\n\t"
            : [sum] "+r" (sum)
            : [i1] "r" (i1), [i2] "r" (i2)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc"
        );
        
        /* Rotate values */
        char tmp = c1;
        c1 = c2; c2 = c3; c3 = c4; c4 = tmp;
    }
    
    return sum + i1 + i2 + i3 + i4 + (int)(intptr_t)p1;
}

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int base_val = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Array with volatile elements for complex addressing */
    volatile int mem_array[64];
    for (int i = 0; i < 64; i++) {
        mem_array[i] = base_val + i;
    }
    
    int checksum = 0;
    
    /* Call functions repeatedly to create reload scenarios */
    for (int i = 0; i < 10; i++) {
        /* Force different code paths */
        g_volatile_seed = (g_volatile_seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Complex addressing with volatile base */
        checksum ^= complex_addressing(i & 0xF, mem_array);
        
        /* Register conflict scenarios */
        register_conflict(base_val + i, base_val - i);
        
        /* Mixed type operations */
        long long ll_result = mixed_type_ops(
            i & 0xFF, 
            i * 100, 
            base_val + i * 1000,
            (long long)base_val * i
        );
        checksum += (int)ll_result;
        
        /* Memory constraints */
        memory_constraints(mem_array + (i & 0x3F), i);
        
        /* High register pressure */
        checksum += high_register_pressure(5);
    }
    
    /* Final computation using all modified memory */
    for (int i = 0; i < 64; i++) {
        checksum += mem_array[i];
    }
    
    checksum += g_volatile_seed;
    
    printf("Checksum: %d\n", checksum);
    return checksum & 0xFF;
}
