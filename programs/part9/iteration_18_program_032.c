/* gcov_tool_test.c - Multi-run profile generator for gcov-tool overlap analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function attributes to control instrumentation */
#define NOINLINE __attribute__((noinline))
#define HOT __attribute__((hot))
#define COLD __attribute__((cold))

/* Global configuration */
static int g_mode = 0;
static int g_iterations = 1000;
static int g_seed = 42;
static int g_algorithm = 0;
static int g_use_fullname_test = 0;

/* ========== HOT FUNCTIONS (high execution count) ========== */

HOT NOINLINE
void hot_loop_processor(int *data, int size) {
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        if (data[i] > 0) {
            sum += data[i] * 2;
        } else {
            sum -= data[i];
        }
        
        /* Nested condition for branch coverage */
        if (i % 3 == 0) {
            data[i] = (data[i] * 3) / 2;
        } else if (i % 3 == 1) {
            data[i] = data[i] + 100;
        } else {
            data[i] = data[i] - 50;
        }
    }
    
    /* Prevent dead code elimination */
    if (sum < 0) {
        printf("Impossible negative sum\n");
    }
}

HOT NOINLINE
void hot_search_algorithm(int *array, int size, int target) {
    int found = 0;
    int comparisons = 0;
    
    /* Different search algorithms based on mode */
    if (g_algorithm == 0) {
        /* Linear search - always executes */
        for (int i = 0; i < size; i++) {
            comparisons++;
            if (array[i] == target) {
                found = 1;
                break;
            }
        }
    } else if (g_algorithm == 1) {
        /* Binary search style (array may not be sorted) */
        int left = 0, right = size - 1;
        while (left <= right && comparisons < 100) {
            comparisons++;
            int mid = left + (right - left) / 2;
            if (array[mid] == target) {
                found = 1;
                break;
            } else if (array[mid] < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    } else {
        /* Random search */
        for (int i = 0; i < size / 10; i++) {
            comparisons++;
            int idx = rand() % size;
            if (array[idx] == target) {
                found = 1;
                break;
            }
        }
    }
    
    if (found && comparisons > size) {
        /* This branch rarely executes */
        printf("Inefficient search detected\n");
    }
}

/* ========== COLD FUNCTIONS (low execution count) ========== */

COLD NOINLINE
void cold_initialization(int *data, int size) {
    /* Executes once per run */
    if (g_mode == 0) {
        for (int i = 0; i < size; i++) {
            data[i] = (i * 3) % 97;
        }
    } else if (g_mode == 1) {
        for (int i = 0; i < size; i++) {
            data[i] = rand() % 1000;
        }
    } else {
        for (int i = 0; i < size; i++) {
            data[i] = i;
        }
    }
}

COLD NOINLINE
void cold_finalizer(int *data, int size) {
    /* Executes once per run with complex branching */
    int checksum = 0;
    for (int i = 0; i < size; i++) {
        switch (data[i] % 5) {
            case 0: checksum += data[i] * 2; break;
            case 1: checksum += data[i] / 2; break;
            case 2: checksum += data[i] + 100; break;
            case 3: checksum += data[i] - 50; break;
            case 4: checksum += data[i] % 23; break;
            default: checksum += 1;
        }
    }
    printf("Final checksum: %d\n", checksum);
}

/* ========== MEDIUM FREQUENCY FUNCTIONS ========== */

NOINLINE
void medium_processor_a(int *data, int size) {
    int temp = 0;
    for (int i = 0; i < size / 2; i++) {
        if (data[i] % 2 == 0) {
            temp += data[i];
        } else {
            temp -= data[i] / 3;
        }
        
        /* Nested loop with varying iterations */
        for (int j = 0; j < (i % 5); j++) {
            temp += j;
        }
    }
    
    /* Rare condition */
    if (temp > 1000000) {
        printf("Unexpected large temp in processor A\n");
    }
}

NOINLINE
void medium_processor_b(int *data, int size) {
    int product = 1;
    for (int i = size / 2; i < size; i++) {
        if (data[i] != 0) {
            product *= (data[i] % 10 + 1);
        }
        
        /* Complex conditional chain */
        if (data[i] < -100) {
            product = -product;
        } else if (data[i] > 1000) {
            product = product / 2;
        } else if (data[i] == 0) {
            product = 1;
        }
    }
    
    /* Prevent optimization */
    if (product == 0) {
        printf("Product zero in processor B\n");
    }
}

NOINLINE
void medium_processor_c(int *data, int size) {
    /* Different behavior based on mode */
    if (g_mode == 0) {
        for (int i = 0; i < size; i += 2) {
            data[i] = data[i] * 2 + 1;
        }
    } else if (g_mode == 1) {
        for (int i = 1; i < size; i += 2) {
            data[i] = data[i] / 2 - 1;
        }
    } else {
        for (int i = 0; i < size; i += 3) {
            data[i] = (data[i] + 7) * 3;
        }
    }
}

/* ========== FULLNAME TEST FUNCTIONS (same name in different scopes) ========== */

/* Namespace simulation using struct */
struct NamespaceA {
    static NOINLINE void process(int *data, int size) {
        int count = 0;
        for (int i = 0; i < size; i++) {
            if (data[i] > 500) count++;
        }
        if (count > size / 2) {
            printf("NamespaceA: Many large values\n");
        }
    }
};

struct NamespaceB {
    static NOINLINE void process(int *data, int size) {
        int sum = 0;
        for (int i = 0; i < size; i++) {
            sum += data[i];
        }
        if (sum < 0) {
            printf("NamespaceB: Negative sum\n");
        }
    }
};

/* ========== MAIN EXECUTION LOGIC ========== */

NOINLINE
void run_simulation(int *data, int size) {
    /* Vary function call order based on mode */
    if (g_mode == 0) {
        cold_initialization(data, size);
        hot_loop_processor(data, size);
        medium_processor_a(data, size);
        medium_processor_b(data, size);
        hot_search_algorithm(data, size, 42);
        cold_finalizer(data, size);
    } else if (g_mode == 1) {
        cold_initialization(data, size);
        medium_processor_c(data, size);
        hot_loop_processor(data, size);
        hot_loop_processor(data, size);  /* Called twice! */
        hot_search_algorithm(data, size, 999);
        medium_processor_a(data, size);
        cold_finalizer(data, size);
    } else {
        cold_initialization(data, size);
        for (int i = 0; i < 3; i++) {
            medium_processor_b(data, size);
        }
        hot_search_algorithm(data, size, -1);
        cold_finalizer(data, size);
    }
    
    /* Call same-named functions for fullname testing */
    if (g_use_fullname_test) {
        NamespaceA::process(data, size);
        NamespaceB::process(data, size);
    }
}

int main(int argc, char *argv[]) {
    /* Parse command-line arguments for variability */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            g_mode = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            g_iterations = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            g_seed = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--algorithm") == 0 && i + 1 < argc) {
            g_algorithm = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--fullname-test") == 0) {
            g_use_fullname_test = 1;
        }
    }
    
    /* Seed RNG for variability between runs */
    srand(g_seed);
    
    /* Create data array */
    int data_size = 1000 + (g_mode * 100);
    int *data = (int*)malloc(data_size * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Run the simulation multiple times */
    for (int iter = 0; iter < g_iterations; iter++) {
        run_simulation(data, data_size);
        
        /* Occasionally modify data between iterations */
        if (iter % 100 == 0) {
            for (int i = 0; i < 10; i++) {
                data[rand() % data_size] = rand() % 2000 - 500;
            }
        }
    }
    
    /* Final output to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < data_size; i++) {
        final_sum += data[i];
    }
    printf("Final data sum: %d (mode=%d, seed=%d, algo=%d)\n", 
           final_sum, g_mode, g_seed, g_algorithm);
    
    free(data);
    return 0;
}
