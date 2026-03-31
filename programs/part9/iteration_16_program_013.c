/* types1.c - Contains various type definitions for gengtype testing */

/* TYPE_UNDEFINED: Forward declaration without definition */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
_Bool global_bool = 1;
long global_long = 1000L;
unsigned int global_uint = 100U;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char* mutable_string = "Mutable";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* Packed struct with attributes */
struct __attribute__((packed)) PackedData {
    char id;
    int value;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int tag;
};

/* TYPE_USER_STRUCT: Typedef for struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* Another typedef with attributes */
typedef struct __attribute__((deprecated)) OldStruct {
    int legacy_field;
} DeprecatedStruct;

/* TYPE_POINTER: Various pointer types */
int* int_ptr = &global_int;
struct Point* point_ptr = 0;
void* void_ptr = 0;
const void* const_void_ptr = 0;
volatile int* volatile_int_ptr = 0;
const volatile char* const_volatile_char_ptr = 0;

/* Complex pointer combinations */
int** double_ptr = 0;
const int* const* complex_ptr = 0;
volatile const int* volatile* very_complex_ptr = 0;

/* TYPE_ARRAY: Fixed-size arrays */
int int_array[10] = {0};
char char_array[20] = "Array of chars";
struct Point point_array[5];
MyStruct mystruct_array[3];

/* Multi-dimensional array */
int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};

/* Array of pointers */
int* ptr_array[5];

/* Nested type: Struct containing array of pointers to unions */
struct Container {
    union Value* values[10];
    int count;
};

/* Function with various parameter types */
void process_data(int scalar, const char* string, struct Point* point, 
                  MyStruct* user_struct, int array[], int** ptr_to_ptr) {
    /* Function body - just for type context */
    (void)scalar;
    (void)string;
    (void)point;
    (void)user_struct;
    (void)array;
    (void)ptr_to_ptr;
}

/* Empty main - types are what matter */
int main(void) {
    return 0;
}
