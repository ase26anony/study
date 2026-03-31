/* test-caller-save.c
 * 
 * This program is designed to trigger the uncovered instruction chain
 * manipulation code in GCC's caller-save.cc (lines 905-913) by creating
 * scenarios that require complex register saves/restores around function calls.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to create opaque call targets */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int g_volatile_int = 12345;
volatile double g_volatile_double = 3.14159;
volatile long g_volatile_long = 987654321L;

/* Function pointers to create indirect calls */
typedef void (*func_ptr_t)(void);
typedef int (*func_ptr_int_t)(int, int, int, int, int, int);

/* Global array to store live values across calls */
volatile int live_values[32];
volatile double live_doubles[16];

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific registers with asm constraints */
#define FORCE_REGISTER(var, reg) \
    register int var asm(reg)

/* Test function 1: Many live integer variables across a call */
__attribute__((noinline, noclone))
int test1(int mode) {
    /* Force many variables to be live across call */
    volatile int a = g_volatile_int;
    volatile int b = a + 1;
    volatile int c = b * 2;
    volatile int d = c - 3;
    volatile int e = d / 4;
    volatile int f = e % 5;
    volatile int g = f ^ 0xFF;
    volatile int h = g << 2;
    volatile int i = h >> 1;
    volatile int j = i | 0xAA;
    
    /* Store to global array to force spills */
    live_values[0] = a;
    live_values[1] = b;
    live_values[2] = c;
    live_values[3] = d;
    live_values[4] = e;
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        opaque_call_1();
    } else {
        /* Inline asm that acts like a call */
        asm volatile(
            "movl $0, %%eax\n"
            "movl $0, %%ebx\n"
            "movl $0, %%ecx\n"
            "movl $0, %%edx\n"
            "movl $0, %%esi\n"
            "movl $0, %%edi\n"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
    
    COMPILER_BARRIER();
    
    /* Complex use of all live variables after call */
    int result = (a + b) * (c - d) + (e * f) - (g / (h + 1)) + (i ^ j);
    
    /* More operations to create register pressure */
    result += live_values[0] * 2;
    result -= live_values[1] / 3;
    result ^= live_values[2];
    
    return result;
}

/* Test function 2: Mixed integer and floating point */
__attribute__((noinline, noclone))
double test2(int mode) {
    /* Mix of integer and FP variables */
    volatile int i1 = g_volatile_int;
    volatile int i2 = i1 * 2;
    volatile double d1 = g_volatile_double;
    volatile double d2 = d1 * 2.0;
    volatile double d3 = d2 + 1.5;
    
    /* Force specific register usage with inline asm */
    FORCE_REGISTER(forced_reg, "r12");
    forced_reg = i1 + i2;
    
    live_values[5] = i1;
    live_values[6] = i2;
    live_doubles[0] = d1;
    live_doubles[1] = d2;
    live_doubles[2] = d3;
    
    COMPILER_BARRIER();
    
    /* Call that might clobber FP registers */
    if (mode & 2) {
        double r = opaque_call_4(d1, d2);
        d3 += r;
    } else {
        /* Clobber both integer and FP regs */
        asm volatile(
            "fldz\n"
            "fstp %%st(0)\n"
            "movl $0, %%r10d\n"
            "movl $0, %%r11d\n"
            : : : "st", "r10", "r11", "memory"
        );
    }
    
    COMPILER_BARRIER();
    
    /* Complex FP computation using all live variables */
    double result = d1 * d2 + d3;
    result += (double)i1 / (double)i2;
    result += (double)forced_reg * 0.5;
    
    /* Use stored values */
    result += live_doubles[0] - live_doubles[1];
    result *= live_doubles[2];
    
    return result;
}

/* Test function 3: Nested calls with irreducible control flow */
__attribute__((noinline, noclone))
int test3(int mode) {
    volatile int x = g_volatile_int;
    volatile int y = x + 100;
    volatile int z = y * 2;
    
    int result = 0;
    
    /* Create complex control flow with goto */
    if (mode & 4) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    {
        volatile int a = x + y;
        live_values[7] = a;
        
        /* Call in one branch */
        opaque_call_2(a);
        
        result += a;
        goto label3;
    }
    
label2:
    {
        volatile int b = y - z;
        live_values[8] = b;
        
        /* Different call in other branch */
        asm volatile(
            "call *%0\n"
            : : "r"((void*)opaque_call_1) : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
        );
        
        result += b;
        goto label3;
    }
    
label3:
    /* Loop with break that creates block boundaries */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            /* Call at loop mid-point */
            int tmp = opaque_call_3(x, y);
            result += tmp;
            break;  /* Creates basic block split */
        }
        result += i;
    }
    
    /* Use all live variables */
    result += x * y - z;
    result += live_values[7] + live_values[8];
    
    return result;
}

