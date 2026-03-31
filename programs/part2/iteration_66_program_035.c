/* test_caller_save.c
 * Designed to trigger GCC's caller-save instruction reordering
 * Compile with: gcc -O3 -fno-inline -fno-omit-frame-pointer -c test_caller_save.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque external functions that clobber caller-saved registers */
extern void __attribute__((noinline)) clobber_many_regs1(void);
extern void __attribute__((noinline)) clobber_many_regs2(void);
extern void __attribute__((noinline)) clobber_many_regs3(void);

/* Prevent constant propagation */
volatile int global_seed = 42;

/* Function with extreme register pressure around calls */
int __attribute__((noinline)) 
test_high_pressure(int x, int y) {
    /* Many local variables that must stay live across calls */
    int a = x + 1;
    int b = y + 2;
    int c = a * b + 3;
    int d = c - x + 4;
    int e = d * y + 5;
    int f = e - a + 6;
    int g = f * b + 7;
    int h = g - c + 8;
    int i = h * d + 9;
    int j = i - e + 10;
    int k = j * f + 11;
    int l = k - g + 12;
    int m = l * h + 13;
    int n = m - i + 14;
    int o = n * j + 15;
    int p = o - k + 16;
    
    /* Force these to be in registers by using them repeatedly */
    int sum1 = a + b + c + d + e + f + g + h;
    
    /* First call that clobbers caller-saved registers */
    clobber_many_regs1();
    
    /* Use variables again - they must be restored if spilled */
    int sum2 = i + j + k + l + m + n + o + p;
    
    /* Second clobbering call */
    clobber_many_regs2();
    
    /* More computation mixing all variables */
    int result = (sum1 * 3) + (sum2 * 2) + (a - b + c - d);
    result += (e * f) - (g / 2) + (h % 3);
    
    /* Third call */
    clobber_many_regs3();
    
    /* Final use of all variables */
    result += (i * j) + (k * l) + (m * n) + (o * p);
    result += (a * c * e * g) % 100;
    
    return result;
}

/* Function with control flow variations */
int __attribute__((noinline))
test_control_flow(int cond, int val) {
    int r1 = val * 2;
    int r2 = val + 100;
    int r3 = val - 50;
    int r4 = val / 3;
    int r5 = val % 7;
    int r6 = val * val;
    int r7 = val << 2;
    int r8 = val >> 1;
    
    if (cond > 0) {
        /* Branch with call and many live variables */
        clobber_many_regs1();
        
        int t1 = r1 + r2 + r3;
        int t2 = r4 * r5 * r6;
        
        clobber_many_regs2();
        
        r1 = t1 + t2;
        r2 = r7 * r8 + cond;
    } else {
        /* Alternative branch with different pattern */
        int t3 = r3 * r4 * r5;
        int t4 = r6 + r7 + r8;
        
        clobber_many_regs3();
        
        r1 = t3 - t4;
        r2 = cond * val;
    }
    
    /* Variables must be live here */
    clobber_many_regs1();
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Function with loop and calls */
int __attribute__((noinline))
test_loop_pressure(int iterations) {
    int acc = 0;
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    int v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live variables across loop iteration */
        int t1 = v1 * i + v2;
        int t2 = v3 + v4 * i;
        
        /* Call that clobbers registers */
        if (i % 3 == 0) {
            clobber_many_regs1();
        } else if (i % 3 == 1) {
            clobber_many_regs2();
        } else {
            clobber_many_regs3();
        }
        
        /* Use variables after call */
        v1 = t1 + v5;
        v2 = t2 + v6;
        v3 = v1 * v7;
        v4 = v2 * v8;
        
        acc += v1 + v2 + v3 + v4;
    }
    
    return acc + v5 + v6 + v7 + v8;
}

/* Function mixing caller-saved and callee-saved usage */
int __attribute__((noinline))
test_mixed_save(int x) {
    /* Variables that might use callee-saved registers */
    register long r8 asm("r12") = x * 2;  /* Hint for callee-saved reg */
    register long r9 asm("r13") = x * 3;
    
    /* Many caller-saved only variables */
    int a = x + 1, b = x + 2, c = x + 3;
    int d = x + 4, e = x + 5, f = x + 6;
    
    /* Take addresses to force stack slots */
    int *ptr1 = &a, *ptr2 = &b, *ptr3 = &c;
    
    /* Use both types */
    clobber_many_regs1();
    
    int sum1 = a + b + c + d + e + f;
    int sum2 = (int)(r8 + r9);
    
    clobber_many_regs2();
    
    /* Force all to be used */
    *ptr1 = sum1 + 1;
    *ptr2 = sum2 + 2;
    *ptr3 = (int)r8 - (int)r9;
    
    return a + b + c + d + e + f + (int)r8 + (int)r9;
}

/* Main driver that uses command-line args to vary paths */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use argv to prevent constant folding */
    int seed = (argc > 1) ? atoi(argv[1]) : global_seed;
    
    /* Test 1: Extreme register pressure */
    result += test_high_pressure(seed, seed * 2);
    
    /* Test 2: Control flow variations */
    result += test_control_flow(seed % 2, seed);
    
    /* Test 3: Loop with pressure */
    result += test_loop_pressure((seed % 5) + 3);
    
    /* Test 4: Mixed caller/callee saved */
    result += test_mixed_save(seed + 100);
    
    /* Additional test with inline asm to clobber specific registers */
    {
        int x1 = seed * 3, x2 = seed * 4, x3 = seed * 5;
        int x4 = seed * 6, x5 = seed * 7, x6 = seed * 8;
        
        /* Inline asm that clobbers caller-saved registers */
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
        
        result += x1 + x2 + x3 + x4 + x5 + x6;
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;  /* Return non-zero to indicate execution */
}

/* Dummy definitions for external functions */
void __attribute__((noinline)) clobber_many_regs1(void) {
    /* Use inline asm to clobber many registers */
    asm volatile (
        "# clobber_many_regs1\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
    );
}

void __attribute__((noinline)) clobber_many_regs2(void) {
    asm volatile (
        "# clobber_many_regs2\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10"
    );
}

void __attribute__((noinline)) clobber_many_regs3(void) {
    asm volatile (
        "# clobber_many_regs3\n"
        :
        :
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9"
    );
}
