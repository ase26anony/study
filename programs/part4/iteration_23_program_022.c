/* test-caller-save.c - Program to trigger uncovered lines in GCC's caller-save.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to create opaque calls */
extern void opaque_func1(void);
extern int opaque_func2(int);
extern double opaque_func3(double);
extern void *opaque_func4(void *);

/* Volatile globals to prevent optimization */
volatile int global_volatile_int = 0;
volatile double global_volatile_double = 0.0;
volatile void *global_volatile_ptr = NULL;

/* Function pointers with varying signatures */
typedef int (*func_int_t)(int, int, int, int, int, int, int, int);
typedef double (*func_double_t)(double, double, double, double);
typedef void (*func_void_t)(void);

/* Complex control flow with register pressure */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many live variables across calls */
    register int r1 asm ("r10") = mode * 2;
    register int r2 asm ("r11") = mode * 3;
    volatile int v1 = mode + 1;
    volatile int v2 = mode + 2;
    volatile int v3 = mode + 3;
    volatile int v4 = mode + 4;
    
    /* Array to force stack usage */
    int stack_array[10];
    for (int i = 0; i < 10; i++) {
        stack_array[i] = mode + i;
    }
    
    /* Complex control flow with goto */
    if (mode & 1) {
        goto label1;
    } else {
        goto label2;
    }
    
label1:
    /* Use all volatile variables before call */
    asm volatile ("" : : "r" (r1), "r" (r2), "m" (v1), "m" (v2) : "memory");
    
    /* Call that clobbers registers */
    asm volatile ("call opaque_func1" : : : "rax", "rcx", "rdx", "rsi", "rdi", 
                  "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3",
                  "xmm4", "xmm5", "xmm6", "xmm7", "memory");
    
    /* Use variables after call - forces save/restore */
    v1 = r1 + r2;
    v2 = stack_array[0] + stack_array[1];
    
    /* Another call via function pointer */
    func_void_t fp = (func_void_t)opaque_func1;
    fp();
    
    /* More register usage */
    r1 = v1 + v2 + v3 + v4;
    asm volatile ("" : "+r" (r1) : : "memory");
    
    if (r1 > 100) {
        goto label3;
    }
    
label2:
    /* Different path with nested calls */
    for (int i = 0; i < 3; i++) {
        volatile int temp = i * mode;
        
        /* Inline asm that acts like a call */
        asm volatile (
            "mov %0, %%rdi\n\t"
            "call opaque_func2"
            : 
            : "r" (temp)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "xmm0", "xmm1", "xmm2", "xmm3", "memory"
        );
        
        /* Use register variable after asm */
        if (r2 > 0) {
            r2--;
        } else {
            break;
        }
    }
    
label3:
    /* Final use of all variables */
    global_volatile_int = r1 + r2 + v1 + v2 + v3 + v4 + stack_array[5];
}

/* Test with floating point and mixed types */
__attribute__((noinline, noclone))
void test2(double base) {
    volatile double d1 = base;
    volatile double d2 = base * 2.0;
    volatile double d3 = base * 3.0;
    register double rd1 asm ("xmm8") = base * 0.5;
    register double rd2 asm ("xmm9") = base * 1.5;
    
    /* Switch statement with calls in cases */
    int choice = (int)base % 4;
    
    switch (choice) {
        case 0:
            /* Call with FP arguments */
            asm volatile (
                "movsd %0, %%xmm0\n\t"
                "movsd %1, %%xmm1\n\t"
                "call opaque_func3"
                : 
                : "m" (d1), "m" (d2)
                : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                  "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                  "memory"
            );
            break;
            
        case 1:
            /* Use register variables around call */
            rd1 = d1 + d2;
            opaque_func1();
            rd2 = rd1 * d3;
            break;
            
        case 2:
            /* Nested control flow */
            for (int i = 0; i < 2; i++) {
                if (i == 0) {
                    opaque_func1();
                } else {
                    /* Force register save before call */
                    double temp = rd1 + rd2;
                    asm volatile ("" : : "x" (rd1), "x" (rd2) : "memory");
                    opaque_func1();
                    rd1 = temp;
                }
            }
            break;
            
        default:
            /* Complex default case with goto */
            {
                volatile int counter = 0;
            default_loop:
                opaque_func1();
                counter++;
                if (counter < 2) {
                    /* Use FP registers before jumping back */
                    d1 = rd1 * rd2;
                    goto default_loop;
                }
            }
            break;
    }
    
    /* Final computation using all FP values */
    global_volatile_double = d1 + d2 + d3 + rd1 + rd2;
}

