/* test-caller-save.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -mno-accumulate-outgoing-args -fno-jump-tables test-caller-save.c -o test-caller-save
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
volatile double global_accumulator = 0.0;
volatile int global_array[100];

/* Function pointer to create indirect calls */
typedef void (*func_ptr_t)(void);
volatile func_ptr_t volatile_fptr;

/* Barrier to prevent reordering */
#define COMPILER_BARRIER() asm volatile("" : : : "memory")

/* Force specific register usage */
#define FORCE_REGISTER(var, reg) register int var asm(reg)

/* Test function 1: Many live variables across a call */
__attribute__((noinline, noclone))
void test1(int mode) {
    /* Force many variables to be live across call */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    /* Use specific registers */
    FORCE_REGISTER(r12_var, "r12") = 100;
    FORCE_REGISTER(r13_var, "r13") = 200;
    
    /* Array to force stack usage */
    int local_array[20];
    for (int i = 0; i < 20; i++) {
        local_array[i] = i * mode;
    }
    
    COMPILER_BARRIER();
    
    /* Function call that clobbers registers */
    if (mode & 1) {
        /* Inline asm that looks like a call */
        asm volatile(
            "movl $0x12345678, %%eax\n\t"
            "movl $0x87654321, %%ebx\n\t"
            "call *%0\n\t"
            : 
            : "r"((void*)opaque_call_1)
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    } else {
        opaque_call_2(mode);
    }
    
    COMPILER_BARRIER();
    
    /* Use all variables after call - forces save/restore */
    int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += r12_var + r13_var;
    for (int i = 0; i < 20; i++) {
        sum += local_array[i];
    }
    
    global_counter += sum;
    COMPILER_BARRIER();
}

/* Test function 2: Complex control flow with calls at block boundaries */
__attribute__((noinline, noclone))
void test2(int n) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile double f = 1.0, g = 2.0, h = 3.0;
    
    /* Irreducible control flow with goto */
    int i = 0;
start_loop:
    if (i >= n) goto end_loop;
    
    /* Force register pressure */
    FORCE_REGISTER(r10_var, "r10") = i * 10;
    FORCE_REGISTER(r11_var, "r11") = i * 20;
    
    /* Conditional with function call at boundary */
    if (i % 3 == 0) {
        opaque_call_3(a, b);
        /* This creates a basic block boundary */
        goto middle_block;
    } else if (i % 3 == 1) {
        /* Different call site */
        asm volatile(
            "pushq %%r10\n\t"
            "pushq %%r11\n\t"
            "call opaque_call_1\n\t"
            "popq %%r11\n\t"
            "popq %%r10\n\t"
            : : : "memory", "rax", "rbx", "rcx", "rdx"
        );
        goto middle_block;
    } else {
        /* No call here - different path */
        c = d * e;
    }
    
middle_block:
    /* Use variables that were live before the call */
    a += r10_var;
    b += r11_var;
    
    /* Another call that might split the block */
    if (i % 2 == 0) {
        double result = opaque_call_4(f + g);
        h += result;
        /* Complex expression requiring temp registers */
        f = g * h + result * 2.0 - (double)a / (double)b;
    }
    
    i++;
    
    /* Jump back creates loop structure */
    if (i % 4 == 0) {
        goto start_loop;
    } else {
        goto middle_block;
    }
    
end_loop:
    global_accumulator += f + g + h;
    COMPILER_BARRIER();
}

/* Test function 3: Switch statement with calls in cases */
__attribute__((noinline, noclone))
void test3(int selector) {
    volatile int x1 = 100, x2 = 200, x3 = 300, x4 = 400;
    volatile long x5 = 500, x6 = 600;
    
    /* Force specific registers to be used */
    register long r14_var asm("r14") = 999;
    register long r15_var asm("r15") = 888;
    
    /* Switch creates complex CFG */
    switch (selector) {
        case 0:
            opaque_call_1();
            x1 = x2 + x3;
            break;
        case 1:
            opaque_call_2(x1);
            x2 = x3 * x4;
            /* Fall through */
        case 2:
            asm volatile(
                "movq %0, %%r14\n\t"
                "movq %1, %%r15\n\t"
                "call *%2\n\t"
                "movq %%r14, %0\n\t"
                "movq %%r15, %1\n\t"
                : "+r"(r14_var), "+r"(r15_var)
                : "r"((void*)opaque_call_1)
                : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
            );
            x3 = x4 - x1;
            break;
        case 3:
            /* Nested call scenario */
            {
                int temp = opaque_call_3(x1, x2);
                x4 = temp + x3;
                /* Immediate use of call result in complex expr */
                x5 = (long)(temp * 2) + r14_var - r15_var;
            }
            break;
        default:
            /* Default case with function pointer call */
            if (volatile_fptr) {
                volatile_fptr();
            }
            x6 = x5 * 2;
            break;
    }
    
    /* Use all variables after switch */
    global_array[selector % 100] = x1 + x2 + x3 + x4 + (int)x5 + (int)x6 + (int)r14_var + (int)r15_var;
    COMPILER_BARRIER();
}

