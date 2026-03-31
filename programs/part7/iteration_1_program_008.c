/* test_caller_save.c - Program to trigger GCC caller-save pass instruction list manipulation */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int) __attribute__((noinline));
int returns_value(void) __attribute__((noinline));

/* Dummy implementations to satisfy linker */
void external_func(void) {
    /* Empty but not inlineable */
    asm volatile("" : : : "memory");
}

void another_external(int x) {
    /* Use the parameter to prevent optimization */
    volatile int y = x;
    (void)y;
}

int returns_value(void) {
    static int counter = 0;
    return counter++;
}

/* Global jump buffer for setjmp/longjmp scenario */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int scenario1(void) __attribute__((noinline, optimize("O2")));
int scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i;
        b -= i;
        c ^= i;
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        a *= 2;
        b += a;
        c = b - c;
        
        /* Conditional that might create basic block boundaries */
        if (i & 1) {
            result += a;
        } else {
            result += b + c;
        }
    }
    
    /* Final computation using all register variables */
    result = a + b + c + result;
    return result;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int scenario2(void) __attribute__((noinline, optimize("O3")));
int scenario2(void) {
    volatile int x = 10;
    volatile int y = 20;
    volatile int z = 30;
    int sum = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 8; i++) {
        x = x + i;
        y = y - i;
        z = z * (i + 1);
        
        /* Call with side effects */
        another_external(x);
        
        /* More volatile ops */
        x = x ^ y;
        y = y | z;
        z = z & x;
        
        /* Another call */
        external_func();
        
        /* Conditional at end of basic block */
        if (z > 100) {
            sum += x;
            /* goto creates control flow */
            if (sum > 50) goto done;
        } else {
            sum += y;
        }
        
        /* Label for goto target */
        done:
        sum += 1;
    }
    
    return sum + x + y + z;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int scenario3(int param) __attribute__((noinline, optimize("O2")));
int scenario3(int param) {
    /* Values that will be in registers for outer call setup */
    int a = param * 2;
    int b = param + 100;
    int c = param - 50;
    
    /* Outer call with arguments in registers */
    another_external(a);
    
    /* Inner call whose arguments depend on outer call's register values */
    int ret1 = returns_value();
    
    /* More computation between calls */
    a = a + ret1;
    b = b - ret1;
    
    /* Switch statement with calls in cases */
    switch (param % 4) {
        case 0:
            external_func();
            a += 10;
            break;  /* Creates jump at end of basic block */
        case 1:
            another_external(b);
            b += 20;
            break;
        case 2:
            ret1 = returns_value();
            c += ret1;
            /* Fall through */
        default:
            external_func();
            another_external(c);
            /* No break - falls through to end */
    }
    
    /* Final call with complex argument expression */
    another_external(a + b + c);
    
    return a * b + c;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int scenario4(void) __attribute__((noinline, optimize("O2")));
int scenario4(void) {
    int x = 0;
    int y = 0;
    
    /* setjmp creates complex control flow */
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        x = 100;
        y = 200;
        
        /* Function call that might longjmp */
        external_func();
        
        /* These values must be preserved across potential longjmp */
        x += returns_value();
        y -= returns_value();
        
        /* Another call */
        another_external(x + y);
    } else {
        /* After longjmp */
        x = 500;
        y = 600;
    }
    
    /* More calls in both paths */
    external_func();
    
    return x + y;
}

/* ===== SCENARIO 5: Computed goto with function calls ===== */
int scenario5(int selector) __attribute__((noinline, optimize("O3")));
int scenario5(int selector) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    int i = 0;
    
    /* Use explicit register for one variable */
    register int r __asm__ ("ebx") = selector * 10;
    
    if (selector < 0 || selector > 3) {
        return -1;
    }
    
    /* Jump to computed label */
    goto *labels[selector];
    
label0:
    r += 5;
    external_func();  /* Call in middle of basic block */
    r *= 2;
    result = r;
    goto end;
    
label1:
    r -= 3;
    another_external(r);
    r += returns_value();
    result = r * 2;
    goto end;
    
label2:
    for (i = 0; i < 3; i++) {
        r += i;
        external_func();  /* Call in loop */
        if (r > 20) goto end;
    }
    result = r;
    goto end;
    
label3:
    r = 100;
    /* Multiple consecutive calls */
    external_func();
    another_external(r);
    external_func();
    result = r + 1;
    /* Fall through to end */
    
end:
    return result;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    total += scenario4();
    total += scenario5(2);
    
    printf("Total result: %d\n", total);
    printf("If you see this, all scenarios compiled and ran successfully.\n");
    
    return 0;
}
