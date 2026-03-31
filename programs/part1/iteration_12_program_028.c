/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int idx, volatile char* base) {
    /* Force base+index addressing with volatile */
    volatile int result;
    volatile char* ptr = base + idx;
    
    /* Inline asm with memory constraint and clobbered registers */
    asm volatile (
        "movb (%1), %%al\n\t"
        "movsbl %%al, %0\n\t"
        : "=r" (result)
        : "r" (ptr)
        : "rax", "memory", "cc"
    );
    
    return result;
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static long register_conflict(long a, long b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register long x asm("r12") = a;
    register long y asm("r13") = b;
    long result;
    
    /* Inline asm with mismatched constraints - output is memory, inputs are registers */
    asm volatile (
        "addq %2, %1\n\t"
        "movq %1, %0\n\t"
        : "=m" (result)      /* Memory output */
        : "r" (x), "r" (y)   /* Register inputs */
        : "r12", "r13", "cc", "memory"
    );
    
    return result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static int64_t mixed_type_ops(char c, short s, int i, volatile long* ptr) {
    /* Operations that cause mode changes */
    int64_t result = 0;
    
    /* char -> int64_t with sign extension */
    result += (int64_t)c;
    
    /* short -> int64_t */
    result += (int64_t)s;
    
    /* int -> int64_t */
    result += (int64_t)i;
    
    /* Memory access with complex addressing */
    result += *ptr;
    
    /* Inline asm that uses the result in different ways */
    asm volatile (
        "movq %1, %%rax\n\t"
        "shlq $3, %%rax\n\t"
        "addq $0x1234, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (result)
        : "r" (result)
        : "rax", "cc"
    );
    
    return result;
}

/* Function with multiple alternative constraints */
__attribute__((noinline))
static void alternative_constraints(volatile int* arr, int n) {
    int i;
    volatile int temp;
    
    for (i = 0; i < n; i++) {
        /* Force reloads by using alternative constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl $1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r,m" (arr[i])    /* Alternative: register or memory */
            : "r,i,m" (i)        /* Alternative: register, immediate, or memory */
            : "rax", "cc", "memory"
        );
    }
}

/* Function with pointer arithmetic and volatile */
__attribute__((noinline))
static int* pointer_arithmetic(volatile int* base, volatile int offset) {
    /* Complex pointer arithmetic that may not fit addressing modes */
    int* result;
    
    /* Force non-simple addressing */
    result = (int*)((char*)base + offset * sizeof(int) + 16);
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "movq %1, %%rdi\n\t"
        "movq %%rdi, %0\n\t"
        : "=r" (result)
        : "r" (result)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", 
          "r14", "r15", "cc", "memory"
    );
    
    return result;
}

/* Union to create subreg operations */
union mixed_union {
    char c[8];
    short s[4];
    int i[2];
    long l;
    void* p;
};

__attribute__((noinline))
static long union_operations(union mixed_union* u) {
    long result = 0;
    
    /* Access different views of the same memory */
    result += u->c[0];
    result += u->s[1];
    result += u->i[0];
    result += u->l;
    
    /* Cast between pointer and integer */
    uintptr_t addr = (uintptr_t)u;
    result += (long)(addr >> 4);
    
    return result;
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) {
    /* Many local variables of different types */
    volatile char c1 = 1, c2 = 2, c3 = 3;
    volatile short s1 = 100, s2 = 200, s3 = 300;
    volatile int i1 = 1000, i2 = 2000, i3 = 3000;
    volatile long l1 = 10000, l2 = 20000, l3 = 30000;
    volatile int* ptr1 = &i1;
    volatile int* ptr2 = &i2;
    volatile int* ptr3 = &i3;
    
    /* Arrays with volatile indices */
    volatile int arr[100];
    volatile int idx = global_seed % 50;
    
    /* Union for type-punning */
    union mixed_union u;
    u.l = 0x123456789ABCDEF0ULL;
    
    /* Initialize array with volatile values */
    for (volatile int j = 0; j < 100; j++) {
        arr[j] = j + global_seed;
    }
    
    long checksum = 0;
    
    /* Call functions repeatedly to create reload pressure */
    for (volatile int iter = 0; iter < 10; iter++) {
        /* Complex addressing */
        checksum += complex_addressing(idx + iter, (volatile char*)arr);
        
        /* Register conflicts */
        checksum += register_conflict(l1 + iter, l2 - iter);
        
        /* Mixed type operations */
        checksum += mixed_type_ops(c1 + iter, s1 + iter, i1 + iter, &l3);
        
        /* Alternative constraints */
        alternative_constraints(arr + 10, 5);
        checksum += arr[10];
        
        /* Pointer arithmetic */
        int* new_ptr = pointer_arithmetic(ptr1, idx + iter);
        checksum += (long)new_ptr;
        
        /* Union operations */
        u.l += iter;
        checksum += union_operations(&u);
        
        /* Force spills by using many variables in computation */
        checksum += c1 * s1 * i1 * l1;
        checksum += (long)(ptr2) - (long)(ptr1);
        checksum += arr[idx] * arr[idx + 1];
        
        /* More inline asm with clobbers */
        asm volatile (
            "movq %1, %%rax\n\t"
            "addq %2, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r" (checksum)
            : "r" (checksum), "r" ((long)iter)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "cc", "memory"
        );
    }
    
    /* Final computation to prevent elimination */
    checksum += argc;
    for (int k = 0; k < argc; k++) {
        checksum += (long)argv[k];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}
