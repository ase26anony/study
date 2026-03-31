/* caller_save_test.c - Test program for GCC caller-save optimization pass */
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions to force calls */
void __attribute__((noinline)) external_func(void) {
    /* Empty but non-inline to force call boundary */
    volatile int sink = 0;
    (void)sink;
}

int __attribute__((noinline)) external_func_with_return(void) {
    volatile int x = 42;
    return x;
}

/* Global jump buffer for setjmp/longjmp scenario */
static jmp_buf jump_buffer;

/* ===== SCENARIO 1: Explicit register variables with loop ===== */
int __attribute__((noinline)) scenario1(void) {
    /* Force use of specific call-clobbered registers on x86 */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ecx") = 2;
    register int c __asm__ ("edx") = 3;
    register int d __asm__ ("esi") = 4;
    
    volatile int result = 0;
    
    /* Complex loop with multiple live values across call */
    for (int i = 0; i < 10; ++i) {
        /* Arithmetic on register variables - must stay live */
        a += i * 2;
        b -= i + 1;
        c ^= (a + b);
        d = d * 3 - i;
        
        /* Function call clobbers registers - forces save/restore */
        external_func();
        
        /* More arithmetic after call - values must be restored */
        a += c;
        b ^= d;
        c = c + a - b;
        d = d * 2 + i;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            external_func();
            result += a;
        } else if (i % 3 == 1) {
            result += b;
        } else {
            external_func();
            result += c + d;
        }
    }
    
    /* Final computation mixing all register variables */
    return result + a + b + c + d;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int sum = 0;
    
    /* Multiple calls with volatile variables used before and after */
    v1 = v2 + v3;
    external_func();
    v4 = v1 * v2;
    
    v5 = v6 - v7;
    int ret = external_func_with_return();
    v8 = v5 + ret;
    
    /* Nested conditionals creating complex basic blocks */
    for (int i = 0; i < 5; ++i) {
        if (v1 > v2) {
            v3 = v4 * v5;
            external_func();
            v6 = v7 + v8;
            if (v9 < v10) {
                v9 = v10 - i;
                external_func();
            }
            /* This creates a basic block ending with call+jump */
            if (i % 2 == 0) {
                external_func();
                goto loop_end;
            }
        }
        v10 = v9 + i;
loop_end:
        sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
    
    return sum;
}

/* ===== SCENARIO 3: Nested calls with register pressure ===== */
int __attribute__((noinline)) scenario3_inner(int x, int y) {
    /* Inner function that uses/clobbers registers */
    register int t1 __asm__ ("eax") = x;
    register int t2 __asm__ ("ecx") = y;
    t1 = t1 * t2 + 123;
    external_func();
    return t1 - t2;
}

int __attribute__((noinline)) scenario3(void) {
    int result = 0;
    
    /* Outer calls with arguments in registers */
    for (int i = 0; i < 8; ++i) {
        register int arg1 __asm__ ("eax") = i * 2;
        register int arg2 __asm__ ("ecx") = i * 3 + 1;
        
        /* Arguments must be preserved across call setup */
        arg1 = arg1 + 5;
        arg2 = arg2 - 3;
        
        /* Nested call - inner function needs registers too */
        int inner_result = scenario3_inner(arg1, arg2);
        
        /* Results must be preserved */
        result += inner_result + arg1 + arg2;
        
        /* Switch statement creates basic blocks with jumps at end */
        switch (i % 4) {
            case 0:
                external_func();
                result += 10;
                break;  /* Creates jump at BB end */
            case 1:
                result += 20;
                external_func();
                break;
            case 2:
                external_func();
                result += 30;
                external_func();  /* Call near BB end */
                break;
            default:
                result += 40;
                break;
        }
    }
    
    return result;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
int __attribute__((noinline)) scenario4(void) {
    volatile int x = 0, y = 0, z = 0;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        x = 1;
        y = 2;
        z = 3;
        
        /* Function call that might be saved across longjmp */
        external_func();
        
        x = x + y + z;
        result = x * 10;
        
        /* Simulate error - jump back */
        longjmp(jump_buffer, 1);
    } else {
        /* After longjmp */
        x = 10;
        y = 20;
        
        /* More calls with register pressure */
        register int a __asm__ ("eax") = x;
        register int b __asm__ ("ecx") = y;
        
        for (int i = 0; i < 3; ++i) {
            a += i;
            b -= i;
            external_func();  /* Must save/restore across call */
            result += a + b;
            
            /* Conditional goto creates BB with call at end */
            if (i == 1) {
                external_func();
                goto special_case;
            }
            a = b * 2;
        }
        
special_case:
        result += 1000;
    }
    
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(void) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3 };
    int result = 0;
    int counter = 0;
    
    register int r1 __asm__ ("eax") = 100;
    register int r2 __asm__ ("ecx") = 200;
    
label0:
    r1 += 1;
    external_func();
    r2 -= 5;
    result += r1 + r2;
    if (counter++ >= 10) goto end;
    goto *labels[(r1 + r2) % 4];
    
label1:
    r1 *= 2;
    external_func();  /* Call in middle of computed goto block */
    r2 /= 2;
    result += r1 - r2;
    if (counter++ >= 10) goto end;
    goto *labels[(r1 * r2) % 4];
    
label2:
    r1 ^= r2;
    external_func();
    r2 ^= r1;
    result += r1 | r2;
    if (counter++ >= 10) goto end;
    goto *labels[(r1 ^ r2) % 4];
    
label3:
    r1 = r1 + r2;
    external_func();
    r2 = r1 - r2;
    result += r1 & r2;
    if (counter++ >= 10) goto end;
    goto *labels[(r1 - r2) % 4];
    
end:
    return result;
}

/* ===== Main function to drive all scenarios ===== */
int main(void) {
    int total = 0;
    
    printf("Running caller-save test scenarios...\n");
    
    total += scenario1();
    printf("Scenario 1 complete\n");
    
    total += scenario2();
    printf("Scenario 2 complete\n");
    
    total += scenario3();
    printf("Scenario 3 complete\n");
    
    total += scenario4();
    printf("Scenario 4 complete\n");
    
    total += scenario5();
    printf("Scenario 5 complete\n");
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = total;
    (void)sink;
    
    return 0;
}
