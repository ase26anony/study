/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
static int use_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - compete for specific registers */
    register int r12_val asm("r12") = a + 1;
    register int r13_val asm("r13") = b + 2;
    register int r14_val asm("r14") = c + 3;
    register int r15_val asm("r15") = d + 4;
    
    int result;
    
    /* Inline asm with mismatched constraints to force reloads */
    /* Output is memory, inputs are registers - may need reload */
    asm volatile (
        "movl %[r12], %[mem1]\n\t"
        "addl %[r13], %[mem1]\n\t"
        "movl %[r14], %[mem2]\n\t"
        "subl %[r15], %[mem2]\n\t"
        "imull %[mem1], %[mem2]\n\t"
        "movl %[mem2], %0"
        : "=r" (result), [mem1] "=m" (*(int*)&r12_val), [mem2] "=m" (*(int*)&r13_val)
        : [r12] "r" (r12_val), [r13] "r" (r13_val), [r14] "r" (r14_val), [r15] "r" (r15_val)
        : "cc", "memory"
    );
    
    return result + e + f;
}

/* Function using volatile addresses and complex addressing modes */
__attribute__((noinline))
static long use_volatile_addresses(volatile char *ptr1, volatile short *ptr2, 
                                   volatile int *ptr3, volatile long *ptr4) {
    long total = 0;
    volatile int idx = g_volatile_seed & 3; /* Non-constant index */
    
    /* Complex addressing with volatile indices */
    for (volatile int i = 0; i < 4; i++) {
        /* Mixed-type operations requiring mode changes */
        char c_val = ptr1[idx + i];
        short s_val = ptr2[idx + i];
        int i_val = ptr3[idx + i];
        long l_val = ptr4[idx + i];
        
        /* Operations causing mode mismatches */
        long temp = (long)c_val + (long)s_val * 256L;
        temp += (long)i_val << 16;
        temp += l_val;
        
        /* Inline asm with memory output and immediate input */
        /* May require reload for immediate->memory */
        asm volatile (
            "addq %[imm], %[mem]\n\t"
            "subq %[val], %[mem]"
            : [mem] "+m" (temp)
            : [imm] "i" (0x1000), [val] "r" (temp)
            : "cc"
        );
        
        total += temp;
    }
    
    return total;
}

/* Function with mixed data types and register clobbering */
__attribute__((noinline))
static int mixed_types_and_clobbers(int x, char y, short z, long w) {
    /* Union to create complex memory access patterns */
    union {
        int i;
        char c[4];
        short s[2];
        long l;
    } u;
    
    u.i = x;
    u.c[1] = y;
    u.s[1] = z;
    u.l = w;
    
    int result;
    
    /* Inline asm with many clobbers to force spills/reloads */
    asm volatile (
        "movl %[union_i], %%eax\n\t"
        "movsbl %[y], %%ebx\n\t"
        "movswl %[z], %%ecx\n\t"
        "movq %[w], %%rdx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=rm" (result)  /* Output can be reg or mem */
        : [union_i] "rm" (u.i),    /* Input can be reg or mem */
          [y] "rm" (y),
          [z] "rm" (z),
          [w] "rm" (w)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static void* pointer_arithmetic(void *base, int offset1, int offset2) {
    volatile char *volatile_ptr = (volatile char*)base;
    
    /* Complex address calculation */
    uintptr_t addr = (uintptr_t)base;
    addr += offset1 * sizeof(long);
    addr += offset2 * sizeof(int);
    
    /* Cast between integer and pointer types */
    volatile char *result_ptr = (volatile char*)addr;
    
    /* Inline asm with alternative constraints */
    long value;
    asm volatile (
        "movq (%[ptr]), %[val]\n\t"
        "rorq $13, %[val]\n\t"
        "movq %[val], (%[ptr])"
        : [val] "=&r" (value)      /* Early clobber reg */
        : [ptr] "r" (result_ptr)
        : "memory"
    );
    
    return (void*)result_ptr;
}

/* Main function creating maximum register pressure */
int main(int argc, char *argv[]) {
    /* Many local variables of different types */
    char c1 = argc > 1 ? argv[1][0] : 'A';
    volatile char c2 = g_volatile_seed;
    short s1 = argc * 100;
    volatile short s2 = g_volatile_seed + 1;
    int i1 = argc * 1000;
    volatile int i2 = g_volatile_seed + 2;
    long l1 = (long)argc * 10000L;
    volatile long l2 = (long)g_volatile_seed + 3L;
    
    /* Arrays with volatile elements */
    volatile char char_arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    volatile short short_arr[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    volatile int int_arr[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    volatile long long_arr[8] = {1000L, 2000L, 3000L, 4000L, 5000L, 6000L, 7000L, 8000L};
    
    /* Pointers to volatiles */
    volatile char *cptr = &c2;
    volatile short *sptr = &s2;
    volatile int *iptr = &i2;
    volatile long *lptr = &l2;
    
    long checksum = 0;
    
    /* Call functions repeatedly with different arguments */
    for (volatile int iter = 0; iter < 3; iter++) {
        /* Force many reloads with explicit register usage */
        int r1 = use_explicit_registers(i1 + iter, i2, s1, s2, c1, c2);
        checksum += r1;
        
        /* Use volatile addresses with complex constraints */
        long r2 = use_volatile_addresses(char_arr, short_arr, int_arr, long_arr);
        checksum += r2;
        
        /* Mixed types causing mode changes */
        int r3 = mixed_types_and_clobbers(i1, c1 + iter, s1, l1);
        checksum += r3;
        
        /* Pointer arithmetic creating complex addressing */
        void *r4 = pointer_arithmetic(int_arr, iter, argc);
        checksum += (long)r4;
        
        /* Additional inline asm with many clobbers */
        asm volatile (
            "movl %[i1], %%eax\n\t"
            "movl %[i2], %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %[tmp]\n\t"
            "movq %[l1], %%rcx\n\t"
            "movq %[l2], %%rdx\n\t"
            "addq %%rdx, %%rcx\n\t"
            "movq %%rcx, %[tmp2]"
            : [tmp] "=m" (i1), [tmp2] "=m" (l1)
            : [i1] "rm" (i1), [i2] "rm" (i2),
              [l1] "rm" (l1), [l2] "rm" (l2)
            : "rax", "rbx", "rcx", "rdx", "cc", "memory"
        );
    }
    
    /* Final computation to prevent elimination */
    checksum += (long)c1 + (long)c2 + (long)s1 + (long)s2 + 
                (long)i1 + (long)i2 + l1 + l2;
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
