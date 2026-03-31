#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int g_result = 0;
volatile float g_float_result = 0.0f;

// Function to generate deterministic data
void init_data(int seed, int* int_arr, unsigned short* short_arr, 
               float* float_arr, double* double_arr, int size) {
    for (int i = 0; i < size; i++) {
        int val = (i * 3 + seed) % 1000;
        int_arr[i] = val;
        short_arr[i] = (unsigned short)(val % 65535);
        float_arr[i] = (float)val * 1.5f;
        double_arr[i] = (double)val * 2.5;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <seed>\n", argv[0]);
        return 1;
    }
    
    int seed = atoi(argv[1]);
    const int SIZE = 64;
    
    // Declare arrays with different types
    int int_arr[SIZE];
    unsigned short short_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    
    // Initialize arrays deterministically
    init_data(seed, int_arr, short_arr, float_arr, double_arr, SIZE);
    
    // Loop-invariant thresholds from volatile sources
    volatile int int_threshold = 300;
    volatile float float_threshold = 450.0f;
    volatile double double_threshold = 600.0;
    volatile unsigned short short_threshold = 200;
    
    // Reduction variables
    int int_max = -1000000;
    int int_min = 1000000;
    float float_max = -1e9f;
    float float_min = 1e9f;
    double double_sum = 0.0;
    unsigned int count_gt = 0;
    unsigned int count_le = 0;
    int cond_sum_int = 0;
    float cond_sum_float = 0.0f;
    
    // ===== Loop 1: GT_EXPR pattern (greater-than) =====
    // Multiple reductions in one loop with nested conditionals
    for (int i = 0; i < SIZE; i++) {
        // Outer if to complicate control flow
        if (i % 2 == 0) {
            // GT_EXPR: if (int_arr[i] > int_max) int_max = int_arr[i];
            if (int_arr[i] > int_max) {
                int_max = int_arr[i];
            }
            
            // Combined with another GT_EXPR using logical AND
            if (int_arr[i] > (int)short_threshold && i < SIZE - 1) {
                count_gt++;
            }
        }
    }
    
    // ===== Loop 2: GE_EXPR pattern (greater-than-or-equal) =====
    // Using while loop variant
    int j = 0;
    while (j < SIZE) {
        // GE_EXPR: conditional sum with >=
        if (float_arr[j] >= float_threshold) {
            cond_sum_float += float_arr[j];
        }
        
        // Another GE_EXPR with different type
        if (short_arr[j] >= short_threshold) {
            cond_sum_int += (int)short_arr[j];
        }
        
        // Nested conditional with logical OR
        if (j > 0 && (float_arr[j] >= float_threshold || short_arr[j] >= short_threshold)) {
            g_result++;  // side effect to prevent elimination
        }
        j++;
    }
    
    // ===== Loop 3: LT_EXPR pattern (less-than) =====
    // Mixed data types with multiple reductions
    for (int i = 0; i < SIZE; i++) {
        // LT_EXPR: if (float_arr[i] < float_min) float_min = float_arr[i];
        if (float_arr[i] < float_min) {
            float_min = float_arr[i];
        }
        
        // Another LT_EXPR with different threshold
        if ((double)float_arr[i] < double_threshold) {
            double_sum += float_arr[i];
        }
        
        // Complex condition with logical operators
        if (i > 10 && i < 50 && float_arr[i] < float_threshold) {
            count_le++;
        }
    }
    
    // ===== Loop 4: LE_EXPR pattern (less-than-or-equal) =====
    // Multiple reductions with LE_EXPR
    int k = 0;
    int local_max = int_arr[0];
    int local_min = int_arr[0];
    while (k < SIZE) {
        // LE_EXPR: if (int_arr[k] <= local_min) local_min = int_arr[k];
        if (int_arr[k] <= local_min) {
            local_min = int_arr[k];
        }
        
        // Combined with GT_EXPR for max (testing multiple patterns)
        if (int_arr[k] > local_max) {
            local_max = int_arr[k];
        }
        
        // LE_EXPR with floating point
        if (float_arr[k] <= float_threshold) {
            g_float_result += float_arr[k];
        }
        
        k++;
    }
    
    // Update global min/max with local results
    if (local_min < int_min) int_min = local_min;
    if (local_max > int_max) int_max = local_max;
    
    // ===== Loop 5: Mixed comparison operators in one loop =====
    // Testing all four operators together
    int sum_all_conditions = 0;
    for (int i = 0; i < SIZE; i++) {
        // GT_EXPR
        if (int_arr[i] > 400) {
            sum_all_conditions += int_arr[i];
        }
        // GE_EXPR  
        else if (int_arr[i] >= 200) {
            sum_all_conditions += int_arr[i] / 2;
        }
        // LT_EXPR
        else if (float_arr[i] < 300.0f) {
            sum_all_conditions += (int)float_arr[i];
        }
        // LE_EXPR
        else if (short_arr[i] <= 100) {
            sum_all_conditions += short_arr[i] * 2;
        }
    }
    
    // Compute final checksum
    int checksum = 0;
    checksum += int_max;
    checksum += int_min;
    checksum += (int)float_max;
    checksum += (int)float_min;
    checksum += (int)double_sum;
    checksum += count_gt;
    checksum += count_le;
    checksum += cond_sum_int;
    checksum += (int)cond_sum_float;
    checksum += sum_all_conditions;
    checksum += g_result;
    checksum += (int)g_float_result;
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
