/* test-caller-save.c
 * 
 * This program is designed to trigger the uncovered instruction chain
 * manipulation code in GCC's caller-save.cc (lines 905-913).
 * It creates complex register pressure scenarios, forces basic block
 * splitting/merging, and requires non-trivial prologue/epilogue sequences.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Global volatile state to prevent dead code elimination */
volatile int global_counter = 0;
volatile long global_checksum = 0;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t volatile func_table[4];

/* Initialize function pointers to create opaque call sites */
__attribute__((constructor)) 
static void init_func_table(void) {
    /* These will be linked to actual functions, creating
     * indirect calls that confuse call analysis */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
}

/* Test function 1: Many integer live variables across call */
__attribute__((noinline, noclone))
static void test1_many_int_vars(void) {
    /* Use explicit register variables to create conflicts */
    register int r10_val asm("r10") = global_counter + 1;
    register int r11_val asm("r11") = global_counter + 2;
    register int r12_val asm("r12") = global_counter + 3;
    register int r13_val asm("r13") = global_counter + 4;
    register int r14_val asm("r14") = global_counter + 5;
    
    /* Volatile array to force spills */
    volatile int save_area[10];
    
    /* Save register values before call */
    save_area[0] = r10_val;
    save_area[1] = r11_val;
    save_area[2] = r12_val;
    save_area[3] = r13_val;
    save_area[4] = r14_val;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ecx\n\t"
        "movl $0x11111111, %%edx\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ecx", "edx", "memory"
    );
    
    /* Make a function call - forces caller-save */
    int result = opaque_func2(global_counter);
    
    /* Complex control flow to force basic block splitting */
    if (result > 0) {
        /* Use saved values in computation */
        int sum = save_area[0] + save_area[1] + save_area[2];
        
        /* Another asm barrier */
        asm volatile ("" : : : "memory");
        
        /* Nested loop with break to create block boundaries */
        for (int i = 0; i < 10; i++) {
            if (i == result) {
                /* Function call at block boundary */
                opaque_func1();
                break;
            }
            sum += i;
        }
        
        global_checksum += sum;
    } else {
        /* Different path with goto for irreducible flow */
        int prod = save_area[3] * save_area[4];
    compute:
        prod *= 2;
        if (prod < 1000) {
            goto compute;
        }
        global_checksum += prod;
    }
    
    /* Force use of all register variables after call */
    asm volatile (
        "addl %%r10d, %0\n\t"
        "addl %%r11d, %0\n\t"
        "addl %%r12d, %0\n\t"
        "addl %%r13d, %0\n\t"
        "addl %%r14d, %0\n\t"
        : "+r" (global_counter)
        : /* no inputs */
        : "r10", "r11", "r12", "r13", "r14", "cc"
    );
}

/* Test function 2: Mixed float and integer, complex expression after call */
__attribute__((noinline, noclone))
static double test2_mixed_types(int mode) {
    volatile double f1 = 1.2345;
    volatile double f2 = 6.7890;
    volatile int i1 = 42;
    volatile int i2 = 73;
    
    /* Register variables with explicit constraints */
    register double fr1 asm("xmm0") = f1;
    register double fr2 asm("xmm1") = f2;
    register int ir1 asm("ebx") = i1;
    register int ir2 asm("r15") = i2;
    
    /* Save to volatile memory */
    volatile double f_save[4];
    volatile int i_save[4];
    f_save[0] = fr1;
    f_save[1] = fr2;
    i_save[0] = ir1;
    i_save[1] = ir2;
    
    /* Switch statement to create complex CFG */
    double result = 0.0;
    switch (mode % 4) {
        case 0:
            /* Direct call */
            result = opaque_func3(fr1);
            /* Use saved values immediately after call */
            result += f_save[0] * i_save[0];
            break;
        case 1:
            /* Indirect call via function pointer */
            func_table[mode % 4]();
            result = fr1 + fr2;
            /* Complex expression requiring temporary */
            result = (result * i_save[1]) / (i_save[0] + 1);
            break;
        case 2:
            /* Nested calls */
            result = opaque_func3(opaque_func3(fr2));
            /* Use both float and int saved values */
            result += (f_save[0] - f_save[1]) * i_save[0];
            break;
        default:
            /* Loop with call inside */
            for (int i = 0; i < 3; i++) {
                if (i == 1) {
                    opaque_func1();
                    continue;
                }
                result += fr1 * i;
            }
            result += i_save[1];
            break;
    }
    
    /* Force register usage across basic block boundaries */
    asm volatile (
        "movq %1, %%xmm0\n\t"
        "movq %2, %%xmm1\n\t"
        : "=x" (result)
        : "x" (fr1), "x" (fr2), "0" (result)
        : "xmm0", "xmm1"
    );
    
    return result;
}

