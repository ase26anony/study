/* sched_context_test.c
 * Program designed to trigger scheduler context saving/freeing logic
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
    return dummy;
}

float __attribute__((noinline)) opaque_float(float x) {
    volatile float dummy = x;
    return dummy;
}

double __attribute__((noinline)) opaque_double(double x) {
    volatile double dummy = x;
    return dummy;
}

void __attribute__((noinline)) scheduling_barrier(void) {
    /* Inline assembly with memory clobber to force scheduling boundary */
    asm volatile("" : : : "memory");
}

/* State machine states */
enum states {
    STATE_A, STATE_B, STATE_C, STATE_D, STATE_E,
    STATE_F, STATE_G, STATE_H, STATE_I, STATE_J,
    STATE_K, STATE_L, STATE_M, STATE_N, STATE_O
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int state = STATE_A;
    volatile int counter = 0;
    volatile int a = seed, b = seed + 1, c = seed + 2;
    volatile float f1 = seed * 1.1f, f2 = seed * 2.2f;
    volatile double d1 = seed * 3.3, d2 = seed * 4.4;
    
    /* Local array with volatile accesses */
    volatile int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = seed + i;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex state machine using switch statement */
        switch (state) {
            case STATE_A:
                /* Chain of dependent integer operations */
                a = b + c;
                b = a * 3;
                c = b - a;
                a = c >> 2;
                b = a | 0xFF;
                c = b ^ a;
                a = c * 7;
                b = a % 13;
                c = b + 11;
                
                /* Mixed type operations */
                f1 = (float)a * 1.5f;
                d1 = (double)b * 2.5;
                f2 = f1 + (float)d1;
                d2 = d1 - (double)f2;
                
                /* Call to noinline function - scheduling boundary */
                a = opaque_int(a);
                f1 = opaque_float(f1);
                scheduling_barrier();
                
                /* Update state based on complex condition */
                state = (a & 1) ? STATE_B : STATE_C;
                break;
                
            case STATE_B:
                /* Different chain of operations */
                a = b * c;
                b = a + 255;
                c = b / 3;
                a = c << 1;
                b = a & 0x7F;
                c = b | 0x80;
                a = c - 64;
                b = a * 9;
                c = b % 17;
                
                /* Floating point chain */
                f1 = f2 * 3.14f;
                f2 = f1 / 2.0f;
                d1 = d2 * 1.618;
                d2 = d1 / 3.14159;
                
                /* Memory access with volatile index */
                volatile int idx = (a % 16);
                arr[idx] = arr[(idx + 1) % 16] + b;
                
                a = opaque_int(a + arr[idx]);
                scheduling_barrier();
                
                state = (c > 100) ? STATE_D : STATE_E;
                break;
                
            case STATE_C:
                /* More complex operations */
                a = (b << 3) | (c & 0xF);
                b = (a >> 2) ^ 0xAA;
                c = b + a * 2;
                a = c - b / 3;
                b = a * 11;
                c = b % 19;
                a = c | b;
                b = a ^ c;
                c = b & a;
                
                /* Mixed operations */
                f1 = (float)a / 10.0f;
                d1 = (double)b * 0.1;
                f2 = f1 * f1;
                d2 = d1 * d1;
                
                scheduling_barrier();
                state = STATE_F;
                break;
                
            case STATE_D:
            case STATE_E:
            case STATE_F:
            case STATE_G:
            case STATE_H:
            case STATE_I:
            case STATE_J:
            case STATE_K:
            case STATE_L:
            case STATE_M:
            case STATE_N:
            case STATE_O:
                /* Each state has unique but similar patterns */
                a = a + state * 7;
                b = b - state * 3;
                c = c ^ state;
                
                f1 = f1 + (float)state;
                d1 = d1 - (double)state;
                
                /* Array access pattern */
                for (int i = 0; i < 8; i++) {
                    volatile int idx2 = (i + state) % 16;
                    arr[idx2] = arr[idx2] + a + i;
                }
                
                /* Periodic calls to noinline functions */
                if (counter % 5 == 0) {
                    a = opaque_int(a);
                }
                if (counter % 7 == 0) {
                    f1 = opaque_float(f1);
                    d1 = opaque_double(d1);
                }
                
                scheduling_barrier();
                
                /* State transition */
                state = (state + 1) % (STATE_O + 1);
                break;
        }
        
        /* Inner loop with memory dependencies */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx3 = (inner + a) % 16;
            volatile int idx4 = (inner + b) % 16;
            
            /* Create memory dependency chain */
            arr[idx3] = arr[idx4] + inner;
            arr[idx4] = arr[idx3] - inner;
            
            /* Dependent arithmetic */
            a = a + arr[idx3];
            b = b - arr[idx4];
            c = c ^ (arr[idx3] & arr[idx4]);
            
            /* Floating point operations */
            f1 = f1 + (float)arr[idx3] * 0.01f;
            f2 = f2 - (float)arr[idx4] * 0.01f;
            d1 = d1 + (double)arr[idx3] * 0.001;
            d2 = d2 - (double)arr[idx4] * 0.001;
        }
        
        counter++;
        
        /* Complex state transition condition */
        if (counter % 13 == 0) {
            state = STATE_A;
        } else if ((a + b + c) % 11 == 0) {
            state = STATE_B;
        } else if ((int)(f1 + f2) % 7 == 0) {
            state = STATE_C;
        }
        
        /* Use inline assembly as scheduling barrier every few iterations */
        if (counter % 3 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Compute checksum from all variables */
    int checksum = a + b + c;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    for (int i = 0; i < 16; i++) {
        checksum += arr[i];
    }
    
    return checksum;
}

/* Additional stress function with computed goto */
int __attribute__((noinline)) computed_goto_stress(int seed) {
    volatile int state = 0;
    volatile int x = seed, y = seed * 2, z = seed * 3;
    
    /* Label array for computed goto */
    static void* labels[] = {
        &&label0, &&label1, &&label2, &&label3, &&label4,
        &&label5, &&label6, &&label7, &&label8, &&label9
    };
    
    for (int i = 0; i < 100; i++) {
        /* Jump to label based on state */
        goto *labels[state % 10];
        
        label0:
            x = y + z;
            y = x * 2;
            z = y - x;
            scheduling_barrier();
            state = (x & 1) ? 1 : 2;
            continue;
            
        label1:
            x = z * y;
            y = x >> 1;
            z = y | x;
            asm volatile("" : : : "memory");
            state = (y > 100) ? 3 : 4;
            continue;
            
        label2:
        label3:
        label4:
        label5:
        label6:
        label7:
        label8:
        label9:
            x = x + state;
            y = y - state;
            z = z ^ state;
            
            /* Unrolled operations to expand scheduler queue */
            x = x + 1; y = y + 1; z = z + 1;
            x = x * 2; y = y * 2; z = z * 2;
            x = x - 1; y = y - 1; z = z - 1;
            x = x >> 1; y = y >> 1; z = z >> 1;
            x = x | 0xF; y = y | 0xF; z = z | 0xF;
            
            scheduling_barrier();
            state = (state + 1) % 10;
            continue;
    }
    
    return x + y + z;
}

int main(void) {
    srand(time(NULL));
    int total_checksum = 0;
    
    printf("Starting scheduling stress test...\n");
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        
        /* Call main scheduling stress function */
        int result1 = scheduling_stress(seed);
        total_checksum += result1;
        
        /* Occasionally call computed goto version */
        if (i % 7 == 0) {
            int result2 = computed_goto_stress(seed);
            total_checksum += result2;
        }
        
        /* Use inline assembly periodically */
        if (i % 13 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    printf("Final checksum: %d\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
