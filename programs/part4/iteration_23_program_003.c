/* caller-save-test.c
 * Test program designed to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction chain manipulation code at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void *opaque_func4(void *);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32];
volatile double global_fp[16];

/* Function pointer with volatile to prevent devirtualization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr = NULL;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Force register usage with explicit register variables */
#ifdef __x86_64__
register long reg_var1 asm("r10");
register long reg_var2 asm("r11");
register long reg_var3 asm("r12");
register long reg_var4 asm("r13");
#elif defined(__i386__)
register long reg_var1 asm("eax");
register long reg_var2 asm("ebx");
register long reg_var3 asm("esi");
register long reg_var4 asm("edi");
#endif

/* Test function 1: Many live variables across call with register pressure */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live values that must survive across call */
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile int v3 = e + f;
    volatile int v4 = g + h;
    
    /* Use explicit register variables */
    reg_var1 = v1 * 2;
    reg_var2 = v2 * 3;
    reg_var3 = v3 * 4;
    reg_var4 = v4 * 5;
    
    /* Complex expression requiring temporary registers */
    int complex = (reg_var1 * reg_var2) + (reg_var3 * reg_var4) - 
                  (a * b * c * d * e * f * g * h);
    
    /* Opaque call that clobbers registers */
    COMPILER_BARRIER();
    opaque_func1();
    COMPILER_BARRIER();
    
    /* Use values after call - forces save/restore */
    int result = complex + reg_var1 + reg_var2 + reg_var3 + reg_var4;
    
    /* More register pressure with inline asm */
    asm volatile(
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (result)
        : "r" (v1), "r" (v2)
        : "cc", "memory"
    );
    
    /* Another call with different convention */
    result = opaque_func2(result);
    
    /* Force basic block boundary manipulation */
    if (result > 1000) {
        goto special_label;
    } else {
        /* This creates a merge point */
        result += v3 + v4;
    }
    
    return result;
    
special_label:
    /* Uncommon path with another call */
    opaque_func1();
    return result * 2;
}

/* Test function 2: Floating point and mixed mode */
__attribute__((noinline, noclone))
double test2(double a, double b, double c, double d, 
             int i, int j, int k, int l) {
    /* Mix FP and integer live ranges */
    volatile double fp1 = a * b;
    volatile double fp2 = c * d;
    volatile int int1 = i * j;
    volatile int int2 = k * l;
    
    /* Force FP register pressure */
    double temp1 = fp1 + fp2;
    double temp2 = fp1 * fp2;
    double temp3 = fp1 / (fp2 + 1.0);
    
    /* Call that might clobber FP registers */
    COMPILER_BARRIER();
    double fp_result = opaque_func3(temp1);
    COMPILER_BARRIER();
    
    /* Integer work after FP call */
    int int_result = int1 + int2;
    
    /* Complex control flow with calls at boundaries */
    switch (int_result % 4) {
        case 0:
            opaque_func1();
            fp_result += 1.0;
            break;
        case 1:
            fp_result -= 1.0;
            opaque_func2(int_result);
            break;
        case 2:
            /* Fall through with call */
            opaque_func1();
            /* Intentional fallthrough */
        default:
            /* This creates a basic block end update scenario */
            fp_result *= 2.0;
            if (volatile_fptr) {
                volatile_fptr();
            }
            break;
    }
    
    /* Final computation using all values */
    return fp_result + temp2 + temp3 + int_result;
}

/* Test function 3: Nested calls and irreducible control flow */
__attribute__((noinline, noclone))
void *test3(void *ptr, int depth) {
    volatile long stack_save[8];
    
    /* Save values that must survive nested calls */
    for (int i = 0; i < 8; i++) {
        stack_save[i] = ((long)ptr << i) + i;
    }
    
    /* Create irreducible control flow with goto */
    if (depth > 0) {
        goto recursive_path;
    }
    
direct_path:
    /* Direct call path */
    void *result = opaque_func4(ptr);
    
    /* Use saved values */
    for (int i = 0; i < 8; i++) {
        global_array[i] = stack_save[i];
    }
    
    return result;
    
recursive_path:
    /* Path with nested call */
    COMPILER_BARRIER();
    opaque_func1();
    COMPILER_BARRIER();
    
    /* Loop with break that creates block boundaries */
    int counter = depth;
    while (counter > 0) {
        if (counter % 2 == 0) {
            /* Call in loop with register pressure */
            int temp = opaque_func2(counter);
            global_counter += temp;
            if (temp > 100) break;
        } else {
            /* Different call pattern */
            opaque_func1();
        }
        counter--;
        
        /* Inline asm that looks like a call */
        asm volatile(
            "movq %0, %%rax\n\t"
            "call *%%rax\n\t"
            : 
            : "r" (volatile_fptr)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
              "memory", "cc"
        );
    }
    
    goto direct_path;
}

