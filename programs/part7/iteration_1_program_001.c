/* caller_save_test.c
 * Test program to trigger specific instruction list manipulation
 * in GCC's caller-save optimization pass (lines 905-913 of caller-save.cc)
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

int __attribute__((noinline)) returning_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
    return x + 1;
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
    for (int i = 0; i < 20; ++i) {
        /* Use all register variables before call */
        a = a + i * 2;
        b = b - i;
        c = c ^ (i + 1);
        d = d * (i % 5 + 1);
        
        /* Mix of arithmetic to create instruction density */
        sum += a;
        sum -= b;
        sum ^= c;
        sum |= d;
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call, requiring reloads */
        a = a + sum % 100;
        b = b ^ (sum >> 2);
        c = c - (sum & 0xFF);
        d = d | (sum << 3);
        
        /* Another call with different arguments */
        another_external(a + b + c + d);
        
        /* Conditional that might affect BB structure */
        if (i % 3 == 0) {
            sum += returning_external(i);
        }
    }
    
    return sum + a + b + c + d;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    int result = 0;
    
    /* Multiple basic blocks with calls at different points */
    for (int i = 0; i < 15; ++i) {
        /* Use volatiles in complex expressions */
        v1 = v1 * v2 + v3;
        v2 = v2 - v4 / (v5 + 1);
        v3 = v3 ^ v6 | v7;
        v4 = v4 & v8 + v9;
        v5 = v5 % (v10 + i);
        
        /* Call in middle of volatile usage */
        external_func();
        
        /* More volatile operations */
        v6 = v6 + v1 * 2;
        v7 = v7 - v2 / 3;
        v8 = v8 ^ v3;
        v9 = v9 | v4;
        v10 = v10 & v5;
        
        /* Another call */
        another_external(v1 + v10);
        
        /* Conditional with call at end of BB */
        if (v1 > 100) {
            result += returning_external(v2);
            /* goto creates edge at BB end */
            if (v3 < 50) goto compute_more;
        } else {
            result -= returning_external(v3);
        }
        
        /* Label creating BB boundary */
        compute_more:
        v1 = v1 + result;
    }
    
    return result + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    /* Chain of computations where inner call args depend on 
       outer call setup in call-clobbered registers */
    int a = x;
    int b = x * 2;
    int c = x + 5;
    
    /* Outer call setup uses multiple registers */
    int r1 = returning_external(a + b);
    
    /* Inner call with args that depend on previous result */
    int r2 = returning_external(r1 + c);
    
    /* Another call where args depend on both previous results */
    another_external(r1 - r2);
    
    /* Complex conditional with calls at BB boundaries */
    switch (x % 4) {
        case 0:
            a = returning_external(r1);
            /* break generates jump at BB end */
            break;
        case 1:
            b = returning_external(r2);
            external_func();
            /* Multiple instructions before BB end */
            a = a + b;
            b = b * 2;
            break;
        case 2:
            c = returning_external(a + b);
            /* Call followed by label */
            external_func();
            if (c > 10) goto special_case;
            break;
        case 3:
            a = returning_external(b + c);
            /* Fall through */
        default:
            special_case:
            b = returning_external(c);
            /* Call at BB end before function return */
            another_external(a + b + c);
    }
    
    return a + b + c + r1 + r2;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int setjmp_var = 0;
    int normal_path = 0;
    int jmp_path = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* Normal path - register pressure across call */
        register int r1 __asm__("eax") = 100;
        register int r2 __asm__("ecx") = 200;
        
        for (int i = 0; i < 10; ++i) {
            r1 = r1 + i * 5;
            r2 = r2 - i * 3;
            
            /* Call that might be affected by setjmp */
            external_func();
            
            normal_path += r1 + r2;
            
            /* Conditional that might create complex BB */
            if (i == 5) {
                another_external(r1);
                /* Potential BB end manipulation */
                goto continue_loop;
            }
            
            r1 = r1 ^ r2;
            r2 = r2 | r1;
            
            continue_loop:
            setjmp_var = i;
        }
        
        return normal_path;
    } else {
        /* longjmp path */
        for (int i = 0; i < 5; ++i) {
            jmp_path += returning_external(i);
            external_func();
        }
        return jmp_path;
    }
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int selector) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    
    int result = 0;
    int a = selector * 10;
    int b = selector * 20;
    
    /* Force values into registers */
    register int r1 __asm__("eax") = a + 1;
    register int r2 __asm__("ecx") = b + 2;
    
    /* Computed goto creates unusual control flow */
    goto *labels[selector % 4];
    
    label0:
        r1 = r1 + 5;
        external_func();  /* Call in middle of BB */
        r2 = r2 - 3;
        result = r1 + r2;
        goto end;
    
    label1:
        r1 = r1 * 2;
        another_external(r1);
        r2 = r2 / 2;
        result = r1 - r2;
        /* Fall through to call at BB end */
        external_func();
        goto end;
    
    label2:
        r1 = returning_external(r1);
        r2 = returning_external(r2);
        /* Multiple calls in sequence */
        external_func();
        another_external(r1 + r2);
        result = r1 * r2;
        goto end;
    
    label3:
        for (int i = 0; i < 3; ++i) {
            r1 = r1 + i;
            external_func();  /* Call in loop */
            r2 = r2 - i;
            if (i == 1) {
                another_external(r1);
                /* Creates BB with call near end */
            }
        }
        result = r1 | r2;
    
    end:
    return result + selector;
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
    total += scenario5(7);
    
    printf("Total checksum: %d\n", total);
    
    /* Verify with a simple computation */
    if (total != 0) {
        printf("Test completed (non-zero result indicates code executed).\n");
    }
    
    return 0;
}
