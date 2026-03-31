#ifndef TEST_GTY_COVERAGE_H
#define TEST_GTY_COVERAGE_H

#include <stddef.h>

/* Test case 1: Struct with parentheses (function pointers) */
struct GTY(()) StructWithParens {
  /* Function pointer with parentheses */
  int (*callback)(int, char*);
  
  /* Complex function pointer with nested parentheses */
  void (*(*complex_callback)(int))(void);
  
  /* Bitfield with parenthesized expression */
  unsigned int bits: (sizeof(int) * 8 - 1);
  
  /* Pointer to array (mixing parentheses and brackets) */
  int (*array_ptr)[10];
};

/* Test case 2: Union with brackets (arrays) */
union GTY(()) UnionWithBrackets {
  /* Simple array */
  int simple_array[10];
  
  /* Multi-dimensional array */
  double matrix[3][3];
  
  /* Flexible array member (requires brackets) */
  char flexible_array[];
  
  /* Array of pointers */
  void* ptr_array[5];
};

/* Test case 3: Struct with braces (nested types) */
struct GTY(()) StructWithBraces {
  /* Anonymous nested union with braces */
  union {
    int as_int;
    float as_float;
    struct {
      char byte1;
      char byte2;
    } GTY(()) bytes;
  } data;
  
  /* Nested struct definition */
  struct GTY(()) NestedStruct {
    int x;
    int y;
  } point;
  
  /* Another anonymous struct */
  struct {
    long id;
    char name[32];
  } info;
};

/* Test case 4: Complex type combining all bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array (parentheses and brackets) */
  int (*(*get_matrix)(int size))[10];
  
  /* Nested struct with array of function pointers */
  struct GTY(()) Handler {
    /* Array of function pointers */
    void (*handlers[5])(struct ComplexType*);
    
    /* Pointer to array with parenthesized size */
    int (*dynamic_array)[(sizeof(int) * 2)];
  } handler;
  
  /* Union containing struct with bitfield */
  union {
    struct {
      unsigned int flag1: 1;
      unsigned int flag2: (2 + 1);
      unsigned int flags[4];
    } GTY(()) bits;
    
    /* Array in union */
    long values[8];
  } storage;
  
  /* Flexible array of structs with function pointers */
  struct GTY(()) Element {
    int id;
    char* (*get_name)(void);
    struct Element* GTY((skip)) next;
  } elements[];
};

/* Test case 5: Type with deeply nested brackets */
typedef struct GTY(()) TreeNode {
  int value;
  /* Array of pointers to functions returning pointers to nodes */
  struct TreeNode* (*(*visitors[3])(void))[2];
  
  /* Parenthesized expression in array size */
  char padding[(sizeof(void*) * 2) - 1];
  
  /* Nested anonymous struct with bitfield */
  struct {
    unsigned int depth: 4;
    unsigned int : (sizeof(unsigned int) * 8 - 4); /* unnamed bitfield */
  } meta;
} TreeNode;

/* Test case 6: Template-like structure (simulating C++ templates in C) */
#define DECLARE_VECTOR(T) \
struct GTY(()) Vector_##T { \
  T* GTY((length("capacity"))) data; \
  size_t size; \
  size_t capacity; \
  int (*compare)(const T*, const T*); \
}

/* Instantiate with different types to create multiple GTY types */
DECLARE_VECTOR(int);
DECLARE_VECTOR(char);
DECLARE_VECTOR(double);

/* Test case 7: Linked list with all bracket types */
struct GTY(()) ListNode {
  /* Data containing arrays and function pointers */
  union {
    int as_array[5];
    struct {
      char* (*allocator)(size_t);
      void (*deallocator)(char*);
    } GTY(()) mem_ops;
  } data;
  
  /* Pointer to next node (circular reference) */
  struct ListNode* GTY((skip)) next;
  
  /* Array of callback functions */
  void (*callbacks[3])(struct ListNode*);
  
  /* Bitfield with parenthesized computation */
  unsigned int magic: (0x1F & (sizeof(void*) - 1));
};

#endif /* TEST_GTY_COVERAGE_H */
