/* reload_stress.c - Stress GCC's reload pass to trigger rld[] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization and constant propagation */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int* create_complex_address(volatile int* base, int offset) {
    return (int*)((char*)base + offset * sizeof(int) + 7);
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + global_seed;
    register int y asm("r13") = b - global_seed;
    int result;
    
    /* Inline asm with mismatched constraints - output is memory, inputs are registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (result)      /* Memory output */
        : "r" (x), "r" (y)   /* Register inputs */
        : "eax", "memory"
    );
    
    /* Force more register pressure */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (result)
        : "r" (x)
        : "cc"
    );
    
    return result;
}

/* Function with volatile addresses and memory constraints */
__attribute__((noinline)) 
static void memory_operations(volatile int* arr, int idx) {
    volatile int temp = global_seed;
    int* complex_addr = create_complex_address(arr, idx);
    
    /* Multiple asm statements with memory constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (*complex_addr)    /* Complex memory address */
        : "m" (temp)              /* Volatile memory input */
        : "eax", "memory"
    );
    
    /* Another with immediate to memory */
    asm volatile (
        "movl $0x5678, %0\n\t"
        : "=m" (arr[idx & 3])
        :
        : "memory"
    );
    
    /* Clobber many registers to force spills */
    asm volatile (
        ""
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long mixed_type_operations(char c, short s, int i, long l) {
    /* Operations that cause mode changes */
    long result = 0;
    volatile char vc = c;
    volatile short vs = s;
    
    /* Mix types in calculations */
    result = (long)vc * 256L;          /* char -> long */
    result += (long)vs * 65536L;       /* short -> long */
    result += (long)i * 4294967296L;   /* int -> long */
    result += l;
    
    /* Use in inline asm with different sized operands */
    int temp_int;
    short temp_short;
    
    asm volatile (
        "movsx %1, %%rax\n\t"          /* Sign extend */
        "movsx %2, %%rbx\n\t"
        "addq %%rbx, %%rax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (temp_int)
        : "r" (vc), "r" (vs)
        : "rax", "rbx", "cc"
    );
    
    asm volatile (
        "movzwl %1, %%eax\n\t"         /* Zero extend */
        "movl %%eax, %0\n\t"
        : "=r" (temp_short)
        : "r" (vc)
        : "eax"
    );
    
    result += temp_int + temp_short;
    return result;
}

/* Function with pointer arithmetic and complex addressing */
__attribute__((noinline))
static int pointer_arithmetic(volatile int* base, volatile int offset) {
    int* ptr1 = (int*)((char*)base + offset);
    int* ptr2 = (int*)((char*)base + offset * 2 + 3);
    int* ptr3 = (int*)((char*)base + offset * 3 + 7);
    
    int sum = 0;
    
    /* Multiple memory accesses with complex addresses */
    asm volatile (
        "movl (%1), %%eax\n\t"
        "addl (%2), %%eax\n\t"
        "addl (%3), %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (sum)
        : "r" (ptr1), "r" (ptr2), "r" (ptr3)
        : "eax", "memory"
    );
    
    /* Force base+index addressing reloads */
    for (volatile int i = 0; i < 4; i++) {
        asm volatile (
            "movl (%1, %2, 4), %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (sum)
            : "r" (base), "r" (i)
            : "eax", "memory"
        );
    }
    
    return sum;
}

/* Function using unions for type punning */
__attribute__((noinline))
static int union_type_punning(int value) {
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = value + global_seed;
    
    /* Access different views of the same data */
    int result = 0;
    result += u.s[0];
    result += u.s[1];
    result += u.c[0] * 256;
    result += u.c[3];
    
    /* Inline asm with subreg-like operations */
    asm volatile (
        "movzwl %1, %%eax\n\t"
        "movsbl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%eax, %0\n\t"
        : "+r" (result)
        : "r" (u.s[0]), "r" (u.c[1])
        : "eax", "ebx", "cc"
    );
    
    return result;
}

/* Main function that creates maximum register pressure */
int main(int argc, char** argv) {
    /* Many local variables of different types */
    volatile int v1 = argc + 1;
    volatile char v2 = argc * 2;
    volatile short v3 = argc * 3;
    volatile long v4 = argc * 100L;
    volatile int* v5 = (int*)&v1;
    
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i + global_seed;
    }
    
    volatile int idx = argc;
    
    /* Call functions repeatedly with different arguments */
    long checksum = 0;
    
    checksum += use_explicit_registers(v1, v2);
    
    memory_operations(arr, idx);
    for (int i = 0; i < 4; i++) {
        checksum += arr[i];
    }
    
    checksum += mixed_type_operations(v2, v3, v1, v4);
    
    checksum += pointer_arithmetic(arr, idx);
    
    checksum += union_type_punning(v1);
    
    /* Additional complex operations in main */
    {
        /* More register pressure */
        register int r1 asm("rbx") = v1;
        register int r2 asm("r12") = v2;
        register int r3 asm("r13") = v3;
        
        asm volatile (
            "leal (%1, %2, 2), %%eax\n\t"
            "addl %3, %%eax\n\t"
            "addl %%eax, %0\n\t"
            : "+r" (checksum)
            : "r" (r1), "r" (r2), "r" (r3)
            : "eax", "cc"
        );
        
        /* Force many spills around clobber */
        asm volatile (
            ""
            :
            :
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "memory"
        );
    }
    
    /* Loop with volatile counter to prevent optimization */
    for (volatile int i = 0; i < 3; i++) {
        checksum += use_explicit_registers(v1 + i, v2 - i);
        checksum += mixed_type_operations(v2 + i, v3 + i, v1 + i, v4 + i);
    }
    
    printf("Checksum: %ld\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
}
