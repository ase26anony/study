/* caller-save-test.c
 * Test program to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mno-accumulate-outgoing-args -fno-jump-tables caller-save-test.c -o caller-save-test -ldl
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

/* External opaque functions to force call instructions */
extern void opaque_call_1(void);
extern void opaque_call_2(int);
extern int opaque_call_3(int, int);
extern double opaque_call_4(double, double);

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile double global_accumulator = 0.0;
volatile void *global_ptr = NULL;

/* Function pointer array to create indirect calls */
typedef void (*func_ptr_t)(void);
static func_ptr_t func_table[10];

/* Complex structure to increase register pressure */
struct RegPressure {
    long a, b, c, d, e, f, g, h;
    double x, y, z;
    void *p1, *p2;
};

/* Force register usage with explicit register variables */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register int reg_var3 asm ("r14");
register double reg_fp1 asm ("xmm8");
register double reg_fp2 asm ("xmm9");

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1(int mode) {
    volatile int v1 = mode * 2;
    volatile int v2 = mode * 3;
    volatile int v3 = mode * 5;
    volatile double f1 = mode * 1.5;
    volatile double f2 = mode * 2.5;
    
    /* Use explicit register variables */
    reg_var1 = v1 + v2;
    reg_var2 = v3 * 2;
    reg_fp1 = f1 + f2;
    
    /* Create many live values */
    int live1 = reg_var1;
    int live2 = reg_var2;
    int live3 = v1 + v2 + v3;
    double live4 = reg_fp1;
    double live5 = f1 * f2;
    
    /* Memory barrier to prevent reordering */
    asm volatile ("" : : : "memory");
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        opaque_call_1();
    } else {
        opaque_call_2(live1);
    }
    
    /* Use all live values after call - forces save/restore */
    int result = live1 + live2 + live3;
    double fresult = live4 + live5;
    
    /* Complex expression requiring temporary registers */
    result += (int)(fresult * 100.0);
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Store to volatile global */
    global_counter += result;
}

/* Test function 2: Nested calls with irreducible control flow */
__attribute__((noinline, noclone))
void test2(int iterations) {
    volatile int array[20];
    for (int i = 0; i < 20; i++) {
        array[i] = i * iterations;
    }
    
    int sum = 0;
    int *ptr = array;
    
    /* Irreducible control flow with goto */
    if (iterations < 0) goto negative_case;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values */
        int a = ptr[0];
        int b = ptr[1];
        int c = ptr[2];
        int d = ptr[3];
        
        /* Inline asm that acts like a call */
        asm volatile (
            "movl %0, %%eax\n\t"
            "movl %1, %%ebx\n\t"
            "addl %%ebx, %%eax\n\t"
            : : "r"(a), "r"(b) : "eax", "ebx", "memory"
        );
        
        /* Function call in loop */
        if (i % 3 == 0) {
            opaque_call_3(a, b);
        } else if (i % 3 == 1) {
            /* Indirect call */
            if (func_table[i % 10]) {
                func_table[i % 10]();
            }
        }
        
        /* Use values after call */
        sum += a + b + c + d;
        ptr += 4;
        
        if (sum > 1000) {
            /* Break creates basic block boundary */
            break;
        } else {
            continue;
        }
        
        /* Unreachable but creates CFG complexity */
        negative_case:
        sum = -sum;
    }
    
    global_counter += sum;
}

/* Test function 3: Switch statement with calls at block boundaries */
__attribute__((noinline, noclone))
void test3(int code) {
    volatile double values[8];
    for (int i = 0; i < 8; i++) {
        values[i] = (double)(code + i) / 2.0;
    }
    
    /* Use floating point register variables */
    reg_fp1 = values[0];
    reg_fp2 = values[1];
    
    double result = 0.0;
    
    switch (code % 5) {
        case 0: {
            double temp1 = reg_fp1;
            double temp2 = reg_fp2;
            opaque_call_4(temp1, temp2);
            result = temp1 + temp2;
            /* Fall through to create merge point */
        }
        case 1: {
            double temp3 = values[2] * values[3];
            /* Call that might split basic block */
            if (code & 1) {
                opaque_call_1();
            }
            result += temp3;
            break;
        }
        case 2: {
            for (int i = 0; i < 3; i++) {
                result += values[i];
                if (i == 1) {
                    opaque_call_2((int)result);
                    /* Continue creates block boundary */
                    continue;
                }
            }
            break;
        }
        case 3: {
            result = values[4] - values[5];
            goto compute;
        }
        default: {
            /* Default case with call */
            opaque_call_3((int)values[6], (int)values[7]);
            result = values[6] * values[7];
            compute:
            result = result * 2.0;
            break;
        }
    }
    
    global_accumulator += result;
}

