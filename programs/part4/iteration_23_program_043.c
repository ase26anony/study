/* caller-save-test.c
 * A test program designed to trigger uncovered code paths in GCC's reload pass,
 * specifically the instruction chain manipulation in caller-save.cc lines 905-913.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External opaque functions to prevent inlining and create call boundaries */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_data[32];
volatile double global_fp[16];

/* Function pointer with volatile assignment to prevent devirtualization */
typedef int (*func_ptr_t)(int, ...);
volatile func_ptr_t volatile_fptr = NULL;

/* ========== Test Function 1: Integer register pressure ========== */
__attribute__((noinline, noclone))
int test1_integer_pressure(int mode) {
    /* Force many integer values to be live across calls */
    register int r0 asm ("r10") = mode + 1;
    register int r1 asm ("r11") = mode + 2;
    register int r2 asm ("r12") = mode + 3;
    volatile int v0 = mode * 2;
    volatile int v1 = mode * 3;
    volatile int v2 = mode * 4;
    
    /* Array to force spills */
    int local_array[8];
    for (int i = 0; i < 8; i++) {
        local_array[i] = mode + i + r0 + r1;
    }
    
    /* Complex expression with inline asm that clobbers registers */
    asm volatile ("# Test1 asm block\n\t"
                  "mov %0, %0\n\t"
                  : "+r" (r0), "+r" (r1)
                  : 
                  : "eax", "ecx", "edx", "r8", "r9", "memory");
    
    /* Function call with many live values */
    int result = opaque_func2(r0 + r1 + r2);
    
    /* Use all live values after call - forces caller-save to restore them */
    v0 = r0 + local_array[0];
    v1 = r1 + local_array[1] + result;
    v2 = r2 + local_array[2] + v0 + v1;
    
    /* Another asm barrier */
    asm volatile ("# Test1 post-call\n\t"
                  : : "r" (r0), "r" (r1), "r" (r2) : "memory");
    
    /* Control flow that can cause block splitting */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Use values in different basic block */
    return r0 + v0 + result;
    
label2:
    /* Different use pattern */
    return r1 + v1 + result * 2;
}

/* ========== Test Function 2: Floating point and mixed ========== */
__attribute__((noinline, noclone))
double test2_fp_mixed(double base, int iterations) {
    volatile double accum = base;
    register double fp0 asm ("xmm0") = base * 1.1;
    register double fp1 asm ("xmm1") = base * 2.2;
    volatile int int_val = (int)base;
    
    /* Loop with function call inside - creates complex CFG */
    for (int i = 0; i < iterations; i++) {
        /* Force FP values to be live across call */
        double temp = fp0 + fp1 + accum;
        
        /* Inline asm that clobbers FP registers */
        asm volatile ("# FP asm clobber\n\t"
                      : "+x" (fp0), "+x" (fp1)
                      : 
                      : "xmm2", "xmm3", "xmm4", "xmm5", "memory");
        
        /* Function call that might clobber registers */
        double result = opaque_func3(temp);
        
        /* Complex control flow */
        switch (i % 3) {
            case 0:
                fp0 = result * 1.5;
                break;
            case 1:
                fp1 = result * 2.5;
                /* Fall through to create merge point */
            case 2:
                accum += result + fp0 + fp1;
                /* Call within switch case */
                int_val += opaque_func2(int_val);
                break;
            default:
                /* Unreachable but compiler doesn't know */
                opaque_func1();
        }
        
        /* Another asm barrier to prevent reordering */
        asm volatile ("# Loop barrier\n\t" : : : "memory");
    }
    
    /* Use all values in final computation */
    return fp0 + fp1 + accum + int_val;
}

/* ========== Test Function 3: Nested calls and goto ========== */
__attribute__((noinline, noclone))
void* test3_nested_irreducible(void* ptr, int depth) {
    /* Create irreducible control flow with gotos */
    volatile int state = 0;
    register void* rptr asm ("r14") = ptr;
    volatile long values[4];
    
    if (depth <= 0) {
        goto cleanup;
    }
    
    /* Label spaghetti to force block splitting */
start:
    values[0] = (long)rptr + state;
    
    /* Function call at block boundary */
    void* new_ptr = opaque_func4(rptr);
    
    if (state & 1) {
        goto odd_path;
    } else {
        goto even_path;
    }
    
odd_path:
    /* Use register after call in different block */
    asm volatile ("# Odd path\n\t" : : "r" (rptr) : "memory");
    state++;
    if (state < 3) {
        goto start;
    }
    goto merge_point;
    
even_path:
    /* Different use pattern */
    rptr = new_ptr;
    values[1] = (long)rptr * 2;
    state += 2;
    if (state < 4) {
        goto start;
    }
    /* Fall through to merge */
    
merge_point:
    /* Another call at merge point */
    opaque_func1();
    
    /* Force register to be live across preceding call */
    values[2] = (long)rptr + values[0] + values[1];
    
    /* Recursive call to create nested save/restore */
    if (depth > 1) {
        test3_nested_irreducible(rptr, depth - 1);
    }
    
cleanup:
    /* Final use of register */
    values[3] = (long)rptr;
    return rptr;
}

