/* caller-save-test.c - Test program to trigger uncovered lines in GCC's caller-save.cc */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force call instructions */
extern void external_func1(void);
extern int external_func2(int, int);
extern double external_func3(double, double);
extern void* external_func4(void*, void*);

/* Global volatile variables to prevent optimization */
volatile int g_volatile_int = 0;
volatile double g_volatile_double = 0.0;
volatile void* g_volatile_ptr = NULL;

/* Function pointer with volatile to prevent constant propagation */
typedef int (*func_ptr_t)(int, int);
volatile func_ptr_t volatile_func_ptr = NULL;

/* Complex structure to force register pressure */
struct ComplexRegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    void* p1, *p2;
};

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific register usage with asm clobbers */
#define CLOBBER_REGS() asm volatile("" : : : \
    "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", \
    "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", \
    "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15")

/* Test function 1: Many live variables across a call with register constraints */
__attribute__((noinline, noclone))
int test1_many_live_vars(int mode) {
    /* Force register allocation with explicit register variables */
    register int r1 asm("r10") = mode * 2;
    register int r2 asm("r11") = mode * 3;
    register int r3 asm("r12") = mode * 4;
    register int r4 asm("r13") = mode * 5;
    register int r5 asm("r14") = mode * 6;
    
    volatile int stack_save[10];
    
    /* Save all register values to stack before call */
    stack_save[0] = r1;
    stack_save[1] = r2;
    stack_save[2] = r3;
    stack_save[3] = r4;
    stack_save[4] = r5;
    
    /* Complex expression that uses all registers */
    int complex_expr = (r1 * r2) + (r3 - r4) * r5;
    
    /* Function call that clobbers registers */
    COMPILER_BARRIER();
    external_func1();
    CLOBBER_REGS();
    COMPILER_BARRIER();
    
    /* Use saved values after call - forces reload */
    int result = stack_save[0] + stack_save[1] + stack_save[2] + 
                 stack_save[3] + stack_save[4] + complex_expr;
    
    /* Create irreducible control flow with goto */
    if (result > 1000) {
        goto label1;
    } else if (result > 500) {
        goto label2;
    }
    
    /* Loop with break inside conditional containing call */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            external_func2(i, result);
            break;
        }
        result += i;
    }
    
label1:
    /* Another call with many arguments */
    result += external_func2(r1, r2);
    
label2:
    /* Force register pressure with inline asm */
    asm volatile (
        "mov %1, %%r10\n\t"
        "mov %2, %%r11\n\t"
        "add %%r10, %%r11\n\t"
        "mov %%r11, %0"
        : "=r" (result)
        : "r" (r3), "r" (r4)
        : "r10", "r11", "memory"
    );
    
    return result;
}

/* Test function 2: Floating point and mixed mode */
__attribute__((noinline, noclone))
double test2_fp_mixed(int a, double b, int c, double d) {
    volatile double fp_save[8];
    volatile int int_save[8];
    
    /* Save all values before call */
    fp_save[0] = b;
    fp_save[1] = d;
    fp_save[2] = b * d;
    int_save[0] = a;
    int_save[1] = c;
    int_save[2] = a * c;
    
    /* Switch statement with calls in different cases */
    int switch_val = a % 4;
    double result = 0.0;
    
    switch (switch_val) {
        case 0:
            result = external_func3(b, d);
            break;
        case 1:
            external_func1();
            result = b + d;
            break;
        case 2: {
            /* Nested scope with call */
            int temp = external_func2(a, c);
            result = b * temp;
            break;
        }
        default: {
            /* Complex default case with multiple calls */
            double temp1 = external_func3(b, 2.0);
            int temp2 = external_func2(c, a);
            result = temp1 * temp2;
            
            /* Loop with continue that contains call */
            for (int i = 0; i < 5; i++) {
                if (i == 2) {
                    external_func1();
                    continue;
                }
                result += i;
            }
            break;
        }
    }
    
    /* Use saved values after switch */
    result += fp_save[0] + fp_save[1] + int_save[0] + int_save[1];
    
    /* Force basic block boundary manipulation */
    if (result > 100.0) {
        goto fp_label;
    }
    
    /* Another call site */
    result = external_func3(result, 3.14159);
    
fp_label:
    /* Complex expression requiring temporary registers */
    result = (result * b) / (d + 1.0) + (a * c);
    
    return result;
}

