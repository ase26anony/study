/* caller-save-coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Specifically targets lines 905-913 in caller-save.cc
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

int __attribute__((noinline)) returning_external(void) {
    return 42;
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Arithmetic on register variables */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More arithmetic after call - values must be restored */
        result += a * b + c;
        
        /* Conditional that creates basic block boundaries */
        if (i & 1) {
            a += returning_external();  /* Another call */
        } else {
            b -= global_counter;
        }
    }
    
    /* Force use of all register variables at end */
    return result + a - b + c;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int sum = 0;
    
    /* Multiple expressions with volatiles across calls */
    v1 = v1 * 2 + v2;
    v2 = v2 - v3;
    external_func();
    v3 = v1 + v2;  /* Must reload v1, v2 after call */
    
    v4 = v4 ^ v3;
    another_external(v4);
    v5 = v5 + v4;  /* Must reload v4 after call */
    
    /* Create a basic block ending with call + jump */
    if (v1 > v2) {
        v1 = returning_external();  /* Call at BB end */
        goto compute_sum;  /* Creates jump at BB end */
    } else {
        v2 = returning_external();
    }
    
compute_sum:
    sum = v1 + v2 + v3 + v4 + v5;
    
    /* Switch to create complex BB structure */
    switch (sum % 4) {
        case 0:
            external_func();
            break;  /* Jump at BB end after call */
        case 1:
            another_external(sum);
            /* Fall through */
        case 2:
            sum += returning_external();
            break;
        default:
            external_func();
            sum *= 2;
            break;
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Inner call's arguments depend on outer call setup */
    int a = x * 2;
    int b = x + 5;
    
    /* First call - setup in registers */
    another_external(a);
    
    /* Nested call pattern */
    int r1 = returning_external();
    another_external(r1 + b);  /* b must survive first returning_external call */
    
    /* More complex nesting */
    for (int i = 0; i < 3; ++i) {
        int temp = returning_external() + i;
        another_external(temp + a);  /* a must survive returning_external */
        a += temp;  /* Update a for next iteration */
    }
    
    /* Basic block with call at end followed by label */
    if (a > 100) {
        external_func();
        goto finish;
    } else {
        another_external(a);
    }
    
    /* Dead code to create additional BB structure */
    if (0) {
        dead_label:
        external_func();
    }
    
finish:
    return a + b;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int value = 0;
    int saved = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        register int r1 __asm__ ("eax") = 100;
        register int r2 __asm__ ("ecx") = 200;
        
        /* Do computation in registers */
        r1 = r1 * 2;
        r2 = r2 / 2;
        
        /* Call that might longjmp */
        external_func();
        
        /* These must be saved/restored due to setjmp */
        saved = r1 + r2;
        value = saved;
        
        /* Another call */
        another_external(saved);
    } else {
        /* After longjmp */
        value = 999;
    }
    
    /* Simulate longjmp call */
    if (global_counter++ > 5) {
        longjmp(jump_buffer, 1);
    }
    
    return value;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    
    /* Force values into registers */
    register int x __asm__ ("eax") = selector * 10;
    register int y __asm__ ("ecx") = selector + 20;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    x += 5;
    external_func();  /* Call in middle of BB */
    y -= 3;
    result = x + y;
    goto end;
    
label1:
    y *= 2;
    another_external(y);
    x = returning_external();  /* Another call */
    result = x - y;
    goto end;
    
label2:
    external_func();
    /* Fall through to label3 */
    
label3:
    x ^= y;
    another_external(x);
    result = x * y;
    /* Fall through to end */
    
end:
    /* Create BB ending with call and jump */
    if (result > 100) {
        external_func();
        return result;  /* Return creates jump */
    } else {
        another_external(result);
    }
    
    return result + 1;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(10);
    total += scenario4();
    total += scenario5(2);
    
    printf("Total result: %d\n", total);
    printf("If you see this, all scenarios compiled and ran.\n");
    printf("Check coverage of caller-save.cc lines 905-913 during compilation.\n");
    
    return 0;
}
