#ifndef DWARF_TEST_H
#define DWARF_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

/* For DW_AT_prototyped - ensure proper function prototypes */
int prototyped_function(int x, double y);

/* For string/array bounds attributes */
extern volatile char global_string[];
extern int array_with_lower_bound[5];

/* For concurrency attributes */
extern volatile int atomic_counter;

#ifdef __cplusplus
}
#endif

#endif /* DWARF_TEST_H */
