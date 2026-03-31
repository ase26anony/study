/* test_auto_profile.c - Test program for GCC AutoFDO coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
int global_array[256] = {0};

/* Function to create phi node from loop iteration */
__attribute__((noinline, noipa))
int create_loop_phi(int iteration, int prev_value) {
    /* Pattern A: Loop-dependent phi node */
    int phi_value;
    if (iteration == 0) {
        phi_value = 1;  /* Start with 1 */
    } else {
        phi_value = prev_value + (iteration % 3);
    }
    
    /* Pattern C: Chain of assignments to obscure origin */
    int temp1 = phi_value;
    int temp2 = temp1;
    int final_value = temp2;
    
    return final_value;
}

/* Function with merge point phi (Pattern B) */
__attribute__((noinline, noipa))
void process_with_merge_phi(int input, volatile int* counter) {
    /* Create phi at merge point */
    int merge_value;
    if (input & 1) {
        merge_value = 1;  /* Could become 1 */
    } else {
        merge_value = 0;  /* Could become 0 */
    }
    
    /* Chain assignments */
    int a = merge_value;
    int b = a;
    int c = b;
    
    /* Critical comparison against 0 or 1 */
    if (c == 0) {  /* Comparison against 0 */
        (*counter)++;
        global_array[input % 256] += 1;
    }
    
    /* Another comparison against 1 */
    if (c == 1) {  /* Comparison against 1 */
        (*counter)--;
        global_array[(input + 128) % 256] += 2;
    }
}

/* Main hot function containing all patterns */
__attribute__((noinline, noipa))
void hot_function(int iterations) {
    int prev_phi_value = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pattern A: Get value from loop phi */
        int loop_phi_val = create_loop_phi(i, prev_phi_value);
        prev_phi_value = loop_phi_val;
        
        /* Use the phi value in comparison against 0 */
        if (loop_phi_val == 0) {
            global_counter += 3;
            global_array[i % 256] ^= 0xAA;
        }
        
        /* Pattern B: Process with merge phi */
        process_with_merge_phi(i, &global_counter);
        
        /* Additional boolean phi pattern */
        bool flag;
        if (i % 7 == 0) {
            flag = true;  /* true becomes 1 */
        } else {
            flag = false; /* false becomes 0 */
        }
        
        /* Boolean comparison (compares against 1/0) */
        if (flag == true) {  /* Comparison against 1 */
            global_array[(i + 64) % 256] |= 0x55;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 13 == 0) {
            global_counter ^= rand() % 100;
        }
    }
}

/* Helper to create checksum */
int compute_checksum(void) {
    int sum = global_counter;
    for (int i = 0; i < 256; i++) {
        sum = (sum * 31 + global_array[i]) % 1000000007;
    }
    return sum;
}

int main(int argc, char* argv[]) {
    int iterations = 1000000;  /* Default large number for hot path */
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000000;
    }
    
    /* Initialize random seed for variability */
    srand(42);
    
    /* Clear globals */
    global_counter = 0;
    memset(global_array, 0, sizeof(global_array));
    
    /* Execute hot function many times */
    hot_function(iterations);
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = compute_checksum();
    printf("Checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