/* Test function 3: Pointer manipulation and __builtin_apply */
__attribute__((noinline, noclone))
void* test3_pointer_ops(void* ptr1, void* ptr2, int size) {
    volatile void* ptr_save[4];
    volatile int int_save[4];
    
    /* Save pointers */
    ptr_save[0] = ptr1;
    ptr_save[1] = ptr2;
    
    /* Use __builtin_apply to create unusual call sequence */
    int (*func)(int, int) = (int (*)(int, int))external_func2;
    
    /* Create argument buffer for __builtin_apply */
    __builtin_apply_args();
    
    /* Complex control flow with nested calls */
    void* result = NULL;
    
    if (size > 0) {
        /* First call */
        int temp1 = external_func2(size, (int)(long)ptr1);
        
        /* Save intermediate result */
        int_save[0] = temp1;
        
        /* Second call with saved value */
        int temp2 = external_func2(temp1, (int)(long)ptr2);
        
        /* Use both results */
        result = (void*)(long)(temp1 + temp2);
        
        /* Loop with function call in condition */
        for (int i = 0; external_func2(i, size) > 0 && i < 10; i++) {
            if (i == 3) {
                external_func1();
                break;
            }
            result = (void*)((long)result + i);
        }
    } else {
        /* Alternative path with direct call */
        result = external_func4(ptr1, ptr2);
    }
    
    /* Use saved pointers */
    if (ptr_save[0] && ptr_save[1]) {
        result = (void*)((long)result + (long)ptr_save[0] + (long)ptr_save[1]);
    }
    
    /* Force register pressure with inline asm that clobbers many regs */
    asm volatile (
        "mov %1, %%rax\n\t"
        "mov %2, %%rbx\n\t"
        "add %%rax, %%rbx\n\t"
        "mov %%rbx, %0\n\t"
        : "=r" (result)
        : "r" (ptr1), "r" (ptr2)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory"
    );
    
    return result;
}

/* Test function 4: Vector-like operations and many arguments */
__attribute__((noinline, noclone))
struct ComplexRegPressure test4_many_args(
    int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8,
    double d1, double d2, double d3, double d4,
    void* p1, void* p2)
{
    struct ComplexRegPressure result = {0};
    volatile int int_stack[20];
    volatile double fp_stack[20];
    volatile void* ptr_stack[10];
    
    /* Save all arguments */
    int_stack[0] = a1; int_stack[1] = a2; int_stack[2] = a3; int_stack[3] = a4;
    int_stack[4] = a5; int_stack[5] = a6; int_stack[6] = a7; int_stack[7] = a8;
    fp_stack[0] = d1; fp_stack[1] = d2; fp_stack[2] = d3; fp_stack[3] = d4;
    ptr_stack[0] = p1; ptr_stack[1] = p2;
    
    /* Complex expression using all arguments before call */
    int int_sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
    double fp_sum = d1 + d2 + d3 + d4;
    
    /* Function call that will clobber registers */
    COMPILER_BARRIER();
    external_func1();
    CLOBBER_REGS();
    COMPILER_BARRIER();
    
    /* Use saved values - forces caller-save code */
    result.a = int_stack[0] + int_stack[1];
    result.b = int_stack[2] + int_stack[3];
    result.c = int_stack[4] + int_stack[5];
    result.d = int_stack[6] + int_stack[7];
    result.e = int_sum;
    
    result.x = fp_stack[0];
    result.y = fp_stack[1];
    result.z = fp_stack[2] + fp_stack[3];
    
    result.p1 = ptr_stack[0];
    result.p2 = ptr_stack[1];
    
    /* Nested loops with calls at boundaries */
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == j) {
                /* Call at loop boundary - may cause BB_END update */
                int temp = external_func2(i, j);
                result.a += temp;
                if (temp > 5) {
                    external_func1();
                    break;
                }
            }
            result.b += i * j;
        }
    }
    
    /* Another call sequence */
    result.c += external_func2(result.a, result.b);
    
    return result;
}

