/* types1.c - First translation unit with basic types */

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
struct Opaque;

/* TYPE_SCALAR: Basic scalar types */
int global_int = 42;
float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
volatile int volatile_int = 100;
const double const_double = 1.414;

/* TYPE_STRING: String types */
const char* global_string = "Hello, World!";
char global_char_array[] = "Test String";

/* TYPE_STRUCT: Plain C struct */
struct Point {
    int x;
    int y;
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* Nested struct with array */
struct ComplexStruct {
    struct Point points[10];
    MyStruct data;
    volatile const int* flags;
};

/* Packed struct with attribute */
struct __attribute__((packed)) PackedData {
    char id;
    int value;
    short count;
};

/* Aligned struct */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[4];
    int tag;
};

/* Deprecated struct */
struct __attribute__((deprecated("Use NewStruct instead"))) OldStruct {
    int legacy_field;
};

/* Use #pragma pack */
#pragma pack(push, 1)
struct TightPacked {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Empty main - types are what matter */
int main(void) {
    return 0;
}
