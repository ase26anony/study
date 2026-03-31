#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;           /* Forward declaration */
union IncompleteUnion;             /* Forward declaration */
typedef struct OpaqueType OpaqueType; /* Opaque typedef */

/* TYPE_STRUCT: Plain C structures */
struct Point {
    int x;
    int y;
};

struct ComplexData {
    struct Point position;
    double timestamp;
};

/* TYPE_USER_STRUCT: Typedef'd structures */
typedef struct {
    int id;
    char name[32];
} UserStruct;

typedef struct Node {
    int data;
    struct Node* next;
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char* s;
};

union TaggedValue {
    int int_val;
    float float_val;
    void* ptr_val;
};

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    long long data[2];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern UserStruct global_user_struct;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);

#endif /* TEST_TYPES_H */
