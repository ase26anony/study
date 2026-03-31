#include <stdio.h>
#include <stdlib.h>
#include "functions.h"

#define HOT_LOOP_COUNT 1000000
#define WARM_LOOP_COUNT 10000
#define COLD_LOOP_COUNT 10

int main(int argc, char *argv[]) {
    int input_value = 0;
    
    // Read input from file or command line
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input value: %d\n", input_value);
    
    // Branch 1: Hot path - executed many times
    if (input_value > 1000) {
        // Hot block - high execution count
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            hot_function(i);
        }
    }
    
    // Branch 2: Warm path
    if (input_value > 100 && input_value <= 1000) {
        for (int i = 0; i < WARM_LOOP_COUNT; i++) {
            warm_function(i);
        }
    }
    
    // Branch 3: Cold path
    if (input_value <= 100) {
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            cold_function(i);
        }
    }
    
    // Switch statement for additional branch coverage
    switch (input_value % 4) {
        case 0:
            function_a();
            break;
        case 1:
            function_b();
            break;
        case 2:
            function_c();
            break;
        case 3:
            function_d();
            break;
    }
    
    // Call object-specific functions
    if (input_value % 2 == 0) {
        obj1_function();
    } else {
        obj2_function();
    }
    
    return 0;
}
