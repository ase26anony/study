/* types1.c - First translation unit with various type definitions */

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
_Bool global_bool = 1;
long long global_long_long = 1234567890LL;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char global_char_array[] = "String literal";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* Nested struct with array */
struct Container {
    struct Point points[10];
    MyStruct items[5];
    volatile int counter;
};

/* Struct with GCC attributes */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

/* Struct with deprecated attribute */
struct __attribute__((deprecated)) OldStruct {
    int legacy_field;
};

/* Use #pragma pack */
#pragma pack(push, 1)
struct TightPacked {
    char flag;
    int value;
    short index;
};
#pragma pack(pop)

/* Global instances */
struct Point origin = {0, 0};
MyStruct my_instance = {100, "test"};

/* Function declarations for callbacks */
typedef int (*comparator_t)(const void*, const void*);
extern void sort_array(int* arr, int size, comparator_t cmp);

/* Main function (minimal) */
int main(void) {
    return 0;
}
