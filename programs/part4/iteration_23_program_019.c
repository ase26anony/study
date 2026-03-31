/* test-caller-save.c
 * Comprehensive test to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets lines 905-913 dealing with instruction chain manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to force call instructions */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;
volatile int global_array[256];

/* Function pointer to create opaque calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile_func_ptr;

/* Complex structure to increase register pressure */
struct LargeStruct {
    long a, b, c, d, e, f, g, h;
    double x, y, z;
    int arr[8];
};

/* Force specific registers with asm constraints */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register int reg_var3 asm ("r14");

/* Test function 1: Many live variables across call */
__attribute__((noinline, noclone))
void test1_many_live_vars(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile double f1 = 1.0, f2 = 2.0, f3 = 3.0;
    volatile long l1 = 100, l2 = 200, l3 = 300;
    
    /* Use inline asm to clobber specific registers */
    asm volatile ("# Clobber eax, r10, r11" : : : "eax", "r10", "r11", "memory");
    
    /* Force a function call with many live variables */
    opaque_call_1();
    
    /* Complex use of all variables after call */
    int sum = v1 + v2 + v3 + v4 + v5;
    double prod = f1 * f2 * f3;
    long diff = l1 - l2 - l3;
    
    /* More asm to prevent optimization */
    asm volatile ("# Use variables" : : "r"(sum), "r"(l1), "x"(prod) : "memory");
    
    global_array[sum % 256] ^= diff;
}

/* Test function 2: Complex control flow with calls at block boundaries */
__attribute__((noinline, noclone))
void test2_complex_cfg(int n) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    int i = 0;
    
    /* Irreducible control flow with goto */
    if (n & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Call at basic block boundary */
    opaque_call_2(a);
    
    /* Force register save/restore around call */
    asm volatile ("# Save rbx, rbp" : : : "rbx", "rbp", "memory");
    
    for (i = 0; i < n; i++) {
        if (i % 3 == 0) {
            /* Call inside loop with break */
            int result = opaque_call_3(i, a);
            if (result > 100) break;
            a += result;
        } else if (i % 3 == 1) {
            /* Continue with another call */
            opaque_call_2(b);
            continue;
        } else {
            /* Default case with switch */
            switch (i % 4) {
                case 0: opaque_call_1(); break;
                case 1: opaque_call_2(c); break;
                case 2: opaque_call_3(d, i); break;
                default: 
                    /* This should create block splitting */
                    if (volatile_func_ptr) volatile_func_ptr();
                    break;
            }
        }
        
        /* Complex expression requiring temporary registers */
        c = (a * b + c * d - i) / (a + 1);
        asm volatile ("# Computation" : "+r"(c) : "r"(a), "r"(b), "r"(d) : "memory");
    }
    
label2:
    /* Another call at block end */
    d = opaque_call_3(c, d);
    global_counter += d;
}

/* Test function 3: Register variables and explicit register usage */
__attribute__((noinline, noclone))
void test3_register_vars(int x) {
    /* Use register variables declared globally */
    reg_var1 = x;
    reg_var2 = x * 2;
    reg_var3 = x * 3;
    
    /* Force them to be live across call */
    asm volatile ("# Setup reg vars" : : "r"(reg_var1), "r"(reg_var2), "r"(reg_var3) : "memory");
    
    /* Call that clobbers registers */
    opaque_call_1();
    
    /* Immediate use after call requiring same registers */
    int sum = reg_var1 + reg_var2 + reg_var3;
    
    /* Inline asm acting as pseudo-call */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(sum)
        : "r"(reg_var1), "r"(reg_var2), "r"(reg_var3)
        : "eax", "memory"
    );
    
    global_checksum ^= sum;
}

