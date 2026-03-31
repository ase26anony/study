/* caller_save_test.c - Test program to trigger uncovered lines in GCC's caller-save.cc */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller_save_test.c -o caller_save_test -ldl */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>

/* Global volatile variables to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32];
volatile double global_fp[16];

/* Opaque function declarations (won't be defined, forcing conservative assumptions) */
void opaque_call_1(void) __attribute__((noinline, noclone));
void opaque_call_2(int, long, double) __attribute__((noinline, noclone));
int opaque_call_3(va_list) __attribute__((noinline, noclone));

/* Function pointer that will be initialized with dlsym */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile_func_ptr = NULL;

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force register pressure with many local variables */
    register int r0 asm ("r10") = mode + 1;
    register int r1 asm ("r11") = mode + 2;
    volatile int v0 = mode * 3;
    volatile int v1 = mode * 4;
    volatile int v2 = mode * 5;
    volatile int v3 = mode * 6;
    volatile int v4 = mode * 7;
    volatile int v5 = mode * 8;
    
    /* Array to force stack usage */
    int stack_array[8];
    for (int i = 0; i < 8; i++) {
        stack_array[i] = mode * (i + 10);
    }
    
    /* Inline asm with clobbers to force specific register usage */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        : 
        : "r" (r0), "r" (r1)
        : "eax", "memory"
    );
    
    /* Function call that clobbers registers */
    opaque_call_1();
    
    /* Use all variables after call - forces reload to insert save/restore */
    v0 = r0 + r1 + v0;
    v1 = v0 * 2 + v1;
    
    /* Another asm barrier */
    asm volatile ("" : : : "memory");
    
    /* Complex expression requiring temporary registers */
    int complex_result = (v0 * v1) + (v2 * v3) - (v4 / (v5 ? v5 : 1));
    for (int i = 0; i < 8; i++) {
        complex_result += stack_array[i];
    }
    
    /* Store to global to prevent elimination */
    global_array[0] = complex_result + r0 + r1;
}

/* Function with mixed types and calling convention stress */
__attribute__((noinline, noclone))
void test2(double a, float b, int c, long d) {
    /* Volatile doubles to force FP register pressure */
    volatile double d1 = a * 2.0;
    volatile double d2 = b * 3.0f;
    volatile double d3 = c * 4.0;
    volatile double d4 = d * 5.0;
    
    /* Integer registers that must survive across call */
    register long rl0 asm ("r12") = d;
    register long rl1 asm ("r13") = c * 2L;
    
    /* Inline asm that acts like a call */
    asm volatile (
        "movq %0, %%rax\n\t"
        "movq %1, %%rbx\n\t"
        "call *%2\n\t"
        : 
        : "r" (rl0), "r" (rl1), "r" (volatile_func_ptr)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory"
    );
    
    /* Use FP values after "call" */
    d1 = d2 + d3;
    d4 = d1 * d4;
    
    /* Conditional that creates basic block boundary */
    if (c > 0) {
        /* Function call at block boundary */
        opaque_call_2(c, d, d4);
        
        /* Complex control flow with goto to force block merging */
        if (d4 > 100.0) {
            goto merge_point;
        } else {
            d1 *= 2.0;
        }
    } else {
        d1 /= 2.0;
    }
    
    /* Another call site */
    opaque_call_1();
    
merge_point:
    /* Use all live variables */
    global_fp[0] = d1 + d2 + d3 + d4;
    global_array[1] = rl0 + rl1 + c;
}