/* Helper function with nested call */
__attribute__((noinline, noclone))
int helper_nested_call(int x, int y) {
    volatile int save[4];
    save[0] = x;
    save[1] = y;
    
    /* Inner call */
    int inner = external_func2(x, y);
    
    /* Use saved values */
    int result = save[0] + save[1] + inner;
    
    /* Another call */
    result += external_func2(inner, result);
    
    return result;
}

/* Main test orchestrator */
int main(int argc, char** argv) {
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize volatile global */
    g_volatile_int = test_mode;
    g_volatile_double = test_mode * 1.5;
    g_volatile_ptr = &&main_label;
    
    /* Initialize function pointer */
    volatile_func_ptr = external_func2;
    
    int total_result = 0;
    double fp_result = 0.0;
    void* ptr_result = NULL;
    struct ComplexRegPressure struct_result;
    
    /* Execute all test functions in different orders based on mode */
    switch (test_mode) {
        case 0:
            total_result += test1_many_live_vars(10);
            fp_result += test2_fp_mixed(5, 3.14, 7, 2.71);
            ptr_result = test3_pointer_ops(&total_result, &fp_result, 20);
            struct_result = test4_many_args(1,2,3,4,5,6,7,8, 1.1,2.2,3.3,4.4, &total_result, &fp_result);
            break;
        case 1:
            fp_result += test2_fp_mixed(3, 1.618, 4, 2.718);
            total_result += test1_many_live_vars(7);
            struct_result = test4_many_args(9,8,7,6,5,4,3,2, 5.5,6.6,7.7,8.8, &total_result, ptr_result);
            ptr_result = test3_pointer_ops(&struct_result, &total_result, 15);
            break;
        case 2:
            ptr_result = test3_pointer_ops(&test_mode, &total_result, 30);
            struct_result = test4_many_args(10,20,30,40,50,60,70,80, 0.1,0.2,0.3,0.4, ptr_result, &fp_result);
            total_result += test1_many_live_vars(15);
            fp_result += test2_fp_mixed(20, 0.577, 30, 1.414);
            break;
        case 3:
            struct_result = test4_many_args(100,200,300,400,500,600,700,800, 10.1,20.2,30.3,40.4, &test_mode, &total_result);
            ptr_result = test3_pointer_ops(&struct_result, &fp_result, 25);
            fp_result += test2_fp_mixed(50, 0.693, 60, 1.732);
            total_result += test1_many_live_vars(20);
            break;
    }
    
    /* Call helper with nested calls */
    total_result += helper_nested_call(total_result, test_mode);
    
    /* Force another call through volatile function pointer */
    if (volatile_func_ptr) {
        total_result += volatile_func_ptr(total_result, test_mode);
    }
    
    /* Complex final computation using all results to prevent DCE */
    int final_result = total_result + (int)fp_result + (int)(long)ptr_result;
    final_result += struct_result.a + struct_result.b + struct_result.c + 
                    struct_result.d + struct_result.e + (int)struct_result.x;
    
    /* Update volatile globals */
    g_volatile_int = final_result;
    g_volatile_double = fp_result;
    g_volatile_ptr = ptr_result;
    
main_label:
    /* Print checksum to prevent optimization */
    printf("Result: %d (mode: %d)\n", final_result, test_mode);
    
    /* Use all volatile globals */
    printf("Global state: %d, %f, %p\n", 
           g_volatile_int, g_volatile_double, g_volatile_ptr);
    
    return final_result % 256;
}

/* Dummy implementations of external functions to satisfy linker */
void external_func1(void) {
    /* Empty but with memory clobber */
    asm volatile("" : : : "memory");
}

int external_func2(int a, int b) {
    return a + b + g_volatile_int;
}

double external_func3(double a, double b) {
    return a * b + g_volatile_double;
}

void* external_func4(void* a, void* b) {
    return (void*)((long)a + (long)b);
}
