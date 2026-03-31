/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile long global_array[32] = {0};
volatile double global_fp[16] = {0.0};

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t func_table[8];

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force register usage with explicit constraints */
#define USE_REGISTER(reg, val) \
    do { \
        register long r asm(reg) = (val); \
        asm volatile("" : "+r"(r)); \
    } while(0)

/* Complex function with many live variables across calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Create many live variables that span calls */
    volatile int v1 = mode + 1;
    volatile long v2 = mode * 2;
    volatile double v3 = mode * 3.14;
    volatile int v4 = mode << 2;
    volatile long v5 = mode | 0xFF;
    volatile double v6 = v3 * 2.0;
    
    /* Use explicit register variables */
    register int r10_val asm("r10") = v1 * 2;
    register int r11_val asm("r11") = v2 & 0xFFFF;
    register double xmm0_val asm("xmm0") = v3;
    
    /* Array to force stack usage */
    long save_area[8];
    for (int i = 0; i < 8; i++) {
        save_area[i] = v2 + i;
    }
    
    /* First call - clobbers registers */
    asm volatile(
        "movl $0x12345678, %%eax\n\t"
        "movq $0x9ABCDEF0, %%r10\n\t"
        : : : "eax", "r10", "r11", "xmm0", "xmm1", "xmm2", "memory"
    );
    
    opaque_call_1();
    COMPILER_BARRIER();
    
    /* Use saved values after call */
    long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += save_area[i] + r10_val + r11_val;
    }
    
    /* Another call with different clobbers */
    asm volatile(
        "movsd %0, %%xmm0\n\t"
        "movsd %1, %%xmm1\n\t"
        : : "m"(v3), "m"(v6) : "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    opaque_call_2(mode);
    COMPILER_BARRIER();
    
    /* Complex expression requiring temporaries */
    double result = (v3 * xmm0_val) + (v6 / 2.0) - sum;
    
    /* Store to volatile global */
    global_fp[mode & 7] = result;
    global_counter += sum;
}

/* Function with switch statement creating complex CFG */
__attribute__((noinline, noclone))
void test2(int mode) {
    volatile int a = mode;
    volatile long b = mode * 3;
    volatile double c = mode * 1.618;
    
    /* Use many call-clobbered registers */
    register int eax_val asm("eax") = a + 1;
    register int edx_val asm("edx") = b >> 4;
    register double xmm7_val asm("xmm7") = c * 2.0;
    
    /* Switch with calls in cases - creates basic block boundaries */
    switch (mode & 3) {
        case 0:
            opaque_call_1();
            /* Fall through */
        case 1:
            asm volatile("" : : : "rax", "rdx", "rcx", "xmm0", "xmm1", "xmm2");
            opaque_call_2(a);
            break;
        case 2:
            opaque_call_3(a, b);
            /* Insert goto to create irreducible flow */
            if (a > 100) goto label1;
            break;
        default:
            opaque_call_4(c, c * 2.0);
            /* Use values after call */
            a = eax_val + edx_val;
            c = xmm7_val * 3.0;
            break;
    }
    
label1:
    /* Use all variables after switch */
    long total = a + b + (long)c + eax_val + edx_val;
    
    /* Another call site */
    asm volatile(
        "pushfq\n\t"
        "popfq\n\t"
        : : : "cc", "memory"
    );
    
    int result = opaque_call_3(total, mode);
    
    /* Store with barrier */
    COMPILER_BARRIER();
    global_array[mode & 31] = total + result;
}

/* Function with loops and nested calls */
__attribute__((noinline, noclone))
void test3(int iterations) {
    volatile long accum = 0;
    volatile double fp_accum = 0.0;
    
    /* Register variables that must survive calls */
    register long r12_val asm("r12") = 0x1234;
    register long r13_val asm("r13") = 0x5678;
    register double xmm8_val asm("xmm8") = 3.14159;
    
    for (int i = 0; i < iterations; i++) {
        /* Save current state */
        long saved_r12 = r12_val;
        double saved_xmm8 = xmm8_val;
        
        /* Call that clobbers registers */
        if (i & 1) {
            asm volatile(
                "movq %0, %%r12\n\t"
                "movq %1, %%r13\n\t"
                : : "r"(i), "r"(i * 2) : "r12", "r13", "r14", "r15"
            );
            opaque_call_1();
        } else {
            asm volatile(
                "movsd %0, %%xmm8\n\t"
                "movsd %1, %%xmm9\n\t"
                : : "m"(fp_accum), "m"(saved_xmm8) : "xmm8", "xmm9", "xmm10"
            );
            opaque_call_2(i);
        }
        
        COMPILER_BARRIER();
        
        /* Restore and use saved values */
        r12_val = saved_r12 + i;
        xmm8_val = saved_xmm8 * 1.1;
        
        /* Complex loop with break/continue creating block boundaries */
        if (i == iterations / 2) {
            /* Call at loop midpoint */
            int temp = opaque_call_3(i, accum);
            if (temp > 1000) {
                break;  /* Creates basic block end update */
            }
            continue;
        }
        
        accum += r12_val + r13_val;
        fp_accum += xmm8_val;
    }
    
    /* Final call with all live values */
    double result = opaque_call_4(fp_accum, accum);
    global_fp[iterations & 7] = result;
    global_counter += accum;
}

