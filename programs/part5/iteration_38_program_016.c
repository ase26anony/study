/* Complex dependency pattern generator for DDG edge coverage */
#include <stdlib.h>
#include <math.h>

/* Global variables for memory dependency creation (Requirement 5) */
static int global_counter = 0;
static float global_accumulator = 0.0f;

/* Static helper function with side effects */
static void update_global(int *arr, int idx) {
    global_counter++;
    global_accumulator += arr[idx] * 0.5f;
    arr[idx] = global_counter % 100;
}

/* Another static function for different data type operations */
static double process_value(double x, int i) {
    volatile double tmp = x;  /* Prevent optimization */
    return tmp * sin(i * 0.1) + global_accumulator;
}

int main(void) {
    const int N = 1024;
    volatile int limit = N;  /* Volatile to prevent optimization */
    
    /* Arrays with different data types (Requirement 6) */
    int int_arr[N];
    float float_arr[N];
    double double_arr[N];
    int index_map[N];  /* For non-affine accesses */
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        int_arr[i] = i % 100;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
        /* Create non-linear index mapping (Requirement 4) */
        index_map[i] = (i * i + i * 3 + 7) % N;
    }
    
    int result = 0;
    float f_result = 0.0f;
    double d_result = 0.0;
    
    /* Primary loop with loop-carried dependency (Requirement 1) */
    for (int i = 1; i < limit; i++) {
        /* TRUE dependency (RAW): loop-carried through int_arr */
        int_arr[i] = int_arr[i-1] + i;  /* Line 1: RAW dependency */
        
        /* Conditional creating different dependency patterns (Requirement 3) */
        if (i % 3 == 0) {
            /* Pattern A: Chain of dependencies */
            float tmp_f = float_arr[i];           /* Read */
            float_arr[i] = tmp_f * 1.1f + i;      /* Write - creates WAR if tmp_f reused */
            float_arr[i] = sqrtf(float_arr[i]);   /* WAW on float_arr[i] */
            
            /* Mixed type operations feeding each other */
            double d_val = double_arr[i] + tmp_f; /* float -> double conversion */
            int_arr[i] += (int)d_val;             /* double -> int conversion */
        } else if (i % 3 == 1) {
            /* Pattern B: Anti-dependencies (WAR) */
            int old_val = int_arr[i];             /* Read */
            int_arr[i] = i * 2;                   /* Write to same location - WAR */
            result += old_val;                    /* Use old value */
            
            /* Output dependency (WAW) on local variable */
            double local = process_value(i, i);   /* First write to local */
            local = local * 0.9 + global_counter; /* Second write to local - WAW */
            d_result += local;
        } else {
            /* Pattern C: Complex mixed dependencies */
            volatile int v_tmp = int_arr[i];      /* Volatile read */
            
            /* Multiple writes creating WAW */
            f_result = float_arr[i] * 2.0f;
            f_result = f_result / 1.5f;           /* WAW on f_result */
            
            /* Function call creating memory dependencies (Requirement 5) */
            update_global(int_arr, i % 100);
        }
        
        /* Anti-dependency within same iteration (Requirement 2) */
        int tmp_storage = int_arr[i];             /* Read */
        int_arr[i] = (int)(float_arr[i] * 10.0f); /* Write - WAR on int_arr[i] */
        float_arr[i] = tmp_storage * 0.5f;        /* Use tmp_storage */
        
        /* Output dependency example */
        double output_var = sin(i * 0.01);
        output_var = cos(output_var);             /* WAW on output_var */
        double_arr[i] = output_var;
    }
    
    /* Nested loop with non-affine array accesses (Requirement 4) */
    for (int i = 0; i < N/2; i++) {
        for (int j = 0; j < 4; j++) {
            /* Non-linear access pattern */
            int idx = index_map[(i * 4 + j) % N];
            
            /* Create dependencies through non-affine access */
            int_arr[idx] = int_arr[index_map[(i * 4 + j + 1) % N]] + 1;
            
            /* Mixed type chain */
            float_arr[idx] = int_arr[idx] * 0.3f;
            double_arr[idx] = float_arr[idx] * 0.7;
            
            /* Conditional store creating control dependency */
            if (idx % 2 == 0) {
                global_accumulator += double_arr[idx];
            }
        }
    }
    
    /* Final aggregation preventing dead code elimination */
    volatile int final_result = 0;
    for (int i = 0; i < N; i++) {
        final_result += int_arr[i] % 256;
        final_result += (int)float_arr[i];
        final_result += (int)double_arr[i];
    }
    
    final_result += (int)global_accumulator + global_counter;
    final_result += (int)f_result + (int)d_result;
    
    return final_result % 256;
}
