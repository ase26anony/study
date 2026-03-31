/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller_save_test.c */
/* Additional flags for x86-64: -mtune=generic -fomit-frame-pointer */

#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return a * b + 3;
}

__attribute__((noinline, noipa)) int helper4(int a, int b) {
    return (a << 2) | (b & 0xF);
}

/* Test function 1: High register pressure with consecutive calls */
void test1(int seed) {
    /* Declare many local variables to pressure registers */
    register int v1 asm("r10") = seed + 1;
    register int v2 asm("r11") = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v3 = v4 + v5;  /* This instruction is live across the next call */
        
        /* Second call - forces spill/restore around v3 assignment */
        v2 = helper2(v3, v6);
        
        /* Use results to prevent elimination */
        v7 = v1 + v2 + v3;
        v8 = v4 - v5 + v6;
        v9 = v7 * v8;
        v10 = v9 >> 1;
        
        /* Create cross-call dependencies */
        v4 = helper3(v7, v8);
        v5 = helper4(v9, v10);
        
        /* Another sequence that could trigger instruction movement */
        v6 = v1 + 1;  /* Potential last instruction in block */
        v1 = helper1(v6, v7);  /* Call that might require moving v6 */
    }
    
    global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Explicit register clobbering with inline asm */
void test2(int seed) {
    int r1 = seed * 2;
    int r2 = seed * 3;
    int r3 = seed * 4;
    int r4 = seed * 5;
    int r5 = seed * 6;
    int r6 = seed * 7;
    int r7 = seed * 8;
    int r8 = seed * 9;
    
    for (int j = 0; j < 4; j++) {
        /* Use inline asm to suggest specific register usage */
        asm volatile("" : "+r"(r1), "+r"(r2), "+r"(r3));
        
        /* Call that clobbers registers */
        r4 = helper2(r1, r2);
        
        /* Instruction that might need to move */
        r5 = r3 + r4;  /* Live value across next call */
        
        /* Another call forcing spills */
        r6 = helper3(r5, r4);
        
        /* More register pressure */
        asm volatile("" : "+r"(r7), "+r"(r8));
        
        /* Final instruction in block before loop back-edge */
        r1 = r5 + r6;  /* This could be BB_END before movement */
        
        /* Call that might cause caller-save to move r1 assignment */
        r2 = helper4(r1, r7);
        
        /* Create complex live ranges */
        r3 = r4 + r5 + r6;
        r7 = r8 - r1;
        r8 = helper1(r3, r7);
    }
    
    global_accumulator += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Test function 3: Mixed pointer and integer operations */
void test3(int seed) {
    int data[8];
    int *ptr = data;
    int a = seed + 100;
    int b = seed + 200;
    int c = seed + 300;
    int d = seed + 400;
    int e = seed + 500;
    int f = seed + 600;
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i * 10;
    }
    
    for (int k = 0; k < 2; k++) {
        /* Pointer arithmetic that uses registers */
        int val1 = *ptr + a;
        ptr++;
        
        /* Call that clobbers registers */
        b = helper1(val1, b);
        
        /* Instruction that might be last in block */
        c = val1 + b;  /* Live across next call */
        
        /* Another call */
        d = helper2(c, *ptr);
        
        /* More operations creating register pressure */
        e = helper3(d, e);
        f = helper4(e, f);
        
        /* Store that might need moving */
        *ptr = c + d;  /* Could be BB_END */
        
        /* Final call in sequence */
        a = helper1(*ptr, f);
        
        /* Update pointer for next iteration */
        if (k == 0) {
            ptr = data + 4;
        }
    }
    
    global_accumulator += a + b + c + d + e + f + data[0] + data[4];
}

/* Test function 4: Nested loops with varying pressure */
void test4(int seed) {
    int x1 = seed, x2 = seed * 2, x3 = seed * 3;
    int y1 = seed + 1, y2 = seed + 2, y3 = seed + 3;
    int z1 = 0, z2 = 0, z3 = 0;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* First computation block */
            x1 = x2 + x3;
            x2 = helper1(x1, y1);
            
            /* Critical instruction - may be moved */
            y2 = x1 + x2;  /* Live across call */
            
            /* Call sequence */
            x3 = helper2(y2, y3);
            y1 = helper3(x3, z1);
            
            /* Another potential BB_END instruction */
            z2 = y1 + y2;  /* Could be last before loop increment */
            
            /* More calls */
            z1 = helper4(z2, z3);
            y3 = helper1(z1, x1);
            
            /* Final operation in inner loop body */
            z3 = x2 + y3;  /* Might be moved by caller-save */
        }
        
        /* Reset some values for next outer iteration */
        x2 = helper2(x1, x3);
        y2 = helper3(y1, y3);
    }
    
    global_accumulator += x1 + x2 + x3 + y1 + y2 + y3 + z1 + z2 + z3;
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int iter = 0; iter < 10; iter++) {
        test1(seed + iter * 100);
        test2(seed + iter * 200);
        test3(seed + iter * 300);
        test4(seed + iter * 400);
        
        /* Modify seed slightly each iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
