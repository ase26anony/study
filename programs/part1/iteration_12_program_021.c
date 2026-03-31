/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent optimization of variables */
static volatile int vol_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void use_complex_addressing(volatile int* arr, int idx) {
    /* Force base+index addressing with reloads */
    register int rbx_val asm("rbx") = idx * 2;
    register int r12_val asm("r12") = vol_seed;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[imm], %[mem]\n\t"
        : [mem] "=m" (arr[rbx_val + r12_val])
        : [imm] "i" (5), "m" (arr[rbx_val + r12_val])
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r12", "cc", "memory"
    );
    
    /* Mixed mode operation */
    char char_var = (char)(arr[idx] & 0xFF);
    long long_var = (long)char_var * 256;
    
    /* Another asm with register/memory alternatives */
    asm volatile (
        "mov %[in], %%rax\n\t"
        "add %%rax, %[out]\n\t"
        : [out] "+m" (arr[idx])
        : [in] "rm" (long_var)
        : "rax", "cc"
    );
}

/* Function with explicit register variables and conflicts */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    /* Explicit register variables that conflict with asm clobbers */
    register int x asm("r10") = a + vol_seed;
    register int y asm("r11") = b - vol_seed;
    register int z asm("r12") = 0;
    
    /* Multiple asm statements with overlapping clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (z)
        : "r" (x), "r" (y)
        : "rax", "cc"
    );
    
    /* Force reload by using value in different mode */
    short short_result = (short)z;
    int int_result = (int)short_result * 2;
    
    /* Clobber many registers around operation */
    asm volatile (
        "push %%rbx\n\t"
        "mov %1, %%ebx\n\t"
        "imul %%ebx, %0\n\t"
        "pop %%rbx\n\t"
        : "+r" (int_result)
        : "r" (vol_seed)
        : "cc", "memory"
    );
    
    return int_result;
}

/* Function with pointer arithmetic and volatile */
__attribute__((noinline))
static void pointer_arithmetic(volatile int* ptr, volatile char* cptr) {
    /* Complex address calculation */
    volatile long offset = (volatile long)(vol_seed & 0xF);
    volatile int* complex_ptr = ptr + offset;
    
    /* Inline asm with memory output and immediate input */
    asm volatile (
        "movl $0x1234, %0\n\t"
        : "=m" (*complex_ptr)
        :
        : "memory"
    );
    
    /* Mixed type pointer casting */
    uintptr_t intptr = (uintptr_t)cptr;
    intptr += (offset * sizeof(int));
    
    /* Access through casted pointer */
    volatile int* aliased = (volatile int*)intptr;
    
    /* Asm with multiple alternatives */
    asm volatile (
        "mov %1, %%eax\n\t"
        "or %%eax, %0\n\t"
        : "+m" (*aliased)
        : "r" (vol_seed)
        : "rax", "cc", "memory"
    );
}

/* Function with bitfields and unions */
__attribute__((noinline))
static int mixed_types_operations(int val) {
    union {
        struct {
            unsigned short a : 4;
            unsigned short b : 12;
        } bits;
        unsigned short full;
    } u;
    
    u.full = (unsigned short)val;
    
    /* Operations causing mode changes */
    char a_char = (char)u.bits.a;
    int a_extended = (int)a_char;  /* Zero/sign extension may need reload */
    
    /* Inline asm with subreg-like behavior */
    register int reg_val asm("r13") = a_extended;
    
    asm volatile (
        "movzbl %%r13b, %%eax\n\t"  /* Zero extend byte */
        "addl %%eax, %0\n\t"
        : "+m" (u.full)
        : 
        : "rax", "r13", "cc"
    );
    
    /* Loop with volatile counter to prevent optimization */
    volatile int i;
    int sum = 0;
    for (i = 0; i < 3; i++) {
        /* Mixed mode operation in loop */
        short temp = (short)(u.full + i);
        sum += (int)temp;
        
        /* Asm that clobbers registers */
        asm volatile (
            "add $1, %0\n\t"
            : "+r" (sum)
            :
            : "cc"
        );
    }
    
    return sum;
}

/* Main function that creates maximum register pressure */
int main(int argc, char* argv[]) {
    /* Use args to prevent constant propagation */
    int arg_base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Many local variables of different types */
    volatile int var1 = arg_base + 1;
    volatile char var2 = (char)(arg_base + 2);
    volatile short var3 = (short)(arg_base + 3);
    volatile long var4 = (long)(arg_base + 4);
    volatile int* ptr1 = &var1;
    volatile char* ptr2 = &var2;
    
    /* Array with volatile index */
    int array[100];
    volatile int idx = vol_seed % 50;
    
    /* Initialize array with non-constant values */
    for (int i = 0; i < 100; i++) {
        array[i] = arg_base + i + vol_seed;
    }
    
    /* Call functions repeatedly to increase reload opportunities */
    int result1 = 0, result2 = 0, result3 = 0;
    
    for (volatile int iter = 0; iter < 5; iter++) {
        use_complex_addressing(array, idx + iter);
        
        result1 = register_conflicts(var1 + iter, var3 - iter);
        
        pointer_arithmetic(ptr1, ptr2);
        
        result2 = mixed_types_operations(var4 + result1);
        
        /* More mixed operations */
        char char_temp = (char)(array[idx] & 0xFF);
        int int_temp = (int)char_temp;
        
        /* Asm with immediate to memory with register constraint */
        asm volatile (
            "movl %1, %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%ecx\n\t"
            "movl %%ecx, %0\n\t"
            : "=m" (array[idx + 10])
            : "r" (int_temp)
            : "rax", "rcx", "cc", "memory"
        );
        
        result3 += result2 + array[idx];
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = var1 + var2 + var3 + (int)var4;
    checksum += result1 + result2 + result3;
    
    for (int i = 0; i < 10; i++) {
        checksum += array[i * 5];
    }
    
    /* Final asm with many clobbers */
    asm volatile (
        "mov %0, %%eax\n\t"
        "add $0xABCD, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "+m" (checksum)
        :
        : "rax", "cc"
    );
    
    printf("Result: %d (Seed: %d)\n", checksum, vol_seed);
    return checksum & 0xFF;
}
