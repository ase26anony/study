#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* Test case 1: Function pointer types with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);
int (*(*complex_func_ptr)(int (*)(int, int), char*))(float, double);

/* Test case 2: Array declarations with multiple dimensions and nested initializers */
int arr_3d[2][3][2] = {
    { /* First 2D slice */
        {1, 2},     /* row 0 */
        {3, 4},     /* row 1 */
        {5, 6}      /* row 2 */
    },
    { /* Second 2D slice */
        {7, 8},     /* row 0 */
        {9, 10},    /* row 1 */
        {11, 12}    /* row 2 */
    }
};

/* Test case 3: Structures with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;    /* bit-field a */
            int b:4;    /* bit-field b */
            int c:16;   /* larger bit-field */
        };
        char raw_bytes[3];
    } nested_union;
    
    struct {
        int x;
        int y;
        struct {
            float f1;
            float f2;
        } inner_point;
    } point_data;
    
    int d[2];
};

/* Test case 4: Complex union with nested structs */
union MegaUnion {
    struct {
        int (*compare)(const void*, const void*);
        void (*print)(int, char**);
    } func_ptrs;
    
    struct {
        int matrix[2][2];
        char* strings[3];
    } data_container;
    
    long long big_value;
};

/* Test case 5: Typedef with function pointer returning array pointer */
typedef int (*(*FactoryFunc)(int size))[10];

/* Test case 6: Nested parentheses in type casts */
#define CAST_EXPR (*(int (*)[5])(&array))

#endif /* COMPLEX_TYPES_H */
