#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct Opaque OpaqueType;

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct ComplexData {
    int id;
    char name[50];
    struct Point location;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int counter;
    float value;
    char label[20];
} MyStruct;

typedef struct Node {
    int data;
    struct Node* next;
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char c;
};

union Variant {
    struct Point point;
    MyStruct mystruct;
    union Data data;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);
typedef struct Point* (*PointFactory)(int, int);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    float matrix[2][2];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_mystruct;
extern union Data global_data;

/* Function declarations using various types */
struct Point* create_point(int x, int y);
void process_data(union Data* data, CallbackFunc callback);
int compare_points(const void* a, const void* b);

#endif /* TEST_TYPES_H */
