/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targets lines 905-913 which handle instruction chain manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void opaque_func4(int, ...);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_accumulator = 0;
volatile double global_fp = 3.14159;
volatile void *global_ptr = NULL;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[8];

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" ::: "memory")

/* Force register usage with explicit constraints */
#define FORCE_REGISTER(var, reg) \
    register long var asm(reg) = (long)(global_counter++)

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many live variables */
    volatile int v1 = mode * 2;
    volatile int v2 = mode * 3;
    volatile int v3 = mode * 5;
    volatile int v4 = mode * 7;
    volatile double f1 = global_fp * mode;
    volatile double f2 = global_fp * (mode + 1);
    
    /* Use explicit register variables to create pressure */
    FORCE_REGISTER(r10_var, "r10");
    FORCE_REGISTER(r11_var, "r11");
    
    /* Array to save values across calls */
    long saved[8];
    saved[0] = v1 + v2;
    saved[1] = v3 * v4;
    saved[2] = (long)(f1 * 1000);
    saved[3] = r10_var;
    saved[4] = r11_var;
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        /* Direct call */
        opaque_func1();
        
        /* Inline asm that looks like a call */
        asm volatile(
            "movq $0x12345678, %%rax\n\t"
            "call *%%rax\n\t"
            : 
            : 
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
              "memory"
        );
    } else {
        /* Indirect call via function pointer */
        func_table[mode % 8]();
    }
    
    COMPILER_BARRIER();
    
    /* Complex use of saved values requiring original registers */
    long result = saved[0] * saved[1] + saved[2] / (saved[3] + 1) - saved[4];
    
    /* Force register pressure with inline asm */
    asm volatile(
        "addq %1, %0\n\t"
        "subq %2, %0\n\t"
        "imulq %3, %0\n\t"
        : "+r" (result)
        : "r" (saved[0]), "r" (saved[1]), "r" (saved[2])
        : "cc"
    );
    
    global_accumulator += result;
}

/* Function with irreducible control flow */
__attribute__((noinline, noclone))
void test2(int iterations) {
    volatile int a = 1, b = 2, c = 3, d = 4;
    volatile double x = 1.0, y = 2.0;
    
    /* Create complex control flow with goto */
    int i = 0;
    
start_loop:
    if (i >= iterations) goto end_loop;
    
    /* Save values before call */
    int saved_a = a;
    int saved_b = b;
    double saved_x = x;
    
    /* Function call in the middle of loop */
    if (i % 3 == 0) {
        opaque_func2(i);
        
        /* Force basic block boundary */
        if (saved_a > saved_b) {
            goto adjust_values;
        }
    } else if (i % 3 == 1) {
        /* Another call site */
        asm volatile(
            "pushq %%rbp\n\t"
            "movq $0xDEADBEEF, %%rax\n\t"
            "call *%%rax\n\t"
            "popq %%rbp\n\t"
            : : : "rax", "rbx", "rcx", "rdx", "memory"
        );
    }
    
    /* Use saved values - forces reload to insert restore instructions */
    c = saved_a * saved_b + (int)saved_x;
    
    /* Complex expression requiring temporary registers */
    d = (saved_a << 2) | (saved_b >> 1) ^ (c * 3);
    
    /* Another potential basic block boundary */
    if (d > 100) {
        x = y * 2.0;
        goto update_counter;
    }
    
adjust_values:
    a = b + 1;
    b = c - 1;
    
update_counter:
    i++;
    goto start_loop;
    
end_loop:
    /* Final computation using all variables */
    global_accumulator += a + b + c + d + (int)(x + y);
}

/* Function with switch statement creating complex CFG */
__attribute__((noinline, noclone))
int test3(int selector) {
    volatile int reg_var1, reg_var2, reg_var3, reg_var4;
    
    /* Use explicit register variables */
    register int r12_var asm("r12") = selector * 2;
    register int r13_var asm("r13") = selector * 3;
    register int r14_var asm("r14") = selector * 5;
    
    /* Save to volatile memory */
    reg_var1 = r12_var;
    reg_var2 = r13_var;
    reg_var3 = r14_var;
    
    COMPILER_BARRIER();
    
    /* Switch with calls in different cases */
    switch (selector % 5) {
        case 0:
            opaque_func1();
            reg_var4 = reg_var1 + reg_var2;
            break;
        case 1:
            opaque_func2(reg_var1);
            reg_var4 = reg_var2 * reg_var3;
            /* Fall through */
        case 2:
            asm volatile(
                "movl $0x1234, %%eax\n\t"
                "call *%%rax\n\t"
                : : : "rax", "rbx", "rcx", "memory"
            );
            reg_var4 = reg_var1 | reg_var2;
            break;
        case 3:
            /* Variadic call */
            opaque_func4(3, reg_var1, reg_var2, reg_var3);
            reg_var4 = reg_var1 ^ reg_var2;
            break;
        default:
            /* Multiple calls in default case */
            opaque_func3(global_fp);
            COMPILER_BARRIER();
            func_table[selector % 8]();
            reg_var4 = reg_var1 & reg_var2;
            
            /* Nested condition with another call */
            if (reg_var4 > 100) {
                opaque_func2(reg_var4);
            }
            break;
    }
    
    COMPILER_BARRIER();
    
    /* Force use of all saved register variables */
    int result = (reg_var1 * reg_var2) + (reg_var3 / (reg_var4 + 1));
    
    /* Complex asm to force register allocation conflicts */
    asm volatile(
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "subl %3, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (reg_var1), "r" (reg_var2), "r" (reg_var3)
        : "eax", "cc"
    );
    
    return result;
}

