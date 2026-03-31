/* gcov_tool_test.c - Generates varied GCOV profiles for overlap analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Forward declarations for functions in different compilation units */
extern void process_data(int *arr, int n, int mode);
extern void analyze_results(int *arr, int n, int threshold);

/* Global configuration */
static int verbose = 0;
static int hot_threshold = 1000;

/* ========== HOT FUNCTIONS (high execution count) ========== */

/* Mark as hot to influence compiler heuristics */
__attribute__((hot)) 
void hot_loop_multiply(int *arr, int n, int factor) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < factor; j++) {
            arr[i] *= (j + 1);
            /* Branch with varying probability */
            if (arr[i] > 1000000) {
                arr[i] %= 1000000;
            }
        }
    }
}

__attribute__((hot, noinline))
void hot_search_algorithm(int *arr, int n, int target) {
    int iterations = 0;
    for (int i = 0; i < n; i++) {
        iterations++;
        if (arr[i] == target) {
            if (verbose) printf("Found at index %d\n", i);
            break;
        }
        /* Nested conditionals for branch coverage */
        if (i > n/2) {
            if (arr[i] > target * 2) {
                i += 2;  /* Skip ahead */
            } else if (arr[i] < target / 2) {
                i--;     /* Step back */
            }
        }
    }
    if (iterations > hot_threshold && verbose) {
        printf("Hot search performed %d iterations\n", iterations);
    }
}

/* ========== COLD FUNCTIONS (low execution count) ========== */

__attribute__((cold, noinline))
void cold_initialization(int *arr, int n, int seed) {
    srand(seed);
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 1000;
    }
    /* Rarely executed debug code */
    if (seed == 0 && verbose) {
        printf("Warning: seed is zero\n");
    }
}

__attribute__((cold))
void cold_validation(int *arr, int n) {
    int errors = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < 0) {
            errors++;
            arr[i] = 0;
        }
    }
    if (errors > 0 && verbose) {
        printf("Fixed %d negative values\n", errors);
    }
}

/* ========== MEDIUM FREQUENCY FUNCTIONS ========== */

__attribute__((noinline))
void process_mode_a(int *arr, int n) {
    /* Different execution path for mode A */
    for (int i = 0; i < n; i += 2) {
        arr[i] = arr[i] * 3 + 7;
        if (i % 3 == 0) {
            arr[i] /= 2;
        }
    }
}

__attribute__((noinline))
void process_mode_b(int *arr, int n) {
    /* Different execution path for mode B */
    for (int i = 1; i < n; i += 2) {
        arr[i] = arr[i] * 5 - 3;
        if (i % 4 == 0) {
            arr[i] += 100;
        }
    }
}

__attribute__((noinline))
void process_mode_c(int *arr, int n) {
    /* Hybrid approach */
    for (int i = 0; i < n; i++) {
        switch (i % 5) {
            case 0: arr[i] += 10; break;
            case 1: arr[i] -= 5; break;
            case 2: arr[i] *= 2; break;
            case 3: arr[i] /= 2; break;
            case 4: arr[i] = arr[i] * arr[i] % 1000; break;
        }
    }
}

/* ========== COMPLEX FUNCTION WITH NESTED BRANCHES ========== */

__attribute__((noinline))
int complex_decision_tree(int *arr, int n, int depth) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] > 500) {
            if (depth > 0) {
                result += arr[i];
                if (arr[i] > 750) {
                    result += 50;
                    if (arr[i] > 900) {
                        result += 100;
                    }
                }
            } else {
                result -= arr[i];
            }
        } else if (arr[i] > 250) {
            result += arr[i] / 2;
        } else {
            if (depth > 1) {
                result += 1;
            }
        }
        
        /* Additional nested condition */
        if (i % 10 == 0) {
            switch (depth % 3) {
                case 0: result += 5; break;
                case 1: result += 10; break;
                case 2: result += 15; break;
            }
        }
    }
    return result;
}

/* ========== MAIN EXECUTION LOGIC ========== */

int main(int argc, char *argv[]) {
    int mode = 0;
    int seed = 1;
    int iterations = 1000;
    int array_size = 100;
    
    /* Parse command-line arguments for different execution paths */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--hot-threshold") == 0 && i + 1 < argc) {
            hot_threshold = atoi(argv[++i]);
        }
    }
    
    /* Allocate and initialize array */
    int *data = (int *)malloc(array_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with seed-dependent values */
    cold_initialization(data, array_size, seed);
    
    /* Execute different code paths based on mode */
    switch (mode % 4) {
        case 0:
            process_mode_a(data, array_size);
            break;
        case 1:
            process_mode_b(data, array_size);
            break;
        case 2:
            process_mode_c(data, array_size);
            break;
        case 3:
            /* Use both A and B */
            process_mode_a(data, array_size);
            process_mode_b(data, array_size);
            break;
    }
    
    /* Execute hot functions with varying iterations */
    int hot_iterations = iterations * (1 + mode);
    for (int i = 0; i < hot_iterations / 1000; i++) {
        hot_loop_multiply(data, array_size, 10 + (i % 5));
    }
    
    /* Search with varying targets */
    int target = 500 + (seed % 300);
    hot_search_algorithm(data, array_size, target);
    
    /* Complex processing */
    int depth = mode + 2;
    int complex_result = complex_decision_tree(data, array_size, depth);
    
    /* Call external functions (from other compilation units) */
    process_data(data, array_size, mode);
    analyze_results(data, array_size, hot_threshold);
    
    /* Validation */
    cold_validation(data, array_size);
    
    /* Calculate checksum for deterministic output */
    unsigned long checksum = 0;
    for (int i = 0; i < array_size; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    checksum = (checksum + complex_result) % 1000000007;
    
    printf("Result: checksum=%lu, complex=%d, mode=%d, seed=%d\n", 
           checksum, complex_result, mode, seed);
    
    free(data);
    return 0;
}