/* Test function 4: Vector-like operations and many args */
#ifdef __SSE2__
#include <emmintrin.h>
__attribute__((noinline, noclone))
__m128i test4(__m128i a, __m128i b, __m128i c, __m128i d,
              int i1, int i2, int i3, int i4) {
    /* Vector operations */
    __m128i sum = _mm_add_epi32(a, b);
    __m128i prod = _mm_mullo_epi16(c, d);
    
    /* Scalar values that must survive */
    volatile int s1 = i1;
    volatile int s2 = i2;
    volatile int s3 = i3;
    volatile int s4 = i4;
    
    /* Call that might clobber vector regs */
    COMPILER_BARRIER();
    opaque_func1();
    COMPILER_BARRIER();
    
    /* Use vectors after call */
    __m128i result = _mm_add_epi32(sum, prod);
    
    /* Complex conditional with call at boundary */
    if (s1 + s2 > s3 + s4) {
        result = _mm_sub_epi32(result, a);
        opaque_func2(s1);
    } else {
        result = _mm_add_epi32(result, b);
        if (volatile_fptr) {
            volatile_fptr();
        }
    }
    
    /* Final use of all scalars */
    int scalar_sum = s1 + s2 + s3 + s4;
    result = _mm_add_epi32(result, _mm_set1_epi32(scalar_sum));
    
    return result;
}
#endif

/* Helper with nested call structure */
__attribute__((noinline, noclone))
int helper_nested(int x, int y) {
    /* Inner function call within helper */
    int inner = opaque_func2(x);
    
    /* Live values across inner call */
    volatile int save_x = x;
    volatile int save_y = y;
    
    /* Complex expression */
    int result = (save_x * save_y) + inner;
    
    /* Conditional with goto */
    if (result > 100) {
        goto large_result;
    }
    
    return result * 2;
    
large_result:
    /* Another call on this path */
    opaque_func1();
    return result / 2;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile function pointer */
    volatile_fptr = (func_ptr_t)opaque_func1;
    
    /* Use command line to vary paths */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize global arrays */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        global_fp[i] = i * 1.5;
    }
    
    int total_result = 0;
    
    /* Execute all tests in different orders based on mode */
    switch (test_mode) {
        case 0:
            total_result += test1(1, 2, 3, 4, 5, 6, 7, 8);
            total_result += (int)test2(1.1, 2.2, 3.3, 4.4, 1, 2, 3, 4);
            total_result += (int)(long)test3(&total_result, 3);
            total_result += helper_nested(10, 20);
            break;
        case 1:
            total_result += (int)test2(5.5, 6.6, 7.7, 8.8, 5, 6, 7, 8);
            total_result += test1(9, 10, 11, 12, 13, 14, 15, 16);
            total_result += helper_nested(30, 40);
            total_result += (int)(long)test3(&total_result, 2);
            break;
        case 2:
            total_result += helper_nested(50, 60);
            total_result += (int)(long)test3(&total_result, 1);
            total_result += test1(17, 18, 19, 20, 21, 22, 23, 24);
            total_result += (int)test2(9.9, 10.1, 11.1, 12.1, 9, 10, 11, 12);
            break;
        case 3:
            total_result += (int)(long)test3(&total_result, 4);
            total_result += helper_nested(70, 80);
            total_result += (int)test2(13.1, 14.1, 15.1, 16.1, 13, 14, 15, 16);
            total_result += test1(25, 26, 27, 28, 29, 30, 31, 32);
            break;
    }
    
#ifdef __SSE2__
    /* Add vector test if available */
    __m128i vec_a = _mm_set_epi32(1, 2, 3, 4);
    __m128i vec_b = _mm_set_epi32(5, 6, 7, 8);
    __m128i vec_c = _mm_set_epi32(9, 10, 11, 12);
    __m128i vec_d = _mm_set_epi32(13, 14, 15, 16);
    
    __m128i vec_result = test4(vec_a, vec_b, vec_c, vec_d, 
                               total_result, total_result+1, 
                               total_result+2, total_result+3);
    
    /* Extract result from vector */
    int vec_arr[4];
    _mm_storeu_si128((__m128i*)vec_arr, vec_result);
    total_result += vec_arr[0] + vec_arr[1] + vec_arr[2] + vec_arr[3];
#endif
    
    /* Compute checksum to prevent elimination */
    long checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (long)global_fp[i];
    }
    checksum += global_counter;
    checksum += total_result;
    
    printf("Result: %ld\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}

/* Dummy implementations of opaque functions to allow linking */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2 + 1;
}

double opaque_func3(double x) {
    return x * 1.5;
}

void *opaque_func4(void *ptr) {
    return (void*)((long)ptr + 1);
}
