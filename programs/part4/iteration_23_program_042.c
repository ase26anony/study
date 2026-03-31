/* test-caller-save.c - Complex program to trigger uncovered code in caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void* opaque_func4(void*);

/* Volatile globals to maintain live ranges across calls */
volatile int global_counter = 0;
volatile long global_data[32];
volatile double global_fp[16];

/* Function pointer with volatile to prevent optimization */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr = NULL;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) \
    register long var asm(reg)

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test_function1(int mode) {
    /* Force many live variables in registers */
    FORCE_REGISTER(r10_var, "r10") = global_counter + 1;
    FORCE_REGISTER(r11_var, "r11") = global_counter + 2;
    FORCE_REGISTER(r12_var, "r12") = global_counter + 3;
    register int r13_var asm("r13") = global_counter + 4;
    register int r14_var asm("r14") = global_counter + 5;
    
    /* Volatile locals to force spills */
    volatile int vol1 = r10_var;
    volatile int vol2 = r11_var;
    volatile int vol3 = r12_var;
    
    /* Array to force stack usage */
    int stack_array[16];
    for (int i = 0; i < 16; i++) {
        stack_array[i] = r10_var + i;
    }
    
    /* Complex control flow with calls at boundaries */
    if (mode & 1) {
        /* Call with many clobbered registers */
        asm volatile(
            "mov %0, %%r10\n\t"
            "mov %1, %%r11\n\t"
            "call *%2\n\t"
            "mov %%r10, %0\n\t"
            "mov %%r11, %1\n\t"
            : "+r" (r10_var), "+r" (r11_var)
            : "r" (volatile_fptr)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
        
        /* Use the saved values immediately after call */
        vol1 = r10_var + r11_var;
        COMPILER_BARRIER();
    }
    
    /* Switch with calls in different cases */
    switch (mode & 3) {
        case 0:
            opaque_func1();
            /* Force use of saved registers */
            r12_var = vol1 + vol2;
            break;
        case 1:
            r13_var = opaque_func2(r13_var);
            /* Complex expression requiring temporary */
            r14_var = (r13_var * r12_var) / (r10_var + 1);
            break;
        case 2:
            /* Nested call scenario */
            {
                double result = opaque_func3(r10_var);
                global_fp[0] = result + r11_var;
            }
            break;
        default:
            /* Irreducible control flow with goto */
            if (r10_var > 100) {
                goto special_label;
            }
            opaque_func4((void*)&global_data);
            break;
    }
    
    /* Label for goto to create complex CFG */
special_label:
    /* Use all register variables in complex computation */
    int sum = r10_var + r11_var + r12_var + r13_var + r14_var;
    for (int i = 0; i < 16; i++) {
        sum += stack_array[i];
    }
    
    global_counter += sum;
    COMPILER_BARRIER();
}

/* Function with floating point and vector-like operations */
__attribute__((noinline, noclone))
void test_function2(double base) {
    /* Mix of FP and integer registers */
    volatile double v1 = base;
    volatile double v2 = base * 2.0;
    volatile double v3 = base * 3.0;
    
    register double fp1 asm("xmm8") = v1;
    register double fp2 asm("xmm9") = v2;
    register double fp3 asm("xmm10") = v3;
    
    /* Integer registers that need to survive calls */
    FORCE_REGISTER(ri1, "r15") = (long)(base * 100.0);
    FORCE_REGISTER(ri2, "r14") = ri1 + 1;
    
    /* Loop with call inside */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* Call that clobbers FP registers */
            asm volatile(
                "call opaque_func1"
                : 
                : 
                : "rax", "rcx", "rdx", "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7", "memory"
            );
            
            /* Immediately use FP registers after call */
            fp1 = fp2 + fp3;
            COMPILER_BARRIER();
        }
        
        /* Complex FP expression */
        double temp = fp1 * fp2 - fp3;
        global_fp[i] = temp + ri1;
        
        /* Conditional goto to create block boundaries */
        if (temp > 1000.0) {
            goto fp_exit;
        }
    }
    
    /* Another call site */
    ri2 = opaque_func2(ri2);
    
fp_exit:
    /* Use all values before return */
    global_fp[15] = fp1 + fp2 + fp3 + ri1 + ri2;
    COMPILER_BARRIER();
}