/* Test function 4: Nested calls with register pressure */
__attribute__((noinline, noclone))
int test4(int depth) {
    if (depth <= 0) {
        return 1;
    }
    
    /* Many local variables that need to be saved across calls */
    volatile int vars[8];
    for (int i = 0; i < 8; i++) {
        vars[i] = i * depth;
    }
    
    /* Register variables that conflict with call-clobbered regs */
    FORCE_REGISTER(eax_var, "eax") = vars[0];
    FORCE_REGISTER(ebx_var, "ebx") = vars[1];
    FORCE_REGISTER(ecx_var, "ecx") = vars[2];
    
    COMPILER_BARRIER();
    
    /* Recursive call - forces prologue/epilogue complexity */
    int result = test4(depth - 1);
    
    COMPILER_BARRIER();
    
    /* Use register variables after call */
    result += eax_var + ebx_var + ecx_var;
    for (int i = 0; i < 8; i++) {
        result += vars[i];
    }
    
    /* Another call */
    opaque_call_2(result);
    
    return result;
}

/* Helper with inline asm that forces specific instruction patterns */
__attribute__((noinline, noclone))
void helper_with_asm(int *ptr) {
    /* Force specific register usage pattern */
    register int r8_var asm("r8") = *ptr;
    register int r9_var asm("r9") = *ptr * 2;
    
    /* Inline asm that acts like a call */
    asm volatile(
        "movl $0xDEADBEEF, %%eax\n\t"
        "movl $0xCAFEBABE, %%ebx\n\t"
        "movl %0, %%ecx\n\t"
        "movl %1, %%edx\n\t"
        "call *%2\n\t"
        "movl %%eax, %0\n\t"
        "movl %%ebx, %1\n\t"
        : "+r"(r8_var), "+r"(r9_var)
        : "r"((void*)opaque_call_1)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    *ptr = r8_var + r9_var;
    COMPILER_BARRIER();
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize function pointer */
    volatile_fptr = (func_ptr_t)opaque_call_1;
    
    /* Use argv to create runtime variability */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 5;
    }
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i;
    }
    
    /* Run all test functions in different orders based on mode */
    for (int i = 0; i < 3; i++) {
        switch ((mode + i) % 4) {
            case 0:
                test1(mode + i);
                test2(10 + i);
                test3(mode);
                break;
            case 1:
                test2(5 + i);
                test3(mode * 2);
                test1(i);
                break;
            case 2:
                test3(mode + i * 3);
                test1(7);
                test2(8);
                break;
            case 3:
                test4(3 + i);
                test1(mode);
                test2(6);
                test3(i);
                break;
        }
        
        /* Call helper with complex asm */
        int value = global_counter + i;
        helper_with_asm(&value);
        global_counter += value;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    checksum += global_counter;
    checksum += (int)global_accumulator;
    for (int i = 0; i < 100; i++) {
        checksum += global_array[i];
    }
    
    /* Use checksum to affect control flow */
    if (checksum % 7 == 0) {
        test1(checksum);
    } else if (checksum % 5 == 0) {
        test2(checksum % 20);
    } else {
        test3(checksum % 4);
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum % 256;
}

/* Dummy definitions for external functions to allow linking */
void opaque_call_1(void) {
    global_counter++;
}

void opaque_call_2(int x) {
    global_accumulator += x;
}

int opaque_call_3(int a, int b) {
    return a + b + global_counter;
}

double opaque_call_4(double x) {
    return x * 1.5 + global_accumulator;
}
