/* caller_save_test.c - Test program to trigger specific caller-save optimization */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External function declarations to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call */
    asm volatile ("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile ("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile ("" : "+r"(x) : : "memory");
    return x + 1;
}

/* Global jmp_buf for setjmp/longjmp test */
jmp_buf jump_buffer;

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
        a = a + i * 2;
        b = b - i;
        c = c ^ (i << 1);
        d = d | (i & 0xF);
        
        /* Function call clobbers registers */
        external_func();
        
        /* More operations after call - values must be restored */
        sum += a + b;
        a = a ^ c;
        b = b & d;
        
        /* Another call with different register pressure */
        another_external(c);
        
        /* Final computation */
        sum += c * d;
        c = c + 1;
        d = d - 1;
    }
    
    /* Force all values to be used */
    return sum + a + b + c + d;
}

/* ===== SCENARIO 2: Volatile variables forcing memory spills ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1234;
    volatile int v2 = 5678;
    volatile int v3 = 9012;
    volatile int v4 = 3456;
    int result = 0;
    
    /* Multiple volatile operations across calls */
    for (int i = 0; i < 8; ++i) {
        v1 = v1 * 2 + i;
        v2 = v2 / (i + 1) + v1;
        
        /* Call with volatile side effects */
        external_func();
        
        v3 = v3 ^ v2;
        v4 = v4 | v1;
        
        /* Another call */
        another_external(v3);
        
        /* Complex conditional with volatile access */
        if (v4 > 10000) {
            v1 = v1 - 500;
            external_func();
            v2 = v2 + 250;
        } else {
            v3 = v3 * 2;
            another_external(v4);
        }
        
        result += v1 + v2 + v3 + v4;
    }
    
    return result;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Create register pressure for nested calls */
    int a = x * 2;
    int b = x + 100;
    int c = x ^ 0x55;
    int d = x & 0xFF;
    
    /* Outer call setup uses multiple registers */
    int r1 = returning_external(a + b);
    
    /* Inner call with arguments depending on previous result */
    int r2 = returning_external(r1 + c);
    
    /* Another call in conditional */
    if (r2 > 1000) {
        external_func();
        a = returning_external(r2);
    } else {
        another_external(d);
        b = returning_external(a);
    }
    
    /* Final call with all values live */
    return returning_external(a + b + c + d + r1 + r2);
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    int a = 100, b = 200, c = 300, d = 400;
    int sum = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First pass - do computations with calls */
        for (int i = 0; i < 5; ++i) {
            a += i * 10;
            b -= i * 5;
            
            /* Call that might be affected by longjmp */
            external_func();
            
            c = c ^ (a & 0xFF);
            d = d | (b & 0xFF);
            
            sum += a + b + c + d;
            
            /* Another call */
            another_external(i);
        }
    } else {
        /* longjmp target - different computations */
        a = a * 2;
        b = b / 2;
        external_func();
        c = c + 1000;
        d = d - 500;
        sum = a + b + c + d;
    }
    
    /* Force register usage before potential longjmp */
    another_external(sum);
    
    return sum;
}

/* ===== SCENARIO 5: Computed goto with function calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static const void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int a = 1, b = 2, c = 3, d = 4;
    int result = 0;
    
    /* Jump to different code paths */
    goto *labels[selector & 3];
    
label0:
    a = a * 10;
    external_func();  /* Call in middle of basic block */
    b = b + a;
    /* Fall through to label1 */
    
label1:
    c = c ^ b;
    another_external(c);
    if (a > 10) goto label3;
    /* else fall through */
    
label2:
    d = d * 2;
    external_func();
    result = a + b + c + d;
    goto end;
    
label3:
    a = a - 5;
    another_external(a);
    d = d / 2;
    result = a * b * c * d;
    /* Fall through to end */
    
end:
    return result;
}

/* ===== SCENARIO 6: Complex switch statement with calls ===== */
int __attribute__((noinline)) scenario6(int code) {
    int x = 0, y = 0, z = 0;
    
    switch (code) {
        case 0:
            x = 100;
            external_func();  /* Call before break */
            y = returning_external(x);
            break;  /* Creates jump at end of basic block */
            
        case 1:
            x = 200;
            y = 300;
            another_external(x + y);
            z = returning_external(y);
            external_func();  /* Call right before break */
            break;
            
        case 2:
            x = 400;
            external_func();
            if (x > 250) {
                y = 500;
                another_external(y);
                /* No break here - fall through */
            } else {
                z = 600;
                break;
            }
            /* Fall through */
            
        case 3:
            x = x * 2;
            external_func();  /* Call in fall-through path */
            y = y + 100;
            z = returning_external(x + y);
            break;
            
        default:
            x = code * 10;
            external_func();  /* Call in default case */
            y = returning_external(x);
            z = y * 2;
            another_external(z);
            break;  /* Terminal break */
    }
    
    return x + y + z;
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
    total += scenario6(3);
    
    /* Force a longjmp for scenario4 */
    longjmp(jump_buffer, 1);
    
    printf("Total checksum: %d\n", total);
    
    /* Return value based on total (prevents dead code elimination) */
    return (total > 1000000) ? 0 : 1;
}
