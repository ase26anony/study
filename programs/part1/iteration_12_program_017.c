/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_complex_addressing(volatile int* arr, int idx1, int idx2) {
    /* Force base+index*scale addressing with reloads */
    register int r12_val asm("r12") = idx1;
    register int r13_val asm("r13") = idx2;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "movl %[idx1], %%eax\n\t"
        "movl %[idx2], %%ebx\n\t"
        "leal (%%rax,%%rbx,4), %%ecx\n\t"
        "movl (%%r12,%%rcx,4), %%edx\n\t"
        "addl $1, %%edx\n\t"
        "movl %%edx, (%%r12,%%rcx,4)"
        : 
        : [idx1] "r" (r12_val), [idx2] "r" (r13_val), "r" (arr)
        : "rax", "rbx", "rcx", "rdx", "memory", "cc"
    );
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b, int c) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r10") = a;
    register int y asm("r11") = b;
    register int z asm("r12") = c;
    
    int result;
    
    /* Inline asm with output in memory, inputs in registers */
    /* This creates operand mismatches requiring reloads */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "imull %[z], %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=m" (result)  /* Output to memory */
        : [x] "r" (x), [y] "r" (y), [z] "r" (z)  /* Inputs in registers */
        : "rax", "cc"
    );
    
    /* More asm with clobbered registers */
    asm volatile (
        "movl %[x], %%ebx\n\t"
        "movl %[y], %%ecx\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %%ecx"
        : 
        : [x] "r" (x), [y] "r" (y)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    return result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long mixed_type_operations(char c, short s, int i, long l) {
    /* Operations that cause mode changes */
    long result = 0;
    
    /* char in 64-bit operation */
    result += (long)c * 256;
    
    /* short in 64-bit operation with shift */
    result += (long)s << 16;
    
    /* int to long conversion */
    result += (long)i * 1000;
    
    /* Complex operation with volatile to prevent optimization */
    volatile char vc = c;
    volatile short vs = s;
    
    /* Inline asm with mismatched operand sizes */
    asm volatile (
        "movsbl %[vc], %%eax\n\t"
        "movswl %[vs], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "cltq\n\t"
        "addq %%rax, %[result]"
        : [result] "+r" (result)
        : [vc] "m" (vc), [vs] "m" (vs)
        : "rax", "rbx", "cc"
    );
    
    return result + l;
}

/* Function with pointer arithmetic and memory constraints */
__attribute__((noinline))
static void pointer_arithmetic_stress(volatile int* ptr, int offset) {
    volatile int local_volatile = offset;
    int* volatile volatile_ptr = (int*)ptr;
    
    /* Complex address calculation */
    int* addr = volatile_ptr + (local_volatile * 3) / 2;
    
    /* Inline asm with memory output and immediate input */
    /* This often requires reloads */
    asm volatile (
        "movl $0x12345678, %[addr]\n\t"
        : [addr] "=m" (*addr)
        : 
        : "memory"
    );
    
    /* More complex asm with multiple constraints */
    int temp;
    asm volatile (
        "movl %[offset], %%eax\n\t"
        "leal (%%rax,%%rax,2), %%ebx\n\t"
        "movl %%ebx, %[temp]"
        : [temp] "=r,m" (temp)  /* Multiple constraints */
        : [offset] "r,m,i" (offset)  /* Register, memory, or immediate */
        : "rax", "rbx", "cc"
    );
    
    /* Use the result */
    *ptr += temp;
}

/* Function with bitfields and unions */
__attribute__((noinline))
static int bitfield_union_ops(int a, int b) {
    union {
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } bits;
        uint32_t value;
    } u;
    
    u.value = a;
    u.bits.mid = b & 0xFF;
    
    /* Operations causing subreg operations */
    char c = u.bits.low;
    short s = u.bits.mid;
    int i = u.bits.high;
    
    /* Inline asm with bitfield extraction */
    int result;
    asm volatile (
        "movl %[val], %%eax\n\t"
        "andl $0xFF, %%eax\n\t"
        "movl %%eax, %[result]"
        : [result] "=r,m" (result)
        : [val] "r,m" (u.value)
        : "rax", "cc"
    );
    
    return result + c + s + i;
}

/* Main function that creates maximum register pressure */
int main(int argc, char* argv[]) {
    /* Many local variables of different types */
    char c1 = argc > 1 ? argv[1][0] : 'A';
    short s1 = argc * 100;
    int i1 = g_volatile_seed;
    long l1 = (long)argc * 1000000;
    
    volatile int v1 = argc;
    volatile char vc = c1 + 1;
    volatile short vs = s1 - 1;
    
    int array[100];
    for (int j = 0; j < 100; j++) {
        array[j] = j * g_volatile_seed;
    }
    
    /* Call functions repeatedly to create reload scenarios */
    int sum = 0;
    
    for (volatile int k = 0; k < 10; k++) {
        /* Mixed type operations */
        l1 = mixed_type_operations(c1 + k, s1 + k, i1 + k, l1);
        
        /* Register conflicts */
        int r = register_conflicts(i1, i1 + 1, i1 + 2);
        sum += r;
        
        /* Bitfield operations */
        sum += bitfield_union_ops(i1, i1 + k);
        
        /* Complex addressing */
        if (k < 5) {
            use_complex_addressing(array, k, k * 2);
        }
        
        /* Pointer arithmetic */
        pointer_arithmetic_stress(&i1, k);
        
        /* Update volatile variables to prevent optimization */
        vc++;
        vs += 2;
        v1 = sum;
    }
    
    /* Final computation using all variables */
    long final_result = (long)sum + l1 + array[10] + v1 + vc + vs;
    
    /* Use the result so it can't be optimized away */
    printf("Result: %ld\n", final_result);
    
    /* Additional stress: nested loops with many variables */
    {
        register int r8 asm("r8") = final_result & 0xFF;
        register int r9 asm("r9") = (final_result >> 8) & 0xFF;
        register int r10 asm("r10") = (final_result >> 16) & 0xFF;
        
        /* Inline asm with many clobbered registers */
        asm volatile (
            "movl %[a], %%eax\n\t"
            "movl %[b], %%ebx\n\t"
            "movl %[c], %%ecx\n\t"
            "addl %%ebx, %%eax\n\t"
            "addl %%ecx, %%eax\n\t"
            "movl %%eax, %[sum]"
            : [sum] "=m" (sum)
            : [a] "r" (r8), [b] "r" (r9), [c] "r" (r10)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc"
        );
    }
    
    printf("Final checksum: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
