/* reload_test.c - Test program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent constant propagation */
volatile int g_volatile_int = 42;
volatile long g_volatile_long = 123456789L;
volatile double g_volatile_double = 3.14159;
volatile char* g_volatile_ptr = "test";

/* Large global array with large offsets */
int global_array[10000];

/* Non-inline function with many arguments to force register pressure */
int __attribute__((noinline)) many_args_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    double f1, double f2, double f3, void* p1, void* p2)
{
    /* Complex computation using all arguments */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    double product = f1 * f2 * f3;
    return sum + (int)product + (int)(long)p1 + (int)(long)p2;
}

/* Test 1: Extreme register pressure with many live scalars */
int __attribute__((noinline)) test_register_pressure(int seed)
{
    /* Use volatile inputs to prevent optimization */
    volatile int a = seed + 1;
    volatile int b = seed + 2;
    volatile int c = seed + 3;
    volatile int d = seed + 4;
    volatile int e = seed + 5;
    volatile int f = seed + 6;
    volatile int g = seed + 7;
    volatile int h = seed + 8;
    volatile int i = seed + 9;
    volatile int j = seed + 10;
    
    /* Many independent computations creating many live temporaries */
    int t1 = a + b;
    int t2 = c + d;
    int t3 = e + f;
    int t4 = g + h;
    int t5 = i + j;
    int t6 = t1 * t2;
    int t7 = t3 * t4;
    int t8 = t5 * t1;
    int t9 = t2 * t3;
    int t10 = t4 * t5;
    int t11 = t6 + t7;
    int t12 = t8 + t9;
    int t13 = t10 + t6;
    int t14 = t7 + t8;
    int t15 = t9 + t10;
    int t16 = t11 * t12;
    int t17 = t13 * t14;
    int t18 = t15 * t11;
    int t19 = t12 * t13;
    int t20 = t14 * t15;
    
    /* More computations to exceed any reasonable register count */
    int t21 = t16 + t17;
    int t22 = t18 + t19;
    int t23 = t20 + t16;
    int t24 = t17 + t18;
    int t25 = t19 + t20;
    int t26 = t21 * t22;
    int t27 = t23 * t24;
    int t28 = t25 * t21;
    int t29 = t22 * t23;
    int t30 = t24 * t25;
    
    /* Force all values to be used */
    return t26 + t27 + t28 + t29 + t30;
}

/* Test 2: Complex addressing modes requiring reloads */
int __attribute__((noinline)) test_complex_addressing(int* base, int index)
{
    int result = 0;
    
    /* Large immediate offset that may not fit in addressing mode */
    result += base[4096];           /* Large constant offset */
    result += base[index * 16];     /* Scaled index */
    
    /* Complex index computation */
    int complex_index = (index * 3 + 7) / 2;
    result += base[complex_index + 1024];  /* Constant + variable */
    
    /* Multi-dimensional-like access */
    result += *(base + index * 32 + 2048);
    
    /* Access with multiple computations */
    result += base[(index << 4) | 0x3F];
    
    /* Misaligned access simulation for 64-bit values */
    long long* llptr = (long long*)base;
    result += (int)llptr[index];  /* May require multiple registers */
    
    /* Double register indirect-like pattern */
    result += *(base + (index & 0xFF) + ((index >> 8) & 0xFF));
    
    return result;
}

/* Test 3: Inline assembly with many clobbered registers */
int __attribute__((noinline)) test_asm_clobber(int x, int y)
{
    int a = x * 3;
    int b = y * 7;
    int c = a + b;
    int d = a * b;
    
    /* Inline assembly that clobbers many registers */
    /* For x86_64 - clobber general purpose, segment, and flags */
    asm volatile (
        "# Start of clobber block\n"
        "mov %0, %%eax\n"
        "mov %1, %%ebx\n"
        :
        : "r"(c), "r"(d)
        : "eax", "ebx", "ecx", "edx", "esi", "edi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory", "cc"
    );
    
    /* More computations after clobber - forces reloads */
    int e = a * 11;
    int f = b * 13;
    int g = c * 17;
    int h = d * 19;
    
    /* Another assembly block with different clobbers */
    asm volatile (
        "# Second clobber block\n"
        :
        :
        : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15"
    );
    
    return e + f + g + h;
}

/* Test 4: Mixed types and calling conventions */
double __attribute__((noinline)) test_mixed_types(int a, double b, long c, float d)
{
    /* Use explicit register variables to force specific register allocation */
    register int r1 asm("r10") = a * 2;
    register int r2 asm("r11") = a * 3;
    
    /* Many floating point computations */
    double f1 = b * 1.1;
    double f2 = b * 2.2;
    double f3 = b * 3.3;
    double f4 = b * 4.4;
    double f5 = b * 5.5;
    double f6 = b * 6.6;
    double f7 = b * 7.7;
    double f8 = b * 8.8;
    
    /* Mix with integer computations */
    long l1 = c + r1;
    long l2 = c + r2;
    long l3 = l1 * l2;
    long l4 = l2 * l1;
    
    /* Call function with many arguments - forces register pressure */
    int result = many_args_function(
        r1, r2, (int)l1, (int)l2, (int)f1,
        (int)f2, (int)f3, (int)f4, (int)f5, (int)f6,
        f7, f8, b, (void*)l3, (void*)l4
    );
    
    return f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + result;
}

/* Test 5: Structure copying and misaligned accesses */
struct LargeStruct {
    int a;
    double b;
    long c;
    float d;
    short e;
    char f[7];
    int g;
    double h;
};

int __attribute__((noinline)) test_struct_ops(struct LargeStruct* s1, struct LargeStruct* s2)
{
    /* Structure copy - may generate multiple moves */
    *s2 = *s1;
    
    /* Access misaligned fields */
    int* misaligned_int = (int*)((char*)&s1->b + 1);  /* Misaligned int access */
    int temp = *misaligned_int;  /* May require special handling */
    
    /* Compute with structure fields */
    double sum = s1->b + s2->h;
    long product = s1->c * s2->c;
    
    return (int)sum + (int)product + temp + s1->a + s2->g;
}

/* Main function that runs all tests */
int main(int argc, char** argv)
{
    int result = 0;
    
    /* Use argv to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 1000;
    
    printf("Starting reload pass stress tests...\n");
    
    /* Test 1: Register pressure */
    printf("Test 1 - Register pressure: ");
    result += test_register_pressure(seed);
    printf("done\n");
    
    /* Test 2: Complex addressing */
    printf("Test 2 - Complex addressing: ");
    /* Initialize global array with some values */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * seed;
    }
    result += test_complex_addressing(global_array, seed % 50);
    printf("done\n");
    
    /* Test 3: Assembly clobber */
    printf("Test 3 - Assembly clobber: ");
    result += test_asm_clobber(seed, seed * 2);
    printf("done\n");
    
    /* Test 4: Mixed types */
    printf("Test 4 - Mixed types: ");
    double dresult = test_mixed_types(seed, seed * 1.5, seed * 100L, seed * 0.25f);
    result += (int)dresult;
    printf("done\n");
    
    /* Test 5: Structure operations */
    printf("Test 5 - Structure operations: ");
    struct LargeStruct s1 = {seed, seed * 2.0, seed * 3L, seed * 4.0f, 
                            (short)seed, "abcdef", seed * 5, seed * 6.0};
    struct LargeStruct s2;
    result += test_struct_ops(&s1, &s2);
    printf("done\n");
    
    printf("All tests completed. Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result == 0 ? 1 : 0;
}
