/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int vol_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile int* restrict ptr) {
    volatile char buffer[256];
    volatile short* sp = (volatile short*)buffer;
    volatile long* lp = (volatile long*)buffer;
    
    /* Force different addressing modes with volatile index */
    int i = idx + vol_seed;
    buffer[i] = i & 0xFF;
    sp[i % 128] = i & 0xFFFF;
    lp[i % 64] = i;
    
    /* Mixed type operations causing mode changes */
    char c = buffer[i];
    short s = sp[i % 128];
    long l = lp[i % 64];
    
    /* Operations requiring extensions/truncations */
    return (c + s + l) & 0xFF;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    /* Explicit register variables - compete for specific registers */
    register int x asm("r12") = a + vol_seed;
    register int y asm("r13") = b - vol_seed;
    register int z asm("r14") = a * b;
    
    int result;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "imull %[z], %%eax\n\t"
        "movl %%eax, %[res]"
        : [res] "=rm" (result)      /* Output: register OR memory */
        : [x] "r" (x),              /* Input: register only */
          [y] "rm" (y),             /* Input: register OR memory */
          [z] "i" (123)             /* Input: immediate (mismatch!) */
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result;
}

/* Function with memory constraints and clobbered registers */
__attribute__((noinline))
static void memory_constraints(volatile int* arr, int n) {
    volatile int temp[10];
    volatile long big_temp[5];
    
    /* Multiple asm blocks clobbering different registers */
    for (volatile int i = 0; i < n; i++) {
        /* Force memory output with register input */
        asm volatile (
            "movl %[val], %[mem]\n\t"
            "addl $1, %[mem]"
            : [mem] "=m" (temp[i % 10])
            : [val] "ri" (i)        /* Register OR immediate */
            : "cc"
        );
        
        /* Another asm with different clobbers */
        asm volatile (
            "movq %[idx], %%rax\n\t"
            "leaq (%[arr],%%rax,4), %%rbx\n\t"
            "movl (%%rbx), %%ecx\n\t"
            "addl %%ecx, %[sum]"
            : [sum] "+rm" (big_temp[i % 5])
            : [arr] "r" (arr), [idx] "r" (i)
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
    }
}

/* Function with mixed types and conversions */
__attribute__((noinline))
static long mixed_type_ops(char c, short s, int i, long l) {
    volatile union {
        char c[8];
        short s[4];
        int i[2];
        long l;
    } u;
    
    /* Force various conversions and extensions */
    u.c[0] = c + vol_seed;
    u.s[1] = s - vol_seed;
    u.i[0] = i * vol_seed;
    u.l = l / (vol_seed + 1);
    
    /* Complex expression with mixed types */
    long result = (long)u.c[0] + (long)u.s[1] * 2L + 
                  (long)u.i[0] * 3L + u.l;
    
    /* Bitfield operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } bits = {0};
    
    bits.a = u.c[0] & 0x7;
    bits.b = (u.c[0] >> 3) & 0x1F;
    bits.c = u.s[1] & 0xFF;
    bits.d = u.i[0] & 0xFFFF;
    
    result += (bits.a << 24) | (bits.b << 19) | 
              (bits.c << 11) | bits.d;
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static int* pointer_arithmetic(volatile int* base, int offset) {
    volatile int* ptr1, *ptr2;
    volatile long diff;
    
    /* Complex pointer arithmetic */
    ptr1 = base + offset;
    ptr2 = base + (offset * 2);
    
    /* Force reloads with pointer differences */
    diff = (long)ptr2 - (long)ptr1;
    
    /* Inline asm with memory constraints on pointers */
    asm volatile (
        "movq %[ptr1], %%rax\n\t"
        "movq %[ptr2], %%rbx\n\t"
        "subq %%rax, %%rbx\n\t"
        "addq %%rbx, %[diff]"
        : [diff] "+rm" (diff)
        : [ptr1] "rm" (ptr1), [ptr2] "rm" (ptr2)
        : "rax", "rbx", "rcx", "cc"
    );
    
    return (int*)((long)base + diff);
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant propagation */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 100;
    vol_seed = seed;
    
    /* Many local variables of different types */
    char c1 = 'A', c2 = 'B', c3 = 'C';
    short s1 = 1000, s2 = 2000, s3 = 3000;
    int i1 = 100000, i2 = 200000, i3 = 300000;
    long l1 = 1000000L, l2 = 2000000L, l3 = 3000000L;
    volatile int array[100];
    volatile long big_array[50];
    
    /* Initialize arrays with non-constant values */
    for (volatile int i = 0; i < 100; i++) {
        array[i] = i + seed;
    }
    for (volatile int i = 0; i < 50; i++) {
        big_array[i] = i * seed;
    }
    
    /* Call functions repeatedly with different arguments */
    int sum = 0;
    
    sum += complex_addressing(10, array);
    sum += complex_addressing(20, array + 10);
    sum += complex_addressing(30, array + 20);
    
    sum += register_conflicts(seed, seed * 2);
    sum += register_conflicts(seed * 3, seed * 4);
    
    memory_constraints(array, 15);
    memory_constraints(array + 25, 10);
    
    l1 += mixed_type_ops(c1, s1, i1, l1);
    l2 += mixed_type_ops(c2, s2, i2, l2);
    l3 += mixed_type_ops(c3, s3, i3, l3);
    
    int* ptr1 = pointer_arithmetic(array, 5);
    int* ptr2 = pointer_arithmetic(array, 10);
    int* ptr3 = pointer_arithmetic(array, 15);
    
    /* Use all results to prevent dead code elimination */
    sum += (int)(l1 + l2 + l3) & 0xFF;
    sum += (int)((long)ptr1 + (long)ptr2 + (long)ptr3) & 0xFF;
    
    /* Access all arrays to ensure they're used */
    for (volatile int i = 0; i < 10; i++) {
        sum += array[i * 3];
        sum += big_array[i * 2];
    }
    
    printf("Result checksum: %d\n", sum);
    return sum != 0 ? 0 : 1;
}
