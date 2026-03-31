/* test-caller-save.c
 * 
 * This program is specifically designed to trigger the uncovered code paths
 * in GCC's caller-save.cc, particularly the instruction chain manipulation
 * logic around lines 905-913.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_data[256];
volatile double global_fp[256];

/* Function pointers with volatile to prevent constant propagation */
void (*volatile fp1)(void) = opaque_call_1;
int (*volatile fp2)(int, int) = (int (*)(int, int))opaque_call_3;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1_many_live_vars(int n) {
    /* Force many registers to be live across call */
    register int r0 asm ("r10") = n + 1;
    register int r1 asm ("r11") = n + 2;
    register int r2 asm ("r12") = n + 3;
    volatile int v0 = n + 4;
    volatile int v1 = n + 5;
    
    /* Complex expression requiring temporary registers */
    int complex = (r0 * r1) + (r2 << 2) - (v0 / v1);
    
    /* Call with clobbered registers */
    asm volatile(
        "movl %0, %%eax\n\t"
        "movl %1, %%ebx\n\t"
        "call *%2\n\t"
        : 
        : "r"(complex), "r"(n), "r"(fp1)
        : "eax", "ebx", "ecx", "edx", "memory"
    );
    
    /* Use all previously live variables after call */
    global_data[0] = r0 + r1 + r2 + v0 + v1 + complex;
    COMPILER_BARRIER();
}

/* Test function 2: Nested calls with register pressure */
__attribute__((noinline, noclone))
void test2_nested_calls(int a, int b, int c, int d, int e, int f) {
    /* Create irreducible control flow with goto */
    volatile int choice = global_counter++ % 3;
    
    if (choice == 0) {
        goto label1;
    } else if (choice == 1) {
        goto label2;
    }
    
    /* Force register spilling by using many variables */
    int x1 = a * b;
    int x2 = c * d;
    int x3 = e * f;
    int x4 = x1 + x2;
    int x5 = x3 - x4;
    int x6 = x5 << 2;
    int x7 = x6 / (a + 1);
    
    /* Call with many arguments */
    int result = fp2(x1, x2);
    
    /* Use variables across call */
    x3 = result + x7;
    
    /* Another call in different basic block */
    if (x3 > 0) {
        asm volatile(
            "pushq %%r10\n\t"
            "pushq %%r11\n\t"
            "call opaque_call_2\n\t"
            "popq %%r11\n\t"
            "popq %%r10\n\t"
            : 
            : "a"(x3)
            : "memory"
        );
    }
    
    goto label_end;
    
label1:
    {
        /* Different basic block with its own live vars */
        volatile int local[10];
        for (int i = 0; i < 10; i++) {
            local[i] = i * a;
        }
        
        /* Call that clobbers registers */
        asm volatile(
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            "call *%0\n\t"
            : 
            : "r"(fp1)
            : "r10", "r11", "memory"
        );
        
        /* Use local array after call */
        global_data[1] = local[5] + local[6];
    }
    goto label_end;
    
label2:
    {
        /* Loop with break that creates block boundaries */
        int sum = 0;
        for (int i = 0; i < 100; i++) {
            if (i == 50) {
                /* Function call at block boundary */
                opaque_call_2(sum);
                break;
            }
            sum += i * b;
        }
        global_data[2] = sum;
    }
    
label_end:
    COMPILER_BARRIER();
}

/* Test function 3: Switch statement with calls at boundaries */
__attribute__((noinline, noclone))
void test3_switch_complex(int val) {
    /* Force switch with many cases */
    switch (val % 7) {
        case 0: {
            register int r asm ("rax") = val;
            asm volatile("" : "+r"(r));
            opaque_call_1();
            global_data[3] = r;
            break;
        }
        case 1: {
            volatile int x = val * 2;
            opaque_call_2(x);
            /* Fall through */
        }
        case 2: {
            int y = val + 100;
            asm volatile(
                "movl %0, %%r10d\n\t"
                "call opaque_call_2\n\t"
                : 
                : "r"(y)
                : "r10", "memory"
            );
            break;
        }
        case 3:
            /* Empty case that falls through */
        case 4: {
            double d = val * 3.14;
            asm volatile(
                "movsd %0, %%xmm0\n\t"
                "call opaque_call_4\n\t"
                : 
                : "m"(d)
                : "xmm0", "memory"
            );
            break;
        }
        default: {
            /* Complex default with multiple calls */
            int t1 = val * val;
            int t2 = t1 + val;
            opaque_call_2(t1);
            int t3 = opaque_call_3(t1, t2);
            opaque_call_2(t3);
            global_data[4] = t3;
        }
    }
    
    COMPILER_BARRIER();
}