/* Test with pointer manipulation and __builtin_apply */
__attribute__((noinline, noclone))
void test3(void *ptr) {
    volatile void *vp1 = ptr;
    volatile void *vp2 = (char *)ptr + 100;
    register void *rp1 asm ("r12") = ptr;
    register void *rp2 asm ("r13") = (char *)ptr + 200;
    
    /* Array of saved values */
    void *saved_ptrs[5];
    saved_ptrs[0] = vp1;
    saved_ptrs[1] = vp2;
    saved_ptrs[2] = rp1;
    saved_ptrs[3] = rp2;
    saved_ptrs[4] = global_volatile_ptr;
    
    /* Loop with calls and register usage */
    for (int i = 0; i < 3; i++) {
        /* Save current state */
        void *temp1 = rp1;
        void *temp2 = rp2;
        
        /* Call that might clobber registers */
        asm volatile (
            "mov %0, %%rdi\n\t"
            "call opaque_func4"
            : 
            : "r" (saved_ptrs[i])
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "r14", "r15", "memory"
        );
        
        /* Restore and use saved values */
        rp1 = (char *)temp1 + i;
        rp2 = (char *)temp2 - i;
        
        /* Conditional break with call */
        if (i == 1) {
            opaque_func1();
            continue;
        }
    }
    
    /* Use __builtin_apply for unusual call sequence */
    {
        double args[3] = {1.0, 2.0, 3.0};
        void *argp = args;
        
        /* This creates complex prologue/epilogue */
        asm volatile (
            "mov %0, %%rdi\n\t"
            "mov $3, %%rsi\n\t"
            "call opaque_func4"
            : 
            : "r" (argp)
            : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
              "r12", "r13", "memory"
        );
    }
    
    global_volatile_ptr = rp1;
}

/* Helper with nested calls to force instruction insertion at block boundaries */
__attribute__((noinline, noclone))
int nested_helper(int depth, int val) {
    volatile int local = val;
    
    if (depth > 0) {
        /* Recursive call */
        int result = nested_helper(depth - 1, val * 2);
        
        /* Use result immediately in complex expression */
        register int r asm ("r10") = result;
        asm volatile ("" : "+r" (r) : : "memory");
        
        /* Call between register uses */
        opaque_func1();
        
        return r + local;
    }
    
    /* Base case with call */
    asm volatile (
        "mov %0, %%edi\n\t"
        "call opaque_func2"
        : 
        : "r" (val)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
    );
    
    return local;
}

/* Main test orchestrator */
int main(int argc, char *argv[]) {
    int mode = 0;
    
    /* Use argv to create runtime variability */
    if (argc > 1) {
        mode = atoi(argv[1]) % 4;
    }
    
    /* Initialize some volatile state */
    volatile int start_val = mode * 100;
    volatile double start_dbl = mode * 100.0;
    volatile char buffer[100];
    
    /* Fill buffer to force stack activity */
    for (int i = 0; i < 100; i++) {
        buffer[i] = (char)((i + mode) & 0xFF);
    }
    
    /* Run all tests in sequence, but with mode-dependent order */
    switch (mode) {
        case 0:
            test1(start_val);
            test2(start_dbl);
            test3(buffer);
            break;
        case 1:
            test2(start_dbl);
            test3(buffer);
            test1(start_val);
            break;
        case 2:
            test3(buffer);
            test1(start_val);
            test2(start_dbl);
            break;
        default:
            /* All tests with nested calls */
            test1(start_val);
            {
                int nested_result = nested_helper(2, start_val);
                global_volatile_int += nested_result;
            }
            test2(start_dbl);
            test3(buffer);
            break;
    }
    
    /* Additional loop to create more call sites */
    for (int i = 0; i < 2; i++) {
        volatile int loop_val = start_val + i;
        
        /* Mixed calls in loop */
        if (i == 0) {
            opaque_func1();
        } else {
            /* Force register save/restore around this call */
            register int r asm ("r10") = loop_val;
            asm volatile ("" : : "r" (r) : "memory");
            
            int (*fp)(int) = opaque_func2;
            int result = fp(loop_val);
            
            asm volatile ("" : : "r" (r) : "memory");
            global_volatile_int += result + r;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = global_volatile_int;
    checksum += (int)global_volatile_double;
    checksum += (int)(long)global_volatile_ptr;
    
    /* Use all volatile buffers */
    for (int i = 0; i < 100; i++) {
        checksum += buffer[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    return checksum == 0 ? 0 : 1;
}

/* Dummy definitions to satisfy linker (normally would be in separate library) */
void opaque_func1(void) {
    /* Empty but marked as used */
    asm volatile ("" : : : "memory");
}

int opaque_func2(int x) {
    asm volatile ("" : "+r" (x) : : "memory");
    return x + 1;
}

double opaque_func3(double x) {
    double result;
    asm volatile ("addsd %1, %0" : "=x" (result) : "x" (x), "0" (x));
    return result;
}

void *opaque_func4(void *x) {
    void *result;
    asm volatile ("mov %1, %0" : "=r" (result) : "r" (x));
    return result;
}
