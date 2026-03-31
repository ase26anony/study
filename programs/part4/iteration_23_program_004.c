/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Specifically targeting the instruction chain manipulation code
 * around lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to create call sites */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double);

/* Global volatile state to prevent optimization */
volatile int global_counter = 0;
volatile int global_checksum = 0;
volatile void *global_ptr = NULL;

/* Function pointer array for indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Complex structure to force register pressure */
struct RegPressure {
    int a, b, c, d, e, f, g, h;
    double x, y, z;
    void *p;
};

/* Force non-inline, no clone to ensure separate prologue/epilogue */
__attribute__((noinline, noclone, optimize("O0")))
void test1(int arg1, int arg2, int arg3, int arg4, 
           int arg5, int arg6, int arg7, int arg8) {
    /* Many live variables across calls */
    volatile int v1 = arg1;
    volatile int v2 = arg2;
    volatile int v3 = arg3;
    volatile int v4 = arg4;
    volatile int v5 = arg5;
    volatile int v6 = arg6;
    volatile int v7 = arg7;
    volatile int v8 = arg8;
    
    /* Use explicit register variables to create conflicts */
    register int r12_val asm ("r12") = v1 + v2;
    register int r13_val asm ("r13") = v3 + v4;
    
    /* Complex control flow with basic block boundaries */
    if (v1 > 0) {
        /* First call site with many live registers */
        asm volatile ("" : : : "memory");  /* Compiler barrier */
        
        /* Inline asm that clobbers call-clobbered registers */
        asm volatile ("# Clobber eax, r10" : : : "eax", "r10", "memory");
        
        /* Function call with side effects */
        opaque_call_1();
        
        /* Use register variables after call - forces save/restore */
        v1 = r12_val + r13_val;
        
        /* Another asm barrier */
        asm volatile ("" : : : "memory");
        
        /* Nested condition to create basic block split */
        if (v2 < 100) {
            /* Second call in nested block */
            opaque_call_2(v1);
            
            /* More register pressure */
            register int r14_val asm ("r14") = v5 + v6;
            v2 = r14_val * 2;
            
            /* goto to create irreducible flow */
            if (v3 == 0)
                goto merge_point;
        }
        
        /* Loop with break to create block boundaries */
        for (int i = 0; i < v4; i++) {
            if (i == v5) {
                /* Call inside loop with break */
                opaque_call_3(i, v6);
                break;
            }
            v7++;
        }
        
        merge_point:
        /* Use all volatile variables to keep them live */
        v8 = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    }
    
    /* Store to global to prevent DCE */
    global_counter += v8;
}

__attribute__((noinline, noclone, optimize("O0")))
void test2(double d1, double d2, double d3) {
    /* Mix of floating point and integer */
    volatile double dv1 = d1;
    volatile double dv2 = d2;
    volatile double dv3 = d3;
    volatile int iv1 = (int)d1;
    volatile int iv2 = (int)d2;
    
    /* Switch statement to create complex CFG */
    switch (iv1 % 4) {
        case 0:
            opaque_call_4(dv1);
            /* fall through */
        case 1:
            /* Call via function pointer */
            if (func_table[0])
                func_table[0]();
            break;
        case 2:
            /* Default case with call */
            opaque_call_1();
            /* goto to create back edge */
            if (iv2 > 0)
                goto switch_end;
            break;
        default:
            /* Complex expression across call */
            dv2 = opaque_call_4(dv3) * 2.0;
            break;
    }
    
    switch_end:
    /* Force register pressure with inline asm */
    asm volatile (
        "# Complex asm with multiple clobbers" 
        : "=r"(iv1), "=r"(iv2)
        : "0"(iv1), "1"(iv2)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", 
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "memory"
    );
    
    global_counter += iv1 + iv2;
}

__attribute__((noinline, noclone, optimize("O0")))
void test3(struct RegPressure *rp) {
    /* Access structure elements to force multiple registers */
    volatile int sum = 0;
    
    /* Unpredictable control flow */
    for (int i = 0; i < 10; i++) {
        if (i & 1) {
            /* Call in one branch */
            opaque_call_2(rp->a + i);
            sum += rp->b;
        } else {
            /* Different call in other branch */
            opaque_call_3(rp->c, rp->d);
            sum += rp->e;
            
            /* Nested condition with goto */
            if (sum > 1000) {
                goto loop_exit;
            }
        }
        
        /* Modify structure between calls */
        rp->f = sum;
        
        /* Inline asm acting as pseudo-call */
        asm volatile (
            "mov %0, %%rax\n"
            "add %1, %%rax\n"
            : 
            : "r"(rp->g), "r"(rp->h)
            : "rax", "memory"
        );
        
        /* Continue creates basic block boundary */
        if (i == 5)
            continue;
            
        rp->h += 1;
    }
    
    loop_exit:
    /* Use __builtin_apply to create unusual register usage */
    void *args = __builtin_apply_args();
    if (args) {
        /* This creates complex prologue/epilogue */
        void *result = __builtin_apply((void (*)())opaque_call_1, args, 64);
        if (result)
            __builtin_return(result);
    }
    
    global_counter += sum;
}

