/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);
extern void *opaque_call_5(void *, void *);

/* Volatile globals to maintain live ranges across calls */
volatile int global_volatile_int = 12345;
volatile double global_volatile_double = 3.14159;
volatile void *global_volatile_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_table[10];

/* Register variables with explicit constraints */
register int reg_var_1 asm ("r10");
register int reg_var_2 asm ("r11");
register double reg_var_d asm ("xmm15");

/* Complex control flow with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    volatile int local_volatile = 42;
    int array[20];
    int i, j;
    
    /* Force register pressure */
    int a = local_volatile + 1;
    int b = local_volatile * 2;
    int c = local_volatile / 3;
    int d = local_volatile - 4;
    int e = local_volatile ^ 0x55;
    
    /* Use explicit register variables */
    reg_var_1 = a + b;
    reg_var_2 = c + d;
    
    /* Complex control flow with goto to create irreducible CFG */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Call with many live variables */
    opaque_call_1();
    
    /* Use all live variables after call */
    array[0] = a + reg_var_1;
    array[1] = b + reg_var_2;
    array[2] = c + e;
    
    /* Another call with different clobbers */
    asm volatile ("movl $0x12345678, %%eax\n\t"
                  "movl $0x87654321, %%ebx\n\t"
                  : : : "eax", "ebx", "memory");
    
    /* More variable usage */
    for (i = 0; i < 10; i++) {
        if (i & 1) {
            array[i + 3] = a + i;
            opaque_call_2(i);
        } else {
            array[i + 3] = b + i;
            /* Inline asm that looks like a call */
            asm volatile ("call *%0" : : "r" (func_table[i % 5]) : "memory");
        }
        
        /* Nested loop with break */
        for (j = 0; j < 5; j++) {
            if (array[i + 3] > 100) break;
            c += j;
        }
    }
    
    goto end;
    
label2:
    /* Different path with switch statement */
    switch (mode & 3) {
        case 0:
            opaque_call_3(a, b);
            break;
        case 1:
            opaque_call_3(b, c);
            /* fall through */
        case 2:
            a = opaque_call_3(c, d);
            /* Complex expression requiring temporary */
            d = (a * b + c * d) ^ e;
            break;
        default:
            /* Call in default case */
            opaque_call_1();
            /* Use __builtin_apply to create unusual pattern */
            {
                void *args = __builtin_apply_args();
                void *result = __builtin_apply((void (*)())opaque_call_1, args, 64);
                __builtin_return(result);
            }
    }
    
end:
    /* Compiler barrier */
    asm volatile ("" : : : "memory");
    
    /* Use all variables to prevent DCE */
    global_volatile_int += a + b + c + d + e + reg_var_1 + reg_var_2;
}

/* Function with floating point and mixed arguments */
__attribute__((noinline, noclone))
void test2(double x, double y) {
    volatile double local_d = x + y;
    double temp[10];
    int i;
    
    /* Use explicit FP register variable */
    reg_var_d = local_d * 2.0;
    
    /* Create many FP live values */
    double d1 = local_d;
    double d2 = local_d * 0.5;
    double d3 = local_d * 1.5;
    double d4 = local_d * 2.5;
    double d5 = local_d * 3.5;
    
    /* Call with FP arguments */
    double result = opaque_call_4(d1, d2);
    
    /* Complex FP expression across call */
    for (i = 0; i < 10; i++) {
        if (i & 1) {
            temp[i] = d3 + result;
            /* Inline asm clobbering FP registers */
            asm volatile ("fldl %0\n\t"
                          "fstpl %1\n\t"
                          : : "m" (temp[i]), "m" (temp[i+1]) : "st", "st(1)", "memory");
        } else {
            temp[i] = d4 * result;
            opaque_call_4(temp[i], d5);
        }
        
        /* Loop with continue creating block boundaries */
        if (temp[i] < 0.0) continue;
        
        d5 += temp[i];
    }
    
    /* Use all FP values */
    global_volatile_double += d1 + d2 + d3 + d4 + d5 + reg_var_d + result;
}

/* Function with pointer manipulation and __builtin_va_arg */
__attribute__((noinline, noclone))
void test3(void *ptr, ...) {
    va_list ap;
    int sum = 0;
    volatile int *volatile_ptr = (volatile int *)ptr;
    
    /* Force register pressure with pointer arithmetic */
    char *p1 = (char *)ptr + 1;
    char *p2 = (char *)ptr + 2;
    char *p3 = (char *)ptr + 3;
    char *p4 = (char *)ptr + 4;
    
    /* Use va_arg which may need special handling */
    va_start(ap, ptr);
    int v1 = va_arg(ap, int);
    double v2 = va_arg(ap, double);
    void *v3 = va_arg(ap, void *);
    va_end(ap);
    
    /* Call between va_arg uses */
    void *new_ptr = opaque_call_5(ptr, v3);
    
    /* Complex pointer expression requiring temporary */
    int offset = (p2 - p1) + (p4 - p3);
    p1 = (char *)new_ptr + offset;
    
    /* Nested function call scenario */
    {
        int (*inner_func)(int) = (int (*)(int))opaque_call_5;
        int inner_result = inner_func(v1);
        
        /* Use all pointer variables */
        *volatile_ptr = v1 + inner_result;
        sum = *(int *)p1 + *(int *)p2 + *(int *)p3 + *(int *)p4;
    }
    
    /* Another call at block end */
    opaque_call_1();
    
    global_volatile_int += sum + v1;
}

