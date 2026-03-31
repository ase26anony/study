/* test-caller-save.c - Complex program to trigger uncovered lines in caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void opaque_func4(long long) __attribute__((noinline, noclone));

/* Volatile globals to maintain live ranges across calls */
volatile int gv1 = 12345;
volatile double gv2 = 3.14159;
volatile long long gv3 = 0xDEADBEEFCAFEBABEULL;
volatile int gv_array[32];

/* Function pointer with volatile to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fp = NULL;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit register variables */
register int reg_var1 asm("r12") __attribute__((unused));
register int reg_var2 asm("r13") __attribute__((unused));
register double reg_var3 asm("xmm8") __attribute__((unused));

/* Test function 1: Many integer arguments forcing register pressure */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int locals[8];
    int i, sum = 0;
    
    /* Save arguments to volatile array */
    locals[0] = a; locals[1] = b; locals[2] = c; locals[3] = d;
    locals[4] = e; locals[5] = f; locals[6] = g; locals[7] = h;
    
    /* Complex control flow with goto to create basic block splits */
    if (a > b) {
        /* Call with many live values */
        int result = opaque_func2(a + b);
        
        /* Use inline asm that clobbers call-clobbered registers */
        asm volatile (
            "movl %%eax, %%ecx\n\t"
            "addl %%ebx, %%ecx\n\t"
            : : : "eax", "ebx", "ecx", "memory"
        );
        
        /* Use saved values after call */
        for (i = 0; i < 4; i++) {
            sum += locals[i] * result;
        }
        
        if (c > d) goto label1;
    } else {
        /* Different path with another call */
        opaque_func1();
        
        /* More register pressure */
        asm volatile (
            "movq %%r10, %%r11\n\t"
            "addq $1, %%r11\n\t"
            : : : "r10", "r11", "memory"
        );
        
        for (i = 4; i < 8; i++) {
            sum += locals[i] * gv1;
        }
        
        if (e > f) goto label2;
    }
    
    /* Merge point with live values from both paths */
    COMPILER_BARRIER();
    
label1:
    /* Nested loop with break/continue creating complex CFG */
    for (i = 0; i < 10; i++) {
        if (i == 5) {
            opaque_func3(gv2);
            break;
        }
        if (i % 2 == 0) continue;
        
        /* Function call inside loop */
        sum += opaque_func2(i);
    }
    
label2:
    /* Switch with default calling external function */
    switch (sum % 4) {
        case 0: sum += a; break;
        case 1: sum += b; break;
        case 2: sum += c; break;
        default:
            opaque_func4(gv3);
            sum += d;
            /* Force insertion after call */
            asm volatile ("" : "=r"(sum) : "0"(sum) : "r10", "r11", "r12");
    }
    
    return sum;
}

/* Test function 2: Floating point and mixed arguments */
__attribute__((noinline, noclone))
double test2(double a, double b, int c, long d) {
    volatile double fp_locals[4];
    volatile int int_locals[4];
    double result = 0.0;
    
    fp_locals[0] = a;
    fp_locals[1] = b;
    int_locals[0] = c;
    int_locals[1] = d;
    
    /* Create irreducible control flow with gotos */
    if (a > b) {
        goto fp_block1;
    } else if (c > 0) {
        goto fp_block2;
    }
    
fp_block1:
    {
        /* Call with FP arguments */
        double tmp = opaque_func3(a);
        
        /* Use result immediately in complex expression */
        result = tmp * b + (c * d) / 256.0;
        
        /* Clobber multiple registers */
        asm volatile (
            "xorpd %%xmm0, %%xmm0\n\t"
            "addsd %0, %%xmm0\n\t"
            : : "m"(result) : "xmm0", "xmm1", "xmm2"
        );
        
        if (result > 100.0) goto fp_end;
    }
    
fp_block2:
    {
        /* Another call path */
        opaque_func1();
        
        /* Register variable usage around call */
        reg_var3 = a + b;
        
        /* Force save/restore around this asm */
        asm volatile (
            "movsd %1, %%xmm8\n\t"
            "addsd %2, %%xmm8\n\t"
            "movsd %%xmm8, %0\n\t"
            : "=m"(result) : "m"(a), "m"(b) : "xmm8", "xmm9"
        );
        
        /* Use saved values */
        result += fp_locals[0] - fp_locals[1];
    }
    
fp_end:
    /* Complex expression requiring temporary registers */
    result = (result * a / b) + (c * d) / (a + 1.0);
    
    return result;
}