/* Test function 4: __builtin_apply usage */
__attribute__((noinline, noclone))
void test4(void) {
    /* Create a stack frame with many locals */
    volatile long locals[16];
    for (int i = 0; i < 16; i++) {
        locals[i] = i * 1000L;
    }
    
    /* Use __builtin_apply to create unusual register pressure */
    void *args = __builtin_apply_args();
    
    /* Save many values before potential call */
    long saved[8];
    for (int i = 0; i < 8; i++) {
        saved[i] = locals[i] + locals[i+8];
    }
    
    /* Complex expression with function call in middle */
    long total = 0;
    for (int i = 0; i < 8; i++) {
        total += saved[i];
        if (i == 4) {
            /* Call at loop midpoint */
            opaque_call_1();
            /* Inline asm with clobber */
            asm volatile (
                "movq %0, %%r10\n\t"
                "addq $1, %%r10\n\t"
                : : "r"(total) : "r10", "memory"
            );
        }
    }
    
    /* Use __builtin_apply to call a function */
    if (args) {
        /* This creates complex prologue/epilogue */
        void *ret = __builtin_apply((void (*)(void))opaque_call_1, args, 64);
        if (ret) {
            __builtin_return(ret);
        }
    }
    
    global_counter += (int)total;
}

/* Helper with nested call */
__attribute__((noinline, noclone))
int helper_with_nested_call(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    
    /* Save to explicit register */
    reg_var3 = a * b;
    
    /* First call */
    int r1 = opaque_call_3(a, b);
    
    /* Many operations between calls */
    int temp = r1 * 2;
    temp += reg_var3;
    
    /* Nested call scenario */
    if (temp > 100) {
        opaque_call_2(temp);
        
        /* Complex expression requiring spill */
        int complex = (temp * a) / (b + 1);
        complex += reg_var3;
        
        return complex;
    }
    
    return temp;
}

/* Test function 5: Mixed types and calling conventions */
__attribute__((noinline, noclone))
void test5(int count) {
    struct RegPressure pressure;
    pressure.a = count;
    pressure.b = count * 2;
    pressure.c = count * 3;
    pressure.d = count * 4;
    pressure.e = count * 5;
    pressure.f = count * 6;
    pressure.x = count * 1.1;
    pressure.y = count * 2.2;
    pressure.z = count * 3.3;
    
    /* Use all structure members */
    long sum = pressure.a + pressure.b + pressure.c + pressure.d;
    sum += pressure.e + pressure.f;
    double fsum = pressure.x + pressure.y + pressure.z;
    
    /* Multiple calls with live structure */
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            opaque_call_1();
        } else if (i == 1) {
            opaque_call_2((int)sum);
        } else {
            opaque_call_4(pressure.x, pressure.y);
        }
        
        /* Modify and use structure between calls */
        pressure.a += i;
        sum += pressure.a;
        fsum += pressure.z;
    }
    
    global_accumulator += fsum;
    global_counter += (int)sum;
}

/* Main function with mode selection */
int main(int argc, char **argv) {
    /* Initialize function pointers */
    for (int i = 0; i < 10; i++) {
        func_table[i] = (func_ptr_t)dlsym(RTLD_DEFAULT, "nonexistent_func");
    }
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 10;
    }
    
    /* Initialize register variables */
    reg_var1 = mode * 10;
    reg_var2 = mode * 20;
    reg_var3 = mode * 30;
    reg_fp1 = mode * 1.5;
    reg_fp2 = mode * 2.5;
    
    /* Run all test functions in sequence */
    test1(mode);
    test2(mode + 1);
    test3(mode + 2);
    test4();
    test5(mode + 3);
    
    /* Call helper with nested call */
    int nested_result = helper_with_nested_call(mode, mode * 2);
    global_counter += nested_result;
    
    /* Create register pressure with many volatile operations */
    volatile int final_check = 0;
    for (int i = 0; i < 100; i++) {
        final_check += global_counter;
        final_check -= (int)global_accumulator;
        
        /* Insert compiler barriers */
        asm volatile ("" : : : "memory");
        
        /* Occasional function pointer call */
        if (i % 23 == 0 && func_table[i % 10]) {
            func_table[i % 10]();
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int checksum = global_counter + (int)global_accumulator + final_check;
    printf("Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}

/* Dummy definitions for external functions */
void opaque_call_1(void) {
    asm volatile ("" : : : "memory");
}

void opaque_call_2(int x) {
    global_counter += x;
    asm volatile ("" : : : "memory");
}

int opaque_call_3(int x, int y) {
    asm volatile ("" : : : "memory");
    return x + y;
}

double opaque_call_4(double x, double y) {
    global_accumulator += x + y;
    asm volatile ("" : : : "memory");
    return x * y;
}
