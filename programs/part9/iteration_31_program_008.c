/* sched_context_test.c
 * Designed to trigger scheduler context saving/freeing logic in haifa-sched.cc
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer sched_context_test.c -o sched_test
 * Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops sched_context_test.c -o sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int __attribute__((noinline)) opaque_int(int x) {
    volatile int dummy = x;
    asm volatile ("" : "+r" (dummy));
    return dummy;
}

float __attribute__((noinline)) opaque_float(float x) {
    volatile float dummy = x;
    asm volatile ("" : "+f" (dummy));
    return dummy;
}

double __attribute__((noinline)) opaque_double(double x) {
    volatile double dummy = x;
    asm volatile ("" : "+f" (dummy));
    return dummy;
}

/* Memory barrier function to force scheduler state save */
void __attribute__((noinline)) scheduling_barrier(void) {
    asm volatile ("" ::: "memory");
}

/* Complex helper that creates scheduling region boundaries */
int __attribute__((noinline)) complex_helper(int a, int b, float c, double d) {
    volatile int v1 = a + b;
    volatile float v2 = c * 1.5f;
    volatile double v3 = d / 2.0;
    
    /* Mixed-type dependency chain */
    int r1 = (int)(v2 * 10.0f);
    float r2 = (float)(v3 * 2.0);
    double r3 = (double)(v1 * 3);
    
    /* Force memory dependencies */
    volatile int array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = (v1 + i) * (r1 - i);
    }
    
    scheduling_barrier();
    
    return r1 + (int)r2 + (int)r3 + array[3];
}

/* State machine with complex control flow */
void __attribute__((noinline)) state_machine(volatile int *state, 
                                           volatile int *counter,
                                           volatile float *fstate,
                                           volatile double *dstate) {
    static void* labels[] = {
        &&state0, &&state1, &&state2, &&state3, &&state4,
        &&state5, &&state6, &&state7, &&state8, &&state9
    };
    
    goto *labels[*state % 10];
    
state0:
    {
        /* Long dependency chain with mixed types */
        volatile int a = *counter + 1;
        volatile float b = *fstate * 2.0f;
        volatile double c = *dstate / 1.5;
        
        int t1 = a * 3;
        float t2 = b + (float)c;
        double t3 = c * (double)t1;
        int t4 = (int)t2 + (int)t3;
        float t5 = t2 * (float)t4;
        double t6 = t3 + (double)t5;
        int t7 = t4 ^ (int)t6;
        
        *counter = opaque_int(t7);
        *fstate = opaque_float((float)t6);
        *dstate = opaque_double(t6 * 0.9);
        
        /* Force scheduling boundary */
        complex_helper(t1, t7, t2, t6);
        
        *state = (*counter & 0x7) + 1;
        scheduling_barrier();
        return;
    }
    
state1:
    {
        volatile int x = *counter * 2;
        volatile float y = *fstate + 1.0f;
        
        for (int i = 0; i < 5; i++) {
            x = (x << 1) | (x >> 31);  /* Rotate */
            y = y * 1.1f + (float)i;
            x = x ^ (int)(y * 100.0f);
        }
        
        *counter = x;
        *fstate = y;
        *state = (x % 9) + 2;
        scheduling_barrier();
        return;
    }
    
state2:
    {
        /* Memory-intensive operations */
        volatile int arr[16];
        volatile int idx = *counter & 0xF;
        
        for (int i = 0; i < 16; i++) {
            arr[i] = (*counter + i) * (i + 1);
        }
        
        int sum = 0;
        for (int i = 0; i < 16; i++) {
            sum += arr[i] - arr[(i + 1) & 0xF];
        }
        
        *counter = sum;
        *state = (sum & 0x3) + 3;
        
        /* Use architecture-specific builtin if available */
        #ifdef __x86_64__
        asm volatile ("rdtsc" : "=a" (idx) : : "rdx");
        *counter ^= idx;
        #endif
        
        scheduling_barrier();
        return;
    }
    
state3:
state4:
state5:
state6:
state7:
state8:
state9:
    {
        /* Similar patterns for other states */
        volatile int base = *state * 100 + *counter;
        volatile float fbase = (float)base / 3.0f;
        
        int chain = base;
        for (int i = 0; i < 8; i++) {
            chain = chain * 3 - i;
            fbase = fbase * 1.5f - (float)chain;
            chain = chain ^ (int)(fbase * 10.0f);
        }
        
        *counter = chain;
        *fstate = fbase;
        *state = (chain % 7) + ((*state + 1) % 10);
        scheduling_barrier();
        return;
    }
}

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int state = seed & 0xF;
    volatile int counter = seed;
    volatile float fstate = (float)seed / 3.0f;
    volatile double dstate = (double)seed / 2.5;
    
    volatile int checksum = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Call state machine multiple times */
        for (int inner = 0; inner < 10; inner++) {
            state_machine(&state, &counter, &fstate, &dstate);
            
            /* Update checksum with volatile values */
            checksum ^= counter + (int)fstate + (int)dstate;
            checksum = (checksum << 3) | (checksum >> 29);  /* Rotate */
        }
        
        /* Array access with volatile indices */
        volatile int indices[4] = {0, 1, 2, 3};
        volatile int data[8];
        
        for (int i = 0; i < 8; i++) {
            data[i] = counter + i * outer;
        }
        
        /* Complex memory dependency chain */
        for (int i = 0; i < 4; i++) {
            int idx1 = indices[i] & 0x7;
            int idx2 = indices[(i + 1) & 0x3] & 0x7;
            data[idx1] = data[idx1] * 3 - data[idx2];
            checksum += data[idx1];
        }
        
        /* Mixed-type operations */
        fstate = fstate * 1.25f + (float)outer;
        dstate = dstate * 0.95 + (double)counter / 100.0;
        
        /* Force scheduling boundary periodically */
        if ((outer & 0x7) == 0) {
            complex_helper(counter, checksum, fstate, dstate);
        }
    }
    
    /* Final complex computation */
    volatile int final = checksum;
    for (int i = 0; i < 5; i++) {
        final = final * 7 + (int)(fstate * 10.0f);
        final = final ^ (int)(dstate * 100.0);
        scheduling_barrier();
    }
    
    return final;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduler context test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int rep = 0; rep < 100; rep++) {
        int seed = rand() ^ (rep * 0x12345);
        int result = scheduling_stress(seed);
        total_checksum ^= result;
        
        /* Print progress occasionally */
        if ((rep % 25) == 0) {
            printf("Iteration %d: result = %d\n", rep, result);
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return total_checksum & 0xFF;
}