/* Test function 3: Vector-like operations and pointer manipulation */
__attribute__((noinline, noclone))
static void* test3_pointer_ops(void* ptr) {
    /* Use many pointer-sized values */
    register void* rcx_ptr asm("rcx") = ptr;
    register void* rdx_ptr asm("rdx") = (char*)ptr + 16;
    register void* rsi_ptr asm("rsi") = (char*)ptr + 32;
    register void* rdi_ptr asm("rdi") = (char*)ptr + 48;
    
    /* Save pointers */
    volatile void* ptr_save[8];
    ptr_save[0] = rcx_ptr;
    ptr_save[1] = rdx_ptr;
    ptr_save[2] = rsi_ptr;
    ptr_save[3] = rdi_ptr;
    
    /* Inline asm simulating a call-like operation */
    asm volatile (
        "pushq %%rbp\n\t"
        "movq %%rsp, %%rbp\n\t"
        "subq $32, %%rsp\n\t"
        "call *%0\n\t"
        "addq $32, %%rsp\n\t"
        "popq %%rbp\n\t"
        : /* no outputs */
        : "r" (func_table[global_counter % 4])
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", 
          "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "memory", "cc"
    );
    
    /* Use __builtin_apply to create unusual register pressure */
    void* args[3] = {ptr_save[0], ptr_save[1], ptr_save[2]};
    void* result = __builtin_apply((void (*)())opaque_func4, args, 24);
    
    /* Complex control flow with labels */
    int choice = global_counter++ % 3;
    
    if (choice == 0) {
        goto label_a;
    } else if (choice == 1) {
        goto label_b;
    } else {
        goto label_c;
    }
    
label_a:
    /* Use saved pointer values */
    result = (char*)result + (long)ptr_save[3];
    goto end;
    
label_b:
    /* Another call */
    opaque_func1();
    result = ptr_save[0];
    goto end;
    
label_c:
    /* Loop with break at boundary */
    for (int i = 0; i < 5; i++) {
        if (i == 2) {
            opaque_func2(i);
            break;
        }
        result = (char*)result + i;
    }
    
end:
    /* Force all pointer registers to be live */
    asm volatile (
        "addq %%rcx, %0\n\t"
        "addq %%rdx, %0\n\t"
        "addq %%rsi, %0\n\t"
        "addq %%rdi, %0\n\t"
        : "+r" (result)
        : /* no inputs */
        : "rcx", "rdx", "rsi", "rdi", "cc"
    );
    
    return result;
}

/* Test function 4: __builtin_va_arg simulation and many args */
__attribute__((noinline, noclone))
static long test4_varargs_simulation(int a, double b, void* c, int d, double e) {
    /* Many arguments force register/stack pressure */
    register int rax_val asm("rax") = a;
    register double xmm0_val asm("xmm0") = b;
    register void* rdi_val asm("rdi") = c;
    register int rsi_val asm("rsi") = d;
    register double xmm1_val asm("xmm1") = e;
    
    /* Save everything */
    volatile int i_save[10];
    volatile double f_save[10];
    volatile void* p_save[10];
    
    i_save[0] = rax_val;
    i_save[1] = rsi_val;
    f_save[0] = xmm0_val;
    f_save[1] = xmm1_val;
    p_save[0] = rdi_val;
    
    /* Simulate va_arg usage pattern */
    long sum = 0;
    for (int i = 0; i < 5; i++) {
        /* Call that might be interpreted as varargs */
        int tmp = opaque_func2(i);
        
        /* Use saved values in complex expression */
        sum += i_save[i % 2] * tmp;
        sum += (long)(f_save[i % 2] * 100);
        
        /* Conditional with call at boundary */
        if (tmp % 3 == 0) {
            /* This creates block boundary insertion point */
            double r = opaque_func3(f_save[0]);
            sum += (long)r;
            continue;
        } else if (tmp % 3 == 1) {
            goto compute_more;
        }
        
        sum += i;
    }
    
    goto finish;
    
compute_more:
    /* Additional basic block */
    sum *= 2;
    opaque_func1();
    
finish:
    /* Use all saved values one more time */
    asm volatile (
        "imulq %1, %0\n\t"
        "addq %2, %0\n\t"
        : "+r" (sum)
        : "r" ((long)i_save[0]), "r" ((long)p_save[0])
        : "cc"
    );
    
    return sum;
}

