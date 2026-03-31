#ifndef OVERLAP_H
#define OVERLAP_H

#include <stddef.h>

/* Function declarations for algorithms.c */
void bubble_sort(int *arr, size_t n);
void quick_sort(int *arr, int low, int high);
int binary_search(int *arr, size_t n, int target);
void matrix_multiply(int **a, int **b, int **result, int size);
void process_data(int *data, size_t size, int mode);

/* Function declarations for hot_cold.c */
__attribute__((hot)) void hot_function_loop(int iterations);
__attribute__((cold)) void cold_function_rare(void);
__attribute__((noinline)) void medium_work(int scale);
void temperature_controlled(int threshold);

/* Function declarations for utils.c */
int calculate_checksum(int *data, size_t size);
void fill_random(int *arr, size_t n, int seed);
void process_with_branches(int *arr, size_t n, int complexity);
void nested_conditions(int a, int b, int c);

/* Global configuration */
extern int global_verbose;
extern int global_hot_threshold;

#endif /* OVERLAP_H */