/* Function with vector types (if supported) */
#ifdef __SSE2__
#include <emmintrin.h>
__attribute__((noinline, noclone))
void test4(__m128i vec) {
    volatile __m128i local_vec = vec;
    __m128i temp[4];
    
    /* Create many vector live values */
    __m128i v1 = _mm_add_epi32(local_vec, _mm_set1_epi32(1));
    __m128i v2 = _mm_add_epi32(local_vec, _mm_set1_epi32(2));
    __m128i v3 = _mm_add_epi32(local_vec, _mm_set1_epi32(3));
    __m128i v4 = _mm_add_epi32(local_vec, _mm_set1_epi32(4));
    
    /* Inline asm that clobbers vector registers */
    asm volatile ("movdqa %0, %%xmm0\n\t"
                  "movdqa %1, %%xmm1\n\t"
                  : : "m" (v1), "m" (v2) : "xmm0", "xmm1", "memory");
    
    /* Call (simulated by asm) */
    asm volatile ("call *%0" : : "r" (func_table[0]) : "memory", "xmm0", "xmm1", "xmm2", "xmm3");
    
    /* Use vectors after call */
    temp[0] = _mm_add_epi32(v1, v2);
    temp[1] = _mm_add_epi32(v3, v4);
    
    /* Complex control flow with loop */
    for (int i = 0; i < 4; i++) {
        if (i & 1) {
            temp[2] = _mm_add_epi32(temp[0], temp[1]);
            opaque_call_2(i);
        } else {
            temp[3] = _mm_sub_epi32(temp[0], temp[1]);
        }
    }
    
    /* Force use of all vectors */
    asm volatile ("" : : "m" (temp[0]), "m" (temp[1]), "m" (temp[2]), "m" (temp[3]) : "memory");
}
#endif

/* Helper with nested calls */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    volatile int local = val;
    
    if (depth <= 0) {
        opaque_call_2(local);
        return local + 1;
    }
    
    /* Recursive call (tail recursion prevented by volatile) */
    int a = nested_helper(depth - 1, local * 2);
    
    /* Call between recursive calls */
    opaque_call_3(a, local);
    
    int b = nested_helper(depth - 2, local / 2);
    
    /* Complex expression with many operands */
    return (a * b + local) ^ (a + b);
}

/* Main test driver */
int main(int argc, char **argv) {
    int mode = 0;
    
    /* Use argv to create runtime-dependent control flow */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Initialize function pointers (some null, some to extern) */
    for (int i = 0; i < 10; i++) {
        func_table[i] = (func_ptr_t)opaque_call_1;
    }
    
    /* Initialize register variables */
    reg_var_1 = global_volatile_int;
    reg_var_2 = global_volatile_int * 2;
    reg_var_d = global_volatile_double;
    
    /* Run all tests with different modes */
    test1(mode);
    test2(global_volatile_double, global_volatile_double * 2);
    
    void *ptr = &global_volatile_int;
    test3(ptr, mode, global_volatile_double, ptr);
    
#ifdef __SSE2__
    test4(_mm_set_epi32(mode, mode+1, mode+2, mode+3));
#endif
    
    /* Test nested call scenario */
    int nested_result = nested_helper(3, mode);
    
    /* Create irreducible loop with calls */
    int counter = 0;
    volatile int loop_control = mode & 7;
    
start_loop:
    if (counter++ > loop_control) goto end_loop;
    
    /* Call in loop with many live variables */
    {
        int x = counter * 2;
        int y = counter * 3;
        int z = counter * 5;
        
        opaque_call_3(x, y);
        
        /* Use variables after call in complex way */
        x = x + y + z + reg_var_1;
        
        /* goto creating unusual control flow */
        if (x & 1) goto odd_case;
        
        y = opaque_call_3(z, x);
        goto start_loop;
        
    odd_case:
        z = opaque_call_3(x, y);
        goto start_loop;
    }
    
end_loop:
    
    /* Final checksum to prevent optimization */
    int checksum = global_volatile_int + (int)global_volatile_double + nested_result;
    printf("Checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (normally would be in separate file) */
void opaque_call_1(void) {
    asm volatile ("" : : : "memory");
}

void opaque_call_2(int x) {
    global_volatile_int += x;
}

int opaque_call_3(int a, int b) {
    return a + b + global_volatile_int;
}

double opaque_call_4(double a, double b) {
    return a * b + global_volatile_double;
}

void *opaque_call_5(void *a, void *b) {
    return (void *)((long)a + (long)b);
}
