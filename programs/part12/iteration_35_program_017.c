/* test_prog.c - Program with varied execution paths for coverage analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations from other modules */
extern void func_a(int iterations);
extern void func_b(int iterations);
extern void func_c(int iterations);
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
    
    /* Complex branching based on input value */
    if (input_value < 0) {
        /* Path 1: Negative values */
        printf("Negative path\n");
        func_a(100);
        cold_function();
    } else if (input_value == 0) {
        /* Path 2: Zero value */
        printf("Zero path\n");
        func_b(50);
        func_c(25);
    } else if (input_value > 0 && input_value <= 100) {
        /* Path 3: Small positive values */
        printf("Small positive path\n");
        for (int i = 0; i < input_value; i++) {
            func_a(1);
            if (i % 10 == 0) {
                func_b(1);
            }
        }
    } else {
        /* Path 4: Large values - create hot blocks */
        printf("Large value path (hot blocks)\n");
        int iterations = input_value > 1000 ? 1000 : input_value;
        
        /* HOT LOOP - will generate high execution counts */
        for (int i = 0; i < iterations * 10; i++) {
            hot_function();
            global_counter++;
            
            /* Occasionally call cold function */
            if (i % 100 == 0) {
                cold_function();
            }
        }
        
        /* More branching */
        switch (input_value % 4) {
            case 0:
                func_a(iterations / 2);
                break;
            case 1:
                func_b(iterations / 3);
                break;
            case 2:
                func_c(iterations / 4);
                break;
            case 3:
                func_a(iterations / 5);
                func_b(iterations / 5);
                func_c(iterations / 5);
                break;
        }
    }
    
    /* Final conditional based on global state */
    if (global_counter > 5000) {
        printf("Very hot execution detected!\n");
    } else if (global_counter > 1000) {
        printf("Warm execution detected\n");
    } else {
        printf("Cold execution detected\n");
    }
    
    return 0;
}
