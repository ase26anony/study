/* Complex type declarations with nested delimiters to trigger consume_balanced calls */

/* Phase 1: Function pointers with complex signatures */
int (*(*complex_callback)(int (*)(float)))[10];

/* Multi-dimensional array with nested initializer braces */
int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};

/* Phase 2: Structures with nested anonymous structs and bit-fields */
struct Outer {
    union {
        struct {
            int a : 5;
            int b : 3;
        };
        long c;
    };
    int (*(*func_ptr_arr[2])(void))[3];
};

/* Phase 3: Type definitions with parentheses for grouping */
typedef int (*array_of_5_funcs[5])(char, ...);

/* Phase 4: Even more complex nested types */
typedef int (*(*(*nested_func_ptr)(int, ...))[10])(void);

/* GCC attributes with nested parentheses */
struct __attribute__((aligned(32), packed)) AlignedStruct {
    int data[8];
    char __attribute__((aligned(16))) aligned_char;
};

/* Function with format attribute containing nested parentheses */
void log_message(const char *format, ...) 
    __attribute__((format(printf, 1, 2)));

/* Phase 5: Macros to generate delimiter-heavy code */
#define MAKE_COMPLEX_TYPE(n) int (*(*var##n)[n])(char (*)[n])
#define NESTED_ARRAY_TYPE(dim1, dim2) int (*array_##dim1##_##dim2)[dim1][dim2]

/* Instantiate macros with different parameters */
MAKE_COMPLEX_TYPE(5);
MAKE_COMPLEX_TYPE(10);
NESTED_ARRAY_TYPE(3, 4) array_ptr;

/* Phase 6: Compound literals with nested braces */
int (*get_matrix(void))[4] {
    static int local_matrix[3][4] = {
        {1, 2, {3, 4}},
        {5, {6}, 7, 8},
        {{9}, 10, 11, 12}
    };
    return local_matrix;
}

/* Phase 7: Complex sizeof expressions */
size_t sizes[] = {
    sizeof(int[10][20]),
    sizeof(int (*[5])(void)),
    sizeof(struct { int a; char b[10]; union { int x; double y; }; }),
    sizeof(__attribute__((aligned(64))) int)
};

/* Phase 8: Cast expressions with nested parentheses */
void *complex_cast = (void *)(int (*[5])(char (*)[10])){0};

/* Phase 9: Variable Length Array in parameter */
void process_vla(int rows, int cols, int matrix[rows][cols]) {
    /* Nested loop with compound statement */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j;
        }
    }
}

/* Phase 10: Designated initializers with nested designators */
struct Point3D {
    int x, y, z;
};

struct NestedContainer {
    struct Point3D points[2][2];
    int data[3];
};

struct NestedContainer container = {
    .points = {
        [0] = {
            {.x = 1, .y = 2, .z = 3},
            {.x = 4, .y = 5, .z = {6}}
        },
        [1] = {
            {.x = 7, .y = 8, .z = 9},
            {.x = 10, .y = 11, .z = 12}
        }
    },
    .data = {[0] = 100, [1] = 200, [2] = 300}
};

/* Main function - minimal but uses complex types */
int main(void) {
    /* Declare and use complex types */
    int (*(*local_callback)(int (*)(float)))[10] = NULL;
    
    /* Use sizeof on complex array type */
    size_t matrix_size = sizeof(int[3][4][5]);
    
    /* Compound literal */
    int (*arr_ptr)[4] = (int (*)[4])&matrix[0];
    
    /* Nested switch with braces */
    int choice = 2;
    switch (choice) {
        case 1: {
            int nested_array[2][3] = {{1,2,3}, {4,5,6}};
            break;
        }
        case 2: {
            struct { int a; char b[5]; } s = {.a = 1, .b = "test"};
            break;
        }
        default: {
            union { int i; float f; } u = {.f = 3.14f};
            break;
        }
    }
    
    /* Call function with VLA parameter */
    int local_matrix[2][3];
    process_vla(2, 3, local_matrix);
    
    return 0;
}
