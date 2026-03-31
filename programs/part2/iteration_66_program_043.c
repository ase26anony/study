/* test_caller_save.c
 * Designed to trigger instruction reordering in GCC's caller-save pass
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
void __attribute__((noinline)) clobber_many_regs_1(void) {
    /* Use inline asm to clobber many caller-saved registers */
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_2(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

void __attribute__((noinline)) clobber_many_regs_3(void) {
    asm volatile ("" : : : "memory", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
}

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) test_high_pressure(int x, int y) {
    /* Many local variables that must stay live across calls */
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
    clobber_many_regs_1();
    
    /* Use all variables after call - they must be preserved */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* Second call with different register pressure */
    clobber_many_regs_2();
    
    /* More computations keeping variables live */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Third call */
    clobber_many_regs_3();
    
    /* Final use of all variables */
    return sum1 + sum2 + (a * b) - (c * d) + (e * f) - (g * h) + 
           (i * j) - (k * l) + (m * n) - (o * p);
}

/* Function with control flow variation */
int __attribute__((noinline)) test_control_flow(int x, int y, int mode) {
    int a = x * 2;
    int b = y * 3;
    int c = a + b;
    int d = b - a;
    int e = c * d;
    int f = d / (c ? c : 1);
    int g = e + f;
    int h = f - e;
    
    if (mode > 0) {
        /* Call in true branch */
        clobber_many_regs_1();
        int i = g * h + a;
        int j = h - g + b;
        
        /* Another call */
        clobber_many_regs_2();
        
        /* Complex computation */
        int k = i * j + c;
        int l = j - i + d;
        
        return k + l + e + f;
    } else {
        /* Different call pattern in false branch */
        clobber_many_regs_3();
        int i = g + h + a;
        int j = h - g + b;
        
        /* Multiple calls in sequence */
        clobber_many_regs_1();
        clobber_many_regs_2();
        
        int k = i * j * c;
        int l = j / (i ? i : 1) + d;
        
        return k - l + e - f;
    }
}

/* Function with loop and calls */
int __attribute__((noinline)) test_loop_pressure(int iterations) {
    int a = 1, b = 2, c = 3, d = 4;
    int e = 5, f = 6, g = 7, h = 8;
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables at loop start */
        int t1 = a * b + c;
        int t2 = d * e + f;
        int t3 = g * h + i;
        
        /* Call inside loop - variables must be preserved */
        clobber_many_regs_1();
        
        /* Use variables after call */
        result += t1 + t2 + t3;
        
        /* Modify variables to keep them live */
        a += t1;
        b += t2;
        c += t3;
        
        /* Another call */
        if (i % 2 == 0) {
            clobber_many_regs_2();
        } else {
            clobber_many_regs_3();
        }
        
        /* More computations */
        d -= a;
        e -= b;
        f -= c;
        g += d;
        h += e;
    }
    
    return result + a + b + c + d + e + f + g + h;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline)) test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r12 asm("r12") = x * 2;
    register long r13 asm("r13") = x * 3;
    register long r14 asm("r14") = x * 4;
    register long r15 asm("r15") = x * 5;
    
    /* Many temporary variables using caller-saved regs */
    int a = x + 1;
    int b = a * 2;
    int c = b + 3;
    int d = c * 4;
    int e = d + 5;
    int f = e * 6;
    
    /* Call that clobbers caller-saved but not callee-saved */
    clobber_many_regs_1();
    
    /* Use both register types */
    long callee_sum = r12 + r13 + r14 + r15;
    int caller_sum = a + b + c + d + e + f;
    
    /* Another call */
    clobber_many_regs_2();
    
    /* More mixing */
    r12 += caller_sum;
    r13 -= caller_sum;
    a += (int)callee_sum;
    b -= (int)callee_sum;
    
    /* Final call */
    clobber_many_regs_3();
    
    return (int)(r12 + r13 + r14 + r15) + a + b + c + d + e + f;
}

/* Function with multiple basic blocks and calls at block boundaries */
int __attribute__((noinline)) test_block_boundaries(int x, int y, int z) {
    int a = x, b = y, c = z;
    int result = 0;
    
    switch (x % 4) {
        case 0:
            a *= 2;
            b *= 3;
            clobber_many_regs_1();
            result = a + b + c;
            break;
        case 1:
            a += 10;
            b += 20;
            clobber_many_regs_2();
            c *= a;
            result = b + c;
            clobber_many_regs_3();
            break;
        case 2:
            a -= 5;
            b -= 10;
            clobber_many_regs_1();
            c = a * b;
            clobber_many_regs_2();
            result = c + x + y;
            break;
        default:
            a /= 2;
            b /= 3;
            clobber_many_regs_3();
            c = a - b;
            result = c * z;
            break;
    }
    
    /* More variables live across the switch */
    int d = result * 2;
    int e = result / 2;
    int f = d + e;
    
    clobber_many_regs_1();
    
    return f + a + b + c;
}

/* Main function that calls all test cases */
int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    int iter = argc > 2 ? atoi(argv[2]) : 10;
    
    srand(seed);
    
    int total = 0;
    
    /* Call all test functions with varying inputs */
    total += test_high_pressure(rand() % 100, rand() % 100);
    total += test_control_flow(rand() % 100, rand() % 100, rand() % 2);
    total += test_loop_pressure(iter);
    total += test_mixed_save(rand() % 100);
    total += test_block_boundaries(rand() % 100, rand() % 100, rand() % 100);
    
    /* Add some volatile operations to prevent optimization */
    volatile int print_me = total;
    
    printf("Result: %d\n", print_me);
    
    return 0;
}
