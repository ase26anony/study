/* ddg_edge_coverage.c
 * Program designed to exercise Data Dependency Graph edge creation
 * in GCC's instruction scheduler
 */

#include <stdlib.h>

/* Global variables for memory dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static function with side effects (Requirement 5) */
static void update_global(int *arr, int idx) {
    global_counter++;
    arr[idx] += global_counter;
    global_accumulator += (float)arr[idx];
}

/* Another static function for anti-dependency pattern */
static float process_value(float a, float b, float *storage) {
    float tmp = *storage;  /* Read from storage (creates anti-dep) */
    *storage = a * b;      /* Write to storage (completes anti-dep) */
    return tmp + 0.5f;
}

/* Non-affine index array (Requirement 4) */
static const int non_linear_indices[16] = {
    0, 3, 1, 2, 7, 5, 4, 6, 15, 11, 9, 13, 8, 12, 10, 14
};

int main(void) {
    /* Declare arrays with different data types (Requirement 6) */
    int int_array[256] = {0};
    float float_array[256] = {0.0f};
    double double_array[256] = {0.0};
    
    /* Volatile loop limit to prevent optimization (Requirement 6) */
    volatile int N = 100;
    
    /* Local variables for dependency patterns */
    int local_accumulator = 0;
    float temp_storage = 1.0f;
    int reuse_var = 0;  /* For output dependencies */
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    for (int i = 1; i < N; i++) {
        /* TRUE DEPENDENCY (RAW): Loop-carried through int_array */
        int_array[i] = int_array[i-1] + i * 2;  /* Line 1 */
        
        /* ANTI-DEPENDENCY (WAR) on float_array (Requirement 2) */
        float old_val = float_array[i];          /* Read */
        float_array[i] = (float)int_array[i] * 0.5f;  /* Write */
        float_array[i] += old_val;               /* Use old value */
        
        /* OUTPUT DEPENDENCY (WAW) on reuse_var (Requirement 2) */
        reuse_var = i * 3;                       /* Write 1 */
        reuse_var = reuse_var + (i % 5);         /* Write 2 */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float tmp1 = float_array[i];
            float tmp2 = tmp1 * 2.0f;      /* Depends on tmp1 */
            int_array[i] += (int)tmp2;     /* Depends on tmp2 */
        } else if (i % 3 == 1) {
            /* Pattern B: Different dependency structure */
            int tmp = int_array[i];
            float_array[i] = (float)tmp / 3.0f;
            local_accumulator += tmp;      /* Loop-carried through accumulator */
        } else {
            /* Pattern C: More complex with function call */
            update_global(int_array, i % 16);
        }
        
        /* Function call creating memory dependencies (Requirement 5) */
        temp_storage = process_value(float_array[i], (float)i, &temp_storage);
        
        /* Mixed data type operations (Requirement 6) */
        double_array[i] = (double)int_array[i] * 1.5;
        if (i % 7 == 0) {
            double_array[i] += (double)float_array[i];
        }
    }
    
    /* Nested loop with non-affine array access (Requirement 4) */
    for (int i = 0; i < 16; i++) {
        int idx = non_linear_indices[i];
        if (idx < 256) {
            /* Create dependencies through non-linear access */
            int_array[idx] += i;
            float_array[idx] = float_array[non_linear_indices[(i + 1) % 16]] * 0.9f;
        }
    }
    
    /* Another loop with output dependencies on array elements */
    for (int i = 0; i < 32; i++) {
        /* Multiple writes to same location */
        double_array[i % 16] = (double)i;
        double_array[i % 16] = double_array[i % 16] * 2.0;
        double_array[i % 16] = double_array[i % 16] + 1.0;
    }
    
    /* Complex loop with mixed dependencies */
    for (int i = 2; i < 50; i++) {
        /* True dependency chain with different data types */
        int int_val = int_array[i-1] + int_array[i-2];
        float float_val = (float)int_val * 0.3f;
        double double_val = (double)float_val * 1.7;
        
        /* Anti-dependency by reusing variable */
        float old_float = float_val;
        float_val = float_array[i] * 2.0f;
        float_array[i] = old_float + float_val;
        
        /* Output dependency through pointer */
        double *ptr = &double_array[i];
        *ptr = double_val;
        *ptr = *ptr * 0.95;
    }
    
    /* Aggregate results to volatile to prevent elimination */
    volatile int final_result = 0;
    for (int i = 0; i < 256; i++) {
        final_result += int_array[i];
        final_result += (int)float_array[i];
        final_result += (int)double_array[i];
    }
    
    final_result += global_counter;
    final_result += (int)global_accumulator;
    final_result += local_accumulator;
    final_result += reuse_var;
    
    return final_result % 256;
}
