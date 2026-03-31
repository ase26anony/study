/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int use_complex_addresses(volatile int *arr, int idx1, int idx2) {
    volatile int temp = 0;
    int *ptr1, *ptr2;
    int result = 0;
    
    /* Force register usage with explicit constraints */
    register int r1 asm("r12") = idx1;
    register int r2 asm("r13") = idx2;
    
    /* Complex addressing with multiple alternatives */
    asm volatile (
        "movl %[idx1], %%ebx\n\t"
        "movl %[idx2], %%ecx\n\t"
        "leal (%%rbx,%%rcx,4), %%edx\n\t"
        "movl (%%rsi,%%rdx,4), %%eax\n\t"
        "addl %%eax, %[result]\n\t"
        : [result] "+r" (result)
        : [idx1] "r" (r1), [idx2] "r" (r2), [arr] "r" (arr)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* More inline asm with mismatched constraints */
    ptr1 = &arr[idx1 * 2];
    ptr2 = &arr[idx2 * 3];
    
    asm volatile (
        "movl (%[ptr1]), %%eax\n\t"
        "addl %%eax, (%[ptr2])\n\t"
        "movl (%[ptr2]), %[out]\n\t"
        : [out] "=r" (result)
        : [ptr1] "r" (ptr1), [ptr2] "r" (ptr2)
        : "rax", "memory", "cc"
    );
    
    return result + temp;
}

/* Function with explicit register variables and constraints */
__attribute__((noinline))
static long mix_data_types(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    register long rl asm("r14") = vl;
    register int ri asm("r15") = vi;
    
    /* Force mode mismatches */
    long result = 0;
    
    /* Inline asm with memory output, register input */
    asm volatile (
        "movsbl %[vc], %%eax\n\t"
        "movswl %[vs], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %[ri], %%eax\n\t"
        "addq %[rl], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "=m" (result)
        : [vc] "m" (vc), [vs] "m" (vs), [ri] "r" (ri), [rl] "r" (rl)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* More operations causing mode changes */
    result = (result & 0xFF) + ((result >> 8) & 0xFFFF);
    
    /* Union to force subreg operations */
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = vi;
    result += u.s[0] * u.c[1];
    
    return result;
}

/* Function with many clobbered registers */
__attribute__((noinline))
static int clobber_many_regs(int a, int b, int c, int d, int e, int f) {
    int r1, r2, r3, r4, r5, r6;
    
    /* Force many values into registers */
    asm volatile (
        "movl %[a], %[r1]\n\t"
        "movl %[b], %[r2]\n\t"
        "movl %[c], %[r3]\n\t"
        "movl %[d], %[r4]\n\t"
        "movl %[e], %[r5]\n\t"
        "movl %[f], %[r6]\n\t"
        : [r1] "=r" (r1), [r2] "=r" (r2), [r3] "=r" (r3),
          [r4] "=r" (r4), [r5] "=r" (r5), [r6] "=r" (r6)
        : [a] "r" (a), [b] "r" (b), [c] "r" (c),
          [d] "r" (d), [e] "r" (e), [f] "r" (f)
        : "memory"
    );
    
    /* Clobber many registers around computation */
    asm volatile (
        "pushq %%rax\n\t"
        "pushq %%rbx\n\t"
        "pushq %%rcx\n\t"
        "pushq %%rdx\n\t"
        "pushq %%rsi\n\t"
        "pushq %%rdi\n\t"
        "movl %[r1], %%eax\n\t"
        "addl %[r2], %%eax\n\t"
        "addl %[r3], %%eax\n\t"
        "addl %[r4], %%eax\n\t"
        "addl %[r5], %%eax\n\t"
        "addl %[r6], %%eax\n\t"
        "movl %%eax, %[sum]\n\t"
        "popq %%rdi\n\t"
        "popq %%rsi\n\t"
        "popq %%rdx\n\t"
        "popq %%rcx\n\t"
        "popq %%rbx\n\t"
        "popq %%rax\n\t"
        : [sum] "=m" (r1)
        : [r1] "m" (r1), [r2] "m" (r2), [r3] "m" (r3),
          [r4] "m" (r4), [r5] "m" (r5), [r6] "m" (r6)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory", "cc"
    );
    
    return r1 + r2 + r3 + r4 + r5 + r6;
}

/* Function with pointer arithmetic and volatile */
__attribute__((noinline))
static int pointer_arithmetic(volatile int *base, volatile int offset) {
    int *ptr1, *ptr2;
    int result = 0;
    volatile int idx = offset;
    
    /* Complex pointer arithmetic */
    ptr1 = (int *)((char *)base + idx * sizeof(int) * 2);
    ptr2 = (int *)((char *)base + (idx + 1) * sizeof(int) * 3);
    
    /* Inline asm with memory constraints */
    asm volatile (
        "movl (%[ptr1]), %%eax\n\t"
        "imull $0x1234, %%eax, %%eax\n\t"
        "movl %%eax, (%[ptr2])\n\t"
        "addl %%eax, %[result]\n\t"
        : [result] "+r" (result)
        : [ptr1] "r" (ptr1), [ptr2] "r" (ptr2)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
    
    /* More complex addressing */
    for (volatile int i = 0; i < 3; i++) {
        int *p = &base[idx + i * 2];
        asm volatile (
            "addl $1, (%[p])\n\t"
            :
            : [p] "r" (p)
            : "memory", "cc"
        );
        result += *p;
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int seed = g_volatile_seed + argc;
    long checksum = 0;
    
    /* Array with volatile elements */
    volatile int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = seed + i;
    }
    
    /* Call functions repeatedly with different arguments */
    for (volatile int i = 0; i < 10; i++) {
        checksum += use_complex_addresses((int *)arr, i, i * 2);
        checksum += mix_data_types(
            (char)(seed + i),
            (short)(seed * i),
            seed + i * 3,
            (long)seed * i * i
        );
        checksum += clobber_many_regs(
            seed + i,
            seed + i * 2,
            seed + i * 3,
            seed + i * 4,
            seed + i * 5,
            seed + i * 6
        );
        checksum += pointer_arithmetic(arr, i % 20);
    }
    
    /* Additional stress with bit-fields */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits;
    
    bits.a = seed & 0x7;
    bits.b = (seed >> 3) & 0x1F;
    bits.c = (seed >> 8) & 0xFF;
    bits.d = (seed >> 16) & 0xFFFF;
    
    /* Operations on bit-fields causing mode changes */
    unsigned int bit_result = bits.a + bits.b * bits.c - bits.d;
    
    /* Cast between pointer and integer types */
    uintptr_t ptr_val = (uintptr_t)&arr[0];
    ptr_val += bit_result * sizeof(int);
    int *derived_ptr = (int *)ptr_val;
    
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl %%eax, %[sum]\n\t"
        : [sum] "+r" (checksum)
        : [ptr] "r" (derived_ptr)
        : "rax", "memory", "cc"
    );
    
    /* Final computation to prevent elimination */
    printf("Checksum: %ld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
