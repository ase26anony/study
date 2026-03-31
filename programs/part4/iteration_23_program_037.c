/* test-caller-save.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(void*, ...);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_data[256];
volatile double global_fp[256];

/* Function pointer with volatile to prevent constant propagation */
void (*volatile func_ptr)(void) = opaque_func1;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit constraints */
#define USE_REGISTER(reg) asm volatile("" : : "r"(reg))

/* Test function 1: Many integer arguments with live ranges across calls */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Force all arguments to be live in registers */
    volatile int save[8];
    save[0] = a; save[1] = b; save[2] = c; save[3] = d;
    save[4] = e; save[5] = f; save[6] = g; save[7] = h;
    
    /* Complex control flow with goto to split basic blocks */
    int result = 0;
    
    if (a > 0) {
        /* First call site - many live registers */
        COMPILER_BARRIER();
        opaque_func1();
        COMPILER_BARRIER();
        
        /* Use saved values after call */
        result += save[0] + save[1];
        
        /* Irreducible control flow with label */
        goto middle_label;
    } else {
        /* Different call site */
        int temp = opaque_func2(b);
        result += temp;
        
        /* Force register pressure */
        register int r12_val asm("r12") = c;
        register int r13_val asm("r13") = d;
        asm volatile("" : "+r"(r12_val), "+r"(r13_val));
        result += r12_val + r13_val;
        
        middle_label:
        /* Merge point - basic block boundary */
        if (global_counter++ & 1) {
            /* Another call with clobbered registers */
            asm volatile("call *%0" : : "r"(func_ptr) : "eax", "ecx", "edx", "r8", "r9", "r10", "r11", "memory");
            
            /* Use all saved values - forcing reloads */
            for (int i = 0; i < 8; i++) {
                result += save[i] * (i + 1);
            }
        }
    }
    
    /* Loop with break inside conditional containing call */
    for (int i = 0; i < 10; i++) {
        if (result > 100) {
            /* Call that clobbers registers */
            asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            break;
        }
        result += i;
        opaque_func1();
    }
    
    return result;
}

/* Test function 2: Floating point and mixed arguments */
__attribute__((noinline, noclone))
double test2(double a, double b, int c, double d, int e, double f) {
    volatile double fp_save[6];
    volatile int int_save[6];
    
    fp_save[0] = a; fp_save[1] = b; fp_save[2] = d; fp_save[3] = f;
    int_save[0] = c; int_save[1] = e;
    
    double result = 0.0;
    
    /* Switch statement with calls in different cases */
    switch (c & 3) {
        case 0:
            opaque_func3(a);
            result = fp_save[0] + fp_save[1];
            break;
        case 1:
            /* Force xmm register pressure */
            asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                        "xmm4", "xmm5", "xmm6", "xmm7");
            result = b * d;
            break;
        case 2:
            opaque_func1();
            result = f - a;
            break;
        default:
            /* Complex call with varargs */
            void* dummy = &global_counter;
            opaque_func4(dummy, a, b, c, d, e, f);
            result = fp_save[2] / fp_save[3];
            if (BB_END_NEEDED) {  /* This label triggers special handling */
                asm volatile("nop");
            }
    }
    
    /* Nested calls to create save/restore sequences */
    for (int i = 0; i < 3; i++) {
        double temp = opaque_func3(result);
        result += temp;
        
        /* Use integer saves after floating point call */
        result += int_save[i % 2];
        
        if (i == 1) {
            /* Another call site */
            opaque_func2(c + i);
        }
    }
    
    return result;
}

/* Test function 3: Vector-like operations with __builtin_apply */
__attribute__((noinline, noclone))
long test3(long a, long b, long c, long d) {
    /* Use __builtin_apply to create unusual register pressure */
    volatile long args[4] = {a, b, c, d};
    
    /* Force values into specific registers */
    register long r10_val asm("r10") = a;
    register long r11_val asm("r11") = b;
    register long r12_val asm("r12") = c;
    register long r13_val asm("r13") = d;
    
    asm volatile("" : "+r"(r10_val), "+r"(r11_val), "+r"(r12_val), "+r"(r13_val));
    
    long result = 0;
    
    /* Complex expression requiring temporaries after call */
    result = (r10_val * r11_val) + (r12_val / (r13_val ? r13_val : 1));
    
    /* Call with many clobbered registers */
    asm volatile("mov %0, %%rax\n\t"
                 "mov %1, %%rbx\n\t"
                 "mov %2, %%rcx\n\t"
                 "mov %3, %%rdx\n\t"
                 "call opaque_func1\n\t"
                 : 
                 : "r"(r10_val), "r"(r11_val), "r"(r12_val), "r"(r13_val)
                 : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory");
    
    /* Use original values after call - forcing saves/restores */
    result += args[0] + args[1] + args[2] + args[3];
    
    /* Loop with conditional call at end */
    int i = 0;
    while (1) {
        result += i++;
        if (i > 5) {
            opaque_func2(i);
            /* Force basic block boundary manipulation */
            if (result > 1000) {
                goto loop_exit;
            }
        }
        if (i >= 10) break;
    }
    loop_exit:
    
    return result;
}

/* Helper with nested calls to create complex save/restore sequences */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    volatile int stack[8];
    for (int i = 0; i < 8; i++) {
        stack[i] = val + i;
    }
    
    /* Recursive call */
    int temp = nested_helper(depth - 1, val * 2);
    
    /* Use stack values after call */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += stack[i];
    }
    
    /* Another call */
    opaque_func2(temp);
    
    return sum + temp;
}

