/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -ldl -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dlfcn.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 42;
volatile double global_volatile_double = 3.14159;
volatile long global_volatile_long = 1234567890;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[4];

/* Complex structure to force register pressure */
struct RegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    long l1, l2, l3;
};

/* Barrier to prevent optimization */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) \
    register int var asm(reg) = global_volatile_int

/* Test function 1: Many live variables across call with register constraints */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force multiple registers to be live */
    register int r10_val asm("r10") = mode * 2;
    register int r11_val asm("r11") = mode * 3;
    register int r12_val asm("r12") = mode * 4;
    register int r13_val asm("r13") = mode * 5;
    
    volatile int stack_slot[8];
    for (int i = 0; i < 8; i++) {
        stack_slot[i] = i + mode;
    }
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        /* Inline asm that looks like a call */
        asm volatile(
            "movl $0x12345678, %%eax\n\t"
            "movl $0x87654321, %%ebx\n\t"
            "call *%0\n\t"
            : 
            : "r" (func_table[mode & 3])
            : "eax", "ebx", "ecx", "edx", "memory"
        );
    } else {
        opaque_call_1();
    }
    
    COMPILER_BARRIER();
    
    /* Use all register variables after call - forces save/restore */
    int sum = r10_val + r11_val + r12_val + r13_val;
    for (int i = 0; i < 8; i++) {
        sum += stack_slot[i];
    }
    
    global_volatile_int = sum;
    
    /* Complex control flow with goto to split basic blocks */
    if (sum > 100) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Another call in a different basic block */
    asm volatile("" : : : "r14", "r15");
    opaque_call_2(sum);
    goto end;
    
label2:
    /* Different path with register pressure */
    register double xmm0 asm("xmm0") = global_volatile_double;
    asm volatile("" : "+x" (xmm0));
    global_volatile_double = xmm0 * 2.0;
    
end:
    /* Use volatile to prevent tail call optimization */
    COMPILER_BARRIER();
}

/* Test function 2: Floating point and mixed mode */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double fp_stack[6];
    volatile int int_stack[6];
    
    /* Create many live FP values */
    double d1 = base * 1.1;
    double d2 = base * 2.2;
    double d3 = base * 3.3;
    double d4 = base * 4.4;
    double d5 = base * 5.5;
    
    /* Save to volatile stack */
    fp_stack[0] = d1; fp_stack[1] = d2; fp_stack[2] = d3;
    fp_stack[3] = d4; fp_stack[4] = d5;
    
    /* Integer values in call-clobbered registers */
    register int eax_val asm("eax") = (int)base;
    register int ebx_val asm("ebx") = (int)(base * 10);
    
    COMPILER_BARRIER();
    
    /* Switch statement to create complex CFG */
    int choice = ((int)base) % 4;
    switch (choice) {
        case 0:
            opaque_call_4(d1, d2);
            break;
        case 1:
            /* Inline asm with many clobbers */
            asm volatile(
                "movq %1, %%xmm0\n\t"
                "movq %2, %%xmm1\n\t"
                "call *%0\n\t"
                : 
                : "r" (func_table[choice]), "r" (d1), "r" (d2)
                : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                  "rax", "rbx", "rcx", "rdx", "memory"
            );
            break;
        case 2:
            /* Nested call scenario */
            {
                int temp = opaque_call_3(eax_val, ebx_val);
                /* Use result immediately in complex expression */
                double result = opaque_call_4(d3, d4) * temp;
                fp_stack[5] = result;
            }
            break;
        default:
            /* Loop with break that creates block boundaries */
            for (int i = 0; i < 10; i++) {
                if (i > (int)base) {
                    opaque_call_2(i);
                    break;  /* Creates basic block split */
                }
                int_stack[i % 6] = i;
            }
    }
    
    COMPILER_BARRIER();
    
    /* Restore and use all values */
    double total = fp_stack[0] + fp_stack[1] + fp_stack[2] +
                   fp_stack[3] + fp_stack[4] + fp_stack[5];
    total += eax_val + ebx_val;
    
    global_volatile_double = total;
}

/* Test function 3: Variable arguments and __builtin_apply */
__attribute__((noinline, noclone))
void test3(int count, ...) {
    va_list args;
    va_start(args, count);
    
    volatile long saved_regs[8];
    
    /* Force many argument registers to be used */
    if (count > 0) {
        register long rdi_val asm("rdi") = va_arg(args, long);
        register long rsi_val asm("rsi") = va_arg(args, long);
        register long rdx_val asm("rdx") = count;
        register long rcx_val asm("rcx") = global_volatile_long;
        
        saved_regs[0] = rdi_val;
        saved_regs[1] = rsi_val;
        saved_regs[2] = rdx_val;
        saved_regs[3] = rcx_val;
        
        /* Use __builtin_apply to create unusual call sequence */
        if (count > 2) {
            void (*func)(long, long, long, long) = (void (*)(long, long, long, long))opaque_call_1;
            __builtin_apply(func, __builtin_apply_args(), 64);
        }
        
        COMPILER_BARRIER();
        
        /* Use saved values - forces reload */
        long sum = saved_regs[0] + saved_regs[1] + saved_regs[2] + saved_regs[3];
        for (int i = 4; i < 8 && i < count; i++) {
            saved_regs[i] = va_arg(args, long);
            sum += saved_regs[i];
        }
        
        global_volatile_long = sum;
    }
    
    va_end(args);
    
    /* Irreducible control flow with labels */
    static int toggle = 0;
    toggle = !toggle;
    
    if (toggle) {
        goto block_a;
    } else {
        goto block_b;
    }
    
block_a:
    asm volatile("" : : : "r8", "r9", "r10", "r11");
    opaque_call_2(count);
    goto block_c;
    
block_b:
    {
        register double xmm6 asm("xmm6") = 1.0;
        register double xmm7 asm("xmm7") = 2.0;
        asm volatile("" : "+x" (xmm6), "+x" (xmm7));
        global_volatile_double = xmm6 + xmm7;
    }
    goto block_c;
    
block_c:
    COMPILER_BARRIER();
}

