/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int vol_seed = 12345;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int* create_complex_address(volatile int* base, int offset) {
    /* Complex pointer arithmetic that may need reloads */
    return (int*)((char*)base + offset * sizeof(int) + 7);
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r12") = a + vol_seed;
    register int y asm("r13") = b - vol_seed;
    int result;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r,m" (result)      /* Output: register OR memory */
        : "r,m" (x), "r,m" (y) /* Inputs: register OR memory */
        : "eax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "cc", "memory"
    );
    
    return result;
}

/* Function with memory output constraints and immediate inputs */
__attribute__((noinline))
static void memory_constraints(volatile int* arr, int size) {
    volatile int temp;
    int* complex_addr;
    
    for (volatile int i = 0; i < size; i++) {
        /* Create complex address that may need reload */
        complex_addr = create_complex_address(arr, i);
        
        /* Inline asm with memory output and immediate input */
        asm volatile (
            "movl %1, %0\n\t"
            "addl $0x7F, %0\n\t"  /* Immediate that may need reload in 64-bit */
            : "=m" (*complex_addr)
            : "i" (i * 2)         /* Immediate constraint */
            : "rax", "rbx", "rcx", "rdx", "memory"
        );
        
        /* Another asm with mismatched modes */
        asm volatile (
            "movb %b1, %b0\n\t"   /* byte move */
            "addb $1, %b0\n\t"
            : "=r,m" (temp)
            : "r,i" (i & 0xFF)    /* byte-sized immediate */
            : "cc"
        );
    }
}

/* Function with mixed data types causing mode changes */
__attribute__((noinline))
static long long mixed_type_operations(char c, short s, int i, long long ll) {
    /* Operations that change machine modes */
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long long vll = ll;
    
    /* Mix types to create subreg/zero_extend operations */
    long long result = 0;
    
    /* char -> long long with sign extension */
    result += (signed char)vc;
    
    /* short -> long long */
    result += (long long)vs * 256;
    
    /* int -> long long with operation */
    result += (long long)vi << 16;
    
    /* Complex operation with all types */
    asm volatile (
        "movsx %1, %q0\n\t"      /* sign extend char to long long */
        "movsx %2, %%rax\n\t"    /* sign extend short */
        "addq %%rax, %0\n\t"
        "movslq %3, %%rax\n\t"   /* sign extend int */
        "addq %%rax, %0\n\t"
        "addq %4, %0\n\t"
        : "+r,m" (result)
        : "r,m" (vc), "r,m" (vs), "r,m" (vi), "r,m" (vll)
        : "rax", "rbx", "rcx", "rdx", "cc", "memory"
    );
    
    return result;
}

/* Function with bitfields and unions */
__attribute__((noinline))
static int bitfield_union_ops(int x) {
    union {
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
            unsigned int d : 16;
        } bits;
        unsigned int full;
    } u;
    
    volatile int* ptr = &u.full;
    
    /* Access bitfields - may create complex RTL */
    u.bits.a = x & 0x7;
    u.bits.b = (x >> 3) & 0x1F;
    u.bits.c = (x >> 8) & 0xFF;
    u.bits.d = (x >> 16) & 0xFFFF;
    
    /* Use in inline asm with memory constraint */
    int result;
    asm volatile (
        "movl (%1), %%eax\n\t"
        "rorl $8, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r,m" (result)
        : "r,m" (ptr)
        : "eax", "memory"
    );
    
    return result ^ u.full;
}

/* Main function that orchestrates everything */
int main(int argc, char* argv[]) {
    /* Use argc to prevent constant folding */
    volatile int seed = argc > 1 ? atoi(argv[1]) : vol_seed;
    
    /* Declare many variables of different types */
    char c1 = seed & 0xFF;
    char c2 = (seed >> 8) & 0xFF;
    short s1 = seed & 0xFFFF;
    short s2 = (seed >> 16) & 0xFFFF;
    int i1 = seed;
    int i2 = seed * 3;
    int i3 = seed / 2;
    int i4 = seed + 1000;
    long long ll1 = (long long)seed * seed;
    long long ll2 = (long long)seed << 32;
    
    /* Array with volatile elements */
    volatile int arr[100];
    for (volatile int i = 0; i < 100; i++) {
        arr[i] = seed + i;
    }
    
    /* Call functions repeatedly to increase reload pressure */
    int sum = 0;
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int iter = 0; iter < 10; iter++) {
        /* Use explicit register variables */
        sum += use_explicit_registers(i1 + iter, i2 - iter);
        
        /* Memory constraints with complex addresses */
        memory_constraints(arr, 20);
        
        /* Mixed type operations */
        ll1 += mixed_type_operations(c1 + iter, s1 - iter, 
                                    i3 + iter, ll2 >> iter);
        
        /* Bitfield and union operations */
        sum += bitfield_union_ops(seed + iter);
        
        /* Additional inline asm with many clobbers */
        asm volatile (
            "movq %1, %%rax\n\t"
            "movq %2, %%rbx\n\t"
            "addq %%rbx, %%rax\n\t"
            "movq %%rax, %0\n\t"
            : "=r,m" (ll2)
            : "r,m" (ll1), "r,m" ((long long)iter)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", 
              "r14", "r15", "cc", "memory"
        );
        
        /* Update array with complex addressing */
        int* addr = create_complex_address((int*)arr, iter);
        *addr = sum + iter;
    }
    
    /* Final computation to prevent dead code elimination */
    long long final_result = (long long)sum + ll1 + ll2;
    
    /* Use all variables in final output */
    for (volatile int i = 0; i < 50; i++) {
        final_result += arr[i] + i;
    }
    
    final_result += c1 + c2 + s1 + s2 + i1 + i2 + i3 + i4;
    
    printf("Result: %lld\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
