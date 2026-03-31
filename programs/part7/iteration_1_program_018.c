/* caller_save_test.c
 * 
 * This program is designed to stress GCC's caller-save optimization pass
 * by creating scenarios where registers must be saved/restored around
 * function calls with complex basic block structures.
 */

#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>

/* External functions that will clobber registers */
void __attribute__((noinline)) external_func(void) {
    /* Empty function that will clobber call-clobbered registers */
    asm volatile ("" : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi");
}

void __attribute__((noinline)) another_external(int x) {
    /* Another function that clobbers registers */
    asm volatile ("" : : "r"(x) : "memory", "eax", "ecx", "edx");
}

int __attribute__((noinline)) returning_external(int x) {
    /* Function that returns a value, clobbering registers */
    asm volatile ("" : "+r"(x) : : "memory", "eax", "ecx");
    return x + 1;
}

/* Global variables to prevent optimizations */
volatile int global_counter = 0;
volatile int global_result = 0;

/* ===== SCENARIO 1: Explicit register variables in a loop ===== */
int __attribute__((noinline)) scenario1(void) {
    int result = 0;
    
    /* Force use of specific call-clobbered registers */
    register int a __asm__ ("eax") = 1;
    register int b __asm__ ("ebx") = 2;
    register int c __asm__ ("ecx") = 3;
    register int d __asm__ ("edx") = 4;
    
    /* Complex loop with register pressure across calls */
    for (int i = 0; i < 20; ++i) {
        /* Do computations in registers before call */
        a = a + i * 2;
        b = b - i / 3;
        c = c ^ (i * 7);
        d = d | (i << 2);
        
        /* Function call that clobbers registers */
        external_func();
        
        /* More computations after call - registers need to be restored */
        result += a * b;
        result -= c ^ d;
        
        /* Conditional that creates basic block boundaries */
        if (i % 3 == 0) {
            a += returning_external(b);
        } else if (i % 5 == 0) {
            b -= returning_external(c);
        }
        
        /* Another call with different register usage */
        another_external(result);
    }
    
    /* Final computation mixing all register variables */
    result = (a + b) * (c - d) + result;
    return result;
}

/* ===== SCENARIO 2: Many volatile variables across calls ===== */
int __attribute__((noinline)) scenario2(void) {
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int result = 0;
    
    /* Complex expression with many volatile accesses around calls */
    for (int i = 0; i < 15; ++i) {
        /* Multiple volatile accesses before call */
        v1 = v1 + v2 * i;
        v3 = v4 - v5 / (i + 1);
        v6 = v7 ^ v8;
        v9 = v10 | (i << 3);
        
        /* Call that forces spills */
        external_func();
        
        /* More volatile accesses after call */
        result += v1 * v3;
        result -= v6 ^ v9;
        
        /* Nested conditional with calls at block ends */
        if (v1 > v2) {
            v2 = returning_external(v3);
            /* This creates a basic block ending with a call */
            if (v3 < v4) {
                v4 = returning_external(v5);
                goto label2;  /* Creates jump at block end */
            }
            v5 = returning_external(v6);
        }
        
    label2:
        /* Switch statement with calls in cases */
        switch (i % 4) {
            case 0:
                v6 = returning_external(v7);
                break;  /* Creates jump at block end */
            case 1:
                v7 = returning_external(v8);
                /* Fall through */
            case 2:
                v8 = returning_external(v9);
                external_func();  /* Call near block end */
                break;
            default:
                v9 = returning_external(v10);
                external_func();  /* Call before break at block end */
                break;
        }
    }
    
    return result + v1 + v2 + v3 + v4 + v5;
}

/* ===== SCENARIO 3: Nested calls with register dependencies ===== */
int __attribute__((noinline)) scenario3(int x) {
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ebx") = x * 2;
    register int r3 __asm__ ("ecx") = x * 3;
    
    /* Outer call setup in registers */
    r1 = r1 + 1;
    r2 = r2 - 2;
    
    /* Nested calls where inner call arguments depend on 
       outer call's register values */
    int result = returning_external(
        returning_external(r1) + 
        returning_external(r2)
    );
    
    /* More register manipulations between calls */
    r3 = r3 ^ result;
    external_func();
    
    /* Complex control flow with calls at block boundaries */
    for (int i = 0; i < 10; ++i) {
        if (r1 > r2) {
            r1 = returning_external(r3);
            /* Basic block ends with call, needs save/restore */
            if (r2 < r3) {
                r2 = returning_external(r1);
                goto loop_end;  /* Jump at block end */
            }
            r3 = returning_external(r2);
        } else {
            r2 = returning_external(r1);
        }
        
        /* Call with computed goto preparation */
        if (i == 5) {
            r1 = r1 * 2;
            external_func();
            /* This creates interesting block structure */
        }
        
    loop_end:
        result += r1 + r2 + r3;
    }
    
    return result;
}

/* ===== SCENARIO 4: setjmp/longjmp with calls ===== */
static jmp_buf jump_buffer;
volatile int jmp_flag = 0;

int __attribute__((noinline)) scenario4(void) {
    register int a __asm__ ("eax") = 100;
    register int b __asm__ ("ebx") = 200;
    register int c __asm__ ("ecx") = 300;
    int result = 0;
    
    if (setjmp(jump_buffer) == 0) {
        /* First time through */
        for (int i = 0; i < 8; ++i) {
            a += i * 11;
            b -= i * 7;
            c ^= i * 13;
            
            /* Call that might be before longjmp */
            external_func();
            
            result += a + b + c;
            
            /* Conditional that might trigger longjmp */
            if (i == 4 && jmp_flag == 0) {
                jmp_flag = 1;
                another_external(result);
                /* setjmp forces conservative register saving */
            }
        }
    } else {
        /* After longjmp - registers need restoration */
        result = a * b / (c + 1);
        external_func();
    }
    
    /* More calls in different basic blocks */
    if (result > 1000) {
        a = returning_external(b);
        goto final_calc;
    } else {
        b = returning_external(c);
    }
    
final_calc:
    result = result + a - b + c;
    return result;
}

/* ===== SCENARIO 5: Computed goto with calls ===== */
int __attribute__((noinline)) scenario5(int x) {
    static void* labels[] = { &&label0, &&label1, &&label2, &&label3 };
    register int r1 __asm__ ("eax") = x;
    register int r2 __asm__ ("ebx") = x * 2;
    int result = 0;
    int i = 0;
    
    /* Computed goto creates unusual control flow */
    goto *labels[x % 4];
    
label0:
    r1 += 10;
    external_func();  /* Call in middle of block with computed goto */
    r2 -= 5;
    goto join;
    
label1:
    r1 *= 2;
    another_external(r1);
    r2 /= 2;
    goto join;
    
label2:
    r1 ^= 0xFF;
    external_func();
    r2 |= 0xAA;
    /* Fall through to label3 */
    
label3:
    r1 += r2;
    returning_external(r1);
    r2 -= r1;
    /* Continue to join */
    
join:
    result = r1 + r2;
    
    /* Loop with calls and complex control flow */
    for (i = 0; i < 12; ++i) {
        if (i % 3 == 0) {
            r1 = returning_external(r2);
            /* Call at block end with following goto */
            if (r1 > 100) goto loop_end;
        } else if (i % 3 == 1) {
            r2 = returning_external(r1);
            external_func();
        } else {
            result += returning_external(result);
        }
        
        /* Additional call that might need save/restore */
        another_external(i);
        
    loop_end:
        result += r1 * r2;
    }
    
    return result;
}

/* ===== SCENARIO 6: Mixed register pressure with complex BB ends ===== */
int __attribute__((noinline)) scenario6(int iterations) {
    int result = 0;
    
    /* Use multiple explicit registers */
    register int a __asm__ ("eax") = iterations;
    register int b __asm__ ("ebx") = iterations * 2;
    register int c __asm__ ("ecx") = iterations * 3;
    register int d __asm__ ("edx") = iterations * 4;
    register int si __asm__ ("esi") = iterations * 5;
    register int di __asm__ ("edi") = iterations * 6;
    
    for (int i = 0; i < iterations; ++i) {
        /* Heavy computation before call */
        a = a + b * c;
        b = b - d / (i + 1);
        c = c ^ si;
        d = d | di;
        si = si + a;
        di = di - b;
        
        /* Function call - all registers need saving */
        external_func();
        
        /* Computation after call */
        result += a + b - c * d + si ^ di;
        
        /* Complex conditional structure with calls at block ends */
        if (a > b) {
            if (c < d) {
                a = returning_external(si);
                /* This creates a basic block ending with call */
                if (si > di) {
                    b = returning_external(di);
                    goto next_iter;  /* Jump at block end */
                }
                c = returning_external(a);
            } else {
                d = returning_external(b);
                external_func();  /* Call before block end */
            }
        } else {
            si = returning_external(c);
            /* Switch inside else branch */
            switch (i % 3) {
                case 0:
                    di = returning_external(d);
                    break;
                case 1:
                    a = returning_external(si);
                    external_func();
                    break;
                default:
                    b = returning_external(di);
                    external_func();  /* Call before break at block end */
                    break;
            }
        }
        
        /* Another call with different register usage */
        if (result % 2 == 0) {
            another_external(result);
        }
        
    next_iter:
        /* Update globals to prevent dead code elimination */
        global_counter++;
    }
    
    /* Final aggregation */
    result = result + a + b + c + d + si + di;
    return result;
}

/* ===== Main function to drive everything ===== */
int main(void) {
    int total = 0;
    
    printf("Starting caller-save stress test...\n");
    
    /* Call each scenario multiple times with different parameters */
    total += scenario1();
    total += scenario2();
    total += scenario3(42);
    
    /* Setup for setjmp scenario */
    jmp_flag = 0;
    total += scenario4();
    
    /* Test longjmp path */
    if (jmp_flag) {
        longjmp(jump_buffer, 1);
    }
    
    total += scenario5(7);
    total += scenario6(8);
    
    /* Add global result */
    total += global_counter;
    
    printf("Final result: %d\n", total);
    printf("Test completed.\n");
    
    return total != 0 ? 0 : 1;
}
