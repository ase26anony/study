/* main.c - Primary source file with multiple functions for gcov profiling */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function declarations */
void hot_function(int iterations);
void cold_function(void);
void medium_function(int count);
void branching_function(int value);
void utility_function(int x);
void data_processing(int size);
void file_operations(void);
void memory_operations(int count);
void recursive_function(int depth, int max_depth);
void error_handling(int mode);

/* External functions from lib.c */
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_complex_function(int param);
extern void lib_data_transform(int *data, int size);
extern void lib_utility(int x);

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default mode */
    int iterations = 1000;
    
    /* Parse command line argument for mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1) mode = 1;
        if (mode > 3) mode = 3;
    }
    
    /* Set iterations based on mode */
    switch (mode) {
        case 1:
            iterations = 10000;  /* Many iterations for hot paths */
            printf("Running in mode 1 (hot paths)\n");
            break;
        case 2:
            iterations = 100;    /* Fewer iterations */
            printf("Running in mode 2 (medium paths)\n");
            break;
        case 3:
            iterations = 10;     /* Very few iterations */
            printf("Running in mode 3 (cold paths)\n");
            break;
    }
    
    /* Seed random number generator */
    srand(time(NULL) + mode);
    
    /* Execute functions with different frequencies based on mode */
    printf("Starting execution with %d iterations...\n", iterations);
    
    /* Always execute these */
    hot_function(iterations);
    cold_function();
    
    /* Mode-dependent execution */
    if (mode == 1) {
        /* Execute all functions many times */
        for (int i = 0; i < 5; i++) {
            medium_function(iterations / 10);
            branching_function(i * 100);
            utility_function(i);
        }
        
        data_processing(100);
        file_operations();
        memory_operations(50);
        recursive_function(1, 5);
        error_handling(0);
        
        /* Call library functions */
        lib_hot_function(iterations / 2);
        lib_cold_function();
        lib_complex_function(mode);
        
        int data[10];
        for (int i = 0; i < 10; i++) data[i] = i * mode;
        lib_data_transform(data, 10);
        lib_utility(mode * 10);
        
    } else if (mode == 2) {
        /* Execute fewer times */
        medium_function(iterations / 100);
        branching_function(50);
        utility_function(2);
        data_processing(20);
        memory_operations(10);
        
        /* Call some library functions */
        lib_hot_function(iterations / 10);
        lib_cold_function();
        
    } else { /* mode == 3 */
        /* Execute minimally */
        medium_function(5);
        branching_function(10);
        
        /* Call one library function */
        lib_cold_function();
    }
    
    printf("Execution complete for mode %d\n", mode);
    return 0;
}

/* Hot function - executed many times */
void hot_function(int iterations) {
    long sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i;
        if (i % 1000 == 0) {
            /* Occasionally do something extra */
            sum += i * 2;
        }
    }
    /* Prevent optimization */
    if (sum < 0) printf("Impossible\n");
}

/* Cold function - executed once */
void cold_function(void) {
    printf("Cold function executed\n");
    
    /* Some branching */
    int x = rand() % 100;
    if (x > 90) {
        printf("Rare path taken\n");
    } else if (x > 70) {
        printf("Uncommon path taken\n");
    } else {
        printf("Common path taken\n");
    }
}

/* Medium frequency function */
void medium_function(int count) {
    int result = 0;
    for (int i = 0; i < count; i++) {
        result += i % 10;
        if (i % 7 == 0) {
            result -= 1;
        }
    }
    
    /* Nested loop */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 5; j++) {
            result += i * j;
        }
    }
}

/* Function with multiple branches */
void branching_function(int value) {
    if (value < 0) {
        printf("Negative value\n");
    } else if (value == 0) {
        printf("Zero value\n");
    } else if (value < 10) {
        printf("Small positive value\n");
    } else if (value < 100) {
        printf("Medium positive value\n");
    } else if (value < 1000) {
        printf("Large positive value\n");
    } else {
        printf("Very large value\n");
    }
    
    /* Switch statement */
    switch (value % 5) {
        case 0:
            /* Do nothing */
            break;
        case 1:
            value += 1;
            break;
        case 2:
            value *= 2;
            break;
        case 3:
            value /= 2;
            break;
        case 4:
            value -= 1;
            break;
    }
}

/* Utility function */
void utility_function(int x) {
    /* Simple calculation */
    int y = x * x + 2 * x + 1;
    
    /* Conditional */
    if (y > 100) {
        y = 100;
    } else if (y < 0) {
        y = 0;
    }
}

/* Data processing function */
void data_processing(int size) {
    int *data = malloc(size * sizeof(int));
    if (!data) return;
    
    for (int i = 0; i < size; i++) {
        data[i] = i * 2;
    }
    
    /* Process data */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        if (data[i] % 3 == 0) {
            sum += 1;
        }
    }
    
    free(data);
}

/* File operations simulation */
void file_operations(void) {
    /* Simulate file operations */
    FILE *fp = fopen("/tmp/gcov_test.tmp", "w");
    if (fp) {
        fprintf(fp, "Test data for gcov\n");
        for (int i = 0; i < 10; i++) {
            fprintf(fp, "Line %d\n", i);
        }
        fclose(fp);
        
        /* Try to read it back */
        fp = fopen("/tmp/gcov_test.tmp", "r");
        if (fp) {
            char buffer[100];
            while (fgets(buffer, sizeof(buffer), fp)) {
                /* Process line */
            }
            fclose(fp);
        }
    }
}

/* Memory operations */
void memory_operations(int count) {
    int **arrays = malloc(count * sizeof(int*));
    if (!arrays) return;
    
    for (int i = 0; i < count; i++) {
        arrays[i] = malloc((i + 1) * sizeof(int));
        if (arrays[i]) {
            for (int j = 0; j <= i; j++) {
                arrays[i][j] = j * i;
            }
        }
    }
    
    /* Clean up */
    for (int i = 0; i < count; i++) {
        free(arrays[i]);
    }
    free(arrays);
}

/* Recursive function */
void recursive_function(int depth, int max_depth) {
    if (depth >= max_depth) {
        return;
    }
    
    /* Do some work */
    int x = depth * 10;
    
    /* Recursive calls */
    recursive_function(depth + 1, max_depth);
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
    }
}

/* Error handling function */
void error_handling(int mode) {
    int error_code = rand() % 10;
    
    switch (error_code) {
        case 0:
            printf("No error\n");
            break;
        case 1:
        case 2:
            printf("Warning %d\n", error_code);
            break;
        case 3:
        case 4:
        case 5:
            printf("Error %d\n", error_code);
            break;
        default:
            printf("Critical error %d\n", error_code);
            break;
    }
    
    /* Try-catch simulation */
    if (mode == 1) {
        /* Try something risky */
        int *ptr = NULL;
        if (error_code < 5) {
            ptr = malloc(sizeof(int));
        }
        
        if (ptr) {
            *ptr = 42;
            free(ptr);
        }
    }
}
