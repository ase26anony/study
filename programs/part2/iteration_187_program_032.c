/* main.c - Primary source file with multiple functions for gcov testing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function declarations from lib.c */
extern void process_data(int iterations, int threshold);
extern int calculate_value(int base, int multiplier);
extern void handle_edge_cases(int scenario);
extern void utility_function_a(void);
extern void utility_function_b(void);
extern void cold_path_function(void);

/* Local function declarations */
static int hot_function_a(int iterations);
static int hot_function_b(int iterations);
static void warm_function(int value);
static void cold_function(void);
static void conditional_execution(int mode, int count);

/* Global variables for varied execution paths */
static int global_counter = 0;
static int execution_mode = 0;

/* Hot function - executed many times */
static int hot_function_a(int iterations) {
    int result = 0;
    for (int i = 0; i < iterations; i++) {
        result += i * 2;
        global_counter++;
        
        /* Branch with different execution frequencies */
        if (i % 3 == 0) {
            result -= i;
        } else if (i % 7 == 0) {
            result += i * 3;
        }
    }
    return result;
}

/* Another hot function with different pattern */
static int hot_function_b(int iterations) {
    int sum = 0;
    int j = 0;
    
    while (j < iterations) {
        sum += j;
        
        /* Nested loop for more complex coverage */
        for (int k = 0; k < 5; k++) {
            if (j % 2 == 0) {
                sum += k;
            } else {
                sum -= k;
            }
        }
        
        j++;
        
        /* Early exit condition */
        if (sum > 10000 && iterations > 100) {
            break;
        }
    }
    
    return sum;
}

/* Warm function - executed moderately */
static void warm_function(int value) {
    printf("Warm function processing: %d\n", value);
    
    switch (value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            break;
        case 2:
            printf("Case 2\n");
            break;
        case 3:
            printf("Case 3\n");
            break;
        default:
            printf("Default case\n");
    }
    
    /* Small loop */
    for (int i = 0; i < value % 10; i++) {
        global_counter += i;
    }
}

/* Cold function - rarely executed */
static void cold_function(void) {
    printf("Cold function executed!\n");
    
    /* Complex but rarely taken path */
    int x = 0;
    while (x < 3) {
        printf("Cold loop iteration %d\n", x);
        
        for (int y = 0; y < 2; y++) {
            if (x == y) {
                printf("x equals y: %d\n", x);
            }
        }
        x++;
    }
}

/* Function with conditional execution based on mode */
static void conditional_execution(int mode, int count) {
    printf("Conditional execution mode: %d\n", mode);
    
    if (mode == 1) {
        /* Mode 1: Heavy computation */
        int result = hot_function_a(count * 10);
        printf("Mode 1 result: %d\n", result);
        
        /* Call library functions */
        process_data(count, 50);
        calculate_value(result, 2);
    } 
    else if (mode == 2) {
        /* Mode 2: Mixed execution */
        hot_function_b(count * 5);
        warm_function(count);
        
        /* Different library calls */
        handle_edge_cases(count % 3);
        utility_function_a();
    }
    else if (mode == 3) {
        /* Mode 3: Cold paths */
        cold_function();
        cold_path_function();
        
        /* Minimal hot execution */
        if (count > 0) {
            hot_function_a(10);
        }
    }
    else {
        /* Default mode: Balanced execution */
        hot_function_a(count);
        hot_function_b(count / 2);
        warm_function(count % 20);
        utility_function_b();
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;
    int iterations = 100;
    
    /* Parse command line arguments for different execution profiles */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Starting gcov test program - Mode: %d, Iterations: %d\n", 
           mode, iterations);
    
    execution_mode = mode;
    
    /* Seed random number generator for varied execution */
    srand(time(NULL) + mode);
    
    /* Main execution loop with varied behavior based on mode */
    for (int run = 0; run < iterations; run++) {
        /* Vary the count parameter */
        int count = (run % 20) + 1;
        
        /* Execute based on mode */
        conditional_execution(mode, count);
        
        /* Occasionally call cold functions */
        if (run % 50 == 0) {
            cold_function();
        }
        
        /* Progress indicator for large runs */
        if (iterations > 1000 && run % 100 == 0) {
            printf("Progress: %d/%d\n", run, iterations);
        }
    }
    
    /* Final summary */
    printf("Execution complete. Global counter: %d\n", global_counter);
    printf("Final calculation: %d\n", 
           calculate_value(global_counter, execution_mode));
    
    return 0;
}