/* Test function 4: Mixed types and __builtin_va_arg simulation */
__attribute__((noinline, noclone))
double test4(int count, ...) {
    va_list args;
    va_start(args, count);
    
    volatile double fp_vals[10];
    volatile int int_vals[10];
    
    /* Read variadic arguments */
    for (int i = 0; i < count && i < 10; i++) {
        if (i % 2 == 0) {
            fp_vals[i] = va_arg(args, double);
        } else {
            int_vals[i] = va_arg(args, int);
        }
    }
    va_end(args);
    
    double result = 0.0;
    
    /* Multiple call sites with intervening computations */
    for (int i = 0; i < 3; i++) {
        /* Save to global to force memory traffic */
        global_fp[i] = fp_vals[i % count];
        global_data[i] = int_vals[i % count];
        
        /* Call that might trigger BB_END update */
        if (i == 1) {
            asm volatile("" : : : "memory");
            opaque_func1();
            asm volatile("" : : : "memory");
            
            /* Complex use of saved values */
            result += fp_vals[0] * int_vals[0];
            result -= fp_vals[1] / (int_vals[1] + 1);
        }
    }
    
    /* Switch with default label containing call */
    switch (count % 4) {
        case 0: result += 1.0; break;
        case 1: result += 2.0; break;
        case 2: 
            opaque_func3(result);
            break;
        default:
            /* This creates a basic block ending with a call */
            void* ptr = &global_counter;
            opaque_func4(ptr, result, count);
            /* Force BB_END update scenario */
            if (global_counter & 1) {
                result *= 2.0;
            }
    }
    
    return result;
}

/* Main function with mode selection */
int main(int argc, char *argv[]) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize some global data */
    for (int i = 0; i < 256; i++) {
        global_data[i] = i * 3;
        global_fp[i] = i * 1.5;
    }
    
    int int_result = 0;
    double fp_result = 0.0;
    long long_result = 0;
    
    /* Execute all test functions regardless of mode, but in different orders */
    switch (mode) {
        case 0:
            int_result = test1(1, 2, 3, 4, 5, 6, 7, 8);
            fp_result = test2(1.1, 2.2, 3, 4.4, 5, 6.6);
            long_result = test3(100, 200, 300, 400);
            fp_result += test4(4, 1.0, 10, 2.0, 20);
            break;
        case 1:
            fp_result = test2(2.2, 3.3, 4, 5.5, 6, 7.7);
            int_result = test1(9, 8, 7, 6, 5, 4, 3, 2);
            fp_result += test4(3, 3.0, 30, 4.0);
            long_result = test3(500, 600, 700, 800);
            break;
        case 2:
            long_result = test3(1000, 2000, 3000, 4000);
            int_result = nested_helper(3, 42);
            fp_result = test2(10.1, 20.2, 30, 40.4, 50, 60.6);
            fp_result += test4(5, 5.0, 50, 6.0, 60, 7.0);
            break;
        case 3:
            fp_result = test4(6, 1.1, 11, 2.2, 22, 3.3, 33);
            int_result = test1(11, 22, 33, 44, 55, 66, 77, 88);
            long_result = test3(150, 250, 350, 450);
            fp_result += test2(15.5, 25.5, 35, 45.5, 55, 65.5);
            break;
        default:
            /* Execute all in sequence with additional nested calls */
            int_result = test1(100, 200, 300, 400, 500, 600, 700, 800);
            fp_result = test2(100.1, 200.2, 300, 400.4, 500, 600.6);
            long_result = test3(10000, 20000, 30000, 40000);
            fp_result += test4(8, 10.0, 100, 20.0, 200, 30.0, 300, 40.0, 400);
            
            /* Additional pressure with nested helper */
            for (int i = 0; i < 3; i++) {
                int_result += nested_helper(2, i * 100);
            }
    }
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_data[i];
        checksum += (long)global_fp[i];
    }
    checksum += int_result + (long)fp_result + long_result + global_counter;
    
    printf("Result: int=%d, fp=%f, long=%ld, checksum=%ld\n", 
           int_result, fp_result, long_result, checksum);
    
    return (checksum & 255);
}

/* Dummy definitions for external functions to allow linking */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2 + global_counter;
}

double opaque_func3(double x) {
    return x * 1.5 + global_counter;
}

void opaque_func4(void* ptr, ...) {
    va_list args;
    va_start(args, ptr);
    /* Consume some arguments */
    for (int i = 0; i < 3; i++) {
        if (i % 2 == 0) {
            double d = va_arg(args, double);
            global_fp[i] += d;
        } else {
            int n = va_arg(args, int);
            global_data[i] += n;
        }
    }
    va_end(args);
    global_counter += 2;
}
