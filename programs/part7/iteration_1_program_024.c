/* caller-save-test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

int __attribute__((noinline)) external_func_with_return(void) {
    return 42;
}

/* Volatile function to prevent optimization */
int __attribute__((noinline, noclone)) volatile_func(int x) {
    volatile int result = x * 2;
    return result;
}

/* Global jump buffer for setjmp/longjmp test */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) test_scenario1(void) {
    int sum = 0;
    
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    
    for (int i = 0; i < 10; ++i) {
        /* Complex arithmetic to create register pressure */
        a = (a * i) + (b / 2);
        b = (b - i) ^ (c * 3);
        c = (c + a) | (b << 2);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call requiring original values */
        a = a + (b % 5);
        b = b - (c & 0xFF);
        c = c ^ (a * 2);
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            a += external_func_with_return();
        }
        
        sum += a + b + c;
    }
    
    return sum;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) test_scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int result = 0;
    
    /* Multiple volatile operations before call */
    v1 = v1 * v2 + v3;
    v2 = v2 - v4 / v5;
    v3 = v3 | (v1 & v2);
    
    /* Function call */
    external_func();
    
    /* Volatile operations after call */
    v4 = v4 ^ v3;
    v5 = v5 + v1 - v2;
    
    /* Another call with different register pressure */
    result = volatile_func(v1 + v2 + v3 + v4 + v5);
    
    /* Complex tail with conditional and call at block end */
    if (result > 100) {
        v1 = external_func_with_return();
        goto compute_result;
    } else {
        v2 = volatile_func(result);
    }
    
compute_result:
    return v1 + v2 + result;
}

/* ===== SCENARIO 3: Nested function calls ===== */
int __attribute__((noinline)) test_scenario3(int x) {
    int a, b, c;
    
    /* Setup values in registers */
    a = x * 2;
    b = x + 7;
    c = x ^ 0x55;
    
    /* Outer call - arguments depend on register values */
    int r1 = volatile_func(a);
    
    /* Inner call with arguments that use results from first call
       and original register values */
    int r2 = volatile_func(r1 + b);
    
    /* Third call with complex expression */
    external_func();
    int r3 = volatile_func(r2 * c + a);
    
    /* Switch statement to create complex control flow */
    switch (x % 4) {
        case 0:
            a = r1 + 1;
            break;
        case 1:
            b = r2 - 1;
            external_func();  /* Call before break */
            break;
        case 2:
            c = r3 * 2;
            /* Fall through */
        default:
            a = volatile_func(a + b + c);
            external_func();  /* Call in default case before break */
            break;
    }
    
    return a + b + c + r1 + r2 + r3;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) test_scenario4(int x) {
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ecx") = x * 2;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First path: do computation with registers */
        r1 = r1 * 3 + 7;
        r2 = r2 / 2 - 1;
        
        /* Function call that might longjmp */
        if (x > 100) {
            external_func();
            /* longjmp would return here, but we don't call it
               to keep the test executable */
        }
        
        /* More register operations */
        r1 = r1 ^ r2;
        result = r1 + r2;
        
        /* Another call */
        result += external_func_with_return();
    } else {
        /* longjmp return path */
        result = r1 - r2;  /* Uses register values across setjmp */
    }
    
    /* Final computation with call at block end */
    result = volatile_func(result);
    if (result < 0) {
        external_func();
        goto final;
    }
    
final:
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) test_scenario5(int x) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int a = x, b = x + 1, c = x + 2;
    int result = 0;
    
    /* Use computed goto to create unusual control flow */
    goto *labels[x % 4];
    
label0:
    a = a * 2;
    external_func();  /* Call in middle of block */
    b = b + a;
    goto join;
    
label1:
    b = b - 3;
    c = volatile_func(c);
    external_func();  /* Call before goto */
    goto join;
    
label2:
    c = c ^ 0xFF;
    /* Call right before computed goto again */
    external_func();
    if (c > 100) goto label3;
    else goto join;
    
label3:
    a = a + b + c;
    external_func();  /* Call at what could be BB_END */
    /* No explicit goto - falls through to join */
    
join:
    result = a + b + c;
    
    /* Loop with call at end of basic block */
    for (int i = 0; i < 5; i++) {
        result += i;
        if (i == 3) {
            external_func();  /* Call at potential BB_END before conditional */
            continue;
        }
        result -= 1;
    }
    
    return result;
}

/* ===== MAIN FUNCTION ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios to ensure code generation */
    total += test_scenario1();
    total += test_scenario2();
    total += test_scenario3(42);
    total += test_scenario4(50);
    total += test_scenario5(25);
    
    printf("Total checksum: %d\n", total);
    printf("(If this prints, all functions compiled and executed)\n");
    
    return total != 0 ? 0 : 1;
}
