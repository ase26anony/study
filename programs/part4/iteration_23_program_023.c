/* caller-save-test.c
 * A comprehensive test to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets the instruction chain manipulation code at lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External opaque functions to prevent inlining and optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;
volatile double global_fp_accum = 0.0;
volatile void* global_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Complex structure to force register pressure */
struct RegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    void* p1, *p2;
};

/* Force many live variables across calls with register constraints */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Use explicit register variables to create conflicts */
    register int r10_val asm("r10") = mode * 2;
    register int r11_val asm("r11") = mode * 3;
    register int r12_val asm("r12") = mode * 4;
    register int r13_val asm("r13") = mode * 5;
    
    volatile int stack_save[8];
    
    /* Save register values to stack */
    asm volatile("" : "=r"(r10_val), "=r"(r11_val), "=r"(r12_val), "=r"(r13_val) : "0"(r10_val), "1"(r11_val), "2"(r12_val), "3"(r13_val));
    stack_save[0] = r10_val;
    stack_save[1] = r11_val;
    stack_save[2] = r12_val;
    stack_save[3] = r13_val;
    
    /* Create complex control flow with goto to split basic blocks */
    if (mode & 1) {
        /* First call site with many live registers */
        opaque_func1();
        
        /* Use saved values immediately after call */
        r10_val = stack_save[0] + 1;
        r11_val = stack_save[1] + r10_val;
        
        /* Inline asm that clobbers call-clobbered registers */
        asm volatile(
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            : 
            : "r"(r10_val), "r"(r11_val)
            : "eax", "memory"
        );
        
        goto merge_point;
    } else {
        /* Alternative path with different register usage */
        register int r14_val asm("r14") = mode * 6;
        register int r15_val asm("r15") = mode * 7;
        
        stack_save[4] = r14_val;
        stack_save[5] = r15_val;
        
        /* Function call via pointer to create indirect jump */
        if (func_table[0]) func_table[0]();
        
        /* Complex expression requiring temporary registers */
        r14_val = (stack_save[4] * stack_save[5]) / (mode + 1);
        r15_val = r14_val ^ stack_save[4];
        
        /* Force register spill by using all available registers */
        asm volatile(
            "movl %0, %%r10d\n\t"
            "movl %1, %%r11d\n\t"
            "addl %%r10d, %%r11d\n\t"
            : 
            : "r"(r14_val), "r"(r15_val)
            : "r10", "r11", "memory"
        );
    }
    
merge_point:
    /* Merge point of basic blocks - may trigger BB_END update */
    int result = opaque_func2(mode);
    
    /* Use result in way that requires register allocation around BB boundary */
    global_counter += result + r10_val + r11_val;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Test with floating point and mixed registers */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double fp_save[4];
    volatile int int_save[8];
    
    /* Create many live floating point values */
    double f1 = base * 1.1;
    double f2 = base * 2.2;
    double f3 = base * 3.3;
    double f4 = base * 4.4;
    
    fp_save[0] = f1;
    fp_save[1] = f2;
    fp_save[2] = f3;
    fp_save[3] = f4;
    
    /* Integer values that span calls */
    register int i1 asm("r10") = (int)base;
    register int i2 asm("r11") = (int)(base * 10);
    register int i3 asm("r12") = (int)(base * 100);
    
    int_save[0] = i1;
    int_save[1] = i2;
    int_save[2] = i3;
    
    /* Switch statement to create complex CFG with calls at case boundaries */
    int choice = ((int)base) % 4;
    
    switch (choice) {
        case 0:
            opaque_func1();
            f1 = fp_save[0] + 1.0;
            break;
        case 1:
            f2 = opaque_func3(fp_save[1]);
            /* Fall through to create merge scenario */
        case 2:
            /* Nested call within switch case */
            {
                int temp = opaque_func2((int)f2);
                i1 = int_save[0] + temp;
            }
            break;
        default:
            /* Default case with function call at end of basic block */
            opaque_func4(&global_ptr);
            /* This should be at BB_END before potential insertion */
            f3 = fp_save[2] * 2.0;
            break;
    }
    
    /* Use all saved values in complex expression */
    global_fp_accum += f1 + f2 + f3 + f4;
    global_accumulator += i1 + i2 + i3;
    
    /* Loop with break that creates block splitting opportunities */
    for (int i = 0; i < 3; i++) {
        if (i == 1) {
            /* Call in middle of loop with live variables */
            int result = opaque_func2(i);
            i1 += result;
            /* Break after call creates block boundary */
            if (result > 100) break;
        }
        i2 += i;
    }
}

