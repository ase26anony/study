/* caller-save-test.c
 * Designed to trigger GCC's caller-save optimization pass instruction list
 * manipulation logic (lines 905-913 in caller-save.cc)
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
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__("eax") = 1;
    register int b __asm__("ecx") = 2;
    register int c __asm__("edx") = 3;
    register int d __asm__("esi") = 4;
    
    int sum = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Use all register variables before call */
        a += i * 2;
        b -= i + 1;
        c = (c * 3) % 17;
        d = d ^ (i << 2);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        sum += a + b;
        sum += c * d;
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            sum += 100;
            external_func();  /* Another call in conditional path */
            a += 5;
        } else {
            sum -= 50;
            b += 3;
        }
        
        /* Mix in another call with different register usage */
        another_external(sum);
    }
    
    return sum + a + b + c + d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    
    int result = 0;
    
    /* Multiple volatile operations around calls */
    for (int j = 0; j < 8; ++j) {
        v1 = v1 * 2 + j;
        v2 = v2 / (j + 2) + v1;
        
        /* Call with volatile side effects */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | (v1 << 3);
        
        /* Another call */
        another_external(v3);
        
        v1 = v1 + v4;
        v2 = v2 - v3;
        
        /* Conditional with call at end of basic block */
        if (v1 > v2) {
            result += v1;
            external_func();  /* Call at potential BB_END */
            /* No explicit jump - fall through */
        } else {
            result -= v2;
            v3 = returning_external();  /* Call returning value */
            /* break-like control flow */
            if (v3 > 1000) goto done;
        }
        
        v4 = v4 * 2;
    }
    
done:
    return result + v1 + v2 + v3 + v4;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Arguments in registers get clobbered by inner calls */
    int a = x * 2;
    int b = x + 100;
    
    /* Outer call setup uses registers that inner call clobbers */
    int temp1 = a + b;
    external_func();  /* Clobbers registers */
    
    /* Inner call with complex argument computation */
    int temp2 = returning_external() + temp1;
    another_external(temp2);
    
    /* More computation requiring original values */
    int c = a * b;
    external_func();
    
    /* Switch statement with calls and breaks */
    switch (x % 4) {
        case 0:
            c += 10;
            external_func();  /* Call before break */
            break;  /* Creates jump at BB_END */
        case 1:
            c += 20;
            another_external(c);
            /* Fall through */
        case 2:
            c += returning_external();
            external_func();
            break;
        default:
            c += 30;
            /* Call at end of default case before break */
            another_external(c * 2);
            break;  /* Jump instruction at BB_END */
    }
    
    return c + temp1 + temp2;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int save1 = 1111;
    volatile int save2 = 2222;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        save1 = 3333;
        save2 = 4444;
        
        /* Function call that might longjmp */
        external_func();
        
        /* These values must be preserved across potential longjmp */
        save1 = save1 * 2;
        save2 = save2 + 100;
        
        /* Another call */
        another_external(save1);
        
        /* Simulate error condition */
        if (global_counter++ > 5) {
            longjmp(jump_buffer, 1);
        }
        
        return save1 + save2;
    } else {
        /* After longjmp */
        return save1 * 3 + save2 * 2;
    }
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int value = 0;
    
    /* Force values into registers */
    register int r1 __asm__("eax") = selector * 10;
    register int r2 __asm__("ecx") = selector * 20;
    
    if (selector < 0 || selector > 3) return -1;
    
    goto *labels[selector];
    
label0:
    r1 += 5;
    external_func();  /* Call in middle of basic block */
    r2 -= 3;
    value = r1 + r2;
    goto end;
    
label1:
    r1 *= 2;
    another_external(r1);
    r2 /= 2;
    value = r1 - r2;
    /* Fall through to call */
    external_func();
    goto end;
    
label2:
    value = returning_external();
    external_func();  /* Call before goto end */
    goto end;
    
label3:
    r1 = r1 ^ r2;
    another_external(r1);
    r2 = r1 * 2;
    external_func();  /* Call at potential BB_END before goto */
    /* No operation after call, just goto */
    
end:
    return value + r1 + r2;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save optimization scenarios...\n");
    
    /* Run all scenarios */
    total += scenario1();
    total += scenario2();
    total += scenario3(7);
    total += scenario4();
    total += scenario5(2);
    
    /* Mix in some direct register pressure in main too */
    {
        register int m1 __asm__("eax") = total;
        register int m2 __asm__("ecx") = total * 2;
        
        for (int i = 0; i < 3; ++i) {
            m1 += i * 10;
            m2 -= i * 5;
            external_func();
            total = m1 + m2;
            another_external(total);
        }
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
