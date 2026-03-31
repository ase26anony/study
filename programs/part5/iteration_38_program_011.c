/* Complex dependency pattern generator to exercise DDG edge creation */
#include <stdlib.h>
#include <math.h>

/* Global variables for function call dependencies */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Requirement 5: Function with side effects */
static void update_global(int idx, float* arr) {
    global_counter += idx;
    global_accumulator += arr[idx % 16];
    arr[idx % 16] = global_accumulator * 0.5f;
}

/* Another static function for output dependencies */
static inline float transform_value(float x, int scale) {
    volatile float result; /* Prevent optimization */
    result = x * scale;
    result = result + sinf(x); /* WAW dependency inside function */
    return result;
}

/* Non-affine index generator */
static void init_indices(int* indices, int size) {
    for (int i = 0; i < size; i++) {
        indices[i] = (i * i + i * 3 + 7) % size; /* Quadratic non-affine */
    }
}

int main(void) {
    const int N = 1024;
    volatile int limit = N; /* Requirement 6: volatile variable */
    
    /* Arrays with different data types */
    int int_array[N];
    float float_array[N];
    double double_buffer[256];
    
    /* Requirement 4: Non-affine index array */
    int indices[N];
    init_indices(indices, N);
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_array[i] = i;
        float_array[i] = i * 0.5f;
    }
    
    float loop_carried_value = 1.0f;
    int loop_carried_int = 0;
    
    /* Primary loop with complex dependencies */
    for (int i = 1; i < limit; i++) {
        /* Requirement 1: Loop-carried true dependency (RAW) */
        float_array[i] = float_array[i-1] * 1.1f + loop_carried_value;
        
        /* Requirement 6: Mixed data type operations */
        int temp_int = int_array[i] * 2;
        float temp_float = (float)temp_int * 0.3f;
        
        /* Requirement 2: Anti-dependency (WAR) pattern */
        float read_first = float_array[i];  /* Read */
        float_array[i] = temp_float * 2.0f; /* Write later - creates WAR */
        double_buffer[i % 256] = read_first;
        
        /* Requirement 2: Output dependency (WAW) on local variable */
        loop_carried_value = temp_float + 1.0f;
        loop_carried_value = transform_value(loop_carried_value, i); /* WAW inside function */
        
        /* Requirement 3: Conditional dependency paths */
        if (i % 3 == 0) {
            /* Path A: Chain of dependencies */
            int_array[i] = int_array[i-1] + global_counter;
            loop_carried_int = int_array[i] * 2;
        } else if (i % 3 == 1) {
            /* Path B: Different dependency pattern */
            loop_carried_int = int_array[i] + loop_carried_int;
            int_array[i] = loop_carried_int / 2;
        } else {
            /* Path C: Output dependencies */
            int_array[i] = i * i;
            int_array[i] = int_array[i] % 100; /* WAW on array element */
        }
        
        /* Requirement 5: Function call with side effects */
        update_global(i, float_array);
        
        /* Additional anti-dependency with scalar */
        float tmp = loop_carried_value;      /* Read */
        loop_carried_value = sinf(tmp) * 2.0f; /* Write - creates WAR */
        float_array[i % 16] = tmp;
    }
    
    /* Nested loop with non-affine accesses */
    float sum = 0.0f;
    for (int i = 0; i < limit - 16; i += 8) {
        for (int j = 0; j < 8; j++) {
            /* Requirement 4: Non-affine array access */
            int idx = indices[i + j];
            sum += float_array[idx] * 0.1f;
            
            /* Complex addressing with multiple dependencies */
            int_array[idx % 64] = int_array[(idx + 1) % 64] + j;
        }
        
        /* Cross-iteration dependency in nested loop */
        double_buffer[(i/8) % 256] = sum * (i + 1);
    }
    
    /* Create register pressure and output dependencies */
    float x = 0.0f;
    for (int i = 0; i < 100; i++) {
        x = x * 1.01f + 0.5f;      /* WAW on x */
        x = x + sinf(x * 0.1f);    /* Another WAW */
        float_array[i % 32] = x;
    }
    
    /* Final aggregation to prevent dead code elimination */
    volatile float final_result = 0.0f;
    for (int i = 0; i < N; i++) {
        final_result += float_array[i] + int_array[i % 64];
    }
    final_result += sum + global_accumulator + loop_carried_value;
    
    return (int)final_result % 256;
}
