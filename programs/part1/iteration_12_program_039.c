#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to force register pressure with explicit register variables */
__attribute__((noinline))
int func_explicit_registers(int a, int b, int c, int d, int e, int f) {
    /* Explicit register variables - these compete for specific registers */
    register int x1 asm("r12") = a + 1;
    register int x2 asm("r13") = b + 2;
    register int x3 asm("r14") = c + 3;
    register int x4 asm("r15") = d + 4;
    
    /* Mixed-type operations causing mode changes */
    char c1 = (char)(a & 0xFF);
    short s1 = (short)(b & 0xFFFF);
    long long ll1 = (long long)c * 1000LL;
    
    /* Inline assembly with conflicting constraints */
    /* Output is memory, inputs are registers - may force reloads */
    int result1, result2;
    asm volatile (
        "movl %[in1], %[out1]\n\t"
        "addl %[in2], %[out1]\n\t"
        "movl %[out1], %[out2]"
        : [out1] "=m" (result1), [out2] "=r" (result2)
        : [in1] "r" (x1), [in2] "r" (x2)
        : "cc", "rax", "rbx", "rcx", "rdx", "rsi", "rdi"
    );
    
    /* More assembly with alternative constraints */
    long long ll_result;
    asm volatile (
        "movq %[in], %[out]"
        : [out] "=r,m" (ll_result)
        : [in] "r,m" (ll1)
        : "cc"
    );
    
    return result1 + result2 + (int)ll_result + x3 + x4 + c1 + s1;
}

/* Function using volatile addresses and complex addressing */
__attribute__((noinline))
int func_volatile_addressing(volatile int* base, int index) {
    volatile char c_arr[64];
    volatile short s_arr[64];
    volatile int i_arr[64];
    volatile long long ll_arr[64];
    
    /* Prevent constant folding of addresses */
    int idx = index + g_volatile_seed;
    
    /* Complex address calculations that may need reloads */
    int* ptr1 = (int*)((char*)base + idx * sizeof(int));
    short* ptr2 = (short*)((char*)&s_arr[0] + idx * sizeof(short));
    char* ptr3 = (char*)((char*)&c_arr[0] + idx * sizeof(char));
    
    /* Inline assembly with memory output and immediate input */
    /* This mismatch can force reloads */
    int temp1, temp2;
    asm volatile (
        "movl $0x12345678, %[out1]\n\t"
        "addl %%eax, %[out2]"
        : [out1] "=m" (temp1), [out2] "=m" (temp2)
        : "a" (idx)
        : "cc", "rbx", "rcx", "rdx"
    );
    
    /* Mixed-type accesses causing mode conversions */
    c_arr[idx] = (char)(temp1 & 0xFF);
    s_arr[idx] = (short)(temp2 & 0xFFFF);
    i_arr[idx] = temp1 + temp2;
    ll_arr[idx] = (long long)temp1 * (long long)temp2;
    
    /* More assembly with clobbered registers */
    long long ll_temp;
    asm volatile (
        "movq %[addr], %%rax\n\t"
        "movq (%%rax), %[out]"
        : [out] "=r" (ll_temp)
        : [addr] "r" (&ll_arr[idx])
        : "rax", "cc"
    );
    
    return (int)ll_temp + c_arr[idx] + s_arr[idx] + i_arr[idx];
}

