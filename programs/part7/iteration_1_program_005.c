/* caller-save-coverage.c
 * Designed to trigger specific instruction list manipulation in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-omit-frame-pointer -c caller-save-coverage.c
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call instruction */
    asm volatile("" : : : "memory");
}

void __attribute__((noinline)) another_external(int x) {
    asm volatile("" : : "r"(x) : "memory");
}

/* Global to prevent optimization */
volatile int global_counter = 0;
jmp_buf jump_buffer;

/* Scenario 1: Explicit register variables with loop */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of call-clobbered registers on x86 */
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
        
        /* More operations after call requiring original values */
        result += a + b - c;
        
        /* Conditional that might create basic block boundaries */
        if (i & 1) {
            a += global_counter;
        } else {
            b -= global_counter;
        }
        
        /* Another call with different register usage */
        another_external(c);
        
        /* More arithmetic mixing */
        c = (c << 2) | (a & 0xF);
    }
    
    return result + a + b + c;
}

/* Scenario 2: Many volatile variables across calls */
int __attribute__((noinline)) scenario2(int x) {
    volatile int v1 = x;
    volatile int v2 = x * 2;
    volatile int v3 = x * 3;
    volatile int v4 = x * 4;
    volatile int v5 = x * 5;
    
    int sum = 0;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 5; i++) {
        v1 = v1 + v2 - v3;
        external_func();
        v2 = v2 * v4 / (v5 + 1);
        another_external(v1);
        v3 = v3 ^ v2;
        external_func();
        v4 = v4 - v1 + v3;
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* This creates a basic block ending with a call and jump */
        if (sum > 1000) {
            external_func();
            goto done;
        }
    }
    
done:
    return sum;
}

/* Scenario 3: Nested calls with register dependencies */
int __attribute__((noinline)) scenario3(int seed) {
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    
    /* Outer call setup in registers */
    a = a + 1;
    b = b - 2;
    
    /* Call with arguments that use current register values */
    another_external(a + b);
    
    /* Inner call context */
    for (int i = 0; i < 3; i++) {
        register int r1 __asm__ ("eax") = a + i;
        register int r2 __asm__ ("ecx") = b - i;
        
        /* Use in computation before call */
        int temp = r1 * r2 + c;
        
        /* Call that clobbers registers */
        external_func();
        
        /* Need original values after call */
        a += r1;
        b += r2;
        c = temp;
        
        /* Switch statement to create complex control flow */
        switch (i) {
            case 0:
                another_external(a);
                break;
            case 1:
                external_func();
                /* Fall through to create different BB structure */
            case 2:
                a = b + c;
                external_func();
                break;
            default:
                another_external(c);
                break;
        }
    }
    
    return a + b + c;
}

/* Scenario 4: setjmp/longjmp with calls */
int __attribute__((noinline)) scenario4(int x) {
    int a = x;
    int b = x * 2;
    
    if (setjmp(jump_buffer) == 0) {
        /* First path with calls */
        for (int i = 0; i < 3; i++) {
            a += i;
            b -= i;
            
            /* Call that might be before BB_END */
            external_func();
            
            /* Conditional at end of basic block */
            if (a > 100) {
                another_external(b);
                /* This creates BB_END with call then jump */
                goto finish;
            }
        }
        
        /* Another call in different basic block */
        another_external(a + b);
    } else {
        /* longjmp target */
        a = a * 2;
        b = b / 2;
    }
    
finish:
    return a + b;
}

/* Scenario 5: Computed goto with calls */
int __attribute__((noinline)) scenario5(int selector) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    int a = selector;
    int b = selector + 1;
    
    /* Force values into registers */
    register int r1 __asm__ ("eax") = a * 2;
    register int r2 __asm__ ("ecx") = b * 3;
    
    /* Computed goto */
    goto *labels[selector % 4];
    
label0:
    r1 += 10;
    external_func();  /* Call in middle of block */
    r2 -= 5;
    result = r1 + r2;
    goto end;

label1:
    r1 *= 2;
    another_external(r1);
    r2 /= 2;
    external_func();  /* Another call */
    result = r1 - r2;
    goto end;

label2:
    external_func();  /* Call early in block */
    r1 = r1 ^ r2;
    another_external(r1);
    result = r1 * r2;
    goto end;

label3:
    r2 += 20;
    another_external(r2);
    r1 -= 10;
    external_func();  /* Call near end of block */
    result = r1 | r2;
    /* Fall through to end */

end:
    return result;
}

/* Main driver that calls all scenarios */
int main(void) {
    int total = 0;
    
    printf("Testing caller-save coverage scenarios...\n");
    
    /* Call each scenario multiple times with different inputs */
    for (int i = 0; i < 3; i++) {
        total += scenario1();
        total += scenario2(i);
        total += scenario3(i * 10);
        
        /* Test setjmp scenario */
        total += scenario4(i * 5);
        
        /* Test computed goto */
        total += scenario5(i);
        
        /* Trigger longjmp occasionally */
        if (i == 1) {
            longjmp(jump_buffer, 1);
        }
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
