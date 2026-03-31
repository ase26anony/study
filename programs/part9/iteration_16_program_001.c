/* types1.c - First translation unit with basic types */

/* TYPE_UNDEFINED: Forward declaration of incomplete struct */
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

/* TYPE_USER_STRUCT: Typedef for a struct */
typedef struct {
    int data;
    char name[32];
} MyStruct;

/* Packed struct with attributes */
struct __attribute__((packed)) PackedData {
    char flag;
    int value;
} __attribute__((aligned(4)));

/* Struct with pointer member */
struct Node {
    int value;
    struct Node* next;  /* TYPE_POINTER inside struct */
};

/* Use #pragma pack */
#pragma pack(push, 1)
struct TightPacked {
    char c;
    int i;
    short s;
};
#pragma pack(pop)

/* Struct with array member */
struct Buffer {
    char data[256];  /* TYPE_ARRAY inside struct */
    int size;
};

/* Function using various types */
void process_data(struct Point* p, MyStruct* ms) {
    /* Function body - not important for gengtype */
    (void)p;
    (void)ms;
}

/* Empty main - just to have a complete translation unit */
int main(void) {
    struct Point pt = {10, 20};
    MyStruct ms = {100, "Test"};
    struct Buffer buf = {{0}, 0};
    
    return 0;
}
