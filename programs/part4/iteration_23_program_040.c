/* test-caller-save.c - Complex program to trigger uncovered lines in GCC's caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External opaque functions to prevent inlining/optimization */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void* opaque_func4(void*);

/* Volatile globals to maintain live ranges across calls */
volatile int global_volatile_int = 0;
volatile double global_volatile_double = 0.0;
volatile void* global_volatile_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[4];

/* Complex structure to force register pressure */
struct LargeStruct {
    int a, b, c, d, e, f;
    double x, y, z;
    void* p1, *p2;
};

/* Force register usage with explicit register variables */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register double reg_var3 asm ("xmm14");
register void* reg_var4 asm ("r15");

/* Test function 1: Many integer arguments forcing caller-saves */
__attribute__((noinline, noclone))
int test1(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    volatile int locals[10];
    int result = 0;
    
    /* Save all arguments to volatile array */
    locals[0] = a; locals[1] = b; locals[2] = c;
    locals[3] = d; locals[4] = e; locals[5] = f;
    locals[6] = g; locals[7] = h; locals[8] = i;
    locals[9] = j;
    
    /* Complex control flow with goto to create block boundaries */
    if (a > b) {
        label1:
        /* Function call that clobbers registers */
        opaque_func1();
        /* Compiler barrier */
        asm volatile ("" : : : "memory");
        
        /* Use saved values requiring original registers */
        result += locals[0] * locals[1];
        if (c < d) goto label2;
    } else {
        /* Another call site */
        int temp = opaque_func2(a + b);
        result += temp;
        goto label1;
    }
    
    label2:
    /* More register pressure */
    result += locals[2] - locals[3] + locals[4] * locals[5];
    
    /* Switch with default that calls function */
    switch (result % 4) {
        case 0: result += 1; break;
        case 1: result += 2; break;
        case 2: result += 3; break;
        default: 
            opaque_func1();
            result += 4;
            break;
    }
    
    return result;
}

/* Test function 2: Floating point and mixed arguments */
__attribute__((noinline, noclone))
double test2(double a, double b, int c, double d, int e, double f) {
    volatile double fp_locals[6];
    volatile int int_locals[6];
    double result = 0.0;
    
    /* Save arguments */
    fp_locals[0] = a; fp_locals[1] = b; fp_locals[2] = d; fp_locals[3] = f;
    int_locals[0] = c; int_locals[1] = e;
    
    /* Loop with break inside conditional containing call */
    for (int i = 0; i < 10; i++) {
        if (i > c) {
            /* Inline asm that looks like a call */
            asm volatile (
                "movq $0x12345678, %%rax\n\t"
                "call *%%rax\n\t"
                : 
                : 
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "xmm0", "xmm1",
                  "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13",
                  "xmm14", "xmm15", "memory"
            );
            
            result += fp_locals[i % 4];
            if (result > 100.0) break;
        } else {
            result += opaque_func3(b);
        }
    }
    
    /* Use __builtin_apply to create unusual call sequence */
    {
        double (*func)(double) = opaque_func3;
        double args[1] = {result};
        void* ptr = __builtin_apply((void (*)())func, args, sizeof(double));
        if (ptr) {
            result = *(double*)ptr;
            __builtin_return(ptr);
        }
    }
    
    return result;
}

/* Test function 3: Vector-like operations and many live variables */
__attribute__((noinline, noclone))
void* test3(void* p1, void* p2, void* p3, void* p4, void* p5) {
    volatile void* saved_ptrs[10];
    volatile int counters[10];
    void* result = NULL;
    
    /* Save all pointers */
    saved_ptrs[0] = p1; saved_ptrs[1] = p2; saved_ptrs[2] = p3;
    saved_ptrs[3] = p4; saved_ptrs[4] = p5;
    
    /* Irreducible control flow with labels */
    int mode = (int)(long)p1 % 3;
    
    if (mode == 0) {
        block_a:
        /* Call via function pointer */
        func_table[0]();
        counters[0]++;
        if (counters[0] < 3) goto block_c;
    } else if (mode == 1) {
        block_b:
        opaque_func4(p2);
        counters[1]++;
        goto block_d;
    } else {
        block_c:
        func_table[1]();
        counters[2]++;
        if (counters[2] % 2) goto block_a;
        else goto block_b;
    }
    
    block_d:
    /* Use saved pointers in complex expression */
    result = (void*)((long)saved_ptrs[0] ^ (long)saved_ptrs[1] ^ 
                     (long)saved_ptrs[2] ^ (long)saved_ptrs[3]);
    
    /* Force register pressure with explicit register vars */
    reg_var1 = (int)(long)result;
    reg_var4 = result;
    
    /* Another call that might need save/restore */
    asm volatile (
        "pushq %%r12\n\t"
        "pushq %%r13\n\t"
        "pushq %%r15\n\t"
        "movq $0x1, %%rax\n\t"
        "syscall\n\t"
        "popq %%r15\n\t"
        "popq %%r13\n\t"
        "popq %%r12\n\t"
        : 
        : 
        : "rax", "rcx", "r11", "memory"
    );
    
    return (void*)(reg_var1 + (long)reg_var4);
}