/* Helper with nested calls to create outer/inner save scenarios */
__attribute__((noinline, noclone))
static int helper_nested_calls(int depth) {
    if (depth <= 0) {
        return global_counter;
    }
    
    /* Many live variables */
    int a = depth * 2;
    int b = depth * 3;
    int c = depth * 5;
    int d = depth * 7;
    
    volatile int save[4];
    save[0] = a;
    save[1] = b;
    save[2] = c;
    save[3] = d;
    
    /* Outer call */
    int result1 = opaque_func2(a);
    
    /* Inner call with different arguments */
    int result2 = helper_nested_calls(depth - 1);
    
    /* Use saved values after both calls */
    int final = save[0] + save[1] + save[2] + save[3];
    final += result1 * result2;
    
    /* Conditional that might split basic block after call */
    if (final % 2 == 0) {
        /* Call at block end boundary */
        opaque_func1();
        return final;
    } else {
        /* Different path */
        return final / 2;
    }
}

/* Main test driver */
int main(int argc, char** argv) {
    int test_mode = 0;
    
    /* Use argv to select mode but ensure all code runs */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize some data for pointer tests */
    char data_buffer[256];
    for (int i = 0; i < 256; i++) {
        data_buffer[i] = i;
    }
    
    /* Run all test functions in sequence, but with mode-dependent ordering */
    switch (test_mode) {
        case 0:
            test1_many_int_vars();
            test2_mixed_types(global_counter);
            test3_pointer_ops(data_buffer);
            test4_varargs_simulation(1, 2.0, data_buffer, 3, 4.0);
            break;
        case 1:
            test4_varargs_simulation(5, 6.0, data_buffer + 64, 7, 8.0);
            test1_many_int_vars();
            test3_pointer_ops(data_buffer + 128);
            test2_mixed_types(global_counter + 1);
            break;
        case 2:
            test2_mixed_types(global_counter + 2);
            test3_pointer_ops(data_buffer + 192);
            test1_many_int_vars();
            test4_varargs_simulation(9, 10.0, data_buffer, 11, 12.0);
            break;
        case 3:
            test3_pointer_ops(data_buffer);
            test4_varargs_simulation(13, 14.0, data_buffer + 64, 15, 16.0);
            test2_mixed_types(global_counter + 3);
            test1_many_int_vars();
            break;
        default:
            /* Run all in different order with nested calls */
            test1_many_int_vars();
            helper_nested_calls(3);
            test2_mixed_types(global_counter);
            test3_pointer_ops(data_buffer);
            test4_varargs_simulation(17, 18.0, data_buffer + 128, 19, 20.0);
            helper_nested_calls(2);
            break;
    }
    
    /* Compute final checksum to prevent optimization */
    long final_checksum = global_checksum + global_counter;
    
    /* Use all kinds of operations in final output */
    printf("Result: %ld\n", final_checksum);
    
    /* Force one more complex call pattern before exit */
    volatile int exit_code = final_checksum > 1000 ? 0 : 1;
    
    return exit_code;
}

/* Dummy implementations of opaque functions to satisfy linker */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2 + global_counter;
}

double opaque_func3(double x) {
    return x * 1.5 + global_counter;
}

void* opaque_func4(void* x) {
    return (char*)x + global_counter++;
}
