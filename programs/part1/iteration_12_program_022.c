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

/* Function with explicit register variables and mismatched constraints */
__attribute__((noinline))
static int register_conflict_test(int a, int b) {
    /* Explicit register variables that conflict with inline asm constraints */
    register int x asm("r12") = a + global_seed;
    register int y asm("r13") = b - global_seed;
    int result;
    
    /* Inline asm with conflicting constraints - input in r12, output in memory */
    asm volatile (
        "movl %1, %%r12d\n\t"          /* Force use of r12 */
        "addl %2, %%r12d\n\t"          /* Add another value */
        "movl %%r12d, %0\n\t"          /* Store result to memory */
        : "=m" (result)                /* Output constraint forces memory */
        : "r" (x), "r" (y)             /* Inputs in registers */
        : "r12", "r13", "cc", "memory"
    );
    
    return result;
}

/* Function with volatile addresses and immediate inputs */
__attribute__((noinline))
static void memory_operand_test(volatile int* arr, int size) {
    volatile int temp = global_seed;
    int* complex_addr;
    
    for (volatile int i = 0; i < size; i = i + 1) {
        /* Create complex address calculation */
        complex_addr = create_complex_address(arr, i);
        
        /* Inline asm with memory output and immediate input */
        asm volatile (
            "movl %1, (%0)\n\t"        /* Store immediate to memory */
            : 
            : "r" (complex_addr), "i" (0x12345678 + i)
            : "memory", "cc"
        );
        
        /* Clobber many registers around the operation */
        asm volatile (
            ""
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
    }
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static long mixed_type_test(char c, short s, int i, long l) {
    volatile char vc = c;
    volatile short vs = s;
    volatile int vi = i;
    volatile long vl = l;
    
    /* Operations causing mode changes */
    long result = 0;
    
    /* char -> long extension */
    result += (long)vc * 256;
    
    /* short -> long with arithmetic */
    result += (long)vs << 8;
    
    /* int -> long with bit operations */
    result |= (long)vi << 32;
    
    /* Mix with pointer arithmetic */
    volatile long* ptr = &vl;
    result += (long)(ptr + (vc & 0x3));
    
    /* Inline asm with mixed size constraints */
    asm volatile (
        "movsx %1, %0\n\t"             /* Sign extend byte to long */
        "add %2, %0\n\t"               /* Add word */
        "add %3, %0\n\t"               /* Add doubleword */
        : "=r" (result)
        : "r" (vc), "r" (vs), "r" (vi)
        : "cc"
    );
    
    return result;
}

/* Function with multiple alternative constraints */
__attribute__((noinline))
static int alternative_constraint_test(int a, int b, int* mem) {
    int result1, result2;
    
    /* Multiple alternative constraints "r,m" */
    asm volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        : "=r,m" (result1)
        : "0" (a), "r,m" (b), "r,m" (global_seed)
        : "cc"
    );
    
    /* Output to memory, input from register */
    asm volatile (
        "imull %1, %0\n\t"
        : "=m" (*mem)
        : "r" (result1)
        : "cc", "memory"
    );
    
    /* Another with immediate constraint */
    asm volatile (
        "leal (%1, %2, 4), %0\n\t"
        : "=r" (result2)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    return result1 + result2 + *mem;
}

/* Complex function with many local variables */
__attribute__((noinline))
static int complex_register_pressure(int iterations) {
    /* Many local variables of different types */
    char c1 = 1, c2 = 2, c3 = 3;
    short s1 = 100, s2 = 200, s3 = 300;
    int i1 = 1000, i2 = 2000, i3 = 3000;
    long l1 = 10000, l2 = 20000, l3 = 30000;
    volatile int vi1 = 4000, vi2 = 5000;
    int* p1 = &i1;
    int* p2 = &i2;
    int* p3 = &i3;
    
    int sum = 0;
    volatile int counter = 0;
    
    for (counter = 0; counter < iterations; counter = counter + 1) {
        /* Mixed type operations causing conversions */
        l1 = (long)c1 + (long)s1 * 256 + (long)i1;
        l2 = (long)c2 | ((long)s2 << 16) | ((long)i2 << 32);
        
        /* Pointer arithmetic with different scales */
        p1 = p1 + (c1 & 0x7);
        p2 = p2 + (s1 >> 8);
        p3 = (int*)((char*)p3 + c3);
        
        /* Inline asm that clobbers many registers */
        asm volatile (
            ""
            : 
            : "r" (l1), "r" (l2), "r" (l3),
              "r" (vi1), "r" (vi2), "r" (counter)
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* Update variables to prevent elimination */
        c1 = (c1 * 3) & 0xFF;
        s1 = (s1 + 7) & 0xFFFF;
        i1 = i1 * 5 + 1;
        vi1 = vi1 + counter;
        
        sum += c1 + s1 + i1 + (int)(l1 & 0xFFFFFFFF);
    }
    
    return sum;
}

int main(int argc, char** argv) {
    int result = 0;
    volatile int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Array with volatile elements for complex addressing */
    volatile int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = seed + i;
    }
    
    /* Test 1: Register conflicts */
    result += register_conflict_test(seed, seed * 2);
    
    /* Test 2: Memory operand with complex addresses */
    memory_operand_test((int*)array, 10);
    result += array[5];
    
    /* Test 3: Mixed type operations */
    result += mixed_type_test('A', 1234, 567890, 987654321);
    
    /* Test 4: Alternative constraints */
    int mem_result = 0;
    result += alternative_constraint_test(seed, seed + 100, &mem_result);
    
    /* Test 5: Complex register pressure */
    result += complex_register_pressure(20);
    
    /* Additional stress: loop with many variables */
    {
        volatile char vc = 'Z';
        volatile short vs = 4321;
        volatile int vi = 87654321;
        volatile long vl = 0x123456789ABCDEF0;
        volatile double vd = 3.14159;
        volatile float vf = 2.71828;
        
        for (volatile int i = 0; i < 5; i++) {
            /* Force different register classes */
            asm volatile (
                "mov %1, %%rax\n\t"
                "add %2, %%rax\n\t"
                "mov %%rax, %0\n\t"
                : "=m" (vl)
                : "r" (vi), "r" ((long)i)
                : "rax", "cc", "memory"
            );
            
            /* Mix integer and floating point */
            vd = vd * (double)vi + (double)vs;
            vf = vf * (float)vc - (float)i;
            
            result += (int)vd + (int)vf + (int)(vl & 0xFFFFFFFF);
        }
    }
    
    /* Compute final checksum */
    for (int i = 0; i < 100; i++) {
        result += array[i];
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
