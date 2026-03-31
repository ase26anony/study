/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* External opaque functions to force call instructions */
extern void opaque_call_1(void);
extern int opaque_call_2(int, int);
extern double opaque_call_3(double, double);

/* Volatile globals to prevent optimization */
volatile int g1 = 1, g2 = 2, g3 = 3, g4 = 4, g5 = 5;
volatile double gd1 = 1.0, gd2 = 2.0, gd3 = 3.0;
volatile void *gp1, *gp2;

/* Function pointer with volatile to prevent devirtualization */
typedef int (*func_ptr_t)(int, ...);
volatile func_ptr_t volatile_fp = NULL;

/* Complex control flow with register pressure */
__attribute__((noinline, noclone))
int test1(int x, int y) {
    volatile int v1 = x, v2 = y;
    register int r1 asm ("r12") = x + 1;
    register int r2 asm ("r13") = y + 2;
    int arr[10];
    
    /* Force register pressure */
    for (int i = 0; i < 10; i++) {
        arr[i] = i + r1 + r2;
    }
    
    /* Complex control flow with goto */
    if (x > y) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Function call with many live values */
    opaque_call_1();
    asm volatile ("" : : : "memory");
    
    /* Use all registers after call */
    int sum = r1 + r2 + v1 + v2 + arr[0] + arr[9];
    
    /* Another call with different convention */
    sum += opaque_call_2(r1, r2);
    
    /* Force basic block boundary manipulation */
    switch (sum % 4) {
        case 0: sum += g1; break;
        case 1: sum += g2; break;
        case 2: sum += g3; break;
        default: 
            opaque_call_1();  /* Call at switch default */
            sum += g4;
            break;
    }
    
    return sum;
    
label2:
    /* Different path with nested calls */
    int tmp = opaque_call_2(v1, v2);
    asm volatile ("" : : : "memory");
    return tmp + r1 + r2;
}

/* Floating point intensive with mixed calls */
__attribute__((noinline, noclone))
double test2(double a, double b) {
    volatile double vd1 = a, vd2 = b;
    register double rd1 asm ("xmm8") = a * 2.0;
    register double rd2 asm ("xmm9") = b * 3.0;
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile ("# dummy asm" : : : 
        "rax", "rcx", "rdx", "rsi", "rdi", 
        "r8", "r9", "r10", "r11",
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7");
    
    /* Call with floating args */
    double result = opaque_call_3(rd1, rd2);
    
    /* Complex expression requiring temporaries */
    result = result + vd1 * vd2 + rd1 / rd2;
    
    /* Loop with break that creates block boundaries */
    for (int i = 0; i < 100; i++) {
        if (i > g1) {
            opaque_call_1();  /* Call inside loop with break */
            break;
        }
        result += i * 0.1;
    }
    
    return result;
}

/* Function with variable arguments */
__attribute__((noinline, noclone))
int test3(int n, ...) {
    va_list ap;
    va_start(ap, n);
    
    int sum = n;
    register int r3 asm ("r14") = g1;
    register int r4 asm ("r15") = g2;
    
    /* Force many live values across va_arg */
    for (int i = 0; i < n; i++) {
        int val = va_arg(ap, int);
        sum += val + r3 + r4;
        
        /* Call in middle of loop */
        if (i == n/2) {
            opaque_call_2(r3, r4);
            asm volatile ("" : : : "memory");
        }
    }
    
    va_end(ap);
    
    /* Irreducible control flow */
    if (sum > 0) {
        goto forward;
backward:
        sum -= g3;
        goto end;
forward:
        sum += g4;
        goto backward;
    }
    
end:
    return sum;
}

/* Nested calls creating complex save/restore sequences */
__attribute__((noinline, noclone))
int test4(int x) {
    volatile int vals[20];
    
    /* Initialize many volatile values */
    for (int i = 0; i < 20; i++) {
        vals[i] = x + i + g1;
    }
    
    /* Outer call */
    int r1 = opaque_call_2(x, g2);
    
    /* Inner call with different register usage */
    {
        register int t1 asm ("r10") = r1 + 1;
        register int t2 asm ("r11") = r1 + 2;
        
        /* Call that might be treated as sibling */
        opaque_call_2(t1, t2);
        
        /* Use values that need to survive across call */
        int sum = 0;
        for (int i = 0; i < 20; i++) {
            sum += vals[i] + t1 + t2;
        }
        
        /* Another call at block end */
        opaque_call_1();
        
        return sum + r1;
    }
}

/* Function using __builtin_apply */
__attribute__((noinline, noclone))
void test5(void *func, ...) {
    /* Build argument frame */
    __builtin_va_list args;
    __builtin_va_start(args, func);
    
    void *arg1 = __builtin_va_arg(args, void*);
    void *arg2 = __builtin_va_arg(args, void*);
    
    /* Use __builtin_apply to force unusual register usage */
    void *ret = __builtin_apply((void (*)(void))func, 
                               __builtin_apply_args(), 64);
    
    /* Force register pressure after builtin */
    volatile int v1 = (int)(long)arg1;
    volatile int v2 = (int)(long)arg2;
    
    asm volatile ("# after builtin_apply" : : : 
        "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
        "r8", "r9", "r10", "r11", "r12", "r13",
        "r14", "r15", "memory");
    
    __builtin_va_end(args);
}

/* Main driver with runtime selection */
int main(int argc, char **argv) {
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize function pointer */
    volatile_fp = (func_ptr_t)opaque_call_2;
    
    int result = 0;
    double dresult = 0.0;
    
    /* Execute tests based on mode, but eventually all */
    switch (mode) {
        case 0:
            result += test1(g1, g2);
            /* Fall through */
        case 1:
            dresult += test2(gd1, gd2);
            result += (int)dresult;
            /* Fall through */
        case 2:
            result += test3(5, g1, g2, g3, g4, g5);
            /* Fall through */
        case 3:
            result += test4(g1);
            /* Fall through */
        case 4:
            test5((void*)opaque_call_1, 
                  (void*)(long)g1, (void*)(long)g2);
            break;
    }
    
    /* Ensure all tests run eventually */
    if (mode != 0) result += test1(g2, g3);
    if (mode != 1) dresult += test2(gd2, gd3);
    if (mode != 2) result += test3(3, g3, g4, g5);
    if (mode != 3) result += test4(g2);
    if (mode != 4) test5((void*)opaque_call_1, 
                        (void*)(long)g3, (void*)(long)g4);
    
    /* Compute checksum to prevent elimination */
    int checksum = result + (int)dresult + g1 + g2 + g3 + g4 + g5;
    
    /* Use all volatile globals */
    gp1 = &checksum;
    gp2 = &dresult;
    
    /* Final opaque call */
    opaque_call_1();
    
    printf("Result: %d\n", checksum);
    return checksum & 255;
}

/* Dummy definitions to satisfy linker */
void opaque_call_1(void) {
    asm volatile ("# opaque call 1" : : : "memory");
}

int opaque_call_2(int a, int b) {
    asm volatile ("# opaque call 2" : : : "memory");
    return a + b + g1;
}

double opaque_call_3(double a, double b) {
    asm volatile ("# opaque call 3" : : : "memory");
    return a + b + gd1;
}
