/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller-save-test.c -o caller-save-test -ldl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;
volatile void *global_ptr = NULL;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Register variables to force specific register allocation */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register int reg_var3 asm ("r14");
register double reg_fp1 asm ("xmm8");
register double reg_fp2 asm ("xmm9");

/* Complex control flow with function calls */
__attribute__((noinline, noclone))
void test1(int n) {
    volatile int local_vars[20];
    int i, j;
    
    /* Initialize with volatile values */
    for (i = 0; i < 20; i++) {
        local_vars[i] = global_counter + i;
    }
    
    /* Force many live variables across call */
    reg_var1 = local_vars[0];
    reg_var2 = local_vars[1];
    reg_var3 = local_vars[2];
    
    /* Complex control flow with goto to create irreducible CFG */
    if (n > 100) {
        goto label1;
    } else if (n > 50) {
        goto label2;
    }
    
    /* Function call with many live registers */
    asm volatile ("" : : : "memory");  /* Compiler barrier */
    opaque_call_1();
    asm volatile ("" : : : "memory");
    
    /* Use register variables after call */
    local_vars[3] = reg_var1 + reg_var2;
    
label1:
    /* Nested loop with break/continue */
    for (i = 0; i < n; i++) {
        if (i % 3 == 0) {
            /* Another function call */
            opaque_call_2(i);
            continue;
        }
        
        for (j = 0; j < 5; j++) {
            if (j == 2) {
                /* Force register pressure */
                reg_var3 = local_vars[i] * j;
                break;
            }
        }
        
        /* Use values saved before call */
        local_vars[i % 20] = reg_var3 + local_vars[(i+1) % 20];
    }
    
label2:
    /* Switch statement with function calls */
    switch (n % 4) {
        case 0:
            opaque_call_3(n, n+1);
            break;
        case 1:
            opaque_call_3(n*2, n*3);
            /* Fall through */
        case 2:
            /* Complex expression requiring temporary registers */
            reg_var1 = (local_vars[0] * local_vars[1]) + 
                      (local_vars[2] * local_vars[3]) -
                      (local_vars[4] * local_vars[5]);
            break;
        default:
            /* Function call at default case */
            opaque_call_2(n);
            /* Force BB_END update scenario */
            if (BB_END) { /* This would be a macro in GCC internals */
                /* Simulate the condition that triggers the uncovered code */
            }
    }
    
    /* Update global state */
    global_counter += reg_var1 + reg_var2 + reg_var3;
}

/* Function with floating point and mixed arguments */
__attribute__((noinline, noclone))
void test2(double x, double y) {
    volatile double fp_vars[10];
    volatile int int_vars[10];
    int i;
    
    /* Initialize with complex expressions */
    reg_fp1 = x;
    reg_fp2 = y;
    
    for (i = 0; i < 10; i++) {
        fp_vars[i] = reg_fp1 * i + reg_fp2;
        int_vars[i] = (int)(fp_vars[i] * 100);
    }
    
    /* Inline asm with clobbered registers */
    asm volatile (
        "movq $0x12345678, %%rax\n\t"
        "movq $0x87654321, %%r10\n\t"
        "addq %%r10, %%rax\n\t"
        : /* no outputs */
        : /* no inputs */
        : "rax", "r10", "cc"
    );
    
    /* Function call that clobbers registers */
    double result = opaque_call_4(reg_fp1, reg_fp2);
    
    /* Use values that were live across call */
    for (i = 0; i < 10; i++) {
        fp_vars[i] = fp_vars[i] * result + int_vars[i];
    }
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    
    /* Indirect function call */
    if (global_ptr) {
        func_ptr_t f = (func_ptr_t)global_ptr;
        f();
    }
    
    /* Update global accumulator */
    global_accumulator += result + fp_vars[0] + fp_vars[5];
}

/* Function with vector operations (if supported) */
#ifdef __SSE2__
#include <emmintrin.h>
__attribute__((noinline, noclone))
void test3(void) {
    volatile __m128i vec_vars[4];
    volatile __m128d fp_vec_vars[4];
    
    /* Initialize vectors */
    vec_vars[0] = _mm_set_epi32(global_counter, global_counter+1, 
                               global_counter+2, global_counter+3);
    vec_vars[1] = _mm_set_epi32(global_counter+4, global_counter+5,
                               global_counter+6, global_counter+7);
    
    /* Function call that might clobber vector registers */
    opaque_call_1();
    
    /* Use vectors after call */
    vec_vars[2] = _mm_add_epi32(vec_vars[0], vec_vars[1]);
    
    /* Complex control flow */
    for (int i = 0; i < 100; i++) {
        if (i == 50) {
            /* Another call in loop */
            opaque_call_2(i);
            continue;
        }
        
        /* Modify vectors */
        if (i % 2 == 0) {
            vec_vars[3] = _mm_slli_epi32(vec_vars[2], 1);
        }
    }
    
    /* Extract results to force register moves */
    int results[4];
    _mm_storeu_si128((__m128i*)results, vec_vars[3]);
    global_counter += results[0] + results[1] + results[2] + results[3];
}
#endif

