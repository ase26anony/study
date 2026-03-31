/* Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer -o scheduler_test scheduler_test.c */
/* Alternative: gcc -O3 -fschedule-insns2 -fno-tree-vectorize -fno-unroll-loops -o scheduler_test scheduler_test.c */

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

/* State machine labels for computed goto */
static void* state_labels[] = {
    &&STATE_0, &&STATE_1, &&STATE_2, &&STATE_3, &&STATE_4,
    &&STATE_5, &&STATE_6, &&STATE_7, &&STATE_8, &&STATE_9
};

/* Complex dependency chain helper */
int __attribute__((noinline)) complex_dependency_chain(int seed) {
    volatile int a = seed;
    volatile int b = a * 3;
    volatile int c = b + 7;
    volatile int d = c ^ 0x55AA55AA;
    volatile int e = d >> 3;
    volatile int f = e * 11;
    volatile int g = f - 19;
    volatile int h = g & 0x00FF00FF;
    
    /* Mixed type operations */
    volatile float fa = (float)h;
    volatile float fb = fa * 1.5f;
    volatile float fc = fb + 3.14f;
    volatile int i = (int)fc;
    
    /* Another chain */
    volatile double da = (double)i;
    volatile double db = da / 2.71828;
    volatile double dc = db * 1.41421;
    volatile int j = (int)dc;
    
    scheduling_barrier();
    return j;
}

/* Memory access pattern creator */
void __attribute__((noinline)) memory_access_pattern(volatile int* arr, int size, int iterations) {
    volatile int idx = 0;
    volatile int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex indexing calculation */
        idx = (idx * 1103515245 + 12345) & (size - 1);
        
        /* Dependent memory operations */
        volatile int val = arr[idx];
        val = opaque_int(val);
        val = val * 3 + 7;
        val = val ^ acc;
        arr[idx] = val;
        acc = val;
        
        /* Additional dependency */
        volatile float fval = (float)val;
        fval = opaque_float(fval);
        fval = fval * 1.2345f - 0.9876f;
        acc += (int)fval;
    }
    
    scheduling_barrier();
}

/* Main scheduling stress function */
int __attribute__((noinline)) scheduling_stress(int seed) {
    volatile int state = seed % 10;
    volatile int counter = 0;
    volatile int result = 0;
    
    /* Local array with volatile accesses */
    volatile int local_array[32];
    for (int i = 0; i < 32; i++) {
        local_array[i] = i * i + seed;
    }
    
    /* Mixed type variables for complex dependencies */
    volatile float f1 = (float)seed * 0.5f;
    volatile float f2 = 3.14f;
    volatile double d1 = (double)seed * 0.25;
    volatile double d2 = 2.71828;
    
    /* Outer loop - creates multiple scheduling regions */
    for (int outer = 0; outer < 50; outer++) {
        /* Update state based on complex condition */
        state = (state * 1103515245 + 12345) % 10;
        
        /* Use computed goto for state machine */
        goto *state_labels[state];
        
        STATE_0: {
            /* Chain of dependent integer operations */
            volatile int a = counter;
            volatile int b = a + outer;
            volatile int c = b * 3;
            volatile int d = c ^ result;
            volatile int e = d >> 2;
            result = opaque_int(e);
            
            /* Mixed type operation */
            f1 = (float)result * f2;
            f1 = opaque_float(f1);
            
            /* Memory access */
            memory_access_pattern((volatile int*)local_array, 32, 5);
            
            scheduling_barrier();
            counter++;
            continue;
        }
        
        STATE_1: {
            /* Different operation chain */
            volatile int a = result;
            volatile int b = a * 7;
            volatile int c = b - outer;
            volatile int d = c & 0xFF;
            volatile int e = d | counter;
            result = opaque_int(e);
            
            /* Floating point chain */
            f2 = f1 * 2.0f;
            f2 = opaque_float(f2);
            d1 = (double)f2 * d2;
            d1 = opaque_double(d1);
            
            scheduling_barrier();
            counter += 2;
            continue;
        }
        
        STATE_2: {
            /* Long dependency chain */
            volatile int v1 = complex_dependency_chain(counter);
            volatile int v2 = complex_dependency_chain(outer);
            volatile int v3 = v1 ^ v2;
            volatile int v4 = v3 * 13;
            volatile int v5 = v4 >> 1;
            result = opaque_int(v5);
            
            /* Memory intensive */
            for (int i = 0; i < 8; i++) {
                volatile int idx = (i * 3 + outer) & 31;
                volatile int val = local_array[idx];
                val = val * 3 + 7;
                local_array[idx] = val;
                result ^= val;
            }
            
            scheduling_barrier();
            counter += 3;
            continue;
        }
        
        STATE_3: {
            /* Another unique chain */
            volatile int a = result * 31;
            volatile int b = a + 0xABCDEF;
            volatile int c = b ^ 0x12345678;
            volatile int d = c - outer * 17;
            result = opaque_int(d);
            
            /* Use rdtsc for x86-specific scheduling */
            #ifdef __x86_64__
            uint64_t tsc1 = __builtin_ia32_rdtsc();
            result ^= (int)(tsc1 & 0xFFFFFFFF);
            uint64_t tsc2 = __builtin_ia32_rdtsc();
            result ^= (int)((tsc2 >> 32) & 0xFFFFFFFF);
            #endif
            
            scheduling_barrier();
            counter += 4;
            continue;
        }
        
        /* Additional states with similar patterns but different operations */
        STATE_4: {
            volatile int a = counter * counter;
            volatile int b = a + result;
            volatile int c = b % 97;
            result = opaque_int(c);
            counter += 5;
            continue;
        }
        
        STATE_5: {
            volatile int a = result | 0xAA;
            volatile int b = a << 3;
            volatile int c = b - outer;
            result = opaque_int(c);
            counter += 6;
            continue;
        }
        
        STATE_6: {
            volatile int a = complex_dependency_chain(result);
            volatile int b = complex_dependency_chain(counter);
            result = opaque_int(a ^ b);
            counter += 7;
            continue;
        }
        
        STATE_7: {
            memory_access_pattern((volatile int*)local_array, 32, 10);
            result = opaque_int(result + counter);
            counter += 8;
            continue;
        }
        
        STATE_8: {
            volatile float tf = (float)result;
            tf = tf * 1.5f + 2.5f;
            result = opaque_int((int)tf);
            counter += 9;
            continue;
        }
        
        STATE_9: {
            volatile double td = (double)result;
            td = td / 3.0 + 1.0;
            result = opaque_int((int)td);
            counter += 10;
            continue;
        }
    }
    
    /* Final aggregation */
    volatile int final = result;
    for (int i = 0; i < 32; i++) {
        final ^= local_array[i];
    }
    final = opaque_int(final);
    
    return final;
}

int main() {
    srand(time(NULL));
    int total_result = 0;
    
    /* Run scheduling stress multiple times */
    for (int i = 0; i < 100; i++) {
        int seed = rand();
        int result = scheduling_stress(seed);
        total_result ^= result;
        
        /* Print progress occasionally */
        if (i % 25 == 0) {
            printf("Iteration %d, result: %d\n", i, result);
        }
    }
    
    printf("Final checksum: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
