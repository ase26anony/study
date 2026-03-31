/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inline helper functions that clobber registers */
int __attribute__((noinline, noipa)) helper1(int a, int b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a + b + 1;
}

int __attribute__((noinline, noipa)) helper2(int a, int b) {
    /* Clobber different registers */
    asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2");
    return a * b - 1;
}

int __attribute__((noinline, noipa)) helper3(int a, int b, int c) {
    /* Clobber many registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    return (a + b) * c;
}

/* Test function 1: High register pressure with consecutive calls */
void __attribute__((noinline)) test1(int seed) {
    /* Declare many local variables to create register pressure */
    register int var1 asm("r10") = seed;
    register int var2 asm("r11") = seed + 1;
    register int var3 asm("r12") = seed + 2;
    register int var4 asm("r13") = seed + 3;
    int var5 = seed + 4;
    int var6 = seed + 5;
    int var7 = seed + 6;
    int var8 = seed + 7;
    int var9 = seed + 8;
    int var10 = seed + 9;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers many registers */
        int res1 = helper1(var1, var2);
        
        /* Critical instruction that should be at the end of a basic block */
        /* This instruction uses registers that might need spilling */
        var1 = var2 + var3;  /* This could be moved by caller-save */
        
        /* Second call - clobbers different registers */
        int res2 = helper2(var3, var4);
        
        /* More operations to create live ranges across calls */
        var2 = var4 + var5;
        var3 = var5 + var6;
        
        /* Third call */
        int res3 = helper3(var6, var7, var8);
        
        /* Another potential end-of-block instruction */
        var4 = var7 + var8;  /* Could be last instruction before loop end */
        
        /* Use results to prevent elimination */
        global_acc += res1 + res2 + res3 + var1 + var2 + var3 + var4;
        
        /* Loop update - creates back edge */
        var5++;
        var6 += 2;
    }
}

/* Test function 2: Explicit register usage with asm statements */
void __attribute__((noinline)) test2(int seed) {
    int a = seed, b = seed + 100, c = seed + 200;
    int d = seed + 300, e = seed + 400, f = seed + 500;
    int g = seed + 600, h = seed + 700, j = seed + 800;
    
    for (int i = 0; i < 4; i++) {
        /* Force specific register usage with inline asm */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c));
        
        /* Call that clobbers registers */
        int t1 = helper1(a, b);
        
        /* Instruction that might be moved to end of block */
        c = a + b + i;  /* This could become BB_END */
        
        /* Another call */
        int t2 = helper2(c, d);
        
        /* More operations */
        d = e + f;
        e = f + g;
        
        /* Call with many arguments */
        int t3 = helper3(g, h, j);
        
        /* Potential end-of-block instruction */
        f = g + h + j;  /* Could be last instruction */
        
        global_acc += t1 + t2 + t3 + c + d + e + f;
        
        /* Update loop variables */
        a += i;
        b += i * 2;
        g++;
    }
}

/* Test function 3: Mixed pointer and integer operations */
void __attribute__((noinline)) test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int idx = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointer before call */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Call that might require saving pointer registers */
        int res1 = helper1(val1, val2);
        
        /* Pointer arithmetic that could be at block end */
        ptr1++;  /* This instruction might be moved */
        
        /* Another call */
        int res2 = helper2(val1, idx);
        
        /* More operations */
        idx = *ptr2 + i;
        *ptr2 = val1 + val2;
        
        /* Call with pointer-derived values */
        int res3 = helper3(*ptr1, *ptr2, idx);
        
        /* Another potential end-of-block instruction */
        ptr2--;  /* Could be last instruction */
        
        global_acc += res1 + res2 + res3 + idx + *ptr1 + *ptr2;
        
        /* Loop control */
        if (i < 2) {
            /* Create conditional basic block */
            ptr1 += 2;
            ptr2 += 2;
        }
    }
}

/* Main function that drives all tests */
int main() {
    volatile int seed = 12345;  /* Prevent constant propagation */
    
    /* Run tests multiple times to increase coverage chances */
    for (int iter = 0; iter < 10; iter++) {
        int base = seed + iter * 100;
        
        test1(base);
        test2(base + 1);
        test3(base + 2);
        
        /* Mix in some direct register pressure */
        int x = base, y = base + 1, z = base + 2;
        for (int i = 0; i < 5; i++) {
            /* Create a small basic block ending with arithmetic */
            x = y + z;
            int r = helper1(x, y);  /* Call */
            y = z + i;  /* This could be moved after spill insertion */
            z = helper2(y, x);
            global_acc += x + y + z + r;
        }
    }
    
    printf("Result: %d\n", global_acc);
    return 0;
}
