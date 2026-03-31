/* caller_save_coverage.c
 * Designed to trigger GCC's caller-save pass instruction list manipulation
 * Specifically targets lines 905-913 in caller-save.cc
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void external_func(void) __attribute__((noinline));
void another_external(int) __attribute__((noinline));
int returns_value(void) __attribute__((noinline));

/* Dummy implementations to satisfy linker */
void external_func(void) {
    volatile int sink = 0;
    (void)sink;
}

void another_external(int x) {
    volatile int sink = x;
    (void)sink;
}

int returns_value(void) {
    return 42;
}

/* Scenario 1: Explicit register variables with loop */
__attribute__((noinline, optimize("O2")))
int scenario1(void) {
    /* Force use of call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    volatile int prevent_opt = 0;
    
    for (int i = 0; i < 10; ++i) {
        /* Complex arithmetic to create register pressure */
        a = a * i + b;
        b = b - i * c;
        c = c ^ (a + d);
        d = d + (b >> 2);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More operations after call requiring same values */
        a = a + (c & 0xFF);
        b = b * (d | 1);
        
        /* Conditional to create basic block structure */
        if (i & 1) {
            c = c + returns_value();
        }
        
        prevent_opt += a + b + c + d;
    }
    
    return a + b + c + d + prevent_opt;
}

/* Scenario 2: Many volatile variables across calls */
__attribute__((noinline, optimize("O3")))
int scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    
    /* Multiple calls with volatile usage in between */
    for (int i = 0; i < 5; ++i) {
        v1 = v1 * v2 + v3;
        v2 = v2 - v4 * v5;
        external_func();
        v3 = v3 ^ (v1 + v6);
        v4 = v4 + (v2 >> v7);
        another_external(v8);
        v5 = v5 & (v9 | v10);
        v6 = v6 + returns_value();
        
        /* This creates a basic block ending with a call and jump */
        if (v1 > v2) {
            external_func();
            goto update;
        }
        
        v7 = v7 * 2;
        
    update:
        v8 = v8 + 1;
    }
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Scenario 3: Nested calls with register dependencies */
__attribute__((noinline, optimize("O2")))
int scenario3(void) {
    int result = 0;
    
    for (int i = 0; i < 8; ++i) {
        /* Outer call setup in registers */
        int arg1 = i * 2;
        int arg2 = i * 3;
        int arg3 = i * 5;
        
        /* These should end up in registers */
        register int r1 __asm__ ("eax") = arg1;
        register int r2 __asm__ ("ecx") = arg2;
        register int r3 __asm__ ("edx") = arg3;
        
        /* Inner call that uses register values */
        another_external(r1 + r2);
        
        /* More computation */
        r1 = r1 * r3;
        r2 = r2 + returns_value();
        
        /* Another call */
        external_func();
        
        /* Use results */
        result += r1 + r2 + r3;
        
        /* Switch statement to create complex basic block ends */
        switch (i % 3) {
            case 0:
                r1 = r1 * 2;
                external_func();
                break;  /* Creates jump at BB end */
            case 1:
                r2 = r2 + 1;
                another_external(r3);
                break;
            default:
                r3 = r3 - 1;
                external_func();
                break;
        }
    }
    
    return result;
}

/* Scenario 4: setjmp/longjmp with calls */
__attribute__((noinline, optimize("O2")))
int scenario4(void) {
    jmp_buf env;
    volatile int counter = 0;
    register int a __asm__ ("eax") = 100;
    register int b __asm__ ("ecx") = 200;
    
    if (setjmp(env) == 0) {
        /* First time through */
        for (int i = 0; i < 5; ++i) {
            a = a + i;
            b = b - i;
            
            /* Call that might longjmp */
            if (i == 3) {
                another_external(a);
            }
            
            external_func();
            
            counter += a + b;
        }
    } else {
        /* After longjmp */
        a = a * 2;
        b = b / 2;
        external_func();
    }
    
    /* Force save/restore around this call */
    another_external(counter);
    
    return a + b + counter;
}

/* Scenario 5: Computed goto with calls */
__attribute__((noinline, optimize("O3")))
int scenario5(void) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    register int x __asm__ ("eax") = 0;
    register int y __asm__ ("ecx") = 1;
    
    for (int i = 0; i < 10; ++i) {
        /* Complex basic block with call and computed goto */
        x = x + i * 2;
        y = y - i;
        
        external_func();
        
        /* Computed goto creates unusual control flow */
        goto *labels[i % 4];
        
    label0:
        x = x * 3;
        another_external(x);
        continue;
        
    label1:
        y = y + returns_value();
        external_func();
        continue;
        
    label2:
        x = x ^ y;
        continue;
        
    label3:
        y = y * 2;
        another_external(y);
        continue;
    }
    
    result = x + y;
    
    /* Final basic block with call at end */
    external_func();
    return result;
}

/* Main function to run all scenarios */
int main(void) {
    int checksum = 0;
    
    printf("Running caller-save coverage scenarios...\n");
    
    checksum += scenario1();
    printf("Scenario 1 complete\n");
    
    checksum += scenario2();
    printf("Scenario 2 complete\n");
    
    checksum += scenario3();
    printf("Scenario 3 complete\n");
    
    checksum += scenario4();
    printf("Scenario 4 complete\n");
    
    checksum += scenario5();
    printf("Scenario 5 complete\n");
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
