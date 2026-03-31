/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call boundary */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
    return x + 1;
}

/* Global volatile to prevent optimizations */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    for (int i = 0; i < 10; ++i) {
        /* Complex arithmetic to create register pressure */
        a = a * i + 1;
        b = b - i * 2;
        c = c ^ (a + b);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call, requiring reloads */
        result += a + b - c;
        a = result % 100;
        b = (b * 3) / 2;
        c = c + global_counter;
        
        /* Another call with different register usage */
        another_external(result);
    }
    
    return result + a + b + c;
}

/* ===== SCENARIO 2: Volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    int sum = 0;
    
    /* Multiple volatile operations before call */
    v1 = v1 * 2 + v2;
    v2 = v2 - v3 / 2;
    v3 = v3 ^ v4;
    v4 = v4 + v1 * 3;
    
    /* Call that forces spills */
    external_func();
    
    /* Complex conditional with volatile usage */
    if (v1 > v2) {
        v3 = v3 + returning_external(v1);
        /* Another call in conditional path */
        another_external(v2);
        sum = v1 + v2;
    } else {
        v4 = v4 - returning_external(v2);
        sum = v3 + v4;
    }
    
    /* Final call near end of basic block */
    external_func();
    
    return sum + v1 + v2 + v3 + v4;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Create register pressure for argument passing */
    int a = x * 2;
    int b = a + 5;
    int c = b * 3;
    int d = c - 7;
    
    /* Nested calls where inner call depends on outer call setup */
    int r1 = returning_external(a);
    /* Use result immediately in another call */
    int r2 = returning_external(r1 + b);
    
    /* Complex expression spanning multiple calls */
    for (int i = 0; i < 5; i++) {
        a = a + returning_external(b);
        b = b + returning_external(c);
        c = c + returning_external(d);
        d = d + returning_external(a);
        
        /* Call with multiple live values */
        another_external(a + b + c + d);
    }
    
    /* Switch statement to create complex basic block structure */
    switch (x % 4) {
        case 0:
            a = returning_external(a);
            break;
        case 1:
            b = returning_external(b);
            /* Call followed by break at BB end */
            external_func();
            break;
        case 2:
            c = returning_external(c);
            another_external(c);
            break;
        default:
            d = returning_external(d);
            /* Multiple calls in default case */
            external_func();
            another_external(d);
            break;
    }
    
    return a + b + c + d + r1 + r2;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int saved = 0;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First execution path */
        register int x __asm__ ("eax") = 100;
        register int y __asm__ ("ecx") = 200;
        
        /* Operations in setjmp path */
        for (int i = 0; i < 3; i++) {
            x = x * 2 + i;
            y = y / 2 - i;
            
            /* Call that might be affected by setjmp */
            external_func();
            
            result += x + y;
        }
        
        saved = result;
        
        /* Simulate longjmp - this won't actually jump here */
        /* but creates uncertainty for register liveness */
        another_external(result);
    } else {
        /* longjmp would return here */
        result = saved * 2;
    }
    
    /* More calls after setjmp context */
    result += returning_external(result);
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int value = 0;
    
    /* Force register usage before computed goto */
    register int r1 __asm__ ("eax") = selector * 10;
    register int r2 __asm__ ("ecx") = selector * 20;
    
    /* Call before computed goto */
    another_external(r1);
    
    goto *labels[selector % 4];
    
label0:
    r1 = r1 + 5;
    external_func();
    value = r1 + r2;
    goto end;
    
label1:
    r2 = r2 - 3;
    /* Call in middle of block */
    another_external(r2);
    r1 = r1 * 2;
    external_func();  /* Call near BB end */
    value = r1 - r2;
    goto end;
    
label2:
    external_func();
    value = returning_external(r1) + returning_external(r2);
    goto end;
    
label3:
    r1 = r1 / 2;
    another_external(r1);
    r2 = r2 + returning_external(r2);
    external_func();  /* Multiple calls in sequence */
    value = r1 * r2;
    /* Fall through to end */
    
end:
    return value + global_counter;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4();
    total += scenario5(3);
    
    /* Add some more calls with different patterns */
    for (int i = 0; i < 5; i++) {
        total += scenario3(i * 7);
        global_counter++;
    }
    
    printf("Result: %d\n", total);
    printf("(This value is not important - the goal is compile-time coverage)\n");
    
    return 0;
}
