/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must be live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = b - a + 4;
    int e = c * d + 5;
    int f = d - c + 6;
    int g = e * f + 7;
    int h = f - e + 8;
    int i = g * h + 9;
    int j = h - g + 10;
    int k = i * j + 11;
    int l = j - i + 12;
    int m = k * l + 13;
    int n = l - k + 14;
    int o = m * n + 15;
    int p = n - m + 16;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use all variables after call to keep them live */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different register pressure */
    clobber_many_regs2();
    
    /* More computations keeping variables live */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + 
           (i * j) - (k * l) + (m * n) - (o * p);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y, int z) {
    /* Many variables that compete for registers */
    int v1 = x * 2;
    int v2 = y * 3;
    int v3 = z * 4;
    int v4 = v1 + v2;
    int v5 = v2 + v3;
    int v6 = v3 + v1;
    int v7 = v4 * v5;
    int v8 = v5 * v6;
    int v9 = v6 * v4;
    int v10 = v7 + v8;
    
    /* Conditional with calls in both branches */
    if (x > y) {
        clobber_many_regs1();
        v1 = v2 * v3;
        v4 = v5 + v6;
        clobber_many_regs2();
    } else {
        clobber_many_regs3();
        v7 = v8 * v9;
        v10 = v1 + v2;
        clobber_many_regs1();
    }
    
    /* Use all variables to keep them live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int iter) {
    int accum = 0;
    
    /* Force many variables to be live in loop */
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    
    for (int n = 0; n < iter; n++) {
        /* Call inside loop - variables must be saved/restored */
        clobber_many_regs1();
        
        /* Use all variables to keep them live */
        accum += a + b + c + d + e + f + g + h + i + j;
        
        /* Modify variables to prevent optimization */
        a += n;
        b -= n;
        c ^= n;
        d |= n;
        
        /* Another call */
        if (n % 2) {
            clobber_many_regs2();
        } else {
            clobber_many_regs3();
        }
        
        /* More variable uses */
        accum += (e * f) - (g * h) + (i * j);
    }
    
    return accum;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Use register hint for some variables */
    register int r1 asm("rbx") = x * 2;  /* Callee-saved on x86-64 */
    register int r2 asm("r12") = x * 3;  /* Callee-saved */
    register int r3 asm("r13") = x * 4;  /* Callee-saved */
    
    /* Many regular variables (will use caller-saved regs) */
    int v1 = x + 1;
    int v2 = x + 2;
    int v3 = x + 3;
    int v4 = x + 4;
    int v5 = x + 5;
    int v6 = x + 6;
    int v7 = x + 7;
    int v8 = x + 8;
    int v9 = x + 9;
    int v10 = x + 10;
    
    /* Take addresses to inhibit optimizations */
    int *p1 = &v1;
    int *p2 = &v2;
    int *p3 = &v3;
    
    /* Call that clobbers caller-saved but not callee-saved */
    clobber_many_regs1();
    
    /* Use all variables including register ones */
    int sum = r1 + r2 + r3 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Another call */
    clobber_many_regs2();
    
    /* More uses through pointers */
    sum += *p1 + *p2 + *p3;
    
    return sum;
}

/* Function with sequential calls and minimal temps */
int __attribute__((noinline)) test_sequential_calls(int x) {
    /* Chain of computations with calls in between */
    int a = x;
    clobber_many_regs1();
    
    int b = a * 2;
    clobber_many_regs2();
    
    int c = b + a;
    clobber_many_regs3();
    
    int d = c * 3;
    clobber_many_regs1();
    
    int e = d - a;
    clobber_many_regs2();
    
    int f = e / 2;
    clobber_many_regs3();
    
    /* All variables must be live at the end */
    return a + b + c + d + e + f;
}

/* Main function that runs all tests with varying inputs */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use command line arguments to prevent constant propagation */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Run all test functions with different parameters */
    result += test_high_pressure(base, base + 1);
    result += test_control_flow(base, base + 2, base + 3);
    result += test_loop_pressure((base % 10) + 5);
    result += test_mixed_save(base + 4);
    result += test_sequential_calls(base + 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
