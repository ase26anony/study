/* caller-save-test.c
 * Test program to trigger uncovered code in GCC's caller-save.cc
 * Specifically targets lines 905-913 which handle instruction chain manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void* opaque_func4(void*);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[256] = {0};
volatile double global_fp[128] = {0.0};

/* Function pointer to create indirect calls */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr = NULL;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit constraints */
#define USE_REGISTER(reg, val) \
    do { \
        register long reg##_val asm(reg) = (val); \
        asm volatile("" : "+r"(reg##_val)); \
    } while(0)

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1_many_live_vars(int mode) {
    /* Force many variables to be live across call */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    /* Use explicit register variables */
    register int r10_val asm("r10") = 100;
    register int r11_val asm("r11") = 101;
    register int r12_val asm("r12") = 102;
    
    COMPILER_BARRIER();
    
    /* Complex control flow with calls at boundaries */
    if (mode & 1) {
        /* Call that clobbers registers */
        asm volatile(
            "movl $0x12345678, %%eax\n\t"
            "movl $0x9ABCDEF0, %%ebx\n\t"
            "call *%0\n\t"
            : 
            : "r"(opaque_func1)
            : "eax", "ebx", "ecx", "edx", "memory"
        );
        
        /* Use all volatile variables after call */
        int sum = v1 + v2 + v3 + v4 + v5 + 
                 v6 + v7 + v8 + v9 + v10 +
                 r10_val + r11_val + r12_val;
        
        global_array[0] = sum;
    } else {
        /* Different path with another call */
        opaque_func2(mode);
        
        /* More register pressure */
        USE_REGISTER("rax", v1);
        USE_REGISTER("rbx", v2);
        USE_REGISTER("rcx", v3);
    }
    
    /* Irreducible control flow with goto */
    if (mode & 2) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Nested call in goto target */
    opaque_func3(3.14);
    goto end;
    
label2:
    /* Another call with different clobbers */
    asm volatile(
        "movq $0x1122334455667788, %%r10\n\t"
        "movq $0x8877665544332211, %%r11\n\t"
        : : : "r10", "r11", "memory"
    );
    opaque_func4((void*)&global_counter);
    
end:
    /* Force register restore needs */
    global_counter += v1 + r10_val;
}

/* Test function 2: Complex loop with calls at block boundaries */
__attribute__((noinline, noclone))
void test2_loop_with_calls(int iterations) {
    volatile int accum = 0;
    volatile int temp[10];
    
    /* Initialize temps */
    for (int i = 0; i < 10; i++) {
        temp[i] = i * 2;
    }
    
    /* Loop with break/continue creating block splits */
    for (int i = 0; i < iterations; i++) {
        /* Register pressure before call */
        register int r8_val asm("r8") = i * 3;
        register int r9_val asm("r9") = i * 5;
        
        if (i % 3 == 0) {
            /* Call at basic block boundary */
            int result = opaque_func2(i);
            
            /* Use result immediately requiring register */
            accum += result * r8_val;
            
            if (accum > 1000) {
                /* Break creates block end */
                opaque_func1();
                break;
            }
        } else if (i % 3 == 1) {
            /* Continue creates another boundary */
            opaque_func3(i * 1.5);
            continue;
        } else {
            /* Default case with asm clobber */
            asm volatile(
                "movl %0, %%eax\n\t"
                "addl $1, %%eax\n\t"
                : : "r"(i) : "eax", "memory"
            );
        }
        
        /* Use all temps after potential calls */
        for (int j = 0; j < 10; j++) {
            accum += temp[j] * r9_val;
        }
        
        COMPILER_BARRIER();
    }
    
    global_array[1] = accum;
}

/* Test function 3: Switch statement with calls in cases */
__attribute__((noinline, noclone))
void test3_switch_with_calls(int value) {
    volatile double fp_acc = 0.0;
    volatile int int_acc = 0;
    
    /* Multiple register variables */
    register double xmm0_val asm("xmm0") = 1.0;
    register double xmm1_val asm("xmm1") = 2.0;
    register int eax_val asm("eax") = 100;
    
    COMPILER_BARRIER();
    
    /* Switch creates complex CFG */
    switch (value % 5) {
        case 0:
            /* Call with FP args */
            fp_acc = opaque_func3(xmm0_val * 2.0);
            /* Fall through */
        case 1:
            /* Another call */
            opaque_func1();
            int_acc += eax_val;
            break;
        case 2:
            /* Call with pointer */
            opaque_func4((void*)&global_counter);
            /* Use FP registers after call */
            fp_acc += xmm1_val;
            break;
        case 3:
            /* Inline asm that looks like call */
            asm volatile(
                "pushfq\n\t"
                "call *%0\n\t"
                "popfq\n\t"
                : : "r"(opaque_func1) : "memory"
            );
            int_acc = eax_val * 2;
            break;
        default:
            /* Default case with multiple calls */
            opaque_func2(value);
            opaque_func3(3.14159);
            opaque_func1();
            /* Force register restore */
            USE_REGISTER("rbx", int_acc);
            USE_REGISTER("rcx", (long)&fp_acc);
            break;
    }
    
    /* Use values after switch */
    global_fp[0] = fp_acc + xmm0_val;
    global_counter += int_acc;
}

/* Test function 4: Nested function calls */
__attribute__((noinline, noclone))
int test4_nested_calls(int depth) {
    if (depth <= 0) {
        return opaque_func2(42);
    }
    
    volatile int saved[8];
    /* Save values that need to survive calls */
    for (int i = 0; i < 8; i++) {
        saved[i] = i * depth;
    }
    
    /* Register variables that conflict with call-clobbered regs */
    register int rdi_val asm("rdi") = depth;
    register int rsi_val asm("rsi") = depth * 2;
    
    /* Make call */
    int result = test4_nested_calls(depth - 1);
    
    /* Use saved values and register vars after call */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += saved[i];
    }
    sum += rdi_val + rsi_val;
    
    COMPILER_BARRIER();
    
    /* Another call */
    opaque_func2(result);
    
    return sum + result;
}

/* Test function 5: __builtin_apply usage */
__attribute__((noinline, noclone))
void test5_builtin_apply(void) {
    /* Create artificial register pressure */
    volatile long vars[16];
    for (int i = 0; i < 16; i++) {
        vars[i] = i * 100;
    }
    
    /* Use __builtin_apply to force unusual register usage */
    void* args = __builtin_apply_args();
    
    /* Save register state around builtin */
    register long r10_save asm("r10");
    register long r11_save asm("r11");
    asm volatile("mov %%r10, %0" : "=r"(r10_save));
    asm volatile("mov %%r11, %0" : "=r"(r11_save));
    
    /* Call through apply */
    if (args) {
        /* This creates complex save/restore sequences */
        void* result = __builtin_apply((void(*)())opaque_func4, args, 64);
        if (result) {
            __builtin_return(result);
        }
    }
    
    /* Restore and use */
    asm volatile("mov %0, %%r10" : : "r"(r10_save));
    asm volatile("mov %0, %%r11" : : "r"(r11_save));
    
    /* Use all vars to keep them live */
    long total = 0;
    for (int i = 0; i < 16; i++) {
        total += vars[i];
    }
    global_array[2] = total;
}

/* Helper with irreducible control flow */
__attribute__((noinline, noclone))
void helper_complex_cfg(int x) {
    volatile int a = x;
    volatile int b = x * 2;
    
    /* Create goto-based irreducible flow */
    if (x & 1) {
        goto block1;
    } else {
        goto block2;
    }
    
block1:
    opaque_func1();
    if (a > b) {
        goto block3;
    } else {
        goto block2;
    }
    
block2:
    opaque_func2(b);
    if (b > a) {
        goto block1;
    } else {
        goto block3;
    }
    
block3:
    /* Final block with register usage */
    register int r12_val asm("r12") = a + b;
    asm volatile("" : : "r"(r12_val));
    global_counter += r12_val;
}

int main(int argc, char** argv) {
    int test_mode = 0;
    
    /* Use argv to create runtime variability */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize function pointer */
    volatile_fptr = opaque_func1;
    
    /* Run all tests in different orders based on mode */
    switch (test_mode) {
        case 0:
            test1_many_live_vars(3);
            test2_loop_with_calls(10);
            test3_switch_with_calls(7);
            test4_nested_calls(3);
            test5_builtin_apply();
            break;
        case 1:
            test3_switch_with_calls(2);
            test1_many_live_vars(5);
            test5_builtin_apply();
            test2_loop_with_calls(7);
            test4_nested_calls(2);
            break;
        case 2:
            test4_nested_calls(4);
            test2_loop_with_calls(5);
            test1_many_live_vars(1);
            test3_switch_with_calls(9);
            test5_builtin_apply();
            break;
        case 3:
            test5_builtin_apply();
            test4_nested_calls(1);
            test3_switch_with_calls(4);
            test2_loop_with_calls(3);
            test1_many_live_vars(7);
            break;
        default:
            /* All tests with helper */
            test1_many_live_vars(0);
            helper_complex_cfg(10);
            test2_loop_with_calls(8);
            helper_complex_cfg(20);
            test3_switch_with_calls(1);
            test4_nested_calls(5);
            test5_builtin_apply();
            break;
    }
    
    /* Compute checksum to prevent elimination */
    long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    for (int i = 0; i < 128; i++) {
        checksum += (long)global_fp[i];
    }
    checksum += global_counter;
    
    printf("Checksum: %ld\n", checksum);
    
    return (int)(checksum % 256);
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2 + 1;
}

double opaque_func3(double x) {
    return x * 1.5;
}

void* opaque_func4(void* p) {
    return (void*)((long)p + 1);
}
