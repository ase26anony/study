/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(void*, void*);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 0;
volatile double global_volatile_double = 0.0;
volatile void* global_volatile_ptr = NULL;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_table[10];

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f;
    double x, y, z;
    void* p1, *p2;
};

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) \
    register typeof(var) var asm(reg) = (typeof(var))0

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many variables to be live across call */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile double d1 = 1.1, d2 = 2.2, d3 = 3.3;
    volatile void* p1 = &v1, *p2 = &v2;
    
    /* Use explicit register variables */
    FORCE_REGISTER(r12_var, "r12");
    FORCE_REGISTER(r13_var, "r13");
    FORCE_REGISTER(r14_var, "r14");
    
    r12_var = v1 + v2;
    r13_var = v3 * v4;
    r14_var = (int)(d1 * 100);
    
    /* Complex expression requiring temporary registers */
    int complex_expr = (v1 * v2) + (v3 / v4) - (v5 << 2);
    
    /* Save to volatile array */
    volatile int saved[10];
    saved[0] = v1; saved[1] = v2; saved[2] = v3;
    saved[3] = v4; saved[4] = v5; saved[5] = complex_expr;
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        /* Direct call with inline asm clobber */
        asm volatile("call opaque_func1" : : : "eax", "ebx", "ecx", "edx", "memory");
    } else {
        /* Indirect call */
        func_table[0]();
    }
    
    COMPILER_BARRIER();
    
    /* Use saved values in complex way */
    int result = saved[0] * saved[1] + saved[2] - saved[3] / saved[4];
    result += r12_var + r13_var - r14_var;
    
    /* Force register pressure with another call */
    opaque_func2(result);
    
    /* Use all variables again */
    global_volatile_int = v1 + v2 + v3 + v4 + v5 + result;
    global_volatile_double = d1 + d2 + d3 + result;
}

/* Test function 2: Nested calls with irreducible control flow */
__attribute__((noinline, noclone))
void test2(int depth) {
    volatile int counters[8] = {0};
    volatile double accumulators[4] = {0.0};
    
    /* Create complex control flow with goto */
    if (depth > 0) {
        int i = 0;
    loop_start:
        for (; i < 5; i++) {
            if (i == 2) {
                /* Function call in the middle of loop */
                opaque_func3(accumulators[i % 4]);
                goto special_case;
            }
            
            counters[i] += i * depth;
            accumulators[i % 4] += counters[i] * 0.5;
            
            if (i == 3 && depth > 2) {
                /* Another call with register pressure */
                int temp = counters[0] + counters[1] + counters[2];
                opaque_func2(temp);
                continue;
            }
            
            special_case:
            if (i == 4) {
                /* Force BB boundary manipulation */
                asm volatile("" : : : "r10", "r11", "r12", "r13", "memory");
                break;
            }
        }
        
        if (depth > 1) {
            /* Recursive-like call pattern */
            test2(depth - 1);
            goto loop_start;
        }
    }
    
    /* Final computation using all live variables */
    double final_sum = 0;
    for (int j = 0; j < 8; j++) {
        final_sum += counters[j];
    }
    for (int j = 0; j < 4; j++) {
        final_sum += accumulators[j];
    }
    
    global_volatile_double += final_sum;
}

/* Test function 3: Switch statement with calls in cases */
__attribute__((noinline, noclone))
void test3(int selector) {
    volatile int reg_vars[16];
    
    /* Initialize with pattern */
    for (int i = 0; i < 16; i++) {
        reg_vars[i] = i * selector;
    }
    
    /* Complex switch creating multiple BBs */
    switch (selector & 7) {
        case 0: {
            /* Many live variables */
            int sum = reg_vars[0] + reg_vars[1] + reg_vars[2];
            opaque_func2(sum);
            /* Fall through */
        }
        case 1: {
            double prod = reg_vars[3] * reg_vars[4] * 1.5;
            opaque_func3(prod);
            break;
        }
        case 2: {
            /* Force register spilling */
            FORCE_REGISTER(r10_var, "r10");
            FORCE_REGISTER(r11_var, "r11");
            r10_var = reg_vars[5] * reg_vars[6];
            r11_var = reg_vars[7] / reg_vars[8];
            opaque_func2(r10_var + r11_var);
            break;
        }
        case 3: {
            /* Multiple calls in sequence */
            for (int i = 0; i < 3; i++) {
                opaque_func2(reg_vars[9 + i]);
            }
            break;
        }
        default: {
            /* Most complex case - many live vars across call */
            int total = 0;
            for (int i = 0; i < 16; i++) {
                total += reg_vars[i];
                if (i % 4 == 3) {
                    /* Call in loop with live variables */
                    opaque_func2(total);
                    total = 0;
                }
            }
            /* One more call at BB end */
            asm volatile("call opaque_func4" : : "r"(&reg_vars[0]), "r"(&reg_vars[8]) : "memory", "rax", "rbx", "rcx", "rdx");
            break;
        }
    }
    
    /* Use all variables again after switch */
    int final_check = 0;
    for (int i = 0; i < 16; i++) {
        final_check ^= reg_vars[i];
    }
    global_volatile_int ^= final_check;
}