/* Test function 4: Many arguments and register pressure */
__attribute__((noinline, noclone))
int test4(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* All arguments are live */
    volatile int v1 = a + b;
    volatile int v2 = c * d;
    volatile int v3 = e - f;
    volatile int v4 = g ^ h;
    
    /* Force spills by using many temporaries */
    int t1 = v1 * 2;
    int t2 = v2 / 3;
    int t3 = v3 << 1;
    int t4 = v4 >> 2;
    int t5 = t1 + t2;
    int t6 = t3 - t4;
    int t7 = t5 * t6;
    int t8 = t7 ^ 0xABCD;
    
    live_values[9] = t1;
    live_values[10] = t2;
    live_values[11] = t3;
    live_values[12] = t4;
    live_values[13] = t5;
    live_values[14] = t6;
    live_values[15] = t7;
    live_values[16] = t8;
    
    COMPILER_BARRIER();
    
    /* Call with many arguments - forces register allocation complexity */
    func_ptr_int_t fp = (func_ptr_int_t)opaque_call_3;
    
    /* Indirect call with register clobbering */
    asm volatile(
        "pushq %%rbx\n"
        "pushq %%rbp\n"
        "pushq %%r12\n"
        "pushq %%r13\n"
        "pushq %%r14\n"
        "pushq %%r15\n"
        "call *%0\n"
        "popq %%r15\n"
        "popq %%r14\n"
        "popq %%r13\n"
        "popq %%r12\n"
        "popq %%rbp\n"
        "popq %%rbx\n"
        : : "r"(fp) : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
    
    COMPILER_BARRIER();
    
    /* Complex computation using all temporaries */
    int result = t1 + t2 - t3 * t4 / t5 ^ t6 | t7 & t8;
    
    /* Chain of operations that might need many registers */
    for (int i = 0; i < 8; i++) {
        result += live_values[9 + i];
        result *= (i + 1);
    }
    
    return result;
}

/* Test function 5: Switch statement with calls in cases */
__attribute__((noinline, noclone))
int test5(int mode) {
    volatile int base = g_volatile_int;
    int result = 0;
    
    /* Switch creates complex CFG */
    switch (mode % 5) {
        case 0: {
            volatile int a = base + 1;
            opaque_call_1();
            result = a * 2;
            break;
        }
        case 1: {
            volatile int b = base - 1;
            asm volatile(
                "movl $0, %%eax\n"
                "cpuid\n"
                : : : "eax", "ebx", "ecx", "edx", "memory"
            );
            result = b / 2;
            break;
        }
        case 2: {
            volatile int c = base * 2;
            /* Fall through to default */
        }
        default: {
            volatile int d = base ^ 0xFF;
            /* Call in default case - might be at block end */
            int r = opaque_call_3(base, d);
            result = r + d;
            
            /* Additional computation after call */
            for (int i = 0; i < 3; i++) {
                result += i;
                if (i == 1) {
                    /* Nested call in loop */
                    opaque_call_2(result);
                }
            }
            break;
        }
    }
    
    return result;
}

/* Helper with nested call */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    volatile int local = val;
    
    if (depth > 0) {
        /* Recursive-like but bounded to prevent infinite recursion */
        int r = opaque_call_3(val, depth);
        local += r;
        
        /* Another call in the helper */
        if (depth > 1) {
            opaque_call_2(local);
        }
    }
    
    return local * 2;
}

/* Test function 6: Nested calls and loops */
__attribute__((noinline, noclone))
int test6(int mode) {
    volatile int start = g_volatile_int;
    int total = 0;
    
    /* Loop with nested function calls */
    for (int i = 0; i < 5; i++) {
        volatile int iter_val = start + i;
        
        /* Call helper which itself makes calls */
        int r = nested_helper(i % 3, iter_val);
        total += r;
        
        /* Inline asm between calls */
        if (i & 1) {
            asm volatile(
                "movq %%rsp, %%rax\n"
                "addq $16, %%rax\n"
                : : : "rax", "memory"
            );
        }
        
        /* Another call in loop */
        if (i == 2 || i == 4) {
            opaque_call_1();
        }
        
        /* Use value after calls */
        total += iter_val * 3;
    }
    
    return total;
}

/* Main function that orchestrates all tests */
int main(int argc, char *argv[]) {
    int mode = 0;
    
    /* Use command line argument to vary behavior */
    if (argc > 1) {
        mode = atoi(argv[1]);
    } else {
        mode = 0x7F;  /* Run all tests */
    }
    
    /* Initialize some volatile state */
    g_volatile_int = 42;
    g_volatile_double = 2.71828;
    g_volatile_long = 123456789;
    
    /* Clear live value arrays */
    memset((void*)live_values, 0, sizeof(live_values));
    memset((void*)live_doubles, 0, sizeof(live_doubles));
    
    int checksum = 0;
    double fp_checksum = 0.0;
    
    /* Run all test functions */
    checksum += test1(mode);
    fp_checksum += test2(mode);
    checksum += test3(mode);
    checksum += test4(1, 2, 3, 4, 5, 6, 7, 8);
    checksum += test5(mode);
    checksum += test6(mode);
    
    /* Use all global volatiles to prevent DCE */
    checksum += g_volatile_int;
    checksum += (int)g_volatile_double;
    checksum += (int)g_volatile_long;
    
    /* Use live arrays */
    for (int i = 0; i < 32; i++) {
        checksum ^= live_values[i];
    }
    
    /* Print result to prevent optimization */
    printf("Result: checksum=%d, fp_checksum=%f\n", checksum, fp_checksum);
    
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
void opaque_call_1(void) {
    asm volatile("" : : : "memory");
}

void opaque_call_2(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int opaque_call_3(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a + b;
}

double opaque_call_4(double a, double b) {
    asm volatile("" : : "x"(a), "x"(b) : "memory");
    return a + b;
}