/* Function using __builtin_apply for unusual calling convention */
__attribute__((noinline, noclone))
void test4(int arg1, double arg2, long arg3) {
    /* Create va_list-like usage */
    volatile int local1 = arg1;
    volatile double local2 = arg2;
    volatile long local3 = arg3;
    
    /* Use many registers */
    register int r8_val asm("r8") = arg1 * 2;
    register int r9_val asm("r9") = arg3 >> 8;
    register double xmm4_val asm("xmm4") = arg2;
    register double xmm5_val asm("xmm5") = arg2 * 2.0;
    
    /* Save to stack */
    double save_fp[4] = {xmm4_val, xmm5_val, local2, local2 * 3.0};
    long save_int[4] = {r8_val, r9_val, local3, local1};
    
    /* Simulate __builtin_apply behavior */
    void* args = __builtin_apply_args();
    
    /* Multiple calls with register pressure */
    for (int i = 0; i < 3; i++) {
        /* Use inline asm as pseudo-call */
        asm volatile(
            "movq %0, %%rax\n\t"
            "movq %1, %%rdx\n\t"
            "movsd %2, %%xmm0\n\t"
            "call *%3\n\t"
            : : "r"(save_int[i]), "r"(save_int[i+1]), 
                "m"(save_fp[i]), "r"(func_table[i & 7])
            : "rax", "rdx", "rcx", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
              "memory"
        );
        
        COMPILER_BARRIER();
        
        /* Use saved values after each call */
        save_int[i] += r8_val + r9_val;
        save_fp[i] += xmm4_val + xmm5_val;
    }
    
    /* Final computation using all values */
    double final = 0.0;
    for (int i = 0; i < 4; i++) {
        final += save_fp[i] + save_int[i];
    }
    
    global_array[arg1 & 31] = final;
}

/* Helper with nested calls */
__attribute__((noinline, noclone))
int helper_nested(int depth, int val) {
    volatile int a = val;
    volatile long b = val * 2;
    
    /* Register variables */
    register int rbx_val asm("rbx") = a + depth;
    register int rbp_val asm("rbp") = b & 0xFF;
    
    if (depth > 0) {
        /* Recursive call */
        int result = helper_nested(depth - 1, val + 1);
        
        /* Use registers after call */
        a = rbx_val + result;
        b = rbp_val * result;
        
        /* Another call */
        asm volatile(
            "movl %0, %%ebx\n\t"
            "movl %1, %%ebp\n\t"
            : : "r"(a), "r"(b) : "rbx", "rbp", "r12", "r13"
        );
        
        opaque_call_2(result);
    } else {
        /* Leaf call with clobbers */
        asm volatile(
            "cpuid\n\t"
            : : : "rax", "rbx", "rcx", "rdx", "memory"
        );
    }
    
    COMPILER_BARRIER();
    return a + b + rbx_val + rbp_val;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Initialize function pointers */
    for (int i = 0; i < 8; i++) {
        func_table[i] = (func_ptr_t)opaque_call_1;
    }
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) & 3;
    }
    
    /* Run all tests with different parameters */
    test1(mode * 10 + 1);
    test2(mode * 7 + 2);
    test3(mode * 5 + 3);
    test4(mode * 3 + 4, mode * 1.414, mode * 1000);
    
    /* Test with nested calls */
    int nested_result = helper_nested(3, mode);
    
    /* Force use of all global variables to prevent DCE */
    long checksum = global_counter + nested_result;
    for (int i = 0; i < 32; i++) {
        checksum += global_array[i];
    }
    for (int i = 0; i < 16; i++) {
        checksum += (long)global_fp[i];
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return (checksum & 0xFF);
}

/* Dummy definitions for external functions */
void opaque_call_1(void) {
    COMPILER_BARRIER();
}

void opaque_call_2(int x) {
    COMPILER_BARRIER();
    global_counter += x;
}

int opaque_call_3(int x, int y) {
    COMPILER_BARRIER();
    return x + y;
}

double opaque_call_4(double x, double y) {
    COMPILER_BARRIER();
    return x * y;
}
