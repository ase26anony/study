/* main.c - Primary source file with multiple functions for gcov testing */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Function declarations */
void hot_function_1(int iterations);
void hot_function_2(int iterations);
void cold_function_1(void);
void cold_function_2(void);
void mixed_function(int mode);
void recursive_function(int depth, int max_depth);
void loop_intensive(int count);
void conditional_intensive(int value);
void file_operations(void);
void memory_operations(int size);

/* External functions from lib.c */
extern void lib_hot_function(int iterations);
extern void lib_cold_function(void);
extern void lib_mixed_function(int mode);
extern double lib_compute_value(int base, int multiplier);
extern void lib_data_processing(int *data, int size);

/* Global variables for varied execution paths */
static int global_counter = 0;
static const char* mode_names[] = {"MODE_A", "MODE_B", "MODE_C", "MODE_D"};

void hot_function_1(int iterations) {
    /* This function should be hot - executed many times */
    int sum = 0;
    for (int i = 0; i < iterations; i++) {
        sum += i * i;
        if (i % 100 == 0) {
            sum -= i;
        }
    }
    global_counter += sum % 1000;
}

void hot_function_2(int iterations) {
    /* Another hot function with different pattern */
    long product = 1;
    for (int i = 1; i <= iterations; i++) {
        product = (product * i) % 10007;
        if (i % 50 == 0) {
            product = (product + 1) % 10007;
        }
    }
    global_counter += product % 100;
}

void cold_function_1(void) {
    /* This function should be cold - executed rarely */
    printf("Cold function 1 executed (rarely)\n");
    global_counter = (global_counter * 1103515245 + 12345) % 1000;
}

void cold_function_2(void) {
    /* Another cold function */
    if (global_counter > 500) {
        printf("Global counter is high: %d\n", global_counter);
    } else {
        printf("Global counter is low: %d\n", global_counter);
    }
}

void mixed_function(int mode) {
    /* Function with mixed execution frequency based on mode */
    switch (mode % 4) {
        case 0:
            hot_function_1(100);
            break;
        case 1:
            hot_function_2(50);
            break;
        case 2:
            cold_function_1();
            break;
        case 3:
            cold_function_2();
            break;
    }
}

void recursive_function(int depth, int max_depth) {
    /* Recursive function for varied call graphs */
    if (depth >= max_depth) {
        global_counter += depth;
        return;
    }
    
    if (depth % 2 == 0) {
        recursive_function(depth + 1, max_depth);
        recursive_function(depth + 2, max_depth);
    } else {
        recursive_function(depth + 1, max_depth);
    }
    
    global_counter += depth * 2;
}

void loop_intensive(int count) {
    /* Loop-intensive function */
    int *array = malloc(count * sizeof(int));
    if (!array) return;
    
    for (int i = 0; i < count; i++) {
        array[i] = i * i;
        if (i % 3 == 0) array[i] += 1;
        if (i % 5 == 0) array[i] *= 2;
        if (i % 7 == 0) array[i] /= 3;
    }
    
    int sum = 0;
    for (int i = 0; i < count; i += 2) {
        sum += array[i];
    }
    
    free(array);
    global_counter += sum % 1000;
}

void conditional_intensive(int value) {
    /* Function with many conditionals */
    if (value < 0) {
        global_counter -= 10;
    } else if (value < 10) {
        global_counter += 1;
    } else if (value < 20) {
        global_counter += 2;
    } else if (value < 30) {
        global_counter += 3;
    } else if (value < 40) {
        global_counter += 4;
    } else {
        global_counter += 5;
    }
    
    switch (value % 6) {
        case 0: hot_function_1(10); break;
        case 1: hot_function_2(10); break;
        case 2: cold_function_1(); break;
        case 3: cold_function_2(); break;
        case 4: loop_intensive(20); break;
        case 5: recursive_function(0, 3); break;
    }
}

void file_operations(void) {
    /* Simulate file operations */
    FILE *temp = tmpfile();
    if (temp) {
        for (int i = 0; i < 100; i++) {
            fprintf(temp, "Line %d: Value = %d\n", i, global_counter + i);
        }
        fclose(temp);
    }
}

void memory_operations(int size) {
    /* Memory allocation and manipulation */
    int *buffer = calloc(size, sizeof(int));
    if (!buffer) return;
    
    for (int i = 0; i < size; i++) {
        buffer[i] = (i * 7) % 13;
    }
    
    // Bubble sort for some CPU cycles
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (buffer[j] > buffer[j + 1]) {
                int temp = buffer[j];
                buffer[j] = buffer[j + 1];
                buffer[j + 1] = temp;
                global_counter++;
            }
        }
    }
    
    free(buffer);
}

int main(int argc, char *argv[]) {
    int mode = 0;
    int iterations = 1000;
    
    /* Parse command line arguments for different profile runs */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    srand(time(NULL) + mode);
    
    printf("Running in mode %d with %d iterations\n", mode, iterations);
    
    /* Vary execution based on mode to create different profiles */
    switch (mode % 3) {
        case 0:  // Mode 0: Heavy on hot functions
            for (int i = 0; i < iterations; i++) {
                hot_function_1(100);
                hot_function_2(50);
                if (i % 100 == 0) {
                    cold_function_1();
                }
                lib_hot_function(50);
            }
            loop_intensive(100);
            break;
            
        case 1:  // Mode 1: Balanced execution
            for (int i = 0; i < iterations / 2; i++) {
                mixed_function(i);
                lib_mixed_function(i % 4);
                conditional_intensive(i % 50);
                if (i % 50 == 0) {
                    recursive_function(0, 4);
                }
            }
            memory_operations(50);
            break;
            
        case 2:  // Mode 2: More cold paths
            for (int i = 0; i < iterations / 10; i++) {
                cold_function_1();
                cold_function_2();
                lib_cold_function();
                if (i % 5 == 0) {
                    hot_function_1(10);
                    lib_hot_function(10);
                }
            }
            file_operations();
            break;
    }
    
    /* Always execute some common code */
    for (int i = 0; i < 10; i++) {
        double val = lib_compute_value(i, mode + 1);
        global_counter += (int)val;
    }
    
    printf("Final global counter: %d\n", global_counter);
    
    return 0;
}
