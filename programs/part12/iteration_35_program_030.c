/* test_prog.c - Program with varied execution paths for coverage analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations from other modules */
extern void func1(int iterations);
extern void func2(int iterations);
extern void func3(int iterations);
extern void hot_function(void);
extern void cold_function(void);

/* Global counter for hot/cold analysis */
static int global_counter = 0;

/* Main function with multiple execution paths */
int main(int argc, char *argv[]) {
    int input_value = 0;
    
    /* Read input value from command line or file */
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        /* Try to read from file if no command line argument */
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input value: %d\n", input_value);
    
    /* Branch 1: Very hot path (high iteration count) */
    if (input_value > 1000) {
        printf("Executing hot path\n");
        for (int i = 0; i < input_value; i++) {
            global_counter++;
            hot_function();
        }
    }
    /* Branch 2: Medium path */
    else if (input_value > 100) {
        printf("Executing medium path\n");
        func1(input_value);
        func2(input_value / 2);
    }
    /* Branch 3: Cold path */
    else if (input_value > 0) {
        printf("Executing cold path\n");
        func3(input_value);
        cold_function();
    }
    /* Branch 4: Default path */
    else {
        printf("Executing default path\n");
        /* Mix of hot and cold blocks */
        for (int i = 0; i < 10; i++) {
            if (i % 3 == 0) {
                hot_function();
            } else {
                cold_function();
            }
        }
    }
    
    /* Nested conditional for more coverage complexity */
    switch (input_value % 4) {
        case 0:
            printf("Case 0\n");
            break;
        case 1:
            printf("Case 1\n");
            func1(10);
            break;
        case 2:
            printf("Case 2\n");
            func2(20);
            break;
        case 3:
            printf("Case 3\n");
            func3(30);
            break;
    }
    
    return 0;
}

/* Local function with its own branches */
static void local_function(int x) {
    if (x > 50) {
        printf("Local: High value\n");
    } else if (x > 25) {
        printf("Local: Medium value\n");
    } else {
        printf("Local: Low value\n");
    }
    
    /* Loop with varying iterations */
    for (int i = 0; i < x % 10; i++) {
        global_counter += i;
    }
}
