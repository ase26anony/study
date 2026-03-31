/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    /* Inline assembly with memory clobber to force scheduler context save */
    asm volatile ("" : : : "memory");
}

/* State machine labels for computed goto */
static void* state_labels[] = {
    &&STATE_0, &&STATE_1, &&STATE_2, &&STATE_3, &&STATE_4,
    &&STATE_5, &&STATE_6, &&STATE_7, &&STATE_8, &&STATE_9
};

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    volatile float f1 = seed * 0.5f;
    volatile float f2 = seed * 1.5f;
    volatile double d1 = seed * 0.25;
    volatile double d2 = seed * 0.75;
    
    volatile int state = seed % 10;
    volatile int counter = 0;
    
    /* Local array with volatile accesses */
    int array[16];
    for (int i = 0; i < 16; i++) {
        array[i] = i * seed;
    }
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow using computed goto */
        goto *state_labels[state];
        
        STATE_0: {
            /* Chain of dependent integer operations */
            v1 = v2 + v3;
            v2 = v1 * v4;
            v3 = v2 >> 2;
            v4 = v3 ^ v1;
            v1 = v4 - v3;
            v2 = v1 * 7;
            v3 = v2 / 3;
            v4 = v3 | 0xFF;
            
            /* Mixed type operations */
            f1 = (float)v1 * 0.1f;
            f2 = f1 + (float)v2 * 0.2f;
            d1 = (double)f1 * 0.3;
            d2 = d1 + (double)f2 * 0.4;
            
            /* Call to noinline function - scheduling boundary */
            v1 = opaque_int(v1);
            scheduling_barrier();
            
            /* Update state with complex condition */
            state = (v1 + v2 + v3 + v4) % 10;
            continue;
        }
        
        STATE_1: {
            /* Different chain of operations */
            v1 = v3 * v4;
            v2 = v1 + 12345;
            v3 = v2 & 0xFFFF;
            v4 = v3 << 3;
            v1 = v4 / 5;
            v2 = v1 ^ 0xAAAA;
            v3 = v2 * 11;
            v4 = v3 - 999;
            
            f1 = (float)v4 * 1.1f;
            f2 = f1 - (float)v3 * 0.9f;
            d1 = (double)v2 * 1.23;
            d2 = d1 / 2.34;
            
            f1 = opaque_float(f1);
            scheduling_barrier();
            
            state = (v1 * 2 + v2 * 3) % 10;
            continue;
        }
        
        STATE_2: {
            v1 = v4 ^ v2;
            v2 = v1 | v3;
            v3 = v2 + 777;
            v4 = v3 * 13;
            v1 = v4 % 17;
            v2 = v1 << 1;
            v3 = v2 >> 2;
            v4 = v3 + 4321;
            
            d1 = (double)v1 * 3.14;
            d2 = d1 + (double)v2 * 2.71;
            f1 = (float)d1 * 0.618f;
            f2 = f1 - (float)d2 * 0.382f;
            
            d1 = opaque_double(d1);
            scheduling_barrier();
            
            state = (v3 + v4 * 2) % 10;
            continue;
        }
        
        /* Additional states with similar patterns */
        STATE_3: {
            v1 = v2 * v3 + v4;
            v2 = v1 / 3;
            v3 = v2 ^ 0x1234;
            v4 = v3 << 2;
            v1 = v4 - 888;
            v2 = v1 * 19;
            v3 = v2 % 23;
            v4 = v3 | 0x8888;
            
            f1 = (float)v1 * 2.5f;
            f2 = (float)v2 * 3.5f;
            d1 = (double)v3 * 4.5;
            d2 = (double)v4 * 5.5;
            
            v1 = opaque_int(v1);
            scheduling_barrier();
            
            state = (v1 + v3) % 10;
            continue;
        }
        
        STATE_4: {
            v1 = v3 + v4 * 2;
            v2 = v1 ^ 0xABCD;
            v3 = v2 >> 1;
            v4 = v3 * 31;
            v1 = v4 % 29;
            v2 = v1 + 6666;
            v3 = v2 & 0x7777;
            v4 = v3 << 4;
            
            f1 = (float)v2 * 0.333f;
            f2 = (float)v3 * 0.667f;
            d1 = (double)v4 * 1.234;
            d2 = (double)v1 * 5.678;
            
            f2 = opaque_float(f2);
            scheduling_barrier();
            
            state = (v2 * 3 + v4) % 10;
            continue;
        }
        
        STATE_5: {
            v1 = v4 - v2;
            v2 = v1 * v3;
            v3 = v2 + 1111;
            v4 = v3 / 7;
            v1 = v4 ^ 0xEEEE;
            v2 = v1 << 3;
            v3 = v2 >> 2;
            v4 = v3 | 0xDDDD;
            
            d1 = (double)v1 * 6.66;
            d2 = (double)v2 * 7.77;
            f1 = (float)v3 * 8.88f;
            f2 = (float)v4 * 9.99f;
            
            d2 = opaque_double(d2);
            scheduling_barrier();
            
            state = (v1 + v2 + v3 + v4) % 10;
            continue;
        }
        
        STATE_6: {
            v1 = v2 | v3;
            v2 = v1 ^ v4;
            v3 = v2 * 41;
            v4 = v3 % 37;
            v1 = v4 + 2222;
            v2 = v1 << 1;
            v3 = v2 >> 3;
            v4 = v3 - 3333;
            
            f1 = (float)v1 * 1.11f;
            f2 = (float)v2 * 2.22f;
            d1 = (double)v3 * 3.33;
            d2 = (double)v4 * 4.44;
            
            v3 = opaque_int(v3);
            scheduling_barrier();
            
            state = (v1 * v2) % 10;
            continue;
        }
        
        STATE_7: {
            v1 = v3 ^ v4;
            v2 = v1 & 0xBBBB;
            v3 = v2 + 4444;
            v4 = v3 * 43;
            v1 = v4 / 11;
            v2 = v1 ^ 0xCCCC;
            v3 = v2 << 2;
            v4 = v3 >> 1;
            
            f1 = (float)v4 * 5.55f;
            f2 = (float)v3 * 6.66f;
            d1 = (double)v2 * 7.77;
            d2 = (double)v1 * 8.88;
            
            f1 = opaque_float(f1);
            scheduling_barrier();
            
            state = (v3 + v4 * 4) % 10;
            continue;
        }
        
        STATE_8: {
            v1 = v4 + v2 * 3;
            v2 = v1 % 31;
            v3 = v2 ^ 0xDDDD;
            v4 = v3 << 3;
            v1 = v4 - 5555;
            v2 = v1 * 47;
            v3 = v2 / 13;
            v4 = v3 | 0xEEEE;
            
            d1 = (double)v1 * 9.99;
            d2 = (double)v2 * 10.10;
            f1 = (float)v3 * 11.11f;
            f2 = (float)v4 * 12.12f;
            
            d1 = opaque_double(d1);
            scheduling_barrier();
            
            state = (v2 + v3 * 2) % 10;
            continue;
        }
        
        STATE_9: {
            v1 = v2 * v4;
            v2 = v1 + v3;
            v3 = v2 ^ 0xFFFF;
            v4 = v3 >> 2;
            v1 = v4 * 53;
            v2 = v1 % 41;
            v3 = v2 << 1;
            v4 = v3 - 7777;
            
            f1 = (float)v1 * 13.13f;
            f2 = (float)v2 * 14.14f;
            d1 = (double)v3 * 15.15;
            d2 = (double)v4 * 16.16;
            
            v4 = opaque_int(v4);
            scheduling_barrier();
            
            state = (v1 * 3 + v3) % 10;
            continue;
        }
        
        /* Inner loop with array accesses using volatile indices */
        for (int inner = 0; inner < 10; inner++) {
            volatile int idx = (v1 + inner) % 16;
            array[idx] = array[idx] + v2;
            idx = (v3 + inner * 2) % 16;
            array[idx] = array[idx] - v4;
            idx = (v2 + inner * 3) % 16;
            array[idx] = array[idx] * 2;
        }
        
        counter++;
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = v1 + v2 + v3 + v4;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    for (int i = 0; i < 16; i++) {
        checksum += array[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        int result = scheduling_stress(seed);
        total_checksum += result;
        
        /* Print progress occasionally */
        if (i % 20 == 0) {
            printf("Iteration %d: checksum = %d\n", i, result);
        }
    }
    
    printf("Final total checksum: %d\n", total_checksum);
    return 0;
}
