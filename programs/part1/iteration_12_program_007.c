/* reload_stress.c - Stress GCC's reload pass to trigger rld[i] initialization */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation and optimization */
volatile int g_volatile_seed = 42;

/* Function to create complex addressing modes */
__attribute__((noinline))
static int complex_addressing(int index, volatile int* base) {
    /* Force non-simple address computation */
    volatile int* ptr = base + index;
    int result;
    
    /* Inline asm with memory output and register input constraints */
    __asm__ volatile (
        "movl %1, %0\n\t"
        "addl $7, %0"
        : "=m" (*ptr)      /* Memory output - may need reload */
        : "ri" (index * 2) /* Register/Immediate input - mismatched */
        : "memory"
    );
    
    /* Access through computed pointer */
    result = *ptr;
    
    /* More complex asm with multiple alternatives */
    __asm__ volatile (
        "imull %%eax, %%ecx\n\t"
        "addl %%ecx, %0"
        : "+m" (result)
        : "a" (index), "c" (g_volatile_seed)
        : "cc"
    );
    
    return result;
}

/* Function with explicit register variables and conflicting constraints */
__attribute__((noinline))
static int register_conflicts(int a, int b) {
    /* Explicit register variables that conflict with asm constraints */
    register int x asm("r12") = a;
    register int y asm("r13") = b;
    int result;
    
    /* Inline asm that clobbers the registers we're using */
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)        /* Output in any register */
        : "r" (x), "r" (y)     /* Inputs in registers */
        : "eax", "r12", "r13"  /* Clobber explicit registers */
    );
    
    /* More asm with mismatched modes */
    char char_var = result & 0xFF;
    long long_var;
    
    __asm__ volatile (
        "movsbl %1, %0\n\t"    /* Sign extend char to long */
        "shlq $32, %0"
        : "=r" (long_var)
        : "r" (char_var)
        : "cc"
    );
    
    return (int)(long_var >> 32) + result;
}

/* Function with mixed data types and mode changes */
__attribute__((noinline))
static int mixed_types_ops(short s, char c, int i, long l) {
    /* Operations that cause mode changes */
    int temp1 = s * c;      /* char promoted to int */
    long temp2 = i + l;     /* int promoted to long */
    
    /* Bit-field operations */
    struct {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
    } bits = {s & 0x7, c & 0x1F, i & 0xFF};
    
    /* Union for type punning */
    union {
        uint32_t i;
        float f;
        char bytes[4];
    } converter;
    
    converter.i = (bits.a << 16) | (bits.b << 8) | bits.c;
    
    /* Inline asm with multiple clobbers */
    int result;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "xorl %%ebx, %%ebx\n\t"
        "xorl %%ecx, %%ecx\n\t"
        "xorl %%edx, %%edx\n\t"
        "xorl %%esi, %%esi\n\t"
        "xorl %%edi, %%edi"
        : "=r" (result)
        : "r" (temp1), "r" ((int)temp2)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "cc"
    );
    
    /* Cast between pointer and integer types */
    volatile int* ptr = (volatile int*)(uintptr_t)result;
    *ptr = converter.bytes[0];
    
    return result + *ptr;
}

/* Function with many local variables to increase register pressure */
__attribute__((noinline))
static int high_register_pressure(int iterations) {
    /* Declare many variables of different types */
    char c1 = 1, c2 = 2, c3 = 3, c4 = 4;
    short s1 = 10, s2 = 20, s3 = 30, s4 = 40;
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400;
    long l1 = 1000, l2 = 2000, l3 = 3000, l4 = 4000;
    volatile int v1 = g_volatile_seed;
    
    /* Pointer variables */
    int* p1 = &i1;
    int* p2 = &i2;
    int* p3 = &i3;
    int* p4 = &i4;
    
    /* Array with volatile index */
    int arr[16];
    volatile int idx = iterations % 16;
    
    /* Complex loop with mixed operations */
    for (volatile int j = 0; j < iterations; j++) {
        /* Force address calculations that may need reloads */
        arr[idx + (j & 3)] = 
            (int)c1 + (int)c2 * (int)c3 - (int)c4 +
            s1 * s2 / (s3 + 1) +
            *p1 + *p2 - *p3 + *p4 +
            (int)(l1 >> 32) + (int)l2;
        
        /* Inline asm that uses and clobbers many registers */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            "movl %2, %%ebx\n\t"
            "subl %%ebx, %%eax\n\t"
            : "+r" (arr[idx])
            : "r" (v1), "r" (j)
            : "eax", "ebx", "cc"
        );
        
        /* Update variables to prevent optimization */
        c1 ^= arr[j & 15];
        c2 += j;
        c3 -= arr[(j + 1) & 15];
        c4 |= 0x55;
        
        s1 = (s1 << 1) | (s1 >> 15);
        s2 += s3;
        s3 ^= s4;
        s4 = s4 * 3 + 1;
        
        /* Pointer arithmetic */
        p1 = &arr[(j + 0) & 15];
        p2 = &arr[(j + 1) & 15];
        p3 = &arr[(j + 2) & 15];
        p4 = &arr[(j + 3) & 15];
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int k = 0; k < 16; k++) {
        sum += arr[k];
    }
    
    return sum + c1 + c2 + c3 + c4 + s1 + s2 + s3 + s4;
}

/* Main function that orchestrates all the stress tests */
int main(int argc, char* argv[]) {
    /* Initialize with volatile to prevent constant folding */
    volatile int base_seed = (argc > 1) ? atoi(argv[1]) : g_volatile_seed;
    int result = 0;
    
    /* Create volatile array for complex addressing */
    volatile int vol_array[32];
    for (int i = 0; i < 32; i++) {
        vol_array[i] = base_seed + i * 3;
    }
    
    /* Test 1: Complex addressing modes */
    printf("Test 1 - Complex addressing...\n");
    for (int i = 0; i < 10; i++) {
        result ^= complex_addressing(i, (volatile int*)vol_array);
    }
    
    /* Test 2: Register conflicts */
    printf("Test 2 - Register conflicts...\n");
    for (int i = 0; i < 8; i++) {
        result += register_conflicts(i * 11, i * 13 + 1);
    }
    
    /* Test 3: Mixed type operations */
    printf("Test 3 - Mixed type operations...\n");
    for (int i = 0; i < 6; i++) {
        result += mixed_types_ops(
            i * 100,
            i * 50 + 25,
            i * 1000 + 123,
            (long)i * 10000 + 4567
        );
    }
    
    /* Test 4: High register pressure */
    printf("Test 4 - High register pressure...\n");
    result += high_register_pressure(20);
    
    /* Final computation to ensure nothing is optimized away */
    volatile int final_check = result;
    for (int i = 0; i < 32; i++) {
        final_check += vol_array[i];
    }
    
    printf("Final result: %d (checksum: 0x%08x)\n", 
           result, final_check);
    
    return (final_check != 0) ? 0 : 1;
}
