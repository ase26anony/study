/* caller-save-test.c - Test program to trigger uncovered code in GCC's reload pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;
volatile void* global_pointer = NULL;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[4];

/* Complex structure to force register pressure */
struct RegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    void* p1, *p2;
};

/* Force register usage with explicit register variables */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_var3 asm ("xmm14");

/* Test function 1: Many integer arguments and live variables across call */
__attribute__((noinline, noclone))
void test1(int mode) {
    volatile int local_vars[16];
    struct RegPressure pressure;
    int i, j, k, l, m, n, o, p;
    
    /* Initialize many local variables */
    for (i = 0; i < 16; i++) {
        local_vars[i] = i + mode;
    }
    
    pressure.a = local_vars[0];
    pressure.b = local_vars[1];
    pressure.c = local_vars[2];
    pressure.d = local_vars[3];
    pressure.e = local_vars[4];
    pressure.f = local_vars[5];
    pressure.g = local_vars[6];
    pressure.h = local_vars[7];
    
    /* Use explicit register variables */
    reg_var1 = pressure.a + pressure.b;
    reg_var2 = pressure.c * pressure.d;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" : : : "memory");
    
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Complex control flow with goto to split basic blocks */
    if (reg_var1 > 100) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Use values that were live before the call */
    pressure.x = (double)pressure.e / (double)pressure.f;
    pressure.y = pressure.x * 2.0;
    /* Another call */
    opaque_func2(pressure.g);
    goto label3;
    
label2:
    pressure.z = (double)pressure.h * 3.14;
    /* Inline asm with clobbers */
    asm volatile("movl %0, %%eax\n\t"
                 "addl %1, %%eax\n\t"
                 : : "r"(pressure.a), "r"(pressure.b) : "eax", "memory");
    goto label3;
    
label3:
    /* Use all pressure fields to keep them live */
    int sum = pressure.a + pressure.b + pressure.c + pressure.d +
              pressure.e + pressure.f + pressure.g + pressure.h;
    global_counter += sum;
    
    /* Switch statement to create complex CFG */
    switch (mode % 4) {
        case 0:
            opaque_func1();
            break;
        case 1:
            opaque_func2(sum);
            break;
        case 2:
            /* Nested loop with break */
            for (j = 0; j < 10; j++) {
                if (j == sum % 5) {
                    opaque_func3((double)j);
                    break;
                }
            }
            break;
        default:
            /* This should trigger BB_END update */
            opaque_func4(&pressure);
            break;
    }
}

/* Test function 2: Floating point and mixed register pressure */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double fp_vars[8];
    volatile int int_vars[8];
    int i;
    
    /* Initialize with complex pattern */
    for (i = 0; i < 8; i++) {
        fp_vars[i] = base * i * 1.414;
        int_vars[i] = (int)(base * i) ^ 0x1234;
    }
    
    /* Use explicit floating point register */
    reg_var3 = fp_vars[0] + fp_vars[1];
    
    /* Inline asm that acts like a call */
    asm volatile("call *%0\n\t"
                 : : "r"(func_table[0]) : "rax", "rcx", "rdx", "rsi", "rdi",
                     "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                     "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10",
                     "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
    
    /* Use values that were live before the asm */
    double sum_fp = 0.0;
    int sum_int = 0;
    
    /* Loop with function call inside */
    for (i = 0; i < 8; i++) {
        if (i % 2 == 0) {
            sum_fp += fp_vars[i];
            opaque_func3(fp_vars[i]);
        } else {
            sum_int += int_vars[i];
            opaque_func2(int_vars[i]);
        }
        
        /* Early exit with goto */
        if (sum_fp > 1000.0 || sum_int > 10000) {
            goto early_exit;
        }
    }
    
    /* Normal exit path */
    global_accumulator += sum_fp;
    global_counter += sum_int;
    return;
    
early_exit:
    /* Different exit path */
    global_accumulator += sum_fp * 0.5;
    global_counter += sum_int / 2;
}

