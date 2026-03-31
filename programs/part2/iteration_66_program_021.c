/* test-caller-save.c
 * Designed to trigger GCC's caller-save instruction reordering logic
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test-caller-save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must live across calls */
    int a = x * 1;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    int i = x * 9;
    int j = x * 10;
    int k = x * 11;
    int l = x * 12;
    int m = x * 13;
    int n = x * 14;
    int o = x * 15;
    
    /* First call that clobbers many registers */
    clobber_many_regs_1();
    
    /* Use all variables to keep them live */
    int sum1 = a + b + c + d + e + f + g;
    
    /* Second call with different clobber pattern */
    clobber_many_regs_2();
    
    /* More variable usage */
    int sum2 = h + i + j + k + l + m + n + o;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final computation using all variables */
    return sum1 + sum2 + a - b + c - d + e - f + g - h + i - j + k - l + m - n + o;
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y) {
    int a = x + 1;
    int b = x + 2;
    int c = x + 3;
    int d = x + 4;
    int e = x + 5;
    int f = x + 6;
    int g = x + 7;
    int h = x + 8;
    
    if (x > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        
        /* Complex computation in true branch */
        a = a * 2;
        b = b * 3;
        c = c * 4;
        
        /* Another call */
        clobber_many_regs_2();
        
        d = d * 5;
        e = e * 6;
    } else {
        /* Different calls in false branch */
        clobber_many_regs_3();
        
        f = f * 7;
        g = g * 8;
        
        clobber_many_regs_1();
        
        h = h * 9;
    }
    
    /* Use all variables across both branches */
    return a + b + c + d + e + f + g + h;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int x, int iterations) {
    int a = x;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Use variables inside loop */
        a += i;
        b += i * 2;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        c += i * 3;
        d += i * 4;
    }
    
    /* Final computation */
    return a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r12_var asm("r12") = x + 100;
    register long r13_var asm("r13") = x + 200;
    register long r14_var asm("r14") = x + 300;
    register long r15_var asm("r15") = x + 400;
    
    /* Many caller-saved variables */
    int a = x * 1;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    int e = x * 5;
    int f = x * 6;
    int g = x * 7;
    int h = x * 8;
    
    /* Call that clobbers caller-saved but preserves callee-saved */
    clobber_many_regs_1();
    
    /* Use both types */
    int sum1 = a + b + c + d;
    long sum2 = r12_var + r13_var + r14_var + r15_var;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixed usage */
    int sum3 = e + f + g + h;
    
    return sum1 + sum3 + (int)sum2;
}

/* Function with pointer variables to inhibit optimizations */
int __attribute__((noinline)) test_pointer_vars(int x) {
    int a = x;
    int b = x * 2;
    int c = x * 3;
    int d = x * 4;
    
    /* Take addresses to force stack allocation */
    int *pa = &a;
    int *pb = &b;
    int *pc = &c;
    int *pd = &d;
    
    /* Call between taking addresses and using values */
    clobber_many_regs_1();
    
    /* Use through pointers */
    *pa += 1;
    *pb += 2;
    
    clobber_many_regs_2();
    
    *pc += 3;
    *pd += 4;
    
    /* Direct use */
    return a + b + c + d;
}

/* Main function that runs all tests with varying inputs */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed + 1);
    
    /* Test 2: Control flow variation */
    result += test_control_flow(seed - 10, seed + 10);
    
    /* Test 3: Loop with pressure */
    int iterations = (argc > 2) ? atoi(argv[2]) : 3;
    result += test_loop_pressure(seed, iterations);
    
    /* Test 4: Mixed caller/callee saved */
    result += test_mixed_save(seed * 2);
    
    /* Test 5: Pointer variables */
    result += test_pointer_vars(seed * 3);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result != 0;
}
