/* test_caller_save.c - Program to trigger specific caller-save optimization paths */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int) __attribute__((noinline));
int third_external(int, int) __attribute__((noinline));

/* Dummy implementations to satisfy linker */
void external_func(void) {
    /* Empty but prevents inlining */
    asm volatile("" : : : "memory");
}

void another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int third_external(int a, int b) {
    asm volatile("" : : "r"(a), "r"(b) : "memory");
    return a + b;
}

/* Global jump buffer for setjmp/longjmp scenario */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i;
        c = a ^ b;
        d = c + i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* Use values after call - must be restored */
        sum += a + b + c + d;
        
        /* More arithmetic to create dense block */
        if (i & 1) {
            a += d;
            b -= c;
        } else {
            c = b ^ a;
            d = a + i;
        }
        
        /* Another call with different arguments */
        another_external(i);
        
        /* More register usage */
        sum += (a * b) - (c / (d ? d : 1));
    }
    
    return sum;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 123;
    volatile int v2 = 456;
    volatile int v3 = 789;
    volatile int v4 = 101112;
    
    int result = 0;
    
    /* Multiple volatile operations across calls */
    for (int j = 0; j < 8; ++j) {
        v1 = v1 * 2 + j;
        v2 = v2 / (j + 1) + v1;
        
        /* Call with side effects */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v1;
        
        /* Conditional with volatile reads */
        if (v3 > v4) {
            result += v1;
            another_external(v2);
        } else {
            result -= v2;
            external_func();
        }
        
        /* More volatile operations */
        v1 = v3 + v4;
        v2 = v4 - v3;
        
        /* Final call in basic block before break/return */
        third_external(v1, v2);
        
        /* Additional operation after call at BB end */
        result += (v1 > 0) ? v1 : -v1;
    }
    
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    int a, b, c, d;
    
    /* Setup values in registers */
    a = x * 2;
    b = x + 100;
    
    /* Outer call - arguments depend on register values */
    int r1 = third_external(a, b);
    
    /* Inner call setup uses same registers */
    c = r1 * 3;
    d = r1 / 2;
    
    /* Nested call chain */
    external_func();
    int r2 = third_external(c, d);
    another_external(r2);
    
    /* Complex conditional with calls at block ends */
    switch (x % 4) {
        case 0:
            a = third_external(r1, r2);
            external_func();  /* Call at end of case block */
            break;
        case 1:
            b = third_external(r2, x);
            another_external(b);
            /* Fall through */
        case 2:
            c = a + b;
            external_func();
            break;
        default:
            d = third_external(a, c);
            another_external(d);
            /* Call followed by break at BB end */
            break;
    }
    
    return a + b + c + d + r1 + r2;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(int val) {
    register int r1 __asm__ ("eax") = val;
    register int r2 __asm__ ("ecx") = val * 2;
    
    if (setjmp(jump_buffer) == 0) {
        /* First path - use registers then call */
        r1 += 100;
        r2 -= 50;
        
        /* Call that might longjmp */
        another_external(r1);
        
        /* More register use */
        r1 = r1 * r2;
        r2 = r1 ^ r2;
        
        /* Another call */
        external_func();
        
        return r1 + r2;
    } else {
        /* Longjmp target - registers must be restored */
        r1 += 1000;
        r2 -= 500;
        
        /* Call in recovery path */
        third_external(r1, r2);
        
        return r1 - r2;
    }
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    register int a __asm__ ("eax") = selector;
    register int b __asm__ ("ecx") = selector * 2;
    
    /* Jump table dispatch */
    goto *labels[selector % 4];
    
label0:
    a += 10;
    external_func();  /* Call in middle of block */
    b -= 5;
    goto end;
    
label1:
    b *= 2;
    another_external(a);
    a /= 2;
    /* Fall through to call */
    external_func();
    goto end;
    
label2:
    a = third_external(a, b);
    b = a + 100;
    /* Call at end before goto */
    another_external(b);
    goto end;
    
label3:
    external_func();
    a = b * 3;
    another_external(a);
    /* Final operation at BB end */
    b = a - selector;
    
end:
    return a + b;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4(17);
    total += scenario5(3);
    
    printf("Final checksum: %d\n", total);
    
    /* Simulate longjmp for scenario 4 */
    longjmp(jump_buffer, 1);
    
    return 0;
}
