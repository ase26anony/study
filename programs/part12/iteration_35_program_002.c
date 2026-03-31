#include <stdio.h>
#include <stdlib.h>
#include "funcs.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

int main(int argc, char *argv[]) {
    int input = 0;
    
    if (argc > 1) {
        input = atoi(argv[1]);
    } else {
        FILE *f = fopen("input.txt", "r");
        if (f) {
            fscanf(f, "%d", &input);
            fclose(f);
        }
    }
    
    printf("Running with input: %d\n", input);
    
    // Branch coverage based on input
    if (input < 0) {
        // Negative path - call function1 heavily
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            function1();
        }
    } else if (input == 0) {
        // Zero path - mixed calls
        function1();
        function2();
        function3();
    } else if (input < 100) {
        // Small positive - call function2 heavily
        for (int i = 0; i < HOT_LOOP_COUNT / 10; i++) {
            function2();
        }
    } else if (input < 1000) {
        // Medium positive - call function3
        function3();
        for (int i = 0; i < COLD_LOOP_COUNT; i++) {
            function2();
        }
    } else {
        // Large positive - hot loop with function1
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            function1();
            if (i % 1000 == 0) {
                function3();
            }
        }
    }
    
    // Additional conditional execution
    switch (input % 4) {
        case 0:
            helper_function_a();
            break;
        case 1:
            helper_function_b();
            break;
        case 2:
            helper_function_c();
            break;
        case 3:
            helper_function_d();
            break;
    }
    
    return 0;
}