/* Function with mixed data types and operations */
__attribute__((noinline))
int func_mixed_types(int iterations) {
    volatile int vi = g_volatile_seed;  /* Volatile to prevent optimization */
    char c = 'A';
    short s = 1000;
    int i = 100000;
    long long ll = 1000000000LL;
    void* ptr = &vi;
    
    /* Union causing type punning - may create complex RTL */
    union {
        int i;
        float f;
        char c[4];
    } u;
    u.i = vi;
    
    /* Loop with volatile counter to prevent optimization */
    volatile int counter = iterations;
    int sum = 0;
    
    while (counter-- > 0) {
        /* Mixed-type operations causing mode changes */
        int temp = (int)c + (int)s + i + (int)(ll >> 32);
        
        /* Bit-field operations */
        struct {
            unsigned int a : 3;
            unsigned int b : 5;
            unsigned int c : 8;
        } bf;
        bf.a = c & 0x7;
        bf.b = s & 0x1F;
        bf.c = i & 0xFF;
        
        /* Pointer arithmetic with type casting */
        uintptr_t addr = (uintptr_t)ptr + counter * sizeof(int);
        int* int_ptr = (int*)addr;
        
        /* Inline assembly with multiple constraints */
        int asm_result;
        asm volatile (
            "movl %[in1], %[out]\n\t"
            "imull %[in2], %[out]"
            : [out] "=r,m" (asm_result)
            : [in1] "r,m" (temp), [in2] "r,m" (bf.c)
            : "cc", "rax", "rdx"
        );
        
        sum += asm_result + bf.a + bf.b + (int)(addr & 0xFFFF);
        
        /* Change types to force different machine modes */
        c = (char)((c + 1) & 0xFF);
        s = (short)((s * 3) & 0xFFFF);
        i = i ^ (i >> 1);
        ll = ll * 3 + counter;
    }
    
    return sum + u.i;
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
int func_high_register_pressure(int a, int b, int c, int d, int e, int f,
                                int g, int h, int i, int j, int k, int l) {
    /* Many local variables competing for registers */
    int v1 = a + b;
    int v2 = b + c;
    int v3 = c + d;
    int v4 = d + e;
    int v5 = e + f;
    int v6 = f + g;
    int v7 = g + h;
    int v8 = h + i;
    int v9 = i + j;
    int v10 = j + k;
    int v11 = k + l;
    int v12 = l + a;
    
    /* Different data types */
    char c1 = (char)v1;
    short s1 = (short)v2;
    long long ll1 = (long long)v3 * v4;
    
    /* Array with volatile access */
    volatile int arr[16];
    for (int idx = 0; idx < 16; idx++) {
        arr[idx] = idx + v1 + v2;
    }
    
    /* Complex expression with many intermediate values */
    int result = (((v1 * v2) + (v3 * v4) - (v5 * v6)) / (v7 + 1)) +
                 (((v8 * v9) + (v10 * v11) - (v12 * v1)) / (v2 + 1)) +
                 c1 + s1 + (int)ll1;
    
    /* Inline assembly that clobbers many registers */
    asm volatile (
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "movl %2, %%ecx\n\t"
        "movl %3, %%edx\n\t"
        "addl %%ebx, %%eax\n\t"
        "addl %%ecx, %%eax\n\t"
        "addl %%edx, %%eax"
        : 
        : "r" (v1), "r" (v2), "r" (v3), "r" (v4)
        : "rax", "rbx", "rcx", "rdx", "cc"
    );
    
    return result + arr[0] + arr[15];
}

int main(int argc, char** argv) {
    /* Use arguments to prevent constant propagation */
    int arg1 = argc > 1 ? atoi(argv[1]) : 1;
    int arg2 = argc > 2 ? atoi(argv[2]) : 2;
    int arg3 = argc > 3 ? atoi(argv[3]) : 3;
    
    /* Initialize volatile variable */
    g_volatile_seed = arg1;
    
    /* Call functions multiple times to increase reload opportunities */
    int sum = 0;
    
    sum += func_explicit_registers(arg1, arg2, arg3, arg1+1, arg2+1, arg3+1);
    
    volatile int volatile_array[100];
    for (int i = 0; i < 100; i++) {
        volatile_array[i] = i + arg1;
    }
    sum += func_volatile_addressing(volatile_array, arg2);
    
    sum += func_mixed_types(arg3 + 5);
    
    /* Call high register pressure function with many arguments */
    sum += func_high_register_pressure(
        arg1, arg2, arg3, arg1+1, arg2+1, arg3+1,
        arg1+2, arg2+2, arg3+2, arg1+3, arg2+3, arg3+3
    );
    
    /* Additional complex operations in main */
    {
        /* Mixed pointer/integer operations */
        uintptr_t ptr_val = (uintptr_t)&sum;
        int* int_ptr = (int*)(ptr_val + (arg1 & 0xF));
        
        /* Inline assembly with memory constraint */
        int temp;
        asm volatile (
            "movl %[in], %[out]"
            : [out] "=m" (temp)
            : [in] "i" (0xDEADBEEF)
            : "cc"
        );
        
        sum += temp + (int)(ptr_val & 0xFFFFFFFF);
    }
    
    /* Compute checksum to prevent elimination */
    printf("Result checksum: %d\n", sum);
    
    return sum != 0 ? 0 : 1;
}
