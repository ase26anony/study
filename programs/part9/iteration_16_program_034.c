/* types1.c - First translation unit with various type definitions */

/* TYPE_UNDEFINED: Forward declaration without definition */
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

/* Struct with __attribute__((packed)) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Struct with alignment attribute */
struct __attribute__((aligned(16))) AlignedStruct {
    double data[2];
    int tag;
};

/* Struct with deprecated attribute */
struct __attribute__((deprecated("Use NewStruct instead"))) OldStruct {
    int legacy_field;
};

/* Use #pragma pack */
#pragma pack(push, 1)
struct TightPacked {
    char flag;
    int value;
    short count;
};
#pragma pack(pop)

/* Complex nested struct */
struct ComplexContainer {
    struct Point points[10];
    MyStruct* items;
    const char* description;
    volatile int counter;
};

/* Function using various types */
void process_data(struct Point* p, MyStruct* ms) {
    p->x += ms->data;
}

/* Empty main - types are what matter */
int main(void) {
    return 0;
}
