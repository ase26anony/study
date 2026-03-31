/* gcov_tool_test.c - Program to generate varied GCOV profiles for testing gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data_hot(int *data, int size, int threshold);
extern void process_data_cold(int *data, int size);
extern int checksum(const int *data, int size);

/* __attribute__ directives to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int verbose = 0;
static int use_fullname_demo = 0;

/* ========== Functions for -f (function-level analysis) ========== */

/* Function 1: Complex branching with varied execution paths */
NOINLINE HOT
void process_mode_a(int *data, int size, int seed) {
    int i;
    int local_sum = 0;
    
    /* Branch that varies based on seed */
    if (seed % 3 == 0) {
        for (i = 0; i < size; i++) {
            if (data[i] > 100) {
                data[i] -= 50;
                local_sum += data[i];
            } else if (data[i] < -100) {
                data[i] += 50;
                local_sum -= data[i];
            } else {
                data[i] *= 2;
                local_sum += data[i] * 2;
            }
        }
    } else if (seed % 3 == 1) {
        for (i = size - 1; i >= 0; i--) {
            if (data[i] % 2 == 0) {
                data[i] /= 2;
                local_sum += data[i];
            } else {
                data[i] = data[i] * 3 + 1;
                local_sum += data[i];
            }
        }
    } else {
        /* Different path for seed % 3 == 2 */
        for (i = 0; i < size; i += 2) {
            if (i + 1 < size) {
                int temp = data[i];
                data[i] = data[i + 1];
                data[i + 1] = temp;
                local_sum += data[i] + data[i + 1];
            }
        }
    }
    
    if (verbose) printf("Mode A processed, local sum: %d\n", local_sum);
}

/* Function 2: Nested loops with threshold-based early exit */
NOINLINE
void process_mode_b(int *data, int size, int threshold) {
    int i, j;
    int iterations = 0;
    
    for (i = 0; i < size; i++) {
        if (data[i] > threshold) {
            /* Hot inner loop - runs many times for certain inputs */
            for (j = 0; j < data[i] % 100; j++) {
                data[i] -= j;
                iterations++;
                if (iterations > 10000) break; /* Safety break */
            }
        } else if (data[i] < -threshold) {
            /* Cold path - rarely executed */
            data[i] = -data[i];
        } else {
            /* Medium path */
            data[i] = data[i] * data[i] % 1000;
        }
    }
    
    if (verbose && iterations > 1000) {
        printf("Mode B had %d inner loop iterations\n", iterations);
    }
}

/* Function 3: Recursive-like processing with varied depth */
NOINLINE COLD
void process_mode_c(int *data, int size, int depth) {
    if (size <= 1 || depth <= 0) return;
    
    int mid = size / 2;
    
    /* Branch that varies execution pattern */
    switch (depth % 4) {
        case 0:
            process_mode_c(data, mid, depth - 1);
            process_mode_c(data + mid, size - mid, depth - 1);
            break;
        case 1:
            for (int i = 0; i < mid; i++) {
                data[i] += data[size - i - 1];
            }
            process_mode_c(data, mid, depth - 2);
            break;
        case 2:
            process_mode_c(data + mid, size - mid, depth - 1);
            process_mode_c(data, mid, depth - 1);
            break;
        case 3:
            /* Minimal processing for this path */
            data[0] = data[size - 1];
            break;
    }
}

/* Function 4: Mathematical computations with different algorithms */
NOINLINE HOT
void transform_data(int *data, int size, int algorithm) {
    int i;
    
    if (algorithm == 1) {
        /* Algorithm 1: Many iterations */
        for (i = 0; i < size * 10; i++) {
            int idx = i % size;
            data[idx] = (data[idx] * 1103515245 + 12345) & 0x7fffffff;
        }
    } else if (algorithm == 2) {
        /* Algorithm 2: Fewer but more complex iterations */
        for (i = 0; i < size; i++) {
            int val = data[i];
            for (int j = 0; j < 5; j++) {
                val = (val >> 1) ^ (-(val & 1) & 0xEDB88320);
            }
            data[i] = val;
        }
    } else {
        /* Algorithm 3: Simple transformation */
        for (i = 0; i < size; i++) {
            data[i] = data[i] ^ 0x55AA55AA;
        }
    }
}

/* Function 5: Data validation with early returns */
NOINLINE
int validate_data(const int *data, int size, int strict) {
    if (size <= 0) return 0;
    
    int valid_count = 0;
    
    for (int i = 0; i < size; i++) {
        if (strict) {
            if (data[i] >= -1000 && data[i] <= 1000) {
                valid_count++;
            } else {
                return -1; /* Early return on strict failure */
            }
        } else {
            /* Non-strict: count valid entries */
            if (data[i] > -10000 && data[i] < 10000) {
                valid_count++;
            }
            
            /* Nested condition */
            if (i % 100 == 0 && data[i] == 0) {
                valid_count += 10; /* Bonus for zeros at intervals */
            }
        }
    }
    
    return valid_count;
}

