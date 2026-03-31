#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void complex_addressing(volatile int* arr, int idx1, int idx2) {
    /* Force base+index addressing with reloads */
    register int r12_val asm("r12") = idx1;
    register int r13_val asm("r13") = idx2;
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[idx2], %[arr+%[idx1]*4]"
        : [arr] "+m" (arr[r12_val])
        : [idx1] "r" (r12_val), [idx2] "rm" (r13_val)
        : "memory", "cc", "rax", "rbx", "rcx", "rdx"
    );
    
    /* Another asm with immediate to memory */
    asm volatile (
        "movl $0x12345678, %[dest]"
        : [dest] "=m" (arr[r13_val])
        :
        : "memory"
    );
}

/* Function with explicit register variables and clobbers */
__attribute__((noinline))
static int register_pressure(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables competing for registers */
    register int x asm("r10") = a;
    register int y asm("r11") = b;
    register int z asm("r12") = c;
    
    /* Inline asm with many clobbers to force spills */
    asm volatile (
        "imull %[y], %[x] \n\t"
        "addl %[z], %[x] \n\t"
        "movl %[x], %[result]"
        : [result] "=rm" (x)
        : [x] "0" (x), [y] "rm" (y), [z] "rm" (z)
        : "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r13", "r14", "r15"
    );
    
    /* Mixed type operations causing mode changes */
    char char_var = (char)x;
    short short_var = (short)y;
    long long_var = (long)z;
    
    /* Operations that may require zero/sign extension */
    long_var = long_var + (long)char_var * (long)short_var;
    
    /* Pointer arithmetic with volatile */
    volatile int* ptr = &a;
    ptr += (int)char_var;
    
    return (int)long_var + *ptr;
}

/* Function with memory constraints and immediate values */
__attribute__((noinline))
static void memory_constraints(volatile int* mem1, volatile short* mem2, 
                               volatile char* mem3) {
    /* Multiple asm statements with memory outputs */
    asm volatile (
        "movw $0xABCD, %[out]"
        : [out] "=m" (*mem2)
        :
        : "memory"
    );
    
    /* Input is immediate, output is memory with offset */
    asm volatile (
        "movb $0xEF, 1(%[out])"
        : 
        : [out] "r" (mem3)
        : "memory"
    );
    
    /* Complex constraint: register OR memory */
    int temp = g_volatile_seed;
    asm volatile (
        "addl $1, %[val]"
        : [val] "+rm" (temp)
        :
        : "cc"
    );
    
    *mem1 = temp;
}

/* Function with mixed types and conversions */
__attribute__((noinline))
static int64_t mixed_type_ops(char c, short s, int i, long l) {
    /* Operations causing mode changes */
    int64_t result = 0;
    
    /* char in 64-bit operation */
    result += (int64_t)c * 100;
    
    /* short in 64-bit operation with sign extension */
    result += (int64_t)s * 1000;
    
    /* 32-bit int in 64-bit operation */
    result += (int64_t)i * 10000;
    
    /* Pointer cast causing potential reloads */
    uintptr_t ptr_val = (uintptr_t)&result;
    result += (int64_t)(ptr_val & 0xFF);
    
    /* Union to force type punning */
    union {
        int32_t i;
        float f;
    } u;
    u.i = i;
    result += (int64_t)u.i;
    
    /* Bit-field operations */
    struct {
        int a : 4;
        int b : 8;
        int c : 12;
    } bits = {c & 0xF, s & 0xFF, i & 0xFFF};
    
    result += bits.a + bits.b + bits.c;
    
    return result;
}

/* Main function creating maximum register pressure */
int main(int argc, char* argv[]) {
    /* Use argc to prevent constant folding */
    volatile int arg_base = argc;
    
    /* Many local variables of different types */
    char c1 = arg_base + 1;
    char c2 = arg_base + 2;
    short s1 = arg_base * 10;
    short s2 = arg_base * 20;
    int i1 = arg_base * 100;
    int i2 = arg_base * 200;
    int i3 = arg_base * 300;
    int i4 = arg_base * 400;
    int i5 = arg_base * 500;
    int i6 = arg_base * 600;
    long l1 = arg_base * 1000L;
    long l2 = arg_base * 2000L;
    
    /* Volatile variables to prevent optimizations */
    volatile int v1 = g_volatile_seed;
    volatile short v2 = g_volatile_seed;
    volatile char v3 = g_volatile_seed;
    
    /* Arrays for complex addressing */
    int arr[100];
    for (int j = 0; j < 100; j++) {
        arr[j] = j + arg_base;
    }
    
    /* Call functions multiple times with different args */
    int sum = 0;
    
    /* Create register pressure */
    sum += register_pressure(i1, i2, i3, i4, i5, i6);
    
    /* Complex addressing modes */
    complex_addressing(arr, v1 % 50, v1 % 25);
    
    /* Memory constraints */
    memory_constraints(&v1, &v2, &v3);
    
    /* Mixed type operations in a loop */
    volatile int loop_counter = 10;
    for (volatile int k = 0; k < loop_counter; k++) {
        int64_t res = mixed_type_ops(
            c1 + k, 
            s1 + k, 
            i1 + k, 
            l1 + k
        );
        sum += (int)res;
        
        /* Modify variables to prevent loop unrolling */
        c1 += (char)res;
        s1 += (short)res;
    }
    
    /* More operations to use all variables */
    sum += arr[v1 % 100];
    sum += v2;
    sum += v3;
    sum += (int)l2;
    
    /* Pointer arithmetic with volatile */
    int* ptr1 = &i1;
    int* ptr2 = &i2;
    ptr1 += (int)c1;
    ptr2 += (int)c2;
    sum += *ptr1 + *ptr2;
    
    /* Final checksum to ensure no dead code elimination */
    printf("Checksum: %d\n", sum);
    
    return sum != 0 ? 0 : 1;
}