/* Function with __builtin_apply to create unusual call sequences */
__attribute__((noinline, noclone))
void test4(int a, int b, int c, int d, int e, int f) {
    /* Save arguments to volatile array */
    volatile int args[6] = {a, b, c, d, e, f};
    
    /* Use register variables */
    reg_var1 = a + b;
    reg_var2 = c + d;
    reg_var3 = e + f;
    
    /* Complex expression requiring temporary registers */
    int complex_result = (a * b) + (c * d) - (e * f) + 
                        (reg_var1 * reg_var2) / (reg_var3 ? reg_var3 : 1);
    
    /* Function call with many arguments */
    int call_result = opaque_call_3(complex_result, args[0]);
    
    /* Use __builtin_apply_args and __builtin_apply to create
     * unusual call sequences that might trigger reload */
    void* arg_buf = __builtin_apply_args();
    
    /* This creates complex prologue/epilogue sequences */
    for (int i = 0; i < 3; i++) {
        /* Alternate between direct and indirect calls */
        if (i % 2 == 0) {
            opaque_call_2(args[i]);
        } else {
            if (func_table[i]) {
                func_table[i]();
            }
        }
        
        /* Modify live variables */
        args[i % 6] += call_result + i;
    }
    
    global_counter += complex_result + call_result;
}

/* Helper function with nested calls */
__attribute__((noinline, noclone))
void nested_helper(int depth, int* result) {
    volatile int local = depth * 2;
    
    if (depth > 0) {
        /* Recursive call */
        nested_helper(depth - 1, result);
        
        /* Use value after call */
        *result += local + reg_var1;
        
        /* Another call */
        opaque_call_2(*result);
    } else {
        /* Base case - function call */
        *result = opaque_call_3(local, local + 1);
    }
    
    /* Force register save/restore around the two calls */
    reg_var2 = *result * 3;
}

/* Main test driver */
int main(int argc, char** argv) {
    int test_mode = 0;
    
    /* Use argv to determine test mode */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize function pointers with dlsym for opaque calls */
    void* handle = dlopen(NULL, RTLD_LAZY);
    if (handle) {
        /* Use dlsym to get function pointers - creates opaque calls */
        void* sym1 = dlsym(handle, "printf");
        void* sym2 = dlsym(handle, "malloc");
        void* sym3 = dlsym(handle, "free");
        
        func_table[0] = (func_ptr_t)sym1;
        func_table[1] = (func_ptr_t)sym2;
        func_table[2] = (func_ptr_t)sym3;
        func_table[3] = (func_ptr_t)opaque_call_1;
        
        dlclose(handle);
    }
    
    /* Initialize global pointer */
    global_ptr = &global_counter;
    
    /* Run all tests in different orders based on mode */
    int result1 = 0, result2 = 0;
    
    switch (test_mode) {
        case 0:
            test1(75);
            test2(3.14, 2.718);
            #ifdef __SSE2__
            test3();
            #endif
            test4(1, 2, 3, 4, 5, 6);
            nested_helper(3, &result1);
            break;
            
        case 1:
            test4(10, 20, 30, 40, 50, 60);
            test1(25);
            nested_helper(2, &result2);
            test2(1.414, 1.732);
            break;
            
        case 2:
            #ifdef __SSE2__
            test3();
            #endif
            test1(100);
            test4(7, 8, 9, 10, 11, 12);
            test2(0.577, 1.618);
            nested_helper(4, &result1);
            break;
            
        case 3:
            for (int i = 0; i < 3; i++) {
                test1(50 + i * 10);
                test2(0.1 * i, 0.2 * i);
            }
            break;
            
        case 4:
            /* Mixed order with loops */
            for (int i = 0; i < 2; i++) {
                test1(30 + i * 20);
                if (i % 2 == 0) {
                    test2(1.0, 2.0);
                } else {
                    test4(i, i+1, i+2, i+3, i+4, i+5);
                }
            }
            nested_helper(5, &result2);
            break;
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long checksum = 0;
    checksum += global_counter;
    checksum += (unsigned long)(global_accumulator * 1000);
    checksum += (unsigned long)global_ptr;
    checksum += reg_var1 + reg_var2 + reg_var3;
    checksum += result1 + result2;
    
    /* Use checksum in output */
    printf("Checksum: %lu\n", checksum);
    
    /* Additional complex control flow in main */
    if (checksum > 1000) {
        goto main_label1;
    }
    
    /* Another function call at basic block boundary */
    opaque_call_1();
    
main_label1:
    /* Loop with break at different points */
    for (int i = 0; i < 10; i++) {
        if (checksum % (i + 2) == 0) {
            /* Function call that might trigger BB_END update */
            test1(i);
            break;
        } else {
            test2(i * 0.5, i * 0.25);
            continue;
        }
    }
    
    return (int)(checksum % 256);
}

/* Dummy definitions for external functions to allow linking */
void opaque_call_1(void) {
    /* Use asm to clobber registers */
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
    global_counter++;
}

void opaque_call_2(int x) {
    asm volatile ("" : : "r"(x) : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10",
                  "xmm0", "xmm1", "xmm2", "xmm3", "memory");
    global_accumulator += x * 0.01;
}

int opaque_call_3(int a, int b) {
    int result;
    asm volatile ("addl %%ebx, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(result)
                  : "a"(a), "b"(b)
                  : "cc");
    return result;
}

double opaque_call_4(double a, double b) {
    double result;
    asm volatile ("addsd %1, %0"
                  : "=x"(result)
                  : "x"(a), "0"(b)
                  : "cc");
    return result;
}
