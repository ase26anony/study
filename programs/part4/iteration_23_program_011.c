/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets instruction chain manipulation around basic block boundaries
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern void opaque_func2(int) __attribute__((noinline, noclone));
extern int opaque_func3(int, int) __attribute__((noinline, noclone));
extern double opaque_func4(double) __attribute__((noinline, noclone));

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile int global_array[256];
volatile double global_fp_array[256];

/* Function pointer with volatile to prevent constant propagation */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fp = NULL;

/* Complex function with many live registers across calls */
__attribute__((noinline, noclone))
void test1(int a, int b, int c, int d, int e, int f) {
    /* Force register pressure with many live variables */
    register int r1 asm ("r10") = a + 1;
    register int r2 asm ("r11") = b + 2;
    register int r3 asm ("r12") = c + 3;
    register int r4 asm ("r13") = d + 4;
    register int r5 asm ("r14") = e + 5;
    register int r6 asm ("r15") = f + 6;
    
    volatile int stack_save[10];
    
    /* Save register values to stack */
    stack_save[0] = r1;
    stack_save[1] = r2;
    stack_save[2] = r3;
    stack_save[3] = r4;
    stack_save[4] = r5;
    stack_save[5] = r6;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Compiler barrier */
    asm volatile("" : : : "memory");
    
    /* Complex control flow with goto to create basic block boundaries */
    if (r1 > 0) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Restore and use saved values - forces reload to insert save/restore */
    r1 = stack_save[0] + opaque_func3(stack_save[1], stack_save[2]);
    if (r1 > 100) {
        /* Nested call in conditional */
        opaque_func2(r1);
        goto label3;
    }
    
label2:
    /* Another call site */
    r2 = stack_save[3] * stack_save[4];
    opaque_func2(r2);
    
label3:
    /* Use all registers in complex expression */
    int result = r1 + r2 + r3 + r4 + r5 + r6;
    global_array[global_counter++ % 256] = result;
    
    /* Switch statement to create complex CFG */
    switch (result % 4) {
        case 0:
            opaque_func1();
            break;
        case 1:
            opaque_func2(result);
            break;
        case 2:
            /* Fall through with call */
            opaque_func3(result, result + 1);
            /* No break - intentional fallthrough */
        default:
            /* This creates a basic block that ends with a call */
            asm volatile (
                "pushf\n\t"
                "popf\n\t"
                : : : "cc"
            );
            opaque_func1();
            /* The uncovered code handles insertion after this point */
    }
}

/* Function with floating point and mixed register usage */
__attribute__((noinline, noclone))
void test2(double a, double b, double c, double d) {
    volatile double saved[8];
    register double fr1 asm ("xmm0") = a * 1.1;
    register double fr2 asm ("xmm1") = b * 2.2;
    register double fr3 asm ("xmm2") = c * 3.3;
    register double fr4 asm ("xmm4") = d * 4.4;
    
    /* Save FP values */
    saved[0] = fr1;
    saved[1] = fr2;
    saved[2] = fr3;
    saved[3] = fr4;
    
    /* Integer registers also live */
    register int ir1 asm ("r10") = (int)a;
    register int ir2 asm ("r11") = (int)b;
    
    /* Loop with break/continue creating block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            /* Call in loop with break */
            opaque_func4(fr1);
            break;
        } else if (i % 2 == 0) {
            /* Continue with register usage */
            fr1 += 1.0;
            continue;
        }
        
        /* Function call that might split basic block */
        if (i == 3) {
            double temp = opaque_func4(fr2);
            fr3 += temp;
            /* goto to create irreducible flow */
            if (temp > 0) goto loop_exit;
        }
    }
    
loop_exit:
    /* Restore and use values */
    fr1 = saved[0] + saved[1];
    fr2 = saved[2] * saved[3];
    
    /* Complex expression requiring temporary registers */
    double result = fr1 * fr2 + (double)ir1 / (double)ir2;
    global_fp_array[global_counter++ % 256] = result;
    
    /* Call via volatile function pointer */
    if (volatile_fp) {
        volatile_fp();
    }
}

/* Function with __builtin_apply to create unusual register pressure */
__attribute__((noinline, noclone))
void test3(void) {
    /* Use __builtin_apply to create complex call sequences */
    void (*func)(int, int, int, int, int, int) = 
        (void (*)(int, int, int, int, int, int))opaque_func2;
    
    /* Create arguments in registers */
    register int a1 asm ("rdi") = 1;
    register int a2 asm ("rsi") = 2;
    register int a3 asm ("rdx") = 3;
    register int a4 asm ("rcx") = 4;
    register int a5 asm ("r8") = 5;
    register int a6 asm ("r9") = 6;
    
    /* Save them */
    volatile int saved[6];
    saved[0] = a1;
    saved[1] = a2;
    saved[2] = a3;
    saved[3] = a4;
    saved[4] = a5;
    saved[5] = a6;
    
    /* Make the call */
    func(a1, a2, a3, a4, a5, a6);
    
    /* Use saved values in way that conflicts with call-clobbered regs */
    int sum = saved[0] + saved[1] + saved[2] + saved[3] + saved[4] + saved[5];
    
    /* Nested calls to force save/restore insertion between calls */
    opaque_func1();
    asm volatile("" : : : "memory");
    opaque_func2(sum);
    asm volatile("" : : : "memory");
    opaque_func3(sum, sum * 2);
    
    global_array[global_counter++ % 256] = sum;
}