/* Test function 4: Nested calls and loop with register pressure */
__attribute__((noinline, noclone))
int test4(int iterations) {
    volatile int results[10];
    struct RegPressure pressure;
    
    /* Initialize pressure structure */
    pressure.a = 1; pressure.b = 2; pressure.c = 3; pressure.d = 4;
    pressure.e = 5; pressure.f = 6; pressure.g = 7; pressure.h = 8;
    pressure.x = 1.1; pressure.y = 2.2; pressure.z = 3.3;
    pressure.l1 = 100; pressure.l2 = 200; pressure.l3 = 300;
    
    int total = 0;
    
    /* Complex loop with function calls and register usage */
    for (int i = 0; i < iterations; i++) {
        /* Live registers in loop */
        register int loop_r10 asm("r10") = i + pressure.a;
        register int loop_r11 asm("r11") = i + pressure.b;
        register double loop_xmm8 asm("xmm8") = pressure.x + i;
        
        COMPILER_BARRIER();
        
        /* Function call inside loop */
        if (i % 3 == 0) {
            opaque_call_3(loop_r10, loop_r11);
        } else if (i % 3 == 1) {
            /* Inline asm that clobbers many registers */
            asm volatile(
                "mov %0, %%r10\n\t"
                "mov %1, %%r11\n\t"
                "movq %2, %%xmm8\n\t"
                "call *%3\n\t"
                : 
                : "r" (loop_r10), "r" (loop_r11), "r" (loop_xmm8),
                  "r" (func_table[i % 4])
                : "r10", "r11", "xmm8", "xmm9", "xmm10", "xmm11",
                  "rax", "rbx", "rcx", "rdx", "r8", "r9", "memory"
            );
        }
        
        COMPILER_BARRIER();
        
        /* Use register values after call */
        results[i % 10] = loop_r10 + loop_r11 + (int)loop_xmm8;
        
        /* Break in middle of loop to split basic block */
        if (results[i % 10] > 1000) {
            opaque_call_2(i);
            break;
        }
        
        /* Continue creates another block boundary */
        if (i % 7 == 0) {
            continue;
        }
        
        total += results[i % 10];
    }
    
    /* Another call after loop */
    opaque_call_1();
    
    return total + pressure.c + pressure.d;
}

/* Helper with nested call */
__attribute__((noinline, noclone))
void nested_call_helper(int depth, int *result) {
    volatile int save[4];
    save[0] = depth;
    save[1] = *result;
    
    if (depth > 0) {
        /* Recursive-like nested call */
        register int r12_depth asm("r12") = depth;
        asm volatile("" : "+r" (r12_depth));
        
        opaque_call_3(depth, *result);
        
        /* Use saved value */
        *result += save[0] + save[1] + r12_depth;
        
        /* Call another function */
        nested_call_helper(depth - 1, result);
    } else {
        /* Base case with register pressure */
        register double xmm15 asm("xmm15") = 3.14159;
        asm volatile("" : "+x" (xmm15));
        *result += (int)xmm15;
    }
}

/* Main test orchestrator */
int main(int argc, char *argv[]) {
    /* Initialize function pointers to opaque functions */
    func_table[0] = (func_ptr_t)opaque_call_1;
    func_table[1] = (func_ptr_t)opaque_call_2;
    func_table[2] = (func_ptr_t)opaque_call_3;
    func_table[3] = (func_ptr_t)opaque_call_4;
    
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Run all test functions in different orders based on mode */
    switch (mode) {
        case 0:
            test1(10);
            test2(5.5);
            test3(6, 100L, 200L, 300L, 400L, 500L, 600L);
            test4(8);
            break;
        case 1:
            test2(7.7);
            test4(5);
            test1(15);
            test3(4, 50L, 60L, 70L, 80L);
            break;
        case 2:
            test3(8, 1L, 2L, 3L, 4L, 5L, 6L, 7L, 8L);
            test1(20);
            test4(6);
            test2(9.9);
            break;
        case 3:
            test4(10);
            test3(5, 10L, 20L, 30L, 40L, 50L);
            test2(3.3);
            test1(25);
            break;
    }
    
    /* Nested call test */
    int nested_result = 0;
    nested_call_helper(3, &nested_result);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_volatile_int + 
                   (int)global_volatile_double + 
                   (int)global_volatile_long +
                   nested_result;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* Dummy definitions to satisfy linker (these would normally be in a library) */
void opaque_call_1(void) {
    COMPILER_BARRIER();
}

void opaque_call_2(int x) {
    COMPILER_BARRIER();
    global_volatile_int += x;
}

int opaque_call_3(int a, int b) {
    COMPILER_BARRIER();
    return a + b + global_volatile_int;
}

double opaque_call_4(double a, double b) {
    COMPILER_BARRIER();
    return a + b + global_volatile_double;
}
