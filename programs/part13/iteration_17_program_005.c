/*
 * Test file to trigger generation of specific DWARF attributes in GCC
 * Each section targets specific DW_AT_* attributes from dwarf2out.cc
 */

#include <stddef.h>

/* ====== DW_AT_string_length, DW_AT_string_length_bit_size, DW_AT_string_length_byte_size ====== */
/* Target: String length attributes for Fortran-style strings or C++ strings */
typedef char fixed_string[32];
fixed_string s = "test_string_for_debug";

/* Simulate Fortran CHARACTER type with length */
struct fortran_string {
    char *data;
    size_t length;  /* This might trigger string length attributes */
};

/* ====== DW_AT_picture_string ====== */
/* Target: COBOL/Fortran PICTURE data - use GCC attribute if available */
#ifdef __GNUC__
struct cobol_picture {
    char data[20];
} __attribute__((packed));
#else
#pragma pack(push, 1)
struct cobol_picture {
    char data[20];
};
#pragma pack(pop)
#endif

/* ====== DW_AT_threads_scaled ====== */
/* Target: OpenMP thread-related variables */
#ifdef _OPENMP
#include <omp.h>
int omp_global_var = 42;
#pragma omp threadprivate(omp_global_var)
#endif

/* Thread-local storage (might trigger threads_scaled in some contexts) */
__thread int thread_local_var = 100;

/* ====== DW_AT_explicit ====== */
/* Target: C++ explicit constructors - will be in C++ section */
/* Note: This requires C++ compilation */

/* ====== DW_AT_is_optional ====== */
/* Target: Optional parameters/types - simulated with pointer and flag */
struct optional_int {
    int has_value;
    int value;
};

/* ====== DW_AT_mutable ====== */
/* Target: C++ mutable members - will be in C++ section */

/* ====== DW_AT_ordering ====== */
/* Target: Array ordering (column-major for Fortran) */
#ifdef __GNUC__
int column_major_array[5][5] __attribute__((column_major));
#else
int column_major_array[5][5];
#endif

/* ====== DW_AT_segment ====== */
/* Target: Segment-specific pointers */
#ifdef __i386__
/* x86 segment registers */
int __seg_fs *fs_seg_ptr;
int __seg_gs *gs_seg_ptr;
#endif

/* Far pointer simulation for segmented architectures */
#ifdef __MSDOS__
int __far far_ptr;
#endif

/* ====== DW_AT_prototyped ====== */
/* Target: Functions with prototypes */
int prototyped_function(int a, char b, double c);
int (*func_ptr)(int, char, double) = &prototyped_function;

int prototyped_function(int a, char b, double c) {
    return a + (int)b + (int)c;
}

/* ====== DW_AT_small ====== */
/* Target: Packed/small types */
struct small_packed {
    unsigned int flag:1;
    unsigned int tiny:3;
} __attribute__((packed));

/* Bitfield structure */
struct bitfield_struct {
    unsigned int a:4;
    unsigned int b:4;
    unsigned int c:8;
    unsigned int d:16;
};

/* ====== Complex type compositions to stress DIE generation ====== */

/* Function returning pointer to array */
int (*complex_func(void))[10] {
    static int arr[10];
    return &arr;
}

/* Pointer to function returning struct */
struct small_packed (*func_returning_struct_ptr)(int);

/* Const volatile qualified types */
const volatile int cv_var = 42;

/* Restrict qualified pointer */
int * restrict restrict_ptr;

/* Static variables at different scopes */
static int file_static = 99;

void test_function(void) {
    static int function_static = 77;
    register int reg_var = 33;
    auto int auto_var = 44;
    volatile int vol_var = 55;
    
    /* Array with multiple dimensions */
    int multi_array[2][3][4];
    
    /* Pointer to array of pointers */
    int *ptr_array[5];
    
    /* Reference to array */
    int (&array_ref)[10] = *(int(*)[10])0;
}

/* Main function to ensure everything is used */
int main(void) {
    test_function();
    
#ifdef _OPENMP
    #pragma omp parallel
    {
        omp_global_var = omp_get_thread_num();
    }
#endif
    
    return 0;
}
