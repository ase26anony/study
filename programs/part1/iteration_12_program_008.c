#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int func_reg_conflict(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + g_volatile_seed;
    register int y asm("r13") = b - g_volatile_seed;
    register int z asm("r14");
    
    /* Inline asm with multiple alternative constraints and clobbers */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[z]\n\t"
        : [z] "=r,m" (z)          /* Output: register OR memory */
        : [x] "r,m,0" (x),        /* Input: register, memory, or same as output 0 */
          [y] "r,m,i" (y)         /* Input: register, memory, or immediate */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    /* Force mode mismatch: char in 64-bit operation */
    char c = (char)z;
    long long ll_result = (long long)c * (long long)x;
    
    return (int)(ll_result & 0xFFFFFFFF);
}

/* Function using volatile addresses and complex addressing modes */
__attribute__((noinline))
static int func_volatile_addressing(volatile int* arr, int idx) {
    volatile int local_vol = idx * 2;
    volatile char char_vol = (char)(idx + 1);
    
    int result;
    int* volatile ptr_vol = (int*)&local_vol;
    
    /* Complex addressing with volatile index */
    asm volatile (
        "movl (%[arr], %[idx], 4), %%eax\n\t"
        "addl %[local], %%eax\n\t"
        "movzbl %[char_vol], %%ebx\n\t"
        "subl %%ebx, %%eax\n\t"
        "movl %%eax, %[result]\n\t"
        : [result] "=m,r" (result)      /* Output: memory OR register */
        : [arr] "r" (arr),
          [idx] "r" (idx),
          [local] "m,r" (local_vol),    /* Input: memory OR register */
          [char_vol] "m,r" (char_vol)   /* Input: memory OR register */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    /* Pointer arithmetic that may need base+index reload */
    int* ptr = arr + (idx & 3);
    volatile int* volatile_ptr = ptr_vol + (local_vol & 1);
    
    return result + *ptr + *volatile_ptr;
}

/* Function with mixed types and mode conversions */
__attribute__((noinline))
static long func_mixed_types(short s, char c, int i, long l) {
    volatile short vs = s;
    volatile char vc = c;
    
    /* Operations causing mode changes */
    long long ll_temp = (long long)vs * (long long)vc;
    int i_temp = (int)ll_temp;
    
    /* Bit-field like operations */
    union {
        int i;
        struct {
            unsigned short low;
            unsigned short high;
        } parts;
    } converter;
    
    converter.i = i_temp;
    unsigned short us_result = converter.parts.low + converter.parts.high;
    
    /* Inline asm with mismatched operand sizes */
    long final_result;
    asm volatile (
        "movswl %[us], %%eax\n\t"        /* Sign extend short to long */
        "addl %[i], %%eax\n\t"
        "cltq\n\t"                       /* Sign extend eax to rax */
        "addq %[l], %%rax\n\t"
        "movq %%rax, %[result]\n\t"
        : [result] "=r,m" (final_result)
        : [us] "r,m" (us_result),
          [i] "r,m,i" (i),
          [l] "r,m,i" (l)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    return final_result;
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
static int func_high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3, c4 = 4;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    long l1 = 1000, l2 = 2000, l3 = 3000, l4 = 4000;
    volatile int vi = iterations;
    
    /* Force spills with many operations */
    for (volatile int v = 0; v < vi; v++) {
        /* Mixed operations causing mode conversions */
        i1 = (int)c1 + (int)s1;
        i2 = (int)c2 * (int)s2;
        l1 = (long)i1 + (long)i2;
        l2 = (long)s3 * (long)c3;
        
        /* Pointer casts creating complex RTL */
        int* ptr1 = (int*)(uintptr_t)l1;
        int* ptr2 = (int*)(uintptr_t)l2;
        volatile int* vptr = &vi;
        
        /* Inline asm with many clobbered registers */
        asm volatile (
            "movl %[i3], %%eax\n\t"
            "addl %[i4], %%eax\n\t"
            "movl %%eax, %[i1]\n\t"
            : [i1] "=m,r" (i1)
            : [i3] "r,m,i" (i3),
              [i4] "r,m,i" (i4)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "cc", "memory"
        );
        
        /* Update variables to prevent elimination */
        c1 = (char)(i1 & 0xFF);
        s1 = (short)(l1 & 0xFFFF);
    }
    
    return i1 + i2 + (int)(l1 & 0xFFFFFFFF) + (int)(l2 & 0xFFFFFFFF);
}

int main(int argc, char** argv) {
    /* Initialize with volatile to prevent constant folding */
    volatile int seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    
    /* Array with volatile elements for complex addressing */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    int checksum = 0;
    
    /* Call functions repeatedly with different arguments */
    for (int i = 0; i < 8; i++) {
        checksum ^= func_reg_conflict(seed + i, seed - i);
        checksum ^= func_volatile_addressing((int*)arr, i & 15);
        checksum ^= (int)func_mixed_types(
            (short)(seed + i), 
            (char)(seed - i), 
            seed * i, 
            (long)seed << i
        );
        
        if (i % 2 == 0) {
            checksum += func_high_register_pressure(i + 1);
        }
    }
    
    /* Additional stress: nested function calls with many arguments */
    {
        char c = 'A';
        short s = 12345;
        int i = 0x12345678;
        long l = 0x87654321;
        
        for (volatile int j = 0; j < 4; j++) {
            checksum += (int)func_mixed_types(s + j, c + j, i + j, l + j);
        }
    }
    
    /* Final computation using all modified values */
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
