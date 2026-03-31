/* caller-save-test.c - Test program to trigger uncovered code in caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern void opaque_func2(int) __attribute__((noinline, noclone));
extern int opaque_func3(int, int) __attribute__((noinline, noclone));
extern double opaque_func4(double) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int g_volatile_int = 12345;
volatile double g_volatile_double = 3.14159;
volatile long g_volatile_long = 987654321L;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_ptrs[4];

/* Complex control flow with register pressure */
__attribute__((noinline, noclone))
static void test1(int mode) {
    /* Force many live variables across calls */
    register int r1 asm ("r10") = g_volatile_int;
    register int r2 asm ("r11") = r1 * 2;
    volatile int stack_var1 = r2;
    volatile int stack_var2 = r1 + 1;
    volatile int stack_var3 = r1 - 1;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile ("# test1 asm1" : : : "r10", "r11", "r12", "r13", "memory");
    
    /* Function call with many live values */
    opaque_func2(r1);
    
    /* Use all registers after call - forces caller-save */
    int sum = r1 + r2 + stack_var1 + stack_var2 + stack_var3;
    
    /* Complex control flow with goto to split basic blocks */
    if (mode & 1) {
        asm volatile ("# test1 branch1" : : : "memory");
        goto label1;
    } else {
        asm volatile ("# test1 branch2" : : : "memory");
    }
    
    /* Another call with different register usage */
    opaque_func1();
    
label1:
    /* Use volatile to prevent dead code elimination */
    g_volatile_int = sum;
    
    /* Switch with default case that calls function */
    switch (mode % 4) {
        case 0:
            r1 = opaque_func3(r1, r2);
            break;
        case 1:
            r2 = opaque_func3(r2, r1);
            break;
        default:
            /* This creates basic block boundary manipulation */
            opaque_func2(r1);
            r1 = r2 * 3;
            /* Force insertion at block end */
            if (r1 > 1000) {
                asm volatile ("# test1 default end" : : : "memory");
                return;
            }
    }
    
    /* Loop with break inside conditional with call */
    for (int i = 0; i < 10; i++) {
        if (i == mode) {
            opaque_func1();
            break;
        }
        r1 += i;
    }
    
    g_volatile_int = r1;
}

/* Test with floating point and mixed types */
__attribute__((noinline, noclone))
static double test2(double input) {
    volatile double v1 = input;
    volatile double v2 = g_volatile_double;
    register double fr1 asm ("xmm0") = v1;
    register double fr2 asm ("xmm1") = v2;
    
    /* Call that clobbers floating point registers */
    double result = opaque_func4(fr1);
    
    /* Complex expression requiring temporary registers */
    double complex_expr = (fr1 * fr2) + (result / fr1) - (v1 * v2);
    
    /* Irreducible control flow with goto */
    if (complex_expr > 100.0) {
        goto fp_label2;
    }
    
    /* Call in one path */
    opaque_func1();
    
    if (complex_expr < 0.0) {
        opaque_func2((int)complex_expr);
        return complex_expr;
    }
    
fp_label2:
    /* Another call at block boundary */
    result = opaque_func4(complex_expr);
    
    /* Loop with nested conditional containing call */
    for (volatile int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            opaque_func1();
            continue;
        }
        result += i;
    }
    
    g_volatile_double = result;
    return result;
}

/* Test with many arguments and __builtin_apply */
__attribute__((noinline, noclone))
static long test3(int a, int b, int c, int d, int e, int f) {
    /* Force register pressure with many live variables */
    volatile int vals[6] = {a, b, c, d, e, f};
    register int r1 asm ("r12") = vals[0];
    register int r2 asm ("r13") = vals[1];
    register int r3 asm ("r14") = vals[2];
    
    /* Memory barrier */
    asm volatile ("# test3 start" : : : "memory", "r12", "r13", "r14");
    
    /* Function call with many arguments - forces register saves */
    int sum1 = opaque_func3(a, b);
    
    /* Use all register variables after call */
    int sum2 = r1 + r2 + r3 + vals[3] + vals[4] + vals[5];
    
    /* Complex control flow with switch and calls */
    switch (sum1 % 3) {
        case 0:
            opaque_func1();
            r1 = sum2;
            break;
        case 1:
            r2 = opaque_func3(sum1, sum2);
            /* Fall through to create block merge */
        case 2:
            opaque_func2(r1);
            r3 = r1 * r2;
            if (BB_END_CONDITION) {  /* Macro to be defined */
                asm volatile ("# test3 case2" : : : "memory");
                goto end_label;
            }
            break;
    }
    
    /* Another call sequence */
    opaque_func1();
    
end_label:
    /* Use __builtin_apply to create unusual call sequence */
    void* args = __builtin_apply_args();
    /* This creates complex reload scenarios */
    
    g_volatile_long = r1 + r2 * 2 + r3 * 3;
    return g_volatile_long;
}

