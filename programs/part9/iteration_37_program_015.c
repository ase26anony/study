/* Test program for reorg.cc delay slot filling optimization */
/* Compile with: gcc -O2 -march=mips32 -fno-gcse -fno-crossjumping test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to keep function calls as separate instructions */
__attribute__((noinline)) 
static int simple_operation(int x) {
    return x + 1;
}

/* Another non-inlineable function to use as delay slot candidate */
__attribute__((noinline, optimize("O0")))
static int safe_operation(int *ptr) {
    /* Simple operation that won't trap */
    return (*ptr) * 2;
}

/* Function with specific optimization level to control instruction generation */
__attribute__((optimize("O2")))
static void test_pattern1(void) {
    volatile int a = 10, b = 20, c = 30;
    volatile int result = 0;
    
    /* Use goto to create a simple jump instruction */
    if (a > 5) {
        goto target_label1;
    }
    
    /* Some code that won't be executed but prevents optimization */
    result = a + b + c;
    
target_label1:
    /* Candidate instruction for delay slot filling:
       - Non-jump instruction
       - Simple arithmetic that doesn't trap
       - Doesn't conflict with jump resources
    */
    asm volatile("" ::: "memory");  /* Compiler barrier to prevent merging */
    
    /* Simple arithmetic operation - good delay slot candidate */
    b = c + 1;
    
    /* Use the result to prevent dead code elimination */
    result = b;
    printf("Pattern1 result: %d\n", result);
}

__attribute__((optimize("O2")))
static void test_pattern2(void) {
    volatile int x = 100, y = 200;
    volatile int arr[10] = {0};
    int *ptr = &arr[0];
    
    /* Create multiple basic blocks to encourage jump optimization */
    if (x > 50) {
        goto compute;
    }
    
    /* Dead code to create separation */
    y = x * 2;
    
compute:
    asm volatile("" ::: "memory");  /* Prevent instruction merging */
    
    /* Good delay slot candidate: 
       - Memory access to stack variable (safe, won't trap)
       - Simple operation
    */
    *ptr = y + x;
    
    /* Function call as potential delay slot candidate */
    int z = simple_operation(x);
    printf("Pattern2: x=%d, z=%d\n", x, z);
}

/* Test with function call at target */
__attribute__((optimize("O1")))  /* O1 to keep structure but allow some optimization */
static void test_pattern3(void) {
    volatile int counter = 0;
    volatile int data[5] = {1, 2, 3, 4, 5};
    
    /* Loop structure that might generate jumps */
    for (int i = 0; i < 3; i++) {
        if (data[i] > 2) {
            goto process;
        }
        counter++;
        continue;
        
    process:
        asm volatile("" ::: "memory");
        
        /* Non-inline function call as delay slot candidate */
        int val = safe_operation((int*)&data[i]);
        
        counter += val;
    }
    
    printf("Pattern3 counter: %d\n", counter);
}

/* Test with asm statement as explicit delay slot candidate */
__attribute__((optimize("O2")))
static void test_pattern4(void) {
    register int r1 asm("t0") = 42;  /* Use call-clobbered register on MIPS */
    register int r2 asm("t1") = 84;
    
    /* Force a simple jump */
    if (r1 != 0) {
        goto asm_target;
    }
    
    r2 = r1 * 2;  /* Unreachable, but creates code structure */
    
asm_target:
    asm volatile("" ::: "memory");
    
    /* Inline asm that's a good delay slot candidate:
       - Register-only operation
       - No memory access
       - No trapping possible
       - Sets flags but jump is simple (not conditional)
    */
    asm volatile("addu %0, %1, %2" 
                 : "=r"(r1) 
                 : "r"(r1), "r"(r2)
                 : /* no clobbers */);
    
    printf("Pattern4 result: %d\n", r1);
}

/* Complex pattern with nested control flow */
__attribute__((optimize("O2"), noinline))
static int test_pattern5(int n) {
    volatile int a = n;
    volatile int b = n * 2;
    
    /* Switch-like structure that might generate jumps */
    if (a < 10) {
        goto small;
    } else if (a < 20) {
        goto medium;
    } else {
        goto large;
    }
    
small:
    asm volatile("" ::: "memory");
    /* Simple assignment - good candidate */
    b = a + 5;
    return b;
    
medium:
    asm volatile("" ::: "memory");
    /* Arithmetic with local variable */
    int temp = b - a;
    b = temp * 3;
    return b;
    
large:
    asm volatile("" ::: "memory");
    /* Bit operation - won't trap */
    b = a & 0xFF;
    return b;
}

int main(void) {
    printf("Testing delay slot filling patterns...\n");
    
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    
    int result = test_pattern5(15);
    printf("Pattern5 result: %d\n", result);
    
    return 0;
}
