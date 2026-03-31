/* Test for software pipelining and modulo scheduling */
#include <stdlib.h>

#define SIZE 1024

void compute_intensive(float *a, float *b, float *c, int n) {
    int i, j;
    /* Triple nested loop creates scheduling pressure */
    for (i = 0; i < n; i++) {
        float sum = 0.0f;
        /* Inner loop with data dependencies */
        for (j = 0; j < 8; j++) {
            /* Mixed operations to create varied instruction types */
            float t = a[i] * b[i] + (float)j;
            t = t * t - t / 2.0f;
            sum += t;
            
            /* Conditional creates branch scheduling needs */
            if (t > 100.0f) {
                sum -= 50.0f;
            } else {
                sum += 25.0f;
            }
        }
        c[i] = sum;
        
        /* Another loop with different pattern */
        for (j = 0; j < 4; j++) {
            c[i] += (a[i] * 0.5f) - (b[i] * 0.25f);
        }
    }
}

/* Function with pointer chasing to create memory dependencies */
void process_linked_list(int *data, int *next, int start, int n) {
    int idx = start;
    int sum = 0;
    volatile int *vsum = &sum; /* Prevent optimization */
    
    while (idx != -1 && n-- > 0) {
        /* Complex expression with multiple operations */
        int val = data[idx];
        val = (val * 3 + 7) >> 2;
        val = val ^ (val << 3);
        *vsum += val;
        
        /* Conditional with side effect */
        if (val > 1000) {
            data[idx] = val / 2;
        } else {
            data[idx] = val * 2;
        }
        
        idx = next[idx];
    }
}

/* Main driver that uses both functions */
int main(int argc, char **argv) {
    float a[SIZE], b[SIZE], c[SIZE];
    int data[SIZE], next[SIZE];
    
    /* Initialize with non-constant values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = (float)(argc + i);
        b[i] = (float)(argc * i);
        data[i] = argc + i * 3;
        next[i] = (i < SIZE - 1) ? i + 1 : -1;
    }
    
    /* Call compute-intensive function */
    compute_intensive(a, b, c, SIZE);
    
    /* Process linked list */
    process_linked_list(data, next, 0, SIZE / 2);
    
    /* Use results to prevent dead code elimination */
    float total = 0.0f;
    for (int i = 0; i < SIZE; i++) {
        total += c[i];
    }
    
    return (int)total % 256; /* Non-zero but bounded */
}