/* Test function 4: Mixed types and __builtin_apply */
__attribute__((noinline, noclone))
void test4(void) {
    /* Create varying types */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3;
    volatile double vd1 = 1.0, vd2 = 2.0;
    volatile long vl1 = 100, vl2 = 200;
    volatile void* vp1 = &vi1, *vp2 = &vi2;
    
    /* Use __builtin_apply to create unusual call sequence */
    typedef int (*vararg_func_t)(int, ...);
    vararg_func_t volatile varfunc = (vararg_func_t)opaque_func2;
    
    /* Build arguments */
    __builtin_va_list args;
    int args_array[10];
    args_array[0] = vi1;
    args_array[1] = vi2;
    args_array[2] = vi3;
    args_array[3] = (int)vd1;
    args_array[4] = (int)vd2;
    
    /* Simulate complex register usage around call */
    FORCE_REGISTER(r15_var, "r15");
    r15_var = vi1 * vi2 * vi3;
    
    COMPILER_BARRIER();
    
    /* Make the call */
    int result = varfunc(5, args_array[0], args_array[1], args_array[2], 
                         args_array[3], args_array[4]);
    
    COMPILER_BARRIER();
    
    /* Use all variables in complex expression */
    long final_result = (long)result * vl1 / vl2 + r15_var;
    final_result += (long)(vd1 * vd2 * 1000);
    
    /* Another call with the result */
    opaque_func2((int)final_result);
    
    /* Store to globals */
    global_volatile_int += (int)final_result;
    global_volatile_ptr = (void*)final_result;
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x, int y) {
    volatile int a = x, b = y;
    volatile double c = x * 1.5, d = y * 2.5;
    
    /* First call */
    int r1 = opaque_func2(a + b);
    
    /* Complex computation between calls */
    int intermediate = (a * b) + (r1 << 2);
    intermediate /= (b ? b : 1);
    
    /* Nested call scenario */
    if (intermediate > 100) {
        FORCE_REGISTER(rbx_var, "rbx");
        rbx_var = intermediate;
        
        /* Call that might trigger save/restore insertion */
        opaque_func3(c + d);
        
        intermediate += rbx_var;
    }
    
    /* Final call */
    return opaque_func2(intermediate);
}

/* Main test driver */
int main(int argc, char** argv) {
    int test_mode = 0;
    
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize function table with opaque functions */
    for (int i = 0; i < 10; i++) {
        func_table[i] = (func_ptr_t)opaque_func1;
    }
    
    /* Run all tests in sequence, but with different orders based on mode */
    switch (test_mode) {
        case 0:
            test1(1); test2(3); test3(5); test4();
            break;
        case 1:
            test4(); test3(7); test2(2); test1(0);
            break;
        case 2:
            test2(4); test1(1); test4(); test3(6);
            break;
        case 3:
            for (int i = 0; i < 2; i++) {
                test1(i); test3(i * 3);
            }
            test2(3); test4();
            break;
        case 4:
            /* Test nested calls extensively */
            for (int i = 0; i < 10; i++) {
                int result = helper_with_nested_call(i, i * 2);
                global_volatile_int += result;
            }
            test1(1); test2(2); test3(3); test4();
            break;
    }
    
    /* Create irreducible control flow with goto */
    volatile int checksum = global_volatile_int;
    volatile double dchecksum = global_volatile_double;
    
    if (checksum > 1000) {
        goto compute_more;
    }
    
    checksum += (int)dchecksum;
    
compute_more:
    /* Force BB manipulation with call at end */
    opaque_func2(checksum);
    
    /* Another BB that might get merged */
    {
        volatile int extra = checksum * 2;
        asm volatile("" : : : "r10", "r11", "r12", "r13", "r14", "r15", "memory");
        opaque_func3(extra * 1.5);
    }
    
    /* Final output to prevent elimination */
    printf("Result: %d (%.2f)\n", checksum, dchecksum);
    
    return checksum & 0xFF;
}

/* Dummy implementations of opaque functions to satisfy linker */
void opaque_func1(void) {
    COMPILER_BARRIER();
}

int opaque_func2(int x) {
    COMPILER_BARRIER();
    return x ^ 0x55AA55AA;
}

double opaque_func3(double x) {
    COMPILER_BARRIER();
    return x * 1.61803398875;
}

void opaque_func4(void* a, void* b) {
    COMPILER_BARRIER();
    global_volatile_ptr = a;
}