/* Function 6: Memory pattern generation */
NOINLINE
void generate_pattern(int *data, int size, int pattern_type) {
    int i;
    
    if (pattern_type == 0) {
        /* Linear pattern */
        for (i = 0; i < size; i++) {
            data[i] = i * 2;
        }
    } else if (pattern_type == 1) {
        /* Random pattern (seeded) */
        for (i = 0; i < size; i++) {
            data[i] = rand() % 2000 - 1000;
        }
    } else {
        /* Fibonacci-like pattern */
        if (size > 0) data[0] = 1;
        if (size > 1) data[1] = 1;
        for (i = 2; i < size; i++) {
            data[i] = data[i-1] + data[i-2];
            if (data[i] > 1000000) data[i] %= 1000;
        }
    }
}

/* Function 7: Final aggregation and reporting */
NOINLINE
int final_processing(int *data, int size, int mode) {
    int total = 0;
    int hot_zone = 0;
    
    /* This loop's execution count varies dramatically */
    for (int i = 0; i < size; i++) {
        if (mode == 1) {
            /* Hot path - many iterations */
            for (int j = 0; j < (abs(data[i]) % 100); j++) {
                total += data[i] + j;
                hot_zone++;
            }
        } else if (mode == 2) {
            /* Medium path */
            total += data[i] * 2;
        } else {
            /* Cold path - minimal processing */
            total += data[i];
        }
    }
    
    /* Conditional that's rarely true */
    if (hot_zone > 10000 && verbose) {
        printf("Hot zone detected: %d iterations\n", hot_zone);
    }
    
    return total;
}

/* ========== Functions with same name in different scopes (for -F fullname) ========== */

/* In C, we simulate this with static functions in different files */
/* This function will be in the main file */
static int helper_function(int x) {
    return x * 3;
}

/* Another static helper with different logic */
static int helper_function_complex(int x, int y) {
    if (x > y) return x - y;
    return y - x + helper_function(x);
}

/* ========== Main driver ========== */

int main(int argc, char *argv[]) {
    int i;
    int data_size = 1000;
    int seed = 12345;
    int mode = 1;
    int iterations = 1;
    int algorithm = 1;
    int threshold = 500;
    int use_hot_path = 0;
    
    /* Parse command line arguments to vary execution paths */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--size") == 0 && i + 1 < argc) {
            data_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--threshold") == 0 && i + 1 < argc) {
            threshold = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--hot") == 0) {
            use_hot_path = 1;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--fullname-demo") == 0) {
            use_fullname_demo = 1;
        }
    }
    
    /* Seed the random number generator - CRITICAL for varied profiles */
    srand(seed);
    
    /* Allocate and initialize data */
    int *data = (int *)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Generate initial data pattern */
    generate_pattern(data, data_size, seed % 3);
    
    int total_result = 0;
    
    /* Main processing loop - execution count varies with --iterations */
    for (int iter = 0; iter < iterations; iter++) {
        /* Vary function calls based on mode and iteration */
        switch ((mode + iter) % 4) {
            case 0:
                process_mode_a(data, data_size, seed + iter);
                process_mode_b(data, data_size, threshold);
                break;
            case 1:
                process_mode_b(data, data_size, threshold / 2);
                process_mode_c(data, data_size, 5 + iter % 3);
                break;
            case 2:
                transform_data(data, data_size, algorithm);
                process_mode_a(data, data_size, seed);
                break;
            case 3:
                process_mode_c(data, data_size, 3);
                transform_data(data, data_size, (algorithm + 1) % 3);
                break;
        }
        
        /* Call hot/cold functions based on flag */
        if (use_hot_path) {
            /* This creates hot spots for -h and -t options */
            for (int j = 0; j < 100; j++) {
                process_data_hot(data, data_size, threshold);
            }
        } else {
            process_data_cold(data, data_size);
        }
        
        /* Validate data (varies execution) */
        int valid = validate_data(data, data_size, (iter % 2 == 0));
        if (valid < 0 && verbose) {
            printf("Iteration %d: Data validation failed\n", iter);
        }
        
        /* Final processing with mode-dependent intensity */
        int processing_mode = (use_hot_path) ? 1 : ((iter % 3 == 0) ? 2 : 3);
        total_result += final_processing(data, data_size, processing_mode);
        
        /* Modify data for next iteration */
        for (int j = 0; j < data_size; j += 10) {
            data[j] += iter * 7;
        }
    }
    
    /* Use helper functions (for fullname demonstration) */
    if (use_fullname_demo) {
        int helper_result = helper_function(total_result % 100);
        helper_result += helper_function_complex(total_result % 50, seed % 30);
        total_result += helper_result;
    }
    
    /* Calculate and output final checksum */
    int final_checksum = checksum(data, data_size);
    final_checksum ^= total_result;
    
    printf("RESULT: checksum=0x%08x, total=%d, seed=%d, mode=%d\n", 
           final_checksum, total_result, seed, mode);
    
    free(data);
    return 0;
}
