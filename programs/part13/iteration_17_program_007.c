/*
 * Test program to trigger generation of specific DWARF attributes
 * Target attributes from dwarf2out.cc lines 7602-7643
 */

#include <stddef.h>

/* For DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size */
/* Fixed-length string type */
typedef char fixed_string[32];
fixed_string global_string = "test_string";

/* Struct with string member */
struct string_struct {
    char data[64];
    int length;
};

/* For DW_AT_picture_string - COBOL/Fortran PICTURE data */
/* Use GCC attribute if available */
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#if __has_attribute(picture) || (defined(__GNUC__) && __GNUC__ >= 5)
#define PICTURE_ATTR __attribute__((picture("999V99")))
#else
#define PICTURE_ATTR
#endif

struct PICTURE_ATTR picture_data {
    char digits[8];
};

/* For DW_AT_threads_scaled - OpenMP threading */
#ifdef _OPENMP
#include <omp.h>
int omp_thread_var;
#pragma omp threadprivate(omp_thread_var)

/* Thread-local storage */
__thread int thread_local_var = 42;
#endif

/* For DW_AT_segment - segment-specific pointers */
/* Use segment attributes if available */
#if defined(__i386__) || defined(__x86_64__)
/* These attributes are compiler-specific */
int * __attribute__((address_space(257))) fs_ptr;  /* FS segment */
int * __attribute__((address_space(256))) gs_ptr;  /* GS segment */
#endif

/* For DW_AT_prototyped */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

/* For DW_AT_small - packed structures */
struct __attribute__((packed)) small_struct {
    unsigned int flag:1;
    unsigned int value:7;
    char data;
};

/* For DW_AT_ordering - array ordering (column-major) */
#if __has_attribute(column_major) || (defined(__GNUC__) && __GNUC__ >= 5)
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* For DW_AT_lower_bound - array with non-zero lower bound */
/* Use GNU extension for array ranges */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
int bounded_array[10] __attribute__((aligned(16)));
typedef int array_type[static 10];
#endif

/* For DW_AT_location - test various storage classes */
static int static_var = 100;
volatile int volatile_var = 200;
register int register_var asm("ebx");  /* Note: compiler may ignore */

/* Complex type composition for stress testing */
typedef int (*complex_func_ptr)(int (*)(char), double[10]);
const volatile complex_func_ptr cv_func_ptr = NULL;

/* Function declarations */
int prototyped_function(int a, char b, double c) {
    return a + b + (int)c;
}

void test_strings(void) {
    fixed_string local_string = "local";
    struct string_struct str = {.data = "test", .length = 4};
    
    /* String length operations */
    size_t len = sizeof(local_string);
    (void)len;
}

void test_openmp(void) {
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_thread_var = omp_get_thread_num();
        thread_local_var = omp_thread_var * 10;
    }
#endif
}

void test_segments(void) {
#if defined(__i386__) || defined(__x86_64__)
    /* Access through segment pointers */
    int local = 42;
    fs_ptr = &local;
    gs_ptr = &local;
#endif
}

void test_arrays(void) {
    /* Test column-major access */
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            column_major_array[i][j] = i * 5 + j;
        }
    }
    
    /* Test bounded array */
#if defined(__GNUC__) && !defined(__STRICT_ANSI__)
    for (int i = 0; i < 10; i++) {
        bounded_array[i] = i * 2;
    }
#endif
}

/* Test small packed structure */
void test_packed(void) {
    struct small_struct s = {.flag = 1, .value = 42, .data = 'X'};
    volatile char *ptr = (volatile char *)&s;
    (void)ptr;  /* Prevent optimization */
}

/* Main function */
int main(void) {
    test_strings();
    test_openmp();
    test_segments();
    test_arrays();
    test_packed();
    
    /* Use function pointer */
    if (func_ptr) {
        int result = prototyped_function(10, 'A', 3.14);
        (void)result;
    }
    
    return 0;
}