/* Test function 4: Nested calls creating complex save/restore sequences */
__attribute__((noinline, noclone))
int test4(int depth) {
    volatile int stack[20];
    int i;
    
    if (depth <= 0) {
        return opaque_func2(42);
    }
    
    /* Save lots of values to force spills */
    for (i = 0; i < 15; i++) {
        stack[i] = depth * 100 + i;
    }
    
    /* Recursive call */
    int child_result = test4(depth - 1);
    
    /* Use saved values after call */
    int sum = 0;
    for (i = 0; i < 15; i++) {
        sum += stack[i] * child_result;
    }
    
    /* Another call */
    child_result += opaque_func2(sum % 100);
    
    /* Complex expression requiring many temporaries */
    return ((sum ^ child_result) | (depth << 16)) & 0x7FFFFFFF;
}

/* Helper with nested call to force instruction insertion at block boundaries */
__attribute__((noinline, noclone))
void helper_with_nested_call(int x) {
    volatile int saved = x;
    
    if (x > 0) {
        /* First call */
        int r1 = opaque_func2(x);
        
        /* Complex basic block that might get split */
        int temp = r1 * 2 + saved;
        
        /* Nested call in conditional */
        if (temp > 100) {
            double r2 = opaque_func3(temp);
            saved = (int)r2;
            
            /* goto to create merge point */
            if (saved < 50) goto merge_point;
        }
        
        /* Another call */
        opaque_func1();
        
        merge_point:
        /* Use value saved before first call */
        global_volatile_int += saved;
    }
}

/* Main function that orchestrates all tests */
int main(int argc, char** argv) {
    int test_mode = 0;
    int checksum = 0;
    
    /* Initialize function pointers */
    func_table[0] = opaque_func1;
    func_table[1] = (func_ptr_t)opaque_func2;
    func_table[2] = (func_ptr_t)opaque_func3;
    func_table[3] = (func_ptr_t)opaque_func4;
    
    /* Use argv to create runtime variability */
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize explicit register variables */
    reg_var1 = 0x12345678;
    reg_var2 = 0x9ABCDEF0;
    reg_var3 = 3.141592653589793;
    reg_var4 = &global_volatile_int;
    
    /* Execute all test functions in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 4) {
            case 0:
                checksum += test1(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
                checksum += test1(10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
                break;
            case 1:
                global_volatile_double += test2(1.1, 2.2, 3, 4.4, 5, 6.6);
                checksum += (int)global_volatile_double;
                break;
            case 2:
                global_volatile_ptr = test3(&checksum, &global_volatile_int,
                                          &global_volatile_double, main, argv);
                checksum += (int)(long)global_volatile_ptr;
                break;
            case 3:
                checksum += test4(3);
                helper_with_nested_call(checksum % 100);
                break;
        }
        
        /* Force register pressure between iterations */
        asm volatile ("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
            "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
            "memory"
        );
    }
    
    /* Use all global volatiles to prevent DCE */
    checksum += global_volatile_int;
    checksum += (int)global_volatile_double;
    checksum += (int)(long)global_volatile_ptr;
    
    /* Use register variables */
    checksum ^= reg_var1;
    checksum ^= reg_var2;
    checksum ^= (int)reg_var3;
    checksum ^= (int)(long)reg_var4;
    
    printf("Final checksum: %d\n", checksum);
    return checksum & 0xFF;
}

/* Dummy definitions to satisfy linker (in real test, these would be in separate file) */
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
    return (void*)((long)x + 1);
}
