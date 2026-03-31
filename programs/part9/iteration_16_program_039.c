/* types1.c - Contains scalar, string, struct, and pointer types */

/* TYPE_UNDEFINED: Forward declaration */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
_Bool global_bool = 1;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char global_char_array[] = "String literal";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    int data;
    char* name;
} MyStruct;

/* Nested struct with complex types */
struct ComplexStruct {
    struct Point* points;      /* TYPE_POINTER to TYPE_STRUCT */
    int* int_ptr;              /* TYPE_POINTER to TYPE_SCALAR */
    char** string_array;       /* TYPE_POINTER to TYPE_POINTER to TYPE_SCALAR */
    MyStruct user_struct;      /* TYPE_USER_STRUCT */
};

/* Packed struct with attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[4];
};

/* Volatile and const qualified pointers */
volatile const int* volatile* complex_ptr;

/* Function with various parameter types */
void process_data(struct Point* p, const char* str, int count) {
    /* Function body - just for type context */
    (void)p;
    (void)str;
    (void)count;
}

/* Main function - minimal */
int main(void) {
    struct Point pt = {10, 20};
    MyStruct ms = {100, "Test"};
    
    return 0;
}