/* Test function 3: Vector-like operations and pointer chasing */
__attribute__((noinline, noclone))
void test3(void* ptr, int depth) {
    volatile int stack[32];
    volatile void* pointers[16];
    int i;
    
    if (depth <= 0) return;
    
    /* Setup local arrays */
    for (i = 0; i < 32; i++) {
        stack[i] = i * depth;
    }
    for (i = 0; i < 16; i++) {
        pointers[i] = (char*)ptr + i * 64;
    }
    
    /* Complex expression requiring many temporaries */
    int result = ((stack[0] * stack[1]) / (stack[2] + 1)) +
                 ((stack[3] << 2) | (stack[4] >> 3)) -
                 (stack[5] ^ stack[6] ^ stack[7]);
    
    /* Function call via pointer with many arguments */
    void* new_ptr = opaque_func4(ptr);
    
    /* Use __builtin_apply to force unusual register usage */
    if (depth > 1) {
        /* Create a complex calling sequence */
        for (i = 0; i < 4; i++) {
            if (stack[i] % 3 == 0) {
                /* Indirect call */
                func_table[i % 4]();
            } else if (stack[i] % 3 == 1) {
                /* Direct call */
                opaque_func2(stack[i]);
            } else {
                /* Inline asm with side effects */
                asm volatile("movq %1, %%rax\n\t"
                             "addq $1, %%rax\n\t"
                             "movq %%rax, %0\n\t"
                             : "=m"(stack[i+8])
                             : "r"(stack[i])
                             : "rax", "memory");
            }
        }
    }
    
    /* Recursive call to create nested call context */
    test3(new_ptr, depth - 1);
    
    /* Use all local arrays after calls */
    int checksum = 0;
    for (i = 0; i < 32; i++) {
        checksum ^= stack[i];
    }
    global_counter ^= checksum;
}

/* Test function 4: Mixed types and __builtin_va_arg simulation */
__attribute__((noinline, noclone))
void test4(int count, ...) {
    volatile int int_args[12];
    volatile double fp_args[12];
    void* ptr_args[12];
    
    /* Simulate va_arg usage */
    int i;
    for (i = 0; i < count && i < 12; i++) {
        /* These would normally be va_arg macros */
        int_args[i] = i * 2;
        fp_args[i] = i * 3.14;
        ptr_args[i] = (void*)(long)(i * 100);
    }
    
    /* Register pressure with mixed types */
    double fp_sum = 0.0;
    int int_sum = 0;
    
    /* Loop with multiple call sites */
    for (i = 0; i < count && i < 12; i++) {
        /* Alternate between function types */
        if (i % 3 == 0) {
            opaque_func2(int_args[i]);
            int_sum += int_args[i];
        } else if (i % 3 == 1) {
            opaque_func3(fp_args[i]);
            fp_sum += fp_args[i];
        } else {
            opaque_func4(ptr_args[i]);
            /* Use inline asm with specific clobber */
            asm volatile("" : "+r"(int_args[i]) : : "r10", "r11", "memory");
        }
        
        /* Conditional goto to force block splitting */
        if (int_sum > 100) {
            goto overflow;
        }
    }
    
    /* Normal path */
    global_accumulator += fp_sum;
    global_counter += int_sum;
    return;
    
overflow:
    /* Alternate path */
    global_accumulator += fp_sum / 2.0;
    global_counter += int_sum / 2;
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
void nested_helper(int level) {
    volatile int locals[8];
    int i;
    
    for (i = 0; i < 8; i++) {
        locals[i] = i + level;
    }
    
    if (level > 0) {
        /* Outer call */
        opaque_func2(locals[0]);
        
        /* Inner call with different convention */
        double temp = (double)locals[1];
        opaque_func3(temp);
        
        /* Use values across calls */
        int sum = locals[0] + locals[1] + locals[2];
        global_counter += sum;
        
        /* Recursive call */
        nested_helper(level - 1);
    }
}

/* Main function with mode selection */
int main(int argc, char** argv) {
    int mode = 0;
    
    /* Use argv to determine mode, preventing constant propagation */
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize function table with opaque functions */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Initialize explicit register variables */
    reg_var1 = 42;
    reg_var2 = 137;
    reg_var3 = 3.14159;
    
    /* Run all test functions in sequence */
    test1(mode);
    test2((double)mode * 1.234);
    test3(&global_counter, 3);
    
    /* Simulate variable arguments */
    test4(8, 1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Nested call scenario */
    nested_helper(2);
    
    /* Force use of all global volatiles to prevent DCE */
    int final_result = global_counter + (int)global_accumulator;
    if (global_pointer) {
        final_result ^= 0xABCD;
    }
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}

/* Dummy definitions to satisfy linker (normally these would be in a library) */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2;
}

double opaque_func3(double x) {
    return x * 1.5;
}

void* opaque_func4(void* x) {
    return (char*)x + 1;
}
