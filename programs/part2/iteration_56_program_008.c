#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* TYPE_UNDEFINED: Forward declarations of incomplete types */
struct IncompleteStruct;
union IncompleteUnion;
typedef struct OpaqueType OpaqueType;

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
    int data;
    float value;
} MyStruct;

typedef struct Node {
    int value;
    struct Node* next;  /* Pointer to same struct type */
} ListNode;

/* TYPE_UNION: Union declarations */
union Data {
    int i;
    float f;
    double d;
    char str[20];
};

typedef union {
    long long int_val;
    double double_val;
    void* ptr_val;
} FlexibleUnion;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);
typedef struct MyStruct* (*FactoryFunc)(void);

/* TYPE_LANG_STRUCT: GCC attribute extensions */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    double c;
};

struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
    float values[8];
};

/* External declarations for multi-file testing */
extern struct Point global_point;
extern MyStruct global_my_struct;
extern const char* global_string;

#endif /* TEST_TYPES_H */