__attribute__((noinline, noclone, optimize("O0")))
int test4(va_list ap) {
    /* Variadic function to stress register allocation */
    int v1 = va_arg(ap, int);
    double v2 = va_arg(ap, double);
    void *v3 = va_arg(ap, void *);
    
    volatile int preserved = v1;
    volatile double preserved_fp = v2;
    
    /* Multiple calls with live variadic arguments */
    for (int i = 0; i < 3; i++) {
        /* Save to stack-like array */
        int saved_int[10];
        double saved_fp[10];
        saved_int[0] = preserved;
        saved_fp[0] = preserved_fp;
        
        /* Call that clobbers registers */
        opaque_call_1();
        
        /* Restore and use - forces save/restore code */
        preserved = saved_int[0] + i;
        preserved_fp = saved_fp[0] * 1.5;
        
        /* Conditional goto to split blocks */
        if (i == 1) {
            opaque_call_2(preserved);
            goto middle;
        }
        
        middle:
        /* Use asm barrier */
        asm volatile ("" : : : "memory");
    }
    
    /* Compute checksum */
    int result = preserved + (int)preserved_fp;
    
    /* Force tail call scenario */
    if (result > 0) {
        opaque_call_3(result, preserved);
    }
    
    return result;
}

__attribute__((noinline, noclone, optimize("O0")))
void nested_calls(int depth) {
    /* Recursive-like nested calls to force prologue/epilogue chains */
    volatile int local = depth * 2;
    
    if (depth > 0) {
        /* Save local to force register save */
        volatile int saved = local;
        
        /* Call with register pressure */
        opaque_call_2(local);
        
        /* Nested call */
        nested_calls(depth - 1);
        
        /* Restore and use saved value */
        local = saved + 1;
        
        /* Another call after nested call */
        opaque_call_1();
    }
    
    /* Use inline asm with specific register clobbers */
    asm volatile (
        "# Depth %0" 
        : 
        : "r"(local)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", "memory"
    );
    
    global_counter += local;
}

/* Opaque function definitions to force external calls */
void opaque_call_1(void) {
    /* Empty but volatile to prevent optimization */
    asm volatile ("" : : : "memory");
    global_checksum ^= 0x1234;
}

void opaque_call_2(int x) {
    asm volatile ("" : : "r"(x) : "memory");
    global_checksum ^= x;
}

int opaque_call_3(int x, int y) {
    int result;
    asm volatile ("add %1, %0" : "=r"(result) : "r"(x), "0"(y) : "cc");
    global_checksum ^= result;
    return result;
}

double opaque_call_4(double x) {
    double result;
    asm volatile ("addsd %1, %0" : "=x"(result) : "x"(x), "0"(x));
    global_checksum ^= (int)result;
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize function table */
    for (int i = 0; i < 10; i++) {
        func_table[i] = opaque_call_1;
    }
    
    /* Parse test mode from argv if provided */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Force runtime-dependent values */
    volatile int runtime_val = argc * 100;
    
    /* Test 1: Many integer arguments forcing register pressure */
    test1(runtime_val + 1, runtime_val + 2, runtime_val + 3, runtime_val + 4,
          runtime_val + 5, runtime_val + 6, runtime_val + 7, runtime_val + 8);
    
    /* Test 2: Floating point mix */
    test2(1.234, 5.678, 9.012);
    
    /* Test 3: Structure with many elements */
    struct RegPressure rp = {0};
    rp.a = runtime_val; rp.b = runtime_val + 1;
    rp.c = runtime_val + 2; rp.d = runtime_val + 3;
    rp.e = runtime_val + 4; rp.f = runtime_val + 5;
    rp.g = runtime_val + 6; rp.h = runtime_val + 7;
    rp.x = 1.1; rp.y = 2.2; rp.z = 3.3;
    rp.p = &runtime_val;
    
    test3(&rp);
    
    /* Test 4: Variadic function */
    va_list ap;
    /* Simulate va_start */
    int var1 = runtime_val + 10;
    double var2 = 3.14159;
    void *var3 = &global_counter;
    
    /* Manually setup variadic args */
    test4(ap);  /* Note: This is simplified - real va_list would be needed */
    
    /* Test 5: Nested calls */
    nested_calls(3);
    
    /* Additional stress based on test mode */
    switch (test_mode) {
        case 0:
            /* Many small functions called in sequence */
            for (int i = 0; i < 100; i++) {
                opaque_call_1();
                opaque_call_2(i);
                if (i % 10 == 0) {
                    opaque_call_3(i, i*2);
                }
            }
            break;
        case 1:
            /* Deep call chain */
            nested_calls(5);
            break;
        case 2:
            /* Mixed calls with gotos */
            {
                int x = 0;
                loop_with_goto:
                opaque_call_2(x);
                x++;
                if (x < 10)
                    goto loop_with_goto;
            }
            break;
        case 3:
            /* Function pointer calls with different targets */
            for (int i = 0; i < 20; i++) {
                func_table[i % 3]();
            }
            break;
        case 4:
            /* Complex expression across multiple calls */
            {
                register int r1 asm ("r12") = 100;
                register int r2 asm ("r13") = 200;
                opaque_call_1();
                int result = r1 + r2;  /* Forces save/restore */
                opaque_call_2(result);
                global_counter += result;
            }
            break;
    }
    
    /* Final checksum to prevent optimization */
    printf("Final checksum: %d\n", global_counter + global_checksum);
    
    return global_counter > 0 ? 0 : 1;
}