/* Helper with nested call to create outer/inner save scenarios */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x, int y) {
    volatile int saved = x + y;
    
    /* Inner call */
    int inner = opaque_func3(x, y);
    
    /* Use saved value after inner call */
    return saved * inner;
}

/* Function that creates many basic block splits */
__attribute__((noinline, noclone))
void test4(int iterations) {
    int i = 0;
    
    /* Unstructured control flow with labels */
    start_loop:
    if (i >= iterations) goto end;
    
    volatile int temp = i * 2;
    
    /* Conditional with call at boundary */
    if (i % 3 == 0) {
        opaque_func1();
        goto next_iter;
    } else if (i % 3 == 1) {
        opaque_func2(temp);
        /* Fall through */
    } else {
        /* Call at end of basic block */
        opaque_func3(temp, i);
        goto special_case;
    }
    
    /* Middle block */
    temp += helper_with_nested_call(temp, i);
    goto next_iter;
    
    special_case:
    /* Different path */
    temp = helper_with_nested_call(i, temp);
    /* No goto - falls through to next_iter */
    
    next_iter:
    global_array[i % 256] = temp;
    i++;
    
    /* Jump back to start - creates loop structure */
    goto start_loop;
    
    end:
    /* Basic block ending with potential insertion */
    return;
}

/* Main function that orchestrates all tests */
int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
        global_fp_array[i] = i * 1.5;
    }
    
    /* Use argv to add runtime variability */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize volatile function pointer */
    volatile_fp = opaque_func1;
    
    /* Execute all test functions in different orders based on mode */
    switch (test_mode) {
        case 0:
            test1(1, 2, 3, 4, 5, 6);
            test2(1.1, 2.2, 3.3, 4.4);
            test3();
            test4(10);
            break;
        case 1:
            test4(5);
            test3();
            test2(5.5, 6.6, 7.7, 8.8);
            test1(10, 20, 30, 40, 50, 60);
            break;
        case 2:
            for (int i = 0; i < 3; i++) {
                test1(i, i+1, i+2, i+3, i+4, i+5);
                test2(i*1.1, i*2.2, i*3.3, i*4.4);
            }
            test3();
            test4(8);
            break;
        case 3:
            test3();
            test3();  /* Call twice */
            test1(100, 200, 300, 400, 500, 600);
            test2(10.1, 20.2, 30.3, 40.4);
            test4(12);
            break;
        default:
            /* All tests in sequence with extra calls */
            test1(0, 1, 2, 3, 4, 5);
            asm volatile("" : : : "memory");
            test2(0.5, 1.5, 2.5, 3.5);
            asm volatile("" : : : "memory");
            test3();
            asm volatile("" : : : "memory");
            test4(15);
            break;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    double fp_checksum = 0.0;
    
    for (int i = 0; i < 256; i++) {
        checksum += global_array[i];
        fp_checksum += global_fp_array[i];
    }
    
    checksum += (int)fp_checksum;
    checksum += global_counter;
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional complex control flow in main */
    if (checksum > 0) {
        void (*funcs[])(void) = { (void(*)())test1, (void(*)())opaque_func1 };
        
        /* Indirect call through array */
        for (int i = 0; i < 2; i++) {
            if (i == 0) {
                /* Cast and call test1 with arguments */
                ((void(*)(int,int,int,int,int,int))funcs[i])(1,2,3,4,5,6);
            } else {
                funcs[i]();
            }
            
            /* Inline asm between calls */
            asm volatile (
                "mov %%rsp, %%rax\n\t"
                "add $16, %%rax\n\t"
                : : : "rax", "memory"
            );
        }
    }
    
    return checksum % 256;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_func1(void) {
    asm volatile ("" : : : "memory");
}

void opaque_func2(int x) {
    global_array[x % 256] += x;
    asm volatile ("" : : : "memory");
}

int opaque_func3(int x, int y) {
    asm volatile ("" : : : "memory");
    return x + y + global_counter;
}

double opaque_func4(double x) {
    asm volatile ("" : : : "memory");
    return x * 1.61803398875; /* golden ratio */
}