/* Function with irreducible control flow */
__attribute__((noinline, noclone))
void test3(int iterations) {
    volatile int counters[10];
    for (int i = 0; i < 10; i++) {
        counters[i] = i * iterations;
    }
    
    /* Create complex loop structure with nested breaks/continues */
    int i = 0;
    volatile int sum = 0;
    
loop_start:
    if (i >= iterations) goto loop_end;
    
    /* Function call inside loop */
    if (i % 3 == 0) {
        opaque_call_1();
    }
    
    /* Switch with default that calls function */
    switch (i % 4) {
        case 0:
            sum += counters[0];
            break;
        case 1:
            sum += counters[1];
            /* Fall through */
        case 2:
            sum += counters[2];
            opaque_call_2(i, sum, 0.0);
            break;
        default:
            /* This creates a call at block end */
            asm volatile ("call *%0" : : "r" (volatile_func_ptr) : "memory");
            sum += counters[3];
            /* Force block split here */
            if (sum > 1000) {
                goto early_exit;
            }
    }
    
    /* Another call site */
    if (i % 5 == 0) {
        opaque_call_1();
        /* Use goto to create irreducible flow */
        if (sum % 2 == 0) {
            goto skip_increment;
        }
    }
    
    i++;
    goto loop_start;
    
skip_increment:
    sum *= 2;
    i++;
    goto loop_start;
    
early_exit:
    /* Early exit path */
    opaque_call_1();
    
loop_end:
    global_array[2] = sum;
}

/* Function using __builtin_apply for unusual calling patterns */
__attribute__((noinline, noclone))
void test4(void) {
    /* Variable argument simulation */
    va_list args;
    int buffer[10];
    
    for (int i = 0; i < 10; i++) {
        buffer[i] = i * 100 + global_counter;
    }
    
    /* Force register pressure around va_arg-like operation */
    register int r0 asm ("r14") = buffer[0];
    register int r1 asm ("r15") = buffer[1];
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Call opaque function */
    int result = opaque_call_3(args);
    
    /* Use registers that must be saved across call */
    r0 = r0 + result;
    r1 = r1 * result;
    
    /* Complex expression with multiple temporaries */
    int temp = 0;
    for (int i = 0; i < 10; i++) {
        temp += buffer[i] * (i + 1);
    }
    
    /* Another call */
    opaque_call_1();
    
    /* Final computation using all values */
    global_array[3] = r0 + r1 + temp;
}

/* Helper with nested calls to force instruction insertion between calls */
__attribute__((noinline, noclone))
int nested_helper(int depth, int value) {
    volatile int local = value;
    
    if (depth > 0) {
        /* Recursive call */
        int result = nested_helper(depth - 1, value * 2);
        
        /* Function call between recursive calls */
        opaque_call_1();
        
        local += result;
    } else {
        /* Base case with call */
        opaque_call_2(value, value * 10L, 3.14);
    }
    
    /* Inline asm to prevent tail call optimization */
    asm volatile ("" : "+r" (local) : : "memory");
    
    return local;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize function pointer with dlsym if available */
    void *handle = dlopen(NULL, RTLD_LAZY);
    if (handle) {
        volatile_func_ptr = (func_ptr_t)dlsym(handle, "printf");
        dlclose(handle);
    }
    
    /* Use argv to create runtime variability */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize globals */
    for (int i = 0; i < 32; i++) {
        global_array[i] = i * 100;
    }
    for (int i = 0; i < 16; i++) {
        global_fp[i] = i * 1.5;
    }
    
    /* Run all tests in sequence, but with different order based on mode */
    switch (test_mode) {
        case 0:
            test1(1);
            test2(1.0, 2.0f, 3, 4L);
            test3(5);
            test4();
            break;
        case 1:
            test2(2.0, 3.0f, 4, 5L);
            test4();
            test1(2);
            test3(6);
            break;
        case 2:
            test3(7);
            test1(3);
            test4();
            test2(3.0, 4.0f, 5, 6L);
            break;
        case 3:
            test4();
            test3(8);
            test2(4.0, 5.0f, 6, 7L);
            test1(4);
            break;
    }
    
    /* Also test nested helper */
    int nested_result = nested_helper(3, 10);
    global_array[4] = nested_result;
    
    /* Compute checksum to prevent dead code elimination */
    long checksum = 0;
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (long)global_fp[i];
    }
    
    /* Use checksum in output */
    printf("Checksum: %ld\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}

/* Dummy definitions to satisfy linker (these won't actually be called 
   if dlsym fails, but prevent linker errors) */
void opaque_call_1(void) {
    /* Empty - compiler doesn't know this */
}

void opaque_call_2(int a, long b, double c) {
    global_counter += a + (int)b + (int)c;
}

int opaque_call_3(va_list args) {
    (void)args;
    return global_counter++;
}
