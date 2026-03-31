#include <stdio.h>
#include <stdlib.h>
#include "func1.h"
#include "func2.h"
#include "func3.h"

#define HOT_LOOP_COUNT 1000000
#define COLD_LOOP_COUNT 100

void process_input(int value, int iteration) {
    if (value > 1000) {
        // Hot path - executed many times
        for (int i = 0; i < HOT_LOOP_COUNT; i++) {
            func1_hot();
        }
        printf("Iteration %d: Hot path taken\n", iteration);
    } else if (value > 500) {
        // Medium path
        func2_medium(value);
        printf("Iteration %d: Medium path taken\n", iteration);
    } else if (value > 100) {
        // Cold path - rarely executed
        func3_cold();
        printf("Iteration %d: Cold path taken\n", iteration);
    } else {
        // Very cold path
        printf("Iteration %d: Very cold path\n", iteration);
    }
    
    // Switch statement for additional coverage
    switch (value % 4) {
        case 0:
            func1_helper();
            break;
        case 1:
            func2_helper();
            break;
        case 2:
            func3_helper();
            break;
        case 3:
            // Empty case for coverage
            break;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_value> <output_file>\n", argv[0]);
        return 1;
    }
    
    int value = atoi(argv[1]);
    const char *output_file = argv[2];
    
    FILE *f = fopen(output_file, "w");
    if (!f) {
        perror("Failed to open output file");
        return 1;
    }
    
    fprintf(f, "Processed value: %d\n", value);
    fclose(f);
    
    // Process multiple times to generate execution counts
    for (int i = 0; i < 10; i++) {
        process_input(value + i, i);
    }
    
    return 0;
}
