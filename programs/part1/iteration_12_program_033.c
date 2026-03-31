#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static void complex_address_calc(volatile int* arr, int idx, int* result) {
    /* Force base+index addressing with potential reloads */
    register int base asm("r12") = (int)(uintptr_t)arr;
    register int index asm("r13") = idx * sizeof(int);
    
    /* Inline asm with mismatched constraints */
    asm volatile (
        "addl %[idx], %[base]\n\t"
        "movl (%[base]), %[res]"
        : [res] "=r" (*result)
        : [base] "r" (base), [idx] "r" (index)
        : "memory"
    );
}

/* Function with explicit register variables and clobbers */
__attribute__((noinline))
static void register_conflict_ops(int a, int b, int* out1, int* out2) {
    /* Explicit register variables that conflict with constraints */
    register int x asm("r10") = a;
    register int y asm("r11") = b;
    
    /* Multiple alternative constraints forcing reloads */
    asm volatile (
        "movl %[x], %%eax\n\t"
        "addl %[y], %%eax\n\t"
        "movl %%eax, %[o1]\n\t"
        "imull %[x], %[y]\n\t"
        "movl %[y], %[o2]"
        : [o1] "=m" (*out1), [o2] "=m" (*out2)
        : [x] "r,m" (x), [y] "r,m" (y)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static void mixed_type_operations(volatile char c, volatile short s, 
                                  volatile long l, int* results) {
    /* Operations causing mode changes */
    long temp1 = (long)c + (long)s;  /* char/short to long extension */
    int temp2 = (int)l + (int)c;     /* long to int truncation */
    short temp3 = (short)(temp1 + temp2); /* mixed mode operation */
    
    /* Inline asm with memory constraints */
    asm volatile (
        "movsx %[c], %%eax\n\t"
        "movsx %[s], %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "movl %%eax, %[r1]\n\t"
        "mov %[l], %%rcx\n\t"
        "movsx %[c], %%edx\n\t"
        "addl %%edx, %%ecx\n\t"
        "movl %%ecx, %[r2]"
        : [r1] "=m" (results[0]), [r2] "=m" (results[1])
        : [c] "r,m" ((int)c), [s] "r,m" ((int)s), [l] "r,m" (l)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
    );
    
    results[2] = temp3;
}

/* Function with pointer arithmetic and volatile addresses */
__attribute__((noinline))
static void pointer_reload_test(volatile int* ptr, int offset, int* output) {
    volatile int local_volatile = g_volatile_seed;
    int* volatile_ptr = (int*)(uintptr_t)ptr + offset + local_volatile;
    
    /* Complex addressing that may require reloads */
    asm volatile (
        "movl (%[ptr]), %%eax\n\t"
        "addl $0x1234, %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (*output)
        : [ptr] "r" (volatile_ptr)
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
}

/* Function creating many local variables to increase register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3;
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000;
    long l1 = 10000, l2 = 20000, l3 = 30000;
    volatile int vi = g_volatile_seed;
    
    int* ptrs[10];
    int values[10];
    
    /* Complex loop with mixed operations */
    for (volatile int v = 0; v < iterations; v++) {
        /* Force reloads through type mixing */
        i1 = (int)c1 + (int)s1 + vi;
        i2 = (int)c2 * (int)s2 - vi;
        i3 = (int)c3 | (int)s3 ^ vi;
        
        /* Pointer operations */
        ptrs[v % 10] = &values[v % 10];
        *ptrs[v % 10] = i1 + i2 + i3 + (int)(l1 >> 32) + (int)l2;
        
        /* Mode changes */
        l1 = (long)i1 << 16;
        l2 = (long)i2 << 8;
        l3 = (long)i3;
        
        /* Inline asm with many clobbers */
        asm volatile (
            "mov %[i1], %%eax\n\t"
            "add %[i2], %%eax\n\t"
            "add %[i3], %%eax\n\t"
            "mov %%eax, %[val]"
            : [val] "=m" (values[v % 10])
            : [i1] "r,m" (i1), [i2] "r,m" (i2), [i3] "r,m" (i3)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "cc", "memory"
        );
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int j = 0; j < 10; j++) {
        sum += values[j] + (int)(uintptr_t)ptrs[j];
    }
    
    return sum + i1 + i2 + i3 + (int)l1 + (int)l2 + (int)l3;
}

int main(int argc, char** argv) {
    /* Initialize with non-constant values */
    int iter = (argc > 1) ? atoi(argv[1]) : 100;
    if (iter < 10) iter = 10;
    
    /* Create arrays with volatile elements */
    volatile int volatile_array[100];
    for (int i = 0; i < 100; i++) {
        volatile_array[i] = g_volatile_seed + i;
    }
    
    /* Variables for results */
    int result1, result2;
    int mixed_results[3];
    int ptr_result;
    
    /* Call functions repeatedly to trigger reloads */
    int total_sum = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Vary parameters to prevent optimization */
        g_volatile_seed += i;
        
        /* Test 1: Complex addressing */
        complex_address_calc(volatile_array, i % 50, &result1);
        total_sum += result1;
        
        /* Test 2: Register conflicts */
        register_conflict_ops(i, i * 2, &result1, &result2);
        total_sum += result1 + result2;
        
        /* Test 3: Mixed type operations */
        mixed_type_operations((char)(i & 0xFF), (short)(i * 10), 
                             (long)i * 100, mixed_results);
        total_sum += mixed_results[0] + mixed_results[1] + mixed_results[2];
        
        /* Test 4: Pointer reloads */
        pointer_reload_test(volatile_array, i % 20, &ptr_result);
        total_sum += ptr_result;
        
        /* Test 5: High register pressure */
        total_sum += high_register_pressure(5);
    }
    
    /* Ensure results are used */
    printf("Total checksum: %d\n", total_sum);
    
    return total_sum != 0 ? 0 : 1;
}
