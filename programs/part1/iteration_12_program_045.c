/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(volatile int* arr, int idx1, int idx2) {
    /* Force base+index*scale addressing with volatile indices */
    volatile int vi1 = idx1;
    volatile int vi2 = idx2;
    
    /* Complex address calculation that may need reloads */
    int* ptr1 = (int*)arr + vi1;
    int* ptr2 = (int*)arr + vi2;
    
    /* Inline asm with memory constraints and clobbers */
    int result;
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (ptr1), "r" (ptr2)
        : "%eax", "memory", "cc"
    );
    
    return result;
}

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static void register_conflict(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a;
    register int y asm("r13") = b;
    
    /* Inline asm with output in memory but input in register */
    volatile int mem_out;
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (mem_out)        /* Output to memory */
        : "r" (x), "r" (y)      /* Inputs in registers */
        : "%eax", "cc"
    );
    
    /* Force use of result to prevent elimination */
    g_volatile_seed += mem_out;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long long type_mixing(char c, short s, int i, long l) {
    /* Operations that change machine modes */
    long long ll1 = (long long)c;  /* zero/sign extend from char */
    long long ll2 = (long long)s;  /* zero/sign extend from short */
    long long ll3 = (long long)i;  /* int to long long */
    long long ll4 = (long long)l;  /* long to long long */
    
    /* Mixed operations that may require reloads */
    volatile long long vll = 0;
    
    /* Inline asm with multiple alternatives and clobbers */
    asm volatile (
        "add %1, %0\n\t"
        "adc %2, %0\n\t"
        "add %3, %0\n\t"
        "adc %4, %0\n\t"
        : "+r" (vll)
        : "r" (ll1), "r" (ll2), "r" (ll3), "r" (ll4)
        : "cc"
    );
    
    return vll;
}

/* Function with pointer arithmetic and volatile addresses */
__attribute__((noinline))
static int* pointer_arithmetic(volatile int* base, int offset) {
    /* Complex pointer arithmetic that may not fit addressing modes */
    volatile int voff = offset;
    
    /* Multiple addressing calculations */
    int* p1 = (int*)base + voff;
    int* p2 = p1 + g_volatile_seed;
    int* p3 = p2 - voff;
    
    /* Inline asm that clobbers many registers */
    int* result;
    asm volatile (
        "mov %1, %%rax\n\t"
        "add %2, %%rax\n\t"
        "sub %3, %%rax\n\t"
        "mov %%rax, %0\n\t"
        : "=r" (result)
        : "r" (p1), "r" (p2), "r" (p3)
        : "%rax", "%rbx", "%rcx", "%rdx", "%rsi", "%rdi", "cc"
    );
    
    return result;
}

/* Function with bitfields and unions causing subreg operations */
__attribute__((noinline))
static int bitfield_ops(int val) {
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bits;
        uint32_t full;
    } u;
    
    u.full = val;
    
    /* Operations on bitfields that create subreg RTL */
    volatile int sum = u.bits.a + u.bits.b + u.bits.c + u.bits.d;
    
    /* Inline asm with immediate constraints */
    asm volatile (
        "addl $0x%0, %1\n\t"
        : "+r" (sum)
        : "i" (0x1234)
        : "cc"
    );
    
    return sum;
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) {
    /* Use argc to prevent constant folding */
    volatile int seed = g_volatile_seed + argc;
    
    /* Declare many variables of different types */
    char c1 = seed & 0xFF;
    char c2 = (seed >> 8) & 0xFF;
    short s1 = seed & 0xFFFF;
    short s2 = (seed >> 16) & 0xFFFF;
    int i1 = seed;
    int i2 = seed * 2;
    int i3 = seed * 3;
    int i4 = seed * 4;
    long l1 = seed * 5L;
    long l2 = seed * 6L;
    long long ll1 = seed * 7LL;
    long long ll2 = seed * 8LL;
    
    /* Array for complex addressing */
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = seed + i;
    }
    
    /* Call functions repeatedly to create reload situations */
    int sum = 0;
    
    /* 1. Complex addressing with volatile indices */
    sum += complex_addressing(arr, i1 & 15, i2 & 15);
    
    /* 2. Register conflicts */
    register_conflict(i1, i2);
    register_conflict(i3, i4);
    
    /* 3. Type mixing */
    ll1 += type_mixing(c1, s1, i1, l1);
    ll2 += type_mixing(c2, s2, i2, l2);
    
    /* 4. Pointer arithmetic */
    int* ptr = pointer_arithmetic(arr, i3 & 31);
    sum += *ptr;
    
    /* 5. Bitfield operations */
    sum += bitfield_ops(i1);
    sum += bitfield_ops(i2);
    sum += bitfield_ops(i3);
    sum += bitfield_ops(i4);
    
    /* 6. More inline asm with mismatched constraints */
    volatile int out1, out2;
    asm volatile (
        "movl $0x%0, %%eax\n\t"
        "movl %%eax, %1\n\t"
        "movl $0x%2, %%ebx\n\t"
        "movl %%ebx, %3\n\t"
        : "=m" (out1), "=m" (out2)
        : "i" (0xDEADBEEF), "i" (0xCAFEBABE)
        : "%eax", "%ebx", "memory"
    );
    
    sum += out1 + out2;
    
    /* 7. Loop with volatile counter causing spills/reloads */
    volatile int vcounter = 10;
    while (vcounter-- > 0) {
        /* Mixed operations in loop */
        c1 += c2;
        s1 += s2;
        i1 += i2;
        l1 += l2;
        
        /* Inline asm that clobbers registers */
        asm volatile (
            "add $1, %0\n\t"
            "add $1, %1\n\t"
            : "+r" (i3), "+r" (i4)
            :
            : "cc"
        );
    }
    
    /* Final computation to prevent elimination */
    long long final_result = ll1 + ll2 + sum + c1 + s1 + i1 + l1 + i3 + i4;
    
    printf("Result: %lld\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