/* Test function 3: Vector-like operations with many live values */
__attribute__((noinline, noclone))
long long test3(int count) {
    volatile long long buffer[16];
    long long sum = 0;
    int i, j;
    
    /* Initialize with pattern */
    for (i = 0; i < 16; i++) {
        buffer[i] = gv3 + i;
    }
    
    /* Nested loops with function calls */
    for (i = 0; i < count; i++) {
        /* Save to register variable before call */
        reg_var1 = i * 2;
        
        /* Function call that might clobber reg_var1's register */
        opaque_func4(buffer[i % 16]);
        
        /* Use reg_var1 after call - may need save/restore */
        sum += reg_var1 * buffer[i % 16];
        
        /* Inner loop with break */
        for (j = 0; j < 4; j++) {
            if (j == 2) {
                /* Call in inner loop */
                int tmp = opaque_func2(j);
                sum += tmp;
                break;
            }
            sum += j;
        }
        
        /* Switch with function pointer call */
        switch (i % 3) {
            case 0:
                if (volatile_fp) volatile_fp();
                break;
            case 1:
                opaque_func1();
                break;
            default:
                /* Force BB_END update scenario */
                asm volatile (
                    "movq %%rax, %%rbx\n\t"
                    "incq %%rbx\n\t"
                    : : : "rax", "rbx", "rcx", "rdx", "memory"
                );
                sum += gv_array[i % 32];
        }
    }
    
    return sum;
}

/* Helper with nested calls to create outer/inner save scenarios */
__attribute__((noinline, noclone))
int nested_calls(int depth, int val) {
    if (depth <= 0) {
        return opaque_func2(val);
    }
    
    volatile int saved = val * 2;
    
    /* Outer call */
    int result1 = opaque_func2(val);
    
    /* Use saved value */
    result1 += saved;
    
    /* Inner call */
    int result2 = nested_calls(depth - 1, result1);
    
    /* Complex use of both results requiring registers */
    asm volatile (
        "addl %%eax, %%ebx\n\t"
        "imull %%ecx, %%ebx\n\t"
        : "+b"(result2) : "a"(result1), "c"(saved) : "cc"
    );
    
    return result2;
}

/* Main test driver */
int main(int argc, char **argv) {
    int test_mode = 0;
    long long total_sum = 0;
    
    /* Use argv to create runtime-dependent control flow */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize volatile array with pattern */
    for (int i = 0; i < 32; i++) {
        gv_array[i] = i * 3 + 1;
    }
    
    /* Initialize function pointer (simulate dlsym-like behavior) */
    volatile_fp = (func_ptr_t)opaque_func1;
    
    /* Execute all test functions in different orders based on mode */
    switch (test_mode) {
        case 0:
            total_sum += test1(1, 2, 3, 4, 5, 6, 7, 8);
            total_sum += test2(1.5, 2.5, 3, 4);
            total_sum += test3(8);
            total_sum += nested_calls(2, 10);
            break;
        case 1:
            total_sum += test3(5);
            total_sum += nested_calls(3, 5);
            total_sum += test1(8, 7, 6, 5, 4, 3, 2, 1);
            total_sum += test2(3.14, 2.71, 10, 20);
            break;
        case 2:
            for (int i = 0; i < 2; i++) {
                total_sum += test1(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7);
                total_sum += nested_calls(1, i*10);
            }
            total_sum += test2(0.1, 0.2, 100, 200);
            break;
        default:
            total_sum += test3(10);
            total_sum += test1(100, 200, 300, 400, 500, 600, 700, 800);
            total_sum += test2(10.0, 20.0, 30, 40);
            total_sum += nested_calls(4, 1);
    }
    
    /* Additional complex control flow with goto */
    if (total_sum > 1000000) {
        goto large_result;
    }
    
    /* More operations to increase register pressure */
    {
        int temp = total_sum;
        for (int i = 0; i < 100; i++) {
            /* Mix of calls and asm */
            if (i % 10 == 0) {
                opaque_func1();
            }
            
            /* Asm that looks like a call */
            asm volatile (
                "call dummy_label\n\t"
                "dummy_label:\n\t"
                "pop %%rax\n\t"
                : : : "rax", "memory"
            );
            
            temp += gv_array[i % 32];
        }
        total_sum = temp;
    }
    
large_result:
    /* Final checksum to prevent elimination */
    printf("Result: %lld\n", total_sum);
    
    /* Use all global volatiles to keep them live */
    gv1 = total_sum % 1000;
    gv2 = total_sum / 1000.0;
    gv3 = total_sum;
    
    return (int)(total_sum & 0x7FFFFFFF);
}

/* Dummy external function definitions to satisfy linker */
void opaque_func1(void) {
    /* Empty but marked noinline so compiler can't see body */
}

int opaque_func2(int x) {
    return x + 1;
}

double opaque_func3(double x) {
    return x * 2.0;
}

void opaque_func4(long long x) {
    gv3 = x;
}
