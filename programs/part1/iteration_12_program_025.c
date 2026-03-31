/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
static volatile int global_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int* create_complex_address(volatile int* base, int offset) {
    return (int*)((char*)base + offset * sizeof(int) * 2);
}

/* Function using explicit register variables with conflicting constraints */
__attribute__((noinline))
static int use_explicit_registers(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + global_seed;
    register int y asm("r13") = b - global_seed;
    int result;
    
    /* Inline asm with mismatched constraints to force reloads */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)          /* Output in register */
        : "m" (x), "m" (y)       /* Inputs from memory (conflict!) */
        : "eax", "r12", "r13", "memory"
    );
    
    return result;
}

/* Function with volatile addresses and memory constraints */
__attribute__((noinline))
static void memory_operations_with_reloads(volatile int* arr, int size) {
    volatile int temp = global_seed;
    int i;
    
    /* Complex addressing that may require base+index reloads */
    for (i = 0; i < size; i++) {
        int* addr = create_complex_address(arr, i);
        
        /* Inline asm with memory output and immediate input */
        asm volatile (
            "movl %1, %0\n\t"
            : "=m" (*addr)        /* Memory output */
            : "i" (temp + i)      /* Immediate input (may need reload) */
            : "memory"
        );
    }
}

/* Function mixing data types to create mode changes */
__attribute__((noinline))
static long mix_data_types(volatile char c, volatile short s, volatile int i) {
    /* Operations causing implicit mode changes */
    long l1 = c;          /* char -> long (zero/sign extend) */
    long l2 = s;          /* short -> long */
    long l3 = i;          /* int -> long */
    
    /* Mixed operations requiring different register sizes */
    long result;
    
    /* Inline asm with clobbered registers to force spills/reloads */
    asm volatile (
        "movsbl %1, %%eax\n\t"    /* Sign extend char */
        "movswl %2, %%ebx\n\t"    /* Sign extend short */
        "movl %3, %%ecx\n\t"      /* Move int */
        "addq %%rax, %%rbx\n\t"
        "addq %%rbx, %%rcx\n\t"
        "movq %%rcx, %0\n\t"
        : "=r" (result)
        : "r" (c), "r" (s), "r" (i)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
    
    return result;
}

/* Function with pointer arithmetic and complex constraints */
__attribute__((noinline))
static int pointer_arithmetic_reloads(volatile int* ptr1, volatile int* ptr2) {
    int diff;
    
    /* Complex address calculation */
    uintptr_t addr1 = (uintptr_t)ptr1 + global_seed;
    uintptr_t addr2 = (uintptr_t)ptr2 - global_seed;
    
    /* Inline asm with multiple alternatives to confuse reload */
    asm volatile (
        "subq %2, %1\n\t"
        "movl %1, %0\n\t"
        : "=r,m" (diff)          /* Multiple output alternatives */
        : "r,m" (addr1), "r,m" (addr2)  /* Multiple input alternatives */
        : "cc", "memory"
    );
    
    return diff;
}

/* Function using unions and bitfields for subreg operations */
__attribute__((noinline))
static int union_bitfield_reloads(void) {
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bits;
        uint32_t full;
    } data;
    
    volatile int seed = global_seed;
    data.full = seed * 0x12345678;
    
    /* Operations on bitfields requiring extractions */
    int result;
    
    asm volatile (
        "movl %1, %%eax\n\t"
        "andl $0xF, %%eax\n\t"      /* Extract bitfield a */
        "movl %1, %%ebx\n\t"
        "shrl $4, %%ebx\n\t"
        "andl $0xFF, %%ebx\n\t"     /* Extract bitfield b */
        "addl %%ebx, %%eax\n\t"
        "movl %1, %%ecx\n\t"
        "shrl $12, %%ecx\n\t"
        "andl $0xFFF, %%ecx\n\t"    /* Extract bitfield c */
        "addl %%ecx, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (data.full)
        : "eax", "ebx", "ecx", "rdx"
    );
    
    return result;
}

/* Main function creating maximum register pressure */
int main(int argc, char** argv) {
    /* Many local variables of different types */
    volatile char c1 = argc > 1 ? argv[1][0] : 'A';
    volatile short s1 = argc * 100;
    volatile int i1 = argc + global_seed;
    volatile long l1 = (long)argc * 1000;
    
    volatile int arr1[64];
    volatile int arr2[32];
    volatile int* ptr1 = &arr1[16];
    volatile int* ptr2 = &arr2[8];
    
    /* Initialize arrays with non-constant values */
    for (int i = 0; i < 64; i++) {
        arr1[i] = global_seed + i * 3;
    }
    for (int i = 0; i < 32; i++) {
        arr2[i] = global_seed - i * 2;
    }
    
    int checksum = 0;
    
    /* Call functions repeatedly to create reload scenarios */
    checksum += use_explicit_registers(i1, argc * 2);
    
    memory_operations_with_reloads(arr1, 16);
    for (int i = 0; i < 16; i++) {
        checksum += arr1[i];
    }
    
    checksum += mix_data_types(c1, s1, i1);
    
    checksum += pointer_arithmetic_reloads(ptr1, ptr2);
    
    checksum += union_bitfield_reloads();
    
    /* Additional complex operations in main */
    {
        /* Mixed size operations */
        long temp = l1;
        for (volatile int i = 0; i < 8; i++) {
            temp += (c1 << i) + (s1 >> i);
        }
        checksum += (int)temp;
        
        /* More inline asm with clobbers */
        asm volatile (
            "movl %0, %%eax\n\t"
            "roll $3, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (checksum)
            :
            : "eax", "ebx", "ecx", "edx", "cc"
        );
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
