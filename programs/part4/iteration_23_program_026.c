/* caller-save-test.c
 * Test program to trigger uncovered code in GCC's caller-save.cc
 * Specifically targeting lines 905-913 which handle instruction chain manipulation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* External functions to force call instructions */
extern void opaque_func1(void);
extern int opaque_func2(int, int);
extern double opaque_func3(double, double);

/* Global volatile variables to prevent optimization */
volatile int global_volatile_int = 12345;
volatile double global_volatile_double = 3.14159;
volatile void *global_volatile_ptr = NULL;

/* Function pointer with volatile to prevent devirtualization */
typedef int (*func_ptr_t)(int, ...);
volatile func_ptr_t volatile_func_ptr = NULL;

/* Complex structure to increase register pressure */
struct LargeStruct {
    long a, b, c, d, e, f;
    double x, y, z;
    void *ptr;
};

/* Force register usage with explicit register variables */
register int reg_var1 asm ("r10");
register int reg_var2 asm ("r11");
register int reg_var3 asm ("r12");

/* __attribute__((noinline, noclone)) to prevent optimization */
__attribute__((noinline, noclone))
void test1_many_args_and_calls(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Create many live variables across calls */
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile int v3 = e + f;
    volatile int v4 = g + h;
    
    /* Use explicit register variables */
    reg_var1 = a * b;
    reg_var2 = c * d;
    
    /* Inline asm with clobber to force caller-save */
    asm volatile (
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        : 
        : "r" (v1), "r" (v2)
        : "eax", "memory"
    );
    
    /* Function call that clobbers registers */
    opaque_func1();
    
    /* Use values after call - forces save/restore */
    int sum = v1 + v2 + v3 + v4 + reg_var1 + reg_var2;
    
    /* Another asm with different clobber */
    asm volatile (
        "movq %0, %%r10\n\t"
        "addq %1, %%r10\n\t"
        : 
        : "r" ((long)sum), "r" ((long)global_volatile_int)
        : "r10", "memory"
    );
    
    /* Complex control flow with goto to create block boundaries */
    if (sum > 1000) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Call within basic block that will be split */
    opaque_func2(a, b);
    /* This instruction might become BB_END */
    v1 = v1 * 2;
    goto end;
    
label2:
    /* Different call pattern */
    opaque_func2(c, d);
    v2 = v2 * 2;
    
end:
    /* Use all volatile variables to keep them live */
    global_volatile_int = v1 + v2;
}

__attribute__((noinline, noclone))
double test2_fp_and_mixed(int a, double b, float c, long d) {
    volatile double vd1 = b * 2.0;
    volatile double vd2 = c * 3.0;
    volatile long vl1 = d * a;
    
    /* Force FP register pressure */
    double arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = b * i + c;
    }
    
    /* Call that clobbers FP registers */
    double result = opaque_func3(b, c);
    
    /* Complex expression requiring temporaries */
    double sum = 0.0;
    for (int i = 0; i < 10; i++) {
        sum += arr[i] * result + vd1 - vd2;
    }
    
    /* Switch statement to create complex CFG */
    switch (a % 4) {
        case 0:
            opaque_func1();
            sum += 1.0;
            break;
        case 1:
            opaque_func2(a, (int)b);
            sum += 2.0;
            break;
        case 2:
            /* Fall through to create merge point */
        case 3:
            opaque_func1();
            /* This might be where BB_END needs updating */
            sum *= 2.0;
            if (vl1 > 0) {
                opaque_func2((int)sum, (int)vd1);
            }
            break;
        default:
            /* Never reached but creates default label */
            sum = 0.0;
    }
    
    return sum + vd1 + vd2 + vl1;
}

__attribute__((noinline, noclone))
void test3_irreducible_cfg(void) {
    volatile int counter = 0;
    volatile int a = 1, b = 2, c = 3, d = 4;
    
    /* Create irreducible control flow with goto */
start:
    counter++;
    
    if (counter % 2 == 0) {
        goto even;
    } else {
        goto odd;
    }
    
even:
    /* Function call at block boundary */
    opaque_func2(a, b);
    a = a * 2;
    
    if (counter < 10) {
        goto start;
    } else {
        goto done;
    }
    
odd:
    /* Different function call */
    opaque_func2(c, d);
    c = c * 3;
    
    if (counter < 10) {
        goto start;
    } else {
        goto done;
    }
    
done:
    /* Use inline asm to force specific register usage */
    asm volatile (
        "movl %0, %%ebx\n\t"
        "movl %1, %%ecx\n\t"
        "addl %%ebx, %%ecx\n\t"
        : 
        : "r" (a), "r" (c)
        : "ebx", "ecx", "memory"
    );
    
    global_volatile_int = a + c;
}