/* Function with nested calls and register pressure */
__attribute__((noinline, noclone))
double test4(int depth) {
    if (depth <= 0) {
        return global_fp;
    }
    
    /* Many live floating point variables */
    volatile double f1 = global_fp * depth;
    volatile double f2 = f1 * 1.5;
    volatile double f3 = f2 * 2.0;
    volatile double f4 = f3 / 1.7;
    
    /* Integer variables mixed in */
    volatile int i1 = depth * 2;
    volatile int i2 = depth * 3;
    
    /* Save to array */
    double saved_fp[4];
    saved_fp[0] = f1;
    saved_fp[1] = f2;
    saved_fp[2] = f3;
    saved_fp[3] = f4;
    
    int saved_int[2];
    saved_int[0] = i1;
    saved_int[1] = i2;
    
    COMPILER_BARRIER();
    
    /* Nested recursive call */
    double nested_result = test4(depth - 1);
    
    COMPILER_BARRIER();
    
    /* Complex computation using saved values */
    double result = saved_fp[0] * nested_result;
    result += saved_fp[1] / (saved_int[0] + 1);
    result -= saved_fp[2] * saved_fp[3];
    result /= (saved_int[1] + 1);
    
    /* Inline asm using xmm registers */
    asm volatile(
        "movsd %1, %%xmm0\n\t"
        "addsd %2, %%xmm0\n\t"
        "mulsd %3, %%xmm0\n\t"
        "movsd %%xmm0, %0\n\t"
        : "=m" (result)
        : "m" (saved_fp[0]), "m" (saved_fp[1]), "m" (nested_result)
        : "xmm0"
    );
    
    return result;
}

/* Function with __builtin_apply to create unusual call sequences */
__attribute__((noinline, noclone))
void test5(void *arg) {
    volatile long params[6];
    for (int i = 0; i < 6; i++) {
        params[i] = (long)arg + i * 100;
    }
    
    /* Simulate __builtin_apply behavior */
    void *args = __builtin_apply_args();
    
    /* Save current register state */
    register long rcx_save asm("rcx");
    register long rdx_save asm("rdx");
    register long rsi_save asm("rsi");
    register long rdi_save asm("rdi");
    
    asm volatile("movq %%rcx, %0" : "=r" (rcx_save));
    asm volatile("movq %%rdx, %0" : "=r" (rdx_save));
    asm volatile("movq %%rsi, %0" : "=r" (rsi_save));
    asm volatile("movq %%rdi, %0" : "=r" (rdi_save));
    
    COMPILER_BARRIER();
    
    /* Make a call that will clobber argument registers */
    opaque_func4(4, params[0], params[1], params[2], params[3]);
    
    COMPILER_BARRIER();
    
    /* Restore and use saved registers */
    long sum = rcx_save + rdx_save + rsi_save + rdi_save;
    
    /* Force spill/restore around this computation */
    for (int i = 0; i < 6; i++) {
        sum += params[i] * (i + 1);
    }
    
    /* Complex asm with many clobbers */
    asm volatile(
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"
        "movq %%rax, %0\n\t"
        : "=r" (sum)
        : "r" (sum), "r" (rcx_save)
        : "rax", "cc"
    );
    
    global_accumulator += sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Initialize function pointers */
    for (int i = 0; i < 8; i++) {
        func_table[i] = opaque_func1;
    }
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 10;
    }
    
    /* Initialize volatile state */
    global_counter = mode;
    global_accumulator = 0;
    global_fp = 3.14159 * (mode + 1);
    
    /* Run all test functions in sequence */
    test1(mode);
    
    /* Create register pressure between tests */
    COMPILER_BARRIER();
    
    test2(5 + (mode % 3));
    
    COMPILER_BARRIER();
    
    int t3_result = test3(mode);
    global_accumulator += t3_result;
    
    COMPILER_BARRIER();
    
    double t4_result = test4(3 + (mode % 2));
    global_accumulator += (long)t4_result;
    
    COMPILER_BARRIER();
    
    test5((void*)(long)mode);
    
    /* Final checksum to prevent dead code elimination */
    long final_checksum = global_accumulator + global_counter + (long)global_fp;
    
    /* Use result in a way that can't be optimized away */
    if (final_checksum != 0) {
        printf("Result: %ld\n", final_checksum);
    }
    
    return (final_checksum == 0) ? 1 : 0;
}

/* Dummy definitions to satisfy linker (these would normally be in a library) */
void opaque_func1(void) {
    global_counter++;
}

int opaque_func2(int x) {
    return x * 2;
}

double opaque_func3(double x) {
    return x * 1.5;
}

void opaque_func4(int n, ...) {
    va_list args;
    va_start(args, n);
    for (int i = 0; i < n; i++) {
        global_accumulator += va_arg(args, int);
    }
    va_end(args);
}