/* Nested calls to create outer/inner save scenarios */
__attribute__((noinline, noclone))
static int test4_helper(int x, int y) {
    volatile int local = x + y;
    /* Call within helper */
    opaque_func2(local);
    
    /* Inline asm with clobbers */
    asm volatile ("# helper asm" : : : "rax", "rcx", "rdx", "memory");
    
    return local * 2;
}

__attribute__((noinline, noclone))
static void test4(int iterations) {
    register int accum asm ("r15") = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Nested call pattern */
        int result = test4_helper(accum, i);
        
        /* Use result immediately in complex expression */
        accum = (accum * 3 + result) / 2;
        
        /* Conditional with call at boundary */
        if (accum > 1000) {
            opaque_func1();
            /* Break creates block end manipulation */
            break;
        } else if (accum < 0) {
            opaque_func2(accum);
            continue;
        }
        
        /* Another call */
        if (i % 3 == 0) {
            opaque_func3(accum, i);
        }
    }
    
    g_volatile_int = accum;
}

/* Function with vector types (if supported) */
#ifdef __SSE2__
#include <emmintrin.h>
__attribute__((noinline, noclone))
static void test5(void) {
    volatile __m128i v1 = _mm_set_epi32(1, 2, 3, 4);
    volatile __m128i v2 = _mm_set_epi32(5, 6, 7, 8);
    
    /* Use vector registers */
    __m128i sum = _mm_add_epi32(v1, v2);
    
    /* Call that might clobber vector regs */
    opaque_func1();
    
    /* Use vector after call */
    volatile int result[4];
    _mm_storeu_si128((__m128i*)result, sum);
    
    g_volatile_int = result[0] + result[3];
}
#endif

/* Dummy implementations of opaque functions to satisfy linker */
void opaque_func1(void) {
    asm volatile ("# opaque1" : : : "memory");
}

void opaque_func2(int x) {
    g_volatile_int += x;
    asm volatile ("# opaque2" : : : "memory", "rax", "rcx");
}

int opaque_func3(int a, int b) {
    asm volatile ("# opaque3" : : : "memory", "r10", "r11");
    return a + b + g_volatile_int;
}

double opaque_func4(double x) {
    asm volatile ("# opaque4" : : : "memory", "xmm0", "xmm1", "xmm2");
    return x * g_volatile_double;
}

int main(int argc, char** argv) {
    /* Initialize function pointers */
    func_ptrs[0] = opaque_func1;
    func_ptrs[1] = (func_ptr_t)opaque_func2;
    func_ptrs[2] = (func_ptr_t)opaque_func3;
    func_ptrs[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Run all tests in sequence, but with mode-dependent order */
    long checksum = 0;
    
    /* Test 1 - Integer register pressure */
    test1(mode);
    checksum += g_volatile_int;
    
    /* Test 2 - Floating point */
    double fp_result = test2(g_volatile_double + mode);
    checksum += (long)fp_result;
    
    /* Test 3 - Many arguments */
    long result3 = test3(mode, mode+1, mode+2, mode+3, mode+4, mode+5);
    checksum += result3;
    
    /* Test 4 - Nested calls */
    test4(3 + mode % 3);
    checksum += g_volatile_int * 2;
    
#ifdef __SSE2__
    /* Test 5 - Vector types */
    test5();
    checksum += g_volatile_int * 3;
#endif
    
    /* Indirect calls through volatile pointer */
    for (int i = 0; i < 2; i++) {
        func_ptrs[i]();
    }
    
    /* Final checksum to prevent elimination */
    printf("Checksum: %ld\n", checksum);
    
    /* Complex final block with goto and call */
    if (checksum > 1000000) {
        goto final_label;
    }
    
    opaque_func2((int)checksum);
    
    if (checksum < 0) {
        return 1;
    }
    
final_label:
    /* One more call at the very end */
    opaque_func1();
    
    return 0;
}
