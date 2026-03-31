#include <stdio.h>
#include <stdlib.h>
#include "hot_functions.h"
#include "cold_functions.h"

int main(int argc, char *argv[]) {
    int input_value = 0;
    
    if (argc > 1) {
        input_value = atoi(argv[1]);
    } else {
        // Read from file if no command line argument
        FILE *fp = fopen("input.txt", "r");
        if (fp) {
            fscanf(fp, "%d", &input_value);
            fclose(fp);
        }
    }
    
    printf("Running with input value: %d\n", input_value);
    
    // Branch based on input value - creates varied coverage
    if (input_value < 0) {
        // Cold path 1
        rarely_called_function();
        cold_function_a();
    } else if (input_value < 10) {
        // Medium path
        moderately_called_function(input_value);
        hot_function_a(5);
    } else if (input_value < 100) {
        // Hot path 1 - large loop
        for (int i = 0; i < input_value * 1000; i++) {
            hot_function_a(i % 10);
        }
        hot_function_b(50);
    } else {
        // Hot path 2 - different hot blocks
        for (int i = 0; i < 50000; i++) {
            hot_function_b(i % 20);
        }
        for (int j = 0; j < 10000; j++) {
            hot_function_c(j);
        }
    }
    
    // Always execute some common code
    common_function();
    
    // Switch statement for additional branch coverage
    switch (input_value % 4) {
        case 0:
            cold_function_b();
            break;
        case 1:
            moderately_called_function(1);
            break;
        case 2:
            hot_function_a(2);
            break;
        case 3:
            rarely_called_function();
            break;
    }
    
    return 0;
}
