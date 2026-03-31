/* test-caller-save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void) __attribute__((noinline, noclone));
extern int opaque_func2(int) __attribute__((noinline, noclone));
extern double opaque_func3(double) __attribute__((noinline, noclone));
extern void* opaque_func4(void*) __attribute__((noinline, noclone));

/* Volatile globals to maintain live ranges across calls */
volatile int gv1 = 1, gv2 = 2, gv3 = 3, gv4 = 4;
volatile double gd1 = 1.0, gd2 = 2.0;
volatile void* gp1, *gp2;

/* Function pointer to create indirect calls */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fp = NULL;

/* Complex control flow with register pressure around calls */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Use explicit register variables to create conflicts */
    register int r1 asm ("r10") = gv1;
    register int r2 asm ("r11") = gv2;
    register int r3 asm ("r12") = gv3;
    volatile int stack_save[8];
    
    /* Save live values to stack */
    for (int i = 0; i < 8; i++) {
        stack_save[i] = gv1 + i;
    }
    
    /* Inline asm that clobbers call-clobbered registers */
    asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3) : "r10", "r11", "r12", "memory");
    
    /* Complex control flow with goto to split basic blocks */
    if (mode & 1) {
        opaque_func1();
        /* Use saved values after call */
        for (int i = 0; i < 8; i++) {
            r1 += stack_save[i];
        }
        goto label1;
    } else {
        r2 = opaque_func2(r2);
        goto label2;
    }
    
label1:
    /* Another call with different register usage */
    asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
    r3 = opaque_func2(r3);
    
    /* Force register spill by using all variables */
    gv1 = r1 + r2 + r3;
    
label2:
    /* Compiler barrier */
    asm volatile ("" : : : "memory");
}

/* Function with floating point and mixed register pressure */
__attribute__((noinline, noclone))
void test2(double arg) {
    volatile double local_save[4];
    register double d1 asm ("xmm0") = arg;
    register double d2 asm ("xmm1") = gd1;
    register int i1 asm ("eax") = gv4;
    
    /* Save across call */
    local_save[0] = d1;
    local_save[1] = d2;
    
    /* Switch with default case that calls function */
    switch (i1 & 3) {
        case 0:
            d1 = opaque_func3(d1);
            break;
        case 1:
            d2 = opaque_func3(d2);
            break;
        default:
            /* This creates basic block boundary manipulation */
            opaque_func1();
            /* Use saved values */
            d1 = local_save[0] + local_save[1];
            break;
    }
    
    /* Loop with break that contains function call */
    for (int i = 0; i < 10; i++) {
        if (i > i1) {
            opaque_func2(i);
            break;
        }
        d2 += 1.0;
    }
    
    gd2 = d1 + d2;
    asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "memory");
}

/* Function with pointer manipulation and __builtin_apply */
__attribute__((noinline, noclone))
void test3(void* ptr) {
    volatile int buffer[16];
    register void* rptr asm ("r15") = ptr;
    
    /* Fill buffer with values that need to be preserved */
    for (int i = 0; i < 16; i++) {
        buffer[i] = gv1 + i;
    }
    
    /* Nested function calls */
    void (*inner)(void) = (void(*)())opaque_func4;
    
    /* Irreducible control flow with labels */
    if (gp1 != NULL) {
        goto middle;
    }
    
    /* Call via function pointer */
    if (volatile_fp) {
        volatile_fp();
    }
    
    /* Use __builtin_apply to create unusual call sequence */
    {
        void* args = __builtin_apply_args();
        /* This creates complex prologue/epilogue */
        void* result = __builtin_apply((void(*)())opaque_func4, args, 64);
        __builtin_return(result);
    }
    
middle:
    /* Restore and use saved values */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += buffer[i];
    }
    
    /* Inline asm acting as pseudo-call */
    asm volatile ("call *%0" : : "r"(inner) : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
    
    gp2 = rptr;
}

/* Function with many live variables across call */
__attribute__((noinline, noclone))
int test4(int a, int b, int c, int d, int e, int f) {
    /* Many live variables to exceed call-saved registers */
    int v1 = a + gv1;
    int v2 = b + gv2;
    int v3 = c + gv3;
    int v4 = d + gv4;
    int v5 = e * 2;
    int v6 = f * 3;
    volatile int save[6];
    
    /* Save all live variables */
    save[0] = v1; save[1] = v2; save[2] = v3;
    save[3] = v4; save[4] = v5; save[5] = v6;
    
    /* Function call that clobbers registers */
    int result = opaque_func2(a);
    
    /* Complex expression using all saved values */
    result = ((save[0] + save[1]) * save[2]) / (save[3] - save[4] + save[5]);
    
    /* Loop with continue that contains call */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            opaque_func2(i);
            continue;
        }
        result += i;
    }
    
    return result;
}

/* Helper with nested call */
__attribute__((noinline, noclone))
void nested_call_helper(int depth) {
    volatile int local = depth;
    
    if (depth > 0) {
        /* Recursive-like but not actually recursive to avoid tail calls */
        opaque_func2(depth);
        nested_call_helper(depth - 1);
        
        /* Use local after nested call */
        gv1 += local;
    }
    
    /* Inline asm barrier */
    asm volatile ("" : : : "memory");
}

/* Main test driver */
int main(int argc, char** argv) {
    int mode = 0;
    
    /* Use argv to create runtime-dependent control flow */
    if (argc > 1) {
        mode = atoi(argv[1]) & 7;
    }
    
    /* Initialize function pointer */
    volatile_fp = opaque_func1;
    
    /* Run all tests with different modes */
    test1(mode);
    test2(gd1 + mode);
    test3(&gv1);
    
    /* Create register pressure */
    int r1 = test4(gv1, gv2, gv3, gv4, mode, argc);
    
    /* Nested call scenario */
    for (int i = 0; i < 3; i++) {
        nested_call_helper(i + 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = gv1 + gv2 + gv3 + gv4 + (int)gd1 + (int)gd2;
    checksum += r1;
    
    printf("Checksum: %d\n", checksum);
    
    return checksum & 255;
}

/* Dummy definitions to satisfy linker (normally would be in separate file) */
void opaque_func1(void) {
    asm volatile ("" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("" : "+r"(x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    asm volatile ("" : "+x"(x) : : "memory");
    return x * 2.0;
}

void* opaque_func4(void* x) {
    asm volatile ("" : "+r"(x) : : "memory");
    return x;
}