/* Function with variable arguments to stress register allocation */
__attribute__((noinline, noclone))
void test_function3(int a, int b, int c, int d, int e, int f, 
                    double g, double h, double i, double j) {
    /* Many parameters = many registers in use */
    volatile int save_a = a;
    volatile int save_b = b;
    volatile int save_c = c;
    volatile int save_d = d;
    volatile int save_e = e;
    volatile int save_f = f;
    
    /* Call that will need to save caller-saved registers */
    int result1 = opaque_func2(a + b);
    
    /* Use saved values */
    int sum = save_a + save_b + save_c + save_d + save_e + save_f;
    
    /* Another call with different arguments */
    double fp_sum = g + h + i + j;
    double result2 = opaque_func3(fp_sum);
    
    /* Complex control flow with switch */
    switch (sum % 4) {
        case 0:
            opaque_func1();
            break;
        case 1:
            /* Inline asm that looks like a call */
            asm volatile(
                "mov %0, %%rdi\n\t"
                "mov %1, %%rsi\n\t"
                "mov %2, %%rdx\n\t"
                "mov %3, %%rcx\n\t"
                "push %%rbp\n\t"
                "call *%4\n\t"
                "pop %%rbp\n\t"
                : 
                : "r" (result1), "r" (result2), "r" (sum), 
                  "r" (global_counter), "r" (volatile_fptr)
                : "rax", "rdi", "rsi", "rdx", "rcx", "r8", "r9", "r10", "r11",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "memory"
            );
            break;
        case 2:
            /* Nested loops with calls */
            for (int x = 0; x < 2; x++) {
                for (int y = 0; y < 2; y++) {
                    if (x == y) {
                        opaque_func4(&global_data[x + y]);
                    } else {
                        global_counter += opaque_func2(x + y);
                    }
                }
            }
            break;
        default:
            /* Unreachable via normal path but compiler doesn't know */
            if (global_counter < 0) {
                goto unreachable_label;
            }
            break;
    }
    
    /* Force basic block boundary */
    if (result1 > 0) {
        global_data[0] = result1 + result2;
    }
    
unreachable_label:
    COMPILER_BARRIER();
}

/* Function with computed goto for irreducible control flow */
__attribute__((noinline, noclone))
void test_function4(int selector) {
    static void* jump_table[] = {
        &&label0, &&label1, &&label2, &&label3,
        &&label4, &&label5
    };
    
    /* Force register usage before goto */
    FORCE_REGISTER(rbx_var, "rbx") = selector * 2;
    FORCE_REGISTER(rbp_var, "rbp") = selector * 3;
    
    volatile int vol = selector;
    
    if (selector < 0 || selector > 5) {
        goto default_label;
    }
    
    /* Computed goto - creates complex CFG */
    goto *jump_table[selector];
    
label0:
    opaque_func1();
    /* Fall through */
label1:
    rbx_var += opaque_func2(rbx_var);
    goto merge_point;
    
label2:
    /* Call with many clobbers */
    asm volatile(
        "call opaque_func1"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "memory"
    );
    rbp_var = rbx_var * 2;
    goto merge_point;
    
label3:
    {
        double res = opaque_func3(rbp_var);
        global_fp[1] = res;
    }
    goto merge_point;
    
label4:
    opaque_func4(&global_counter);
    goto merge_point;
    
label5:
    /* Empty case - falls through to default */
    
default_label:
    /* Function call at block end */
    global_counter += opaque_func2(selector);
    
merge_point:
    /* Use all register variables */
    global_data[selector % 32] = rbx_var + rbp_var + vol;
    COMPILER_BARRIER();
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
int nested_call_helper(int depth, int value) {
    if (depth <= 0) {
        return opaque_func2(value);
    }
    
    /* Recursive call (tail recursion prevented by extra computation) */
    int temp = nested_call_helper(depth - 1, value + 1);
    
    /* Use value after call */
    volatile int saved = value;
    
    /* Another call */
    opaque_func1();
    
    return temp + saved + global_counter;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize volatile function pointer */
    volatile_fptr = (func_ptr_t)opaque_func1;
    
    /* Initialize global data */
    for (int i = 0; i < 32; i++) {
        global_data[i] = i * 3;
    }
    for (int i = 0; i < 16; i++) {
        global_fp[i] = i * 1.5;
    }
    
    /* Parse test mode from argv if provided */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 16;
    } else {
        /* Use some entropy from stack address */
        test_mode = ((long)&test_mode) % 16;
    }
    
    COMPILER_BARRIER();
    
    /* Run all test functions in sequence */
    test_function1(test_mode);
    test_function2(test_mode * 1.25);
    test_function3(test_mode, test_mode + 1, test_mode + 2,
                   test_mode + 3, test_mode + 4, test_mode + 5,
                   test_mode * 1.1, test_mode * 1.2,
                   test_mode * 1.3, test_mode * 1.4);
    test_function4(test_mode % 6);
    
    /* Test nested call scenario */
    int nested_result = 0;
    for (int i = 0; i < 3; i++) {
        nested_result += nested_call_helper(i, test_mode + i);
        COMPILER_BARRIER();
    }
    
    /* Final computation using all global state to prevent DCE */
    long final_checksum = global_counter + nested_result;
    for (int i = 0; i < 32; i++) {
        final_checksum += global_data[i];
    }
    for (int i = 0; i < 16; i++) {
        final_checksum += (long)global_fp[i];
    }
    
    /* Use the result */
    printf("Checksum: %ld\n", final_checksum);
    
    /* Force exit with result to prevent tail call optimization */
    asm volatile(
        "mov %0, %%rdi\n\t"
        "mov $60, %%rax\n\t"
        "syscall"
        : 
        : "r" ((long)(final_checksum & 255))
        : "rdi", "rax", "memory"
    );
    
    return (int)final_checksum;
}

/* Dummy definitions to satisfy linker (normally would be in separate library) */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2 + global_counter;
}

double opaque_func3(double x) {
    return x * 1.5 + global_counter;
}

void* opaque_func4(void* p) {
    return (void*)((long)p + global_counter);
}