/* ========== Test Function 4: Varargs and builtin_apply ========== */
__attribute__((noinline, noclone))
long test4_varargs_pressure(int count, ...) {
    va_list args;
    va_start(args, count);
    
    volatile long accum = 0;
    /* Force many values into registers */
    register long r0 asm ("r10") = count;
    register long r1 asm ("r11") = 0;
    register long r2 asm ("r12") = 0;
    
    /* Process varargs - creates register pressure */
    for (int i = 0; i < count && i < 8; i++) {
        long val = va_arg(args, long);
        accum += val;
        
        /* Rotate registers */
        long temp = r0;
        r0 = r1;
        r1 = r2;
        r2 = temp + val;
        
        /* Call within loop */
        if (i % 2 == 0) {
            opaque_func2((int)val);
        }
    }
    
    va_end(args);
    
    /* Complex expression requiring all registers */
    asm volatile ("# Varargs final\n\t"
                  : "+r" (r0), "+r" (r1), "+r" (r2)
                  : 
                  : "rax", "rcx", "rdx", "r8", "r9", "memory");
    
    return r0 + r1 + r2 + accum;
}

/* ========== Test Function 5: Vector types and asm ========== */
typedef long v2di __attribute__((vector_size(16)));
__attribute__((noinline, noclone))
v2di test5_vector_ops(v2di a, v2di b) {
    volatile v2di result;
    register v2di v0 asm ("xmm0") = a;
    register v2di v1 asm ("xmm1") = b;
    volatile int control;
    
    /* Inline asm with vector clobbers */
    asm volatile ("# Vector shuffle\n\t"
                  "movdqa %1, %0\n\t"
                  "pshufd $0xE4, %0, %0\n\t"
                  : "=x" (v0)
                  : "x" (v0), "x" (v1)
                  : "xmm2", "xmm3", "xmm4", "xmm5");
    
    /* Function call that might clobber vector regs */
    control = opaque_func2(v0[0] + v0[1]);
    
    /* Conditional with goto to split blocks */
    if (control > 0) {
        goto positive;
    } else {
        goto negative;
    }
    
positive:
    /* Use vectors after call in one path */
    v1 = v0 + (v2di){control, control};
    result = v0 + v1;
    goto done;
    
negative:
    /* Different computation in other path */
    v0 = v1 - (v2di){-control, -control};
    result = v0 * v1;
    /* Another call at block end */
    opaque_func1();
    
done:
    /* Final asm that uses all */
    asm volatile ("# Vector result\n\t"
                  : "+x" (v0), "+x" (v1)
                  : 
                  : "memory");
    
    return result;
}

/* ========== Helper with nested calls ========== */
__attribute__((noinline, noclone))
int helper_nested_calls(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    register int r0 asm ("r10") = x * 2;
    
    /* First call */
    int mid = opaque_func2(x + y);
    
    /* Nested call sequence */
    for (int i = 0; i < 2; i++) {
        /* Save register value before call */
        int saved_r0 = r0;
        
        /* Call that might clobber r0 */
        int temp = opaque_func2(mid + i);
        
        /* Restore and use saved value */
        r0 = saved_r0 + temp;
        a += r0;
        
        /* Another asm barrier */
        asm volatile ("# Nested loop\n\t" : : "r" (r0) : "memory");
    }
    
    /* Final computation using register */
    return a + b + r0 + mid;
}

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    /* Initialize volatile function pointer */
    volatile_fptr = (func_ptr_t)opaque_func2;
    
    /* Parse test mode from args */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize global data */
    for (int i = 0; i < 32; i++) {
        global_data[i] = i * 3 + test_mode;
    }
    for (int i = 0; i < 16; i++) {
        global_fp[i] = i * 1.5 + test_mode;
    }
    
    long total_result = 0;
    
    /* Execute all tests in sequence, but with mode-dependent order */
    int execution_order[5];
    for (int i = 0; i < 5; i++) {
        execution_order[i] = (i + test_mode) % 5;
    }
    
    for (int i = 0; i < 5; i++) {
        switch (execution_order[i]) {
            case 0:
                total_result += test1_integer_pressure(test_mode + i);
                break;
            case 1:
                total_result += (long)test2_fp_mixed(global_fp[i], 3 + (test_mode % 3));
                break;
            case 2:
                total_result += (long)test3_nested_irreducible(&global_data[0], 2);
                break;
            case 3:
                total_result += test4_varargs_pressure(4, 
                    (long)global_data[0], (long)global_data[1],
                    (long)global_data[2], (long)global_data[3]);
                break;
            case 4: {
                v2di vec_a = {global_data[0], global_data[1]};
                v2di vec_b = {global_data[2], global_data[3]};
                v2di vec_result = test5_vector_ops(vec_a, vec_b);
                total_result += vec_result[0] + vec_result[1];
                break;
            }
        }
        
        /* Call helper with nested calls between tests */
        total_result += helper_nested_calls(test_mode, i);
        
        /* Memory barrier */
        asm volatile ("# Main loop barrier\n\t" : : : "memory");
    }
    
    /* Use volatile function pointer to prevent optimization */
    if (volatile_fptr != NULL) {
        total_result += volatile_fptr((int)total_result);
    }
    
    /* Final checksum to prevent dead code elimination */
    printf("Result: %ld\n", total_result);
    
    /* Access all globals to keep them live */
    for (int i = 0; i < 32; i++) {
        global_counter += global_data[i] > 0 ? 1 : 0;
    }
    
    return (total_result > 0) ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_func1(void) {
    asm volatile ("# Opaque func1\n\t" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("# Opaque func2\n\t" : "+r" (x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile ("# Opaque func3\n\t" : "+x" (x) : : "memory");
    return x * 1.5;
}

void* opaque_func4(void* x) {
    asm volatile ("# Opaque func4\n\t" : "+r" (x) : : "memory");
    return (void*)((long)x + 1);
}