__attribute__((noinline, noclone))
void test4_nested_calls_and_va_args(int n, ...) {
    va_list args;
    va_start(args, n);
    
    /* Force many values into registers */
    int v1 = va_arg(args, int);
    double v2 = va_arg(args, double);
    long v3 = va_arg(args, long);
    int v4 = va_arg(args, int);
    
    va_end(args);
    
    volatile int save1 = v1;
    volatile double save2 = v2;
    volatile long save3 = v3;
    
    /* Nested call scenario */
    void inner_call(int x, double y) {
        /* Local function to create nested call context */
        volatile int inner = x + (int)y;
        opaque_func2(inner, global_volatile_int);
        
        /* Loop with break to create block boundaries */
        for (int i = 0; i < 5; i++) {
            if (i == 3) {
                opaque_func1();  /* Call at potential block end */
                break;
            }
            inner += i;
        }
        
        global_volatile_int = inner;
    }
    
    /* Call the inner function */
    inner_call(v1, v2);
    
    /* Use __builtin_apply to create unusual call sequence */
    typedef void (*simple_func)(int);
    simple_func f = (simple_func)opaque_func1;
    
    /* This creates complex register allocation around calls */
    __builtin_apply((void (*)(void))f, __builtin_apply_args(), 64);
    
    /* Use saved values after calls */
    int result = save1 + (int)save2 + save3 + v4;
    
    /* Another complex control flow */
    switch (result % 5) {
        case 0: case 4:
            opaque_func2(result, save1);
            /* Potential BB_END update point */
            result *= 2;
            break;
        case 1: case 3:
            opaque_func1();
            result /= 2;
            break;
        case 2:
            /* Empty case to create fallthrough */
        default:
            result = 0;
    }
    
    global_volatile_int = result;
}

__attribute__((noinline, noclone))
void test5_vector_types_and_asm(void) {
    /* Use vector types to increase register pressure */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    volatile v4si vsave1 = v1;
    volatile v4si vsave2 = v2;
    
    /* Complex asm with many clobbers */
    asm volatile (
        "movdqa %0, %%xmm0\n\t"
        "movdqa %1, %%xmm1\n\t"
        "paddd %%xmm1, %%xmm0\n\t"
        "movdqa %%xmm0, %0\n\t"
        : "+x" (v1)
        : "x" (v2)
        : "xmm0", "xmm1", "memory"
    );
    
    /* Function call that might clobber vector regs */
    opaque_func1();
    
    /* Use saved values */
    v4si v3 = vsave1 + vsave2;
    
    /* Loop with function call in the middle */
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        if (i == 4) {
            opaque_func2(i, sum);  /* Call at loop midpoint */
            /* This creates a block split */
        }
        sum += v1[i % 4] + v3[i % 4];
        
        /* Conditional goto to create irreducible flow */
        if (i == 6) {
            goto special;
        }
    }
    
    goto end;
    
special:
    /* Another call at block boundary */
    opaque_func2(sum, global_volatile_int);
    sum *= 3;
    
end:
    global_volatile_int = sum;
}

/* Helper to create register pressure */
__attribute__((noinline, noclone))
int pressure_helper(int a, int b, int c, int d, int e, int f, int g, int h) {
    /* Use all arguments in complex ways */
    volatile int v1 = a + b + c;
    volatile int v2 = d + e + f;
    volatile int v3 = g + h;
    
    /* Multiple calls */
    opaque_func2(v1, v2);
    
    int result = v1 * v2 + v3;
    
    opaque_func1();
    
    /* More complex computation */
    result = result * 2 - v1 + v2 - v3;
    
    /* Conditional with call */
    if (result > 0) {
        opaque_func2(result, global_volatile_int);
    } else {
        opaque_func1();
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Initialize volatile function pointer */
    volatile_func_ptr = (func_ptr_t)opaque_func2;
    
    /* Use command line argument to vary execution path */
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize explicit register variables */
    reg_var1 = 100;
    reg_var2 = 200;
    reg_var3 = 300;
    
    /* Array to store checksum */
    volatile int checksum = 0;
    
    /* Execute all tests in different orders based on mode */
    for (int cycle = 0; cycle < 3; cycle++) {
        switch ((test_mode + cycle) % 5) {
            case 0:
                test1_many_args_and_calls(1, 2, 3, 4, 5, 6, 7, 8);
                checksum += global_volatile_int;
                break;
            case 1:
                {
                    double r = test2_fp_and_mixed(5, 3.14, 2.71f, 1000L);
                    checksum += (int)r;
                }
                break;
            case 2:
                test3_irreducible_cfg();
                checksum += global_volatile_int;
                break;
            case 3:
                test4_nested_calls_and_va_args(4, 10, 20.5, 30L, 40);
                checksum += global_volatile_int;
                break;
            case 4:
                test5_vector_types_and_asm();
                checksum += global_volatile_int;
                break;
        }
        
        /* Create register pressure between tests */
        int pressure = pressure_helper(
            cycle, cycle*2, cycle*3, cycle*4,
            cycle*5, cycle*6, cycle*7, cycle*8
        );
        checksum += pressure;
    }
    
    /* Use scanf to prevent optimization */
    int user_input = 0;
    if (argc > 2) {
        user_input = atoi(argv[2]);
    } else {
        printf("Checksum: %d\n", checksum);
        printf("Enter a number: ");
        scanf("%d", &user_input);
    }
    
    /* Final complex call sequence */
    for (int i = 0; i < user_input % 5; i++) {
        opaque_func1();
        
        /* Inline asm barrier */
        asm volatile ("" : : : "memory");
        
        /* Call through volatile pointer */
        if (volatile_func_ptr) {
            int r = volatile_func_ptr(i, checksum);
            checksum += r;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum % 256;
}

/* Dummy definitions to satisfy linker (in real test, these would be in a library) */
void opaque_func1(void) {
    /* Empty but marked noinline */
    asm volatile ("nop" : : : "memory");
}

int opaque_func2(int a, int b) {
    return a + b + global_volatile_int;
}

double opaque_func3(double a, double b) {
    return a * b + global_volatile_double;
}
