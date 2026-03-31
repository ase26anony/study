/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Or: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */
/* Or: gcc -Os -fschedule-insns -fno-crossjumping -fno-optimize-sibling-calls -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Opaque functions to prevent optimization and create scheduling boundaries */
int opaque_int(int x) __attribute__((noinline));
float opaque_float(float x) __attribute__((noinline));
double opaque_double(double x) __attribute__((noinline));
void scheduling_barrier(void) __attribute__((noinline));

/* Helper to force memory dependencies */
void memory_access(int *arr, volatile int idx) __attribute__((noinline));

/* Main scheduling stress function */
int scheduling_stress(int seed) __attribute__((noinline));

/* Opaque functions implementation */
int opaque_int(int x) {
    volatile int temp = x;
    asm volatile ("" : "+r" (temp) : : "memory");
    return temp;
}

float opaque_float(float x) {
    volatile float temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

double opaque_double(double x) {
    volatile double temp = x;
    asm volatile ("" : "+f" (temp) : : "memory");
    return temp;
}

void scheduling_barrier(void) {
    asm volatile ("" : : : "memory");
}

void memory_access(int *arr, volatile int idx) {
    volatile int temp = arr[idx % 16];
    arr[(idx + 1) % 16] = temp * 2 + 1;
    asm volatile ("" : : : "memory");
}

/* State machine labels for computed goto */
#define STATE_0 0
#define STATE_1 1
#define STATE_2 2
#define STATE_3 3
#define STATE_4 4
#define STATE_5 5
#define STATE_6 6
#define STATE_7 7
#define STATE_8 8
#define STATE_9 9

int scheduling_stress(int seed) {
    /* Volatile variables to prevent optimization and create dependencies */
    volatile int v1 = seed;
    volatile int v2 = seed * 2 + 1;
    volatile int v3 = seed * 3 + 2;
    volatile int v4 = seed * 4 + 3;
    volatile float f1 = (float)seed / 3.14159f;
    volatile float f2 = (float)seed * 2.71828f;
    volatile double d1 = (double)seed / 1.41421356;
    volatile double d2 = (double)seed * 1.61803399;
    
    /* Local array for memory dependencies */
    int arr[16];
    for (int i = 0; i < 16; i++) {
        arr[i] = (i + seed) % 256;
    }
    
    /* State machine implementation with computed goto */
    static void *state_labels[] = {
        &&state_0, &&state_1, &&state_2, &&state_3, &&state_4,
        &&state_5, &&state_6, &&state_7, &&state_8, &&state_9
    };
    
    int state = STATE_0;
    volatile int loop_counter = 0;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Complex control flow based on volatile variables */
        if (opaque_int(v1) % 7 == 0) {
            state = (state + 1) % 10;
        } else if (opaque_int(v2) % 11 == 0) {
            state = (state + 2) % 10;
        } else {
            state = opaque_int(v3) % 10;
        }
        
        /* Jump to current state */
        goto *state_labels[state];
        
    state_0:
        /* Chain of dependent integer operations */
        v1 = v2 + v3;
        v2 = v1 * v4;
        v3 = v2 >> (v4 & 0x7);
        v4 = v3 - v1;
        v1 = v4 ^ v2;
        v2 = v1 | v3;
        v3 = v2 & v4;
        v4 = v3 * 1103515245 + 12345;
        
        /* Mixed type operations */
        f1 = (float)v1 / 3.14159f;
        f2 = f1 * (float)v2;
        d1 = (double)f2 * 1.41421356;
        d2 = d1 + (double)v3;
        
        /* Scheduling boundary */
        v1 = opaque_int(v1);
        scheduling_barrier();
        goto state_end;
        
    state_1:
        /* Different chain of operations */
        v2 = v3 * v4;
        v3 = v2 + v1;
        v4 = v3 - v2;
        v1 = v4 ^ v3;
        v2 = v1 << (v3 & 0x3);
        v3 = v2 | v4;
        v4 = v3 & v1;
        v1 = v4 * 1664525 + 1013904223;
        
        f2 = (float)v2 * 2.71828f;
        f1 = f2 / (float)v3;
        d2 = (double)f1 / 1.61803399;
        d1 = d2 - (double)v4;
        
        v2 = opaque_int(v2);
        scheduling_barrier();
        goto state_end;
        
    state_2:
        v3 = v4 + v1;
        v4 = v3 * v2;
        v1 = v4 >> (v2 & 0x5);
        v2 = v1 - v3;
        v3 = v2 ^ v4;
        v4 = v3 | v1;
        v1 = v4 & v2;
        v2 = v1 * 1103515245 + 12345;
        
        f1 = (float)v3 / 3.14159f;
        f2 = f1 * (float)v4;
        d1 = (double)f2 * 1.41421356;
        d2 = d1 + (double)v1;
        
        v3 = opaque_int(v3);
        scheduling_barrier();
        goto state_end;
        
    /* Additional states follow similar patterns but with different operations */
    state_3:
        v4 = v1 * v2;
        v1 = v4 + v3;
        v2 = v1 - v4;
        v3 = v2 ^ v1;
        v4 = v3 << (v1 & 0x7);
        v1 = v4 | v2;
        v2 = v1 & v3;
        v3 = v2 * 1664525 + 1013904223;
        
        f2 = (float)v4 * 2.71828f;
        f1 = f2 / (float)v1;
        d2 = (double)f1 / 1.61803399;
        d1 = d2 - (double)v2;
        
        v4 = opaque_int(v4);
        scheduling_barrier();
        goto state_end;
        
    state_4:
        v1 = v2 + v3;
        v2 = v1 * v4;
        v3 = v2 >> (v4 & 0x3);
        v4 = v3 - v1;
        v1 = v4 ^ v2;
        v2 = v1 | v3;
        v3 = v2 & v4;
        v4 = v3 * 1103515245 + 12345;
        
        f1 = opaque_float(f1);
        f2 = opaque_float(f2);
        d1 = opaque_double(d1);
        d2 = opaque_double(d2);
        
        scheduling_barrier();
        goto state_end;
        
    /* States 5-9 follow similar patterns with variations */
    state_5:
    case_5:
        /* Using switch statement as alternative control flow */
        v2 = v3 * v4;
        v3 = v2 + v1;
        v4 = v3 - v2;
        v1 = v4 ^ v3;
        for (int i = 0; i < 5; i++) {
            v2 = v1 + i;
            v3 = v2 * (i + 1);
        }
        scheduling_barrier();
        goto state_end;
        
    state_6:
    case_6:
        v3 = v4 + v1;
        v4 = v3 * v2;
        v1 = v4 >> 2;
        /* Memory dependencies with volatile index */
        memory_access(arr, v1);
        scheduling_barrier();
        goto state_end;
        
    state_7:
    case_7:
        v4 = v1 * v2;
        v1 = v4 + v3;
        /* Inline assembly with memory clobber */
        asm volatile (
            "movl %0, %%eax\n"
            "imull %1, %%eax\n"
            "movl %%eax, %0\n"
            : "+r" (v2) : "r" (v3) : "%eax", "memory"
        );
        scheduling_barrier();
        goto state_end;
        
    state_8:
    case_8:
        /* Mixed operations */
        v1 = v2 + v3;
        f1 = (float)v1 / 3.14159f;
        d1 = (double)f1 * 2.0;
        v2 = (int)d1;
        v3 = v2 * v4;
        scheduling_barrier();
        goto state_end;
        
    state_9:
    case_9:
        /* Long dependency chain */
        v4 = v1;
        for (int i = 0; i < 8; i++) {
            v4 = v4 * 1103515245 + 12345;
            v1 = v4 ^ v2;
            v2 = v1 + v3;
            v3 = v2 >> 1;
        }
        scheduling_barrier();
        goto state_end;
        
    state_end:
        /* Inner loop with array accesses and volatile indices */
        volatile int idx = v1 % 16;
        for (int inner = 0; inner < 10; inner++) {
            arr[idx] = arr[(idx + 1) % 16] + arr[(idx + 2) % 16];
            idx = (idx + arr[idx] % 3) % 16;
            /* Create memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Update loop counter based on complex condition */
        loop_counter = opaque_int(loop_counter + 1);
        if ((v1 ^ v2 ^ v3 ^ v4) % 13 == 0) {
            state = (state + opaque_int(v1)) % 10;
        }
        
        /* Alternative switch-based state machine (creates more basic blocks) */
        if (outer % 3 == 0) {
            switch (opaque_int(v2) % 10) {
                case 0: goto case_5;
                case 1: goto case_6;
                case 2: goto case_7;
                case 3: goto case_8;
                case 4: goto case_9;
                default: break;
            }
        }
    }
    
    /* Compute checksum from all volatile variables */
    int checksum = v1 + v2 + v3 + v4;
    checksum += (int)f1 + (int)f2;
    checksum += (int)d1 + (int)d2;
    
    for (int i = 0; i < 16; i++) {
        checksum ^= arr[i];
    }
    
    return checksum;
}

int main() {
    srand(time(NULL));
    int total_checksum = 0;
    
    /* Repeated calls to stress the scheduler */
    for (int i = 0; i < 100; i++) {
        int seed = rand() % 1000;
        total_checksum ^= scheduling_stress(seed);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    return 0;
}