/* Test function 4: Mixed float/int with many calls */
__attribute__((noinline, noclone))
void test4_mixed_types(float f1, float f2, double d1, int i1, int i2) {
    /* Force both integer and FP registers to be live */
    volatile float vf1 = f1;
    volatile float vf2 = f2;
    volatile double vd1 = d1;
    register int ri1 asm ("r14") = i1;
    register int ri2 asm ("r15") = i2;
    
    /* Create control flow that splits blocks */
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            /* Call that clobbers registers */
            asm volatile(
                "pxor %%xmm0, %%xmm0\n\t"
                "call opaque_call_1\n\t"
                : 
                : 
                : "xmm0", "memory"
            );
        } else if (i % 3 == 1) {
            /* Use integer registers */
            int temp = ri1 + ri2 + i;
            opaque_call_2(temp);
        } else {
            /* Use FP registers */
            double temp = vd1 * i;
            asm volatile(
                "movsd %0, %%xmm1\n\t"
                "call opaque_call_4\n\t"
                : 
                : "m"(temp)
                : "xmm1", "memory"
            );
        }
        
        /* Modify live variables in loop */
        vf1 += 1.0f;
        vd1 += 0.5;
        ri1++;
    }
    
    /* Force all variables to be used after loop */
    global_fp[0] = vf1 + vf2 + vd1;
    global_data[5] = ri1 + ri2;
    COMPILER_BARRIER();
}

/* Helper with nested call to create outer/inner save scenarios */
__attribute__((noinline, noclone))
int helper_nested(int x, int y) {
    /* Inner call that needs saves */
    int inner = opaque_call_3(x, y);
    
    /* Complex expression requiring register */
    int result = (inner * x) / (y + 1);
    
    /* Another call */
    opaque_call_2(result);
    
    return result;
}

/* Test function 5: Uses helper with nested calls */
__attribute__((noinline, noclone))
void test5_nested_helper(int base) {
    volatile int results[10];
    
    /* Loop calling helper */
    for (int i = 0; i < 10; i++) {
        if (i == 5) {
            /* Insert a call at loop boundary */
            asm volatile(
                "movl $999, %%edi\n\t"
                "call opaque_call_2\n\t"
                : 
                : 
                : "edi", "memory"
            );
        }
        
        /* Call helper (which has its own nested call) */
        results[i] = helper_nested(base + i, i * 2);
        
        /* Force spill by using many temporaries */
        int t1 = results[i] * 2;
        int t2 = t1 + base;
        int t3 = t2 << 1;
        int t4 = t3 / (i + 1);
        
        /* Use inline asm to clobber specific registers */
        asm volatile(
            "movl %0, %%r10d\n\t"
            "movl %1, %%r11d\n\t"
            "addl %%r10d, %%r11d\n\t"
            : 
            : "r"(t3), "r"(t4)
            : "r10", "r11"
        );
    }
    
    /* Checksum */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
    }
    global_data[6] = sum;
    COMPILER_BARRIER();
}

/* Main function that drives all tests */
int main(int argc, char *argv[]) {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_data[i] = i;
        global_fp[i] = i * 0.1;
    }
    
    /* Use argv to create runtime variability */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Run all tests in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 5) {
            case 0:
                test1_many_live_vars(cycle * 100);
                test2_nested_calls(cycle, cycle+1, cycle+2, cycle+3, cycle+4, cycle+5);
                break;
            case 1:
                test3_switch_complex(cycle * 50);
                test4_mixed_types(1.1f * cycle, 2.2f * cycle, 
                                3.3 * cycle, cycle * 10, cycle * 20);
                break;
            case 2:
                test5_nested_helper(cycle * 30);
                test1_many_live_vars(cycle * 40);
                break;
            case 3:
                test2_nested_calls(cycle, cycle*2, cycle*3, cycle*4, cycle*5, cycle*6);
                test3_switch_complex(cycle * 60);
                break;
            case 4:
                test4_mixed_types(0.5f * cycle, 1.5f * cycle,
                                2.5 * cycle, cycle * 15, cycle * 25);
                test5_nested_helper(cycle * 35);
                break;
        }
        
        /* Force periodic opaque calls */
        if (cycle % 2 == 0) {
            /* Use function pointer */
            fp1();
        } else {
            /* Direct call */
            asm volatile(
                "movl $42, %%edi\n\t"
                "call opaque_call_2\n\t"
                : 
                : 
                : "edi", "memory"
            );
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < 256; i++) {
        checksum += global_data[i];
        checksum += (unsigned long long)(global_fp[i] * 1000);
    }
    
    printf("Checksum: %llu\n", checksum);
    
    return (checksum > 0) ? 0 : 1;
}

/* Dummy definitions to satisfy linker (in real test, these would be
 * in a separate library or implemented with __attribute__((weak)) */
void opaque_call_1(void) {
    global_counter++;
}

void opaque_call_2(int x) {
    global_data[x % 256] += x;
}

int opaque_call_3(int a, int b) {
    return a + b + global_counter++;
}

double opaque_call_4(double x) {
    return x * 1.1 + global_counter++;
}
