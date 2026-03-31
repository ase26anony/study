/* sched_context_test.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 * Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline, returns_twice));
float opaque_float(float x) __attribute__((noinline, returns_twice));
double opaque_double(double x) __attribute__((noinline, returns_twice));

int opaque_int(int x) {
    volatile int sink = x;
    /* Memory clobber to force scheduling barrier */
    asm volatile ("" : : : "memory");
    return sink;
}

float opaque_float(float x) {
    volatile float sink = x;
    asm volatile ("" : : : "memory");
    return sink;
}

double opaque_double(double x) {
    volatile double sink = x;
    asm volatile ("" : : : "memory");
    return sink;
}

/* Helper to create complex data dependencies */
int complex_dependency_chain(volatile int a, volatile int b, volatile int c, 
                            volatile float d, volatile double e) __attribute__((noinline));

int complex_dependency_chain(volatile int a, volatile int b, volatile int c,
                            volatile float d, volatile double e) {
    /* Long chain of mixed-type operations */
    int t1 = a + b;
    float t2 = d * (float)t1;
    int t3 = t1 ^ c;
    double t4 = e + (double)t2;
    int t5 = t3 * (int)t4;
    float t6 = t2 / (d + 1.0f);
    int t7 = t5 - (int)(t4 * 100.0);
    double t8 = t4 - (double)t6;
    int t9 = t7 ^ (int)t8;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    return opaque_int(t9);
}

/* State machine with computed goto */
void scheduling_stress(void) __attribute__((noinline, noipa));

void scheduling_stress(void) {
    volatile int state = 0;
    volatile int counter = 0;
    volatile int var1 = 1, var2 = 2, var3 = 3;
    volatile float fvar1 = 1.5f, fvar2 = 2.5f;
    volatile double dvar1 = 3.14159;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = i * 3;
    }
    
    /* Labels for computed goto state machine */
    static void* states[] = {
        &&state0, &&state1, &&state2, &&state3, &&state4,
        &&state5, &&state6, &&state7, &&state8, &&state9
    };
    
    /* Outer loop - forces scheduler to handle loop boundaries */
    for (int outer = 0; outer < 50; outer++) {
        /* Update state based on complex condition */
        state = (var1 ^ var2 + outer) % 10;
        
        /* Jump to state */
        goto *states[state];
        
    state0:
        /* Chain of dependent operations */
        var1 = var1 + var2;
        var2 = var2 * var3;
        fvar1 = fvar1 * 1.1f;
        var3 = var1 ^ var2;
        dvar1 = dvar1 + (double)fvar1;
        var1 = var3 - var1;
        fvar2 = fvar1 / fvar2;
        var2 = opaque_int(var2 + 1);
        asm volatile ("" : : : "memory");
        goto state_end;
        
    state1:
        var2 = var2 << 2;
        var3 = var3 | 0x55;
        fvar1 = fvar1 + 2.0f;
        var1 = var2 * var3;
        dvar1 = dvar1 * 1.01;
        fvar2 = opaque_float(fvar2 * 0.9f);
        var3 = var1 >> 1;
        asm volatile ("" : : : "memory");
        goto state_end;
        
    state2:
        var3 = var3 + var1;
        var1 = var1 & 0xFF;
        fvar2 = fvar2 - 0.5f;
        var2 = var3 ^ var1;
        dvar1 = opaque_double(dvar1 - 0.1);
        var1 = var2 + var3;
        asm volatile ("" : : : "memory");
        goto state_end;
        
    state3:
    state4:
    state5:
    state6:
    state7:
    state8:
    state9:
        /* Similar patterns for other states */
        var1 = complex_dependency_chain(var1, var2, var3, fvar1, dvar1);
        var2 = opaque_int(var2 + var3);
        var3 = var1 * var2;
        fvar1 = opaque_float(fvar1 * fvar2);
        dvar1 = dvar1 + (double)var1;
        asm volatile ("" : : : "memory");
        goto state_end;
        
    state_end:
        /* Inner loop with array accesses using volatile indices */
        volatile int idx = counter % 16;
        for (int inner = 0; inner < 10; inner++) {
            /* Create memory dependencies */
            arr[idx] = arr[(idx + 1) % 16] + var1;
            idx = (idx + 3) % 16;
            arr[idx] = arr[(idx + 2) % 16] * var2;
            idx = (idx + 5) % 16;
            
            /* Mix in floating point operations */
            fvar1 = fvar1 + (float)arr[idx];
            dvar1 = dvar1 * (1.0 + (double)inner * 0.01);
        }
        
        /* Update counter with complex condition */
        counter = opaque_int(counter + (var1 & 0x3) + (var2 >> 4));
        
        /* Force scheduling boundary */
        asm volatile ("" : : : "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    volatile int sink = var1 + var2 + var3 + (int)fvar1 + (int)fvar2 + (int)dvar1;
    for (int i = 0; i < 16; i++) {
        sink += arr[i];
    }
    
    /* Final memory barrier */
    asm volatile ("" : : : "memory");
}

/* Main driver */
int main(void) {
    srand(time(NULL));
    
    int checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        scheduling_stress();
        
        /* Vary conditions slightly each iteration */
        volatile int r = rand() % 100;
        checksum += r;
        
        /* Scheduling barrier between calls */
        asm volatile ("" : : : "memory");
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