/* Test with vector-like operations and __builtin_apply */
__attribute__((noinline, noclone))
void test3(void* arg) {
    struct RegPressure pressure;
    
    /* Initialize all fields to create register pressure */
    pressure.a = 1; pressure.b = 2; pressure.c = 3; pressure.d = 4;
    pressure.e = 5; pressure.f = 6; pressure.g = 7; pressure.h = 8;
    pressure.x = 1.0; pressure.y = 2.0; pressure.z = 3.0;
    pressure.p1 = arg; pressure.p2 = &global_counter;
    
    /* Save to volatile array */
    volatile int int_backup[8];
    int_backup[0] = pressure.a; int_backup[1] = pressure.b;
    int_backup[2] = pressure.c; int_backup[3] = pressure.d;
    int_backup[4] = pressure.e; int_backup[5] = pressure.f;
    int_backup[6] = pressure.g; int_backup[7] = pressure.h;
    
    /* Use __builtin_apply to create unusual call sequence */
    void* args[3];
    args[0] = &pressure.a;
    args[1] = &pressure.b;
    args[2] = &pressure.c;
    
    /* Simulate va_arg-like usage */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        /* Function call inside loop with many live values */
        if (i & 1) {
            sum += opaque_func2(int_backup[i]);
        } else {
            /* Inline asm that looks like a call */
            asm volatile(
                "call *%0\n\t"
                : 
                : "r"(func_table[i % 3])
                : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
            );
        }
        
        /* Use saved values after each potential call */
        pressure.a = int_backup[0] + sum;
        pressure.b = int_backup[1] + pressure.a;
    }
    
    /* Irreducible control flow with labels and goto */
    if (sum > 1000) goto label_a;
    
    pressure.c = opaque_func2(pressure.a + pressure.b);
    
    if (pressure.c < 0) goto label_b;
    
label_a:
    pressure.d = opaque_func2(pressure.c);
    goto label_c;
    
label_b:
    pressure.e = opaque_func2(pressure.d);
    
label_c:
    /* All paths merge here - potential BB_END update location */
    global_counter += pressure.a + pressure.b + pressure.c + pressure.d + pressure.e;
}

/* Function with many arguments to force register/stack passing */
__attribute__((noinline, noclone))
long test4(int a1, int a2, int a3, int a4, int a5, 
           int a6, int a7, int a8, double f1, double f2) {
    
    /* All arguments are live across the call */
    volatile int arg_save[8];
    arg_save[0] = a1; arg_save[1] = a2; arg_save[2] = a3; arg_save[3] = a4;
    arg_save[4] = a5; arg_save[5] = a6; arg_save[6] = a7; arg_save[7] = a8;
    
    volatile double fp_save[2];
    fp_save[0] = f1; fp_save[1] = f2;
    
    /* Create a situation where BB_END might need updating */
    long result = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Nested conditional with call at end */
        if (i == 0) {
            result += opaque_func2(a1 + a2);
            /* This could be BB_END before insertion */
        } else if (i == 1) {
            result += opaque_func2(a3 + a4);
            /* Force block split */
            continue;
        } else {
            /* Call with result used immediately in complex expr */
            int temp = opaque_func2(a5 + a6);
            result += temp * 2;
            
            /* Inline asm that uses specific registers */
            asm volatile(
                "movq %1, %%rax\n\t"
                "addq %2, %%rax\n\t"
                "movq %%rax, %0\n\t"
                : "=r"(result)
                : "r"(result), "r"(temp)
                : "rax", "memory"
            );
        }
        
        /* Use all saved arguments */
        a1 = arg_save[0] + 1;
        a2 = arg_save[1] + a1;
        f1 = fp_save[0] * 2.0;
    }
    
    /* Final computation using all live values */
    return result + a1 + a2 + a3 + a4 + (long)(f1 + f2);
}

/* Helper with nested calls to create save/restore chains */
__attribute__((noinline, noclone))
void nested_call_helper(int depth, int* result) {
    volatile int save = *result;
    
    if (depth > 0) {
        /* Recursive-like nested call */
        opaque_func1();
        *result += save * depth;
        
        /* Inner call within outer call's live range */
        int temp = opaque_func2(*result);
        
        /* Use result immediately requiring register */
        *result = temp + save;
        
        /* Another call */
        opaque_func3((double)*result);
    }
    
    /* Memory barrier between calls */
    asm volatile("" ::: "memory");
}

int main(int argc, char** argv) {
    /* Initialize function table with opaque functions */
    func_table[0] = (func_ptr_t)opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Run all tests in sequence with mode-dependent variations */
    test1(mode);
    
    double base = (double)mode * 1.2345;
    test2(base);
    
    test3(&global_counter);
    
    /* Test with many arguments */
    long result4 = test4(
        mode, mode+1, mode+2, mode+3,
        mode+4, mode+5, mode+6, mode+7,
        base, base * 2.0
    );
    global_accumulator += result4;
    
    /* Test nested call scenario */
    int nested_result = mode * 10;
    for (int i = 0; i < 2; i++) {
        nested_call_helper(i, &nested_result);
    }
    global_counter += nested_result;
    
    /* Compute checksum to prevent optimization */
    long final_checksum = global_counter + global_accumulator + (long)global_fp_accum;
    printf("Checksum: %ld\n", final_checksum);
    
    /* Additional complex control flow in main */
    if (mode == 0) {
        volatile int* ptr = (int*)malloc(sizeof(int) * 10);
        if (ptr) {
            opaque_func4(ptr);
            free(ptr);
        }
    }
    
    return (int)(final_checksum % 256);
}

/* Dummy definitions to satisfy linker (normally would be in separate file) */
void opaque_func1(void) {
    asm volatile("" ::: "memory");
}

int opaque_func2(int x) {
    asm volatile("" : "+r"(x) :: "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile("" : "+x"(x) :: "memory");
    return x * 1.5;
}

void* opaque_func4(void* x) {
    asm volatile("" : "+r"(x) :: "memory");
    return x;
}
