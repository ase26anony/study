/* test_caller_save.c
 * Designed to trigger GCC's caller-save optimization pass
 * to execute the uncovered instruction reordering block
 * in caller-save.cc lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation and inlining */
volatile int global_seed = 42;

/* Function 1: High register pressure with single call */
int __attribute__((noinline)) 
test_high_pressure_single_call(int x) {
    /* Many local variables, all live across call */
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    int i = x + 9;
    int j = x + 10;
    int k = x + 11;
    int l = x + 12;
    int m = x + 13;
    int n = x + 14;
    int o = x + 15;
    int p = x + 16;
    
    /* Call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables after call */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p;
}

/* Function 2: Multiple calls with live variables between them */
int __attribute__((noinline))
test_multiple_calls(int x) {
    int v1 = x * 1;
    int v2 = x * 2;
    int v3 = x * 3;
    int v4 = x * 4;
    int v5 = x * 5;
    
    clobber_many_regs1();
    
    int v6 = v1 + v2;
    int v7 = v3 + v4;
    
    clobber_many_regs2();
    
    int v8 = v5 + v6;
    int v9 = v7 + v8;
    
    clobber_many_regs3();
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

/* Function 3: Control flow variation with calls in branches */
int __attribute__((noinline))
test_control_flow(int x, int flag) {
    int a = x + 100;
    int b = x + 200;
    int c = x + 300;
    int d = x + 400;
    int e = x + 500;
    
    if (flag > 0) {
        clobber_many_regs1();
        a = b + c;
        clobber_many_regs2();
        d = e + a;
    } else {
        clobber_many_regs3();
        b = c + d;
        clobber_many_regs1();
        e = a + b;
    }
    
    /* Force all variables to be live at return */
    return a + b + c + d + e;
}

/* Function 4: Loop with calls inside */
int __attribute__((noinline))
test_loop_calls(int x, int iterations) {
    int sum = 0;
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    
    for (int i = 0; i < iterations; i++) {
        int temp = a + b + c + i;
        clobber_many_regs1();
        sum += temp;
        a = b + 1;
        b = c + 1;
        c = temp % 100;
    }
    
    return sum + a + b + c;
}

/* Function 5: Mix of caller-saved and callee-saved usage */
int __attribute__((noinline))
test_mixed_register_usage(int x) {
    /* Variables that might use callee-saved registers */
    register long r1 asm("") = x + 1000;
    register long r2 asm("") = x + 2000;
    register long r3 asm("") = x + 3000;
    
    /* Many temporary variables using caller-saved regs */
    int t1 = x * 2;
    int t2 = x * 3;
    int t3 = x * 4;
    int t4 = x * 5;
    int t5 = x * 6;
    int t6 = x * 7;
    int t7 = x * 8;
    int t8 = x * 9;
    
    clobber_many_regs1();
    clobber_many_regs2();
    
    /* Use all variables */
    return (r1 + r2 + r3) + (t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8);
}

/* Function 6: Address taken to inhibit optimizations */
int __attribute__((noinline))
test_address_taken(int x) {
    int a = x + 10;
    int b = x + 20;
    int c = x + 30;
    int d = x + 40;
    int e = x + 50;
    
    /* Taking addresses forces variables to memory */
    int *ptr1 = &a;
    int *ptr2 = &b;
    int *ptr3 = &c;
    
    clobber_many_regs1();
    
    /* Use through pointers */
    *ptr1 += 1;
    *ptr2 += 2;
    *ptr3 += 3;
    
    clobber_many_regs2();
    
    return a + b + c + d + e + *ptr1 + *ptr2 + *ptr3;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argc to vary paths and prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Run all test functions */
    result += test_high_pressure_single_call(seed);
    result += test_multiple_calls(seed);
    result += test_control_flow(seed, seed % 2);
    result += test_loop_calls(seed, 3);
    result += test_mixed_register_usage(seed);
    result += test_address_taken(seed);
    
    /* Additional complex scenario combining multiple patterns */
    for (int i = 0; i < 2; i++) {
        int temp = seed + i;
        int a = temp * 11;
        int b = temp * 13;
        int c = temp * 17;
        int d = temp * 19;
        int e = temp * 23;
        
        if (i % 2 == 0) {
            /* Inline asm to clobber specific registers */
            /* x86-64 example - adjust for your architecture */
            asm volatile (
                "mov $0, %%rax\n"
                "mov $0, %%rcx\n"
                "mov $0, %%rdx\n"
                "mov $0, %%rsi\n"
                "mov $0, %%rdi\n"
                "mov $0, %%r8\n"
                "mov $0, %%r9\n"
                "mov $0, %%r10\n"
                "mov $0, %%r11\n"
                :
                :
                : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
                  "memory"
            );
        } else {
            /* Different clobber pattern */
            asm volatile (
                "xor %%eax, %%eax\n"
                "xor %%ebx, %%ebx\n"
                "xor %%ecx, %%ecx\n"
                "xor %%edx, %%edx\n"
                :
                :
                : "rax", "rbx", "rcx", "rdx", "memory"
            );
        }
        
        result += a + b + c + d + e;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Dummy definitions for external functions */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Use inline asm to clobber caller-saved registers */
    /* Generic version - will be architecture-specific */
    asm volatile ("" ::: "memory");
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    asm volatile ("" ::: "memory");
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    asm volatile ("" ::: "memory");
}