/* Test function 4: Floating point and mixed mode */
__attribute__((noinline, noclone))
double test4_fp_mixed(int count, ...) {
    va_list ap;
    double fp_acc = 0.0;
    int int_acc = 0;
    
    va_start(ap, count);
    
    for (int i = 0; i < count; i++) {
        if (i % 2 == 0) {
            double val = va_arg(ap, double);
            fp_acc += opaque_call_4(val, fp_acc);
            
            /* Force FP register save/restore */
            asm volatile ("# FP clobber" : : : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        } else {
            int val = va_arg(ap, int);
            int_acc += opaque_call_3(val, int_acc);
            
            /* Mix with FP operations */
            fp_acc += (double)int_acc;
        }
        
        /* Call at potential block boundary */
        if (i == count / 2) {
            opaque_call_2(int_acc);
        }
    }
    
    va_end(ap);
    
    /* Complex expression with function result */
    return fp_acc * 2.0 - (double)int_acc;
}

/* Test function 5: Nested calls and __builtin_apply */
__attribute__((noinline, noclone))
void test5_builtin_apply(void) {
    struct LargeStruct ls;
    memset(&ls, 0, sizeof(ls));
    
    /* Initialize with volatile values */
    for (int i = 0; i < 8; i++) {
        ls.arr[i] = global_array[i] + i;
    }
    
    /* Force many registers to be live */
    long r1 = ls.a = 1;
    long r2 = ls.b = 2;
    long r3 = ls.c = 3;
    long r4 = ls.d = 4;
    double f1 = ls.x = 1.5;
    double f2 = ls.y = 2.5;
    
    /* Simulate __builtin_apply behavior */
    void* args = __builtin_apply_args();
    
    /* Call with many arguments */
    asm volatile (
        "# Complex call setup\n\t"
        : : : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "memory"
    );
    
    /* Nested call scenario */
    opaque_call_1();
    
    /* Use values after call */
    long sum = r1 + r2 + r3 + r4;
    double fsum = f1 + f2;
    
    /* Another call */
    opaque_call_2((int)sum);
    
    /* Final computation */
    global_checksum += (int)(sum + fsum);
}

/* Helper with nested call */
__attribute__((noinline, noclone))
int helper_nested_call(int depth, int val) {
    if (depth <= 0) {
        return val;
    }
    
    volatile int saved = val;
    
    /* Inner call */
    int result = opaque_call_3(val, depth);
    
    /* Outer call context still has 'saved' live */
    asm volatile ("# Keep saved live" : : "r"(saved) : "memory");
    
    /* Recursive call */
    return helper_nested_call(depth - 1, result + saved);
}

/* Test function 6: Switch with calls in default */
__attribute__((noinline, noclone))
void test6_switch_default(int key) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile int e = 5, f = 6, g = 7, h = 8;
    
    /* Complex switch creating many basic blocks */
    switch (key) {
        case 0:
            a = opaque_call_3(b, c);
            break;
        case 1:
            opaque_call_2(d);
            break;
        case 2:
            e = f + g;
            opaque_call_1();
            break;
        case 3:
            asm volatile ("# Case 3" : : : "rax", "rbx", "rcx", "memory");
            break;
        default:
            /* This creates block boundary manipulation */
            if (key > 100) {
                opaque_call_1();
                a = b + c;
            } else {
                opaque_call_2(key);
                c = d + e;
            }
            
            /* Force insertion at block end */
            f = opaque_call_3(g, h);
            
            /* More complex control flow */
            for (int i = 0; i < key % 10; i++) {
                if (i % 2 == 0) {
                    opaque_call_1();
                } else {
                    opaque_call_2(i);
                }
            }
            break;
    }
    
    /* Use all variables to keep them live */
    global_array[key % 256] = a + b + c + d + e + f + g + h;
}

/* Main function with mode selection */
int main(int argc, char **argv) {
    int test_mode = 0;
    
    /* Parse test mode from args */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 7;
    }
    
    /* Initialize volatile function pointer */
    volatile_func_ptr = opaque_call_1;
    
    /* Initialize global array with pattern */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Execute all tests but with different order based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 7) {
            case 0:
                test1_many_live_vars();
                break;
            case 1:
                test2_complex_cfg(cycle + 10);
                break;
            case 2:
                test3_register_vars(cycle * 5 + 1);
                break;
            case 3:
                {
                    double result = test4_fp_mixed(5, 1.0, 2, 3.0, 4, 5.0);
                    global_checksum += (int)result;
                }
                break;
            case 4:
                test5_builtin_apply();
                break;
            case 5:
                {
                    int nested_result = helper_nested_call(3, cycle * 7);
                    global_counter += nested_result;
                }
                break;
            case 6:
                test6_switch_default(cycle * 50 + 25);
                break;
        }
        
        /* Compiler barrier */
        asm volatile ("# Cycle %0" : : "r"(cycle) : "memory");
    }
    
    /* Compute final checksum to prevent DCE */
    int final_checksum = global_counter + global_checksum;
    for (int i = 0; i < 256; i++) {
        final_checksum ^= global_array[i];
    }
    
    /* Use scanf to make values runtime-dependent */
    if (argc > 2) {
        int extra;
        sscanf(argv[2], "%d", &extra);
        final_checksum += extra;
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return final_checksum & 255;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_call_1(void) {
    global_counter++;
}

void opaque_call_2(int x) {
    global_checksum ^= x;
}

int opaque_call_3(int a, int b) {
    return a + b + global_counter;
}

double opaque_call_4(double a, double b) {
    return a * b + global_counter;
}
