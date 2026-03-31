#ifndef TEST_OMP_TEMPORARIES_H
#define TEST_OMP_TEMPORARIES_H

void test_reduction_temporaries(int n, int *results);
void test_scan_temporaries(int n, float *results);
void test_conditional_temporaries(int n, volatile int cond_var, double *results);
void test_enter_data_to(int n, int *array);

#endif
