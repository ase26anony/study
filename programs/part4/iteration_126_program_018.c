#ifndef TEST_GTY_COVERAGE_H
#define TEST_GTY_COVERAGE_H

#include <stddef.h>

/* Test case 1: Struct with parentheses - triggers case '(' */
struct GTY(()) StructWithParens {
  /* Function pointer with parentheses */
  int (*callback)(int, char*);
  
  /* Bitfield with parenthesized expression */
  unsigned int flags: (sizeof(int) * 8 - 1);
  
  /* Nested function pointer type */
  void (*complex_callback)(int (*)(double), char);
};

/* Test case 2: Union with brackets - triggers case '[' */
union GTY(()) UnionWithBrackets {
  /* Fixed-size array */
  int fixed_array[10];
  
  /* Multi-dimensional array */
  double matrix[3][4];
  
  /* Zero-length array (GCC extension) */
  char flexible_array[0];
  
  /* Array with computed size */
  unsigned char data[sizeof(long double)];
};

/* Test case 3: Struct with braces - triggers case '{' */
struct GTY(()) StructWithBraces {
  int id;
  
  /* Anonymous nested union with braces */
  union {
    struct {
      int x;
      int y;
    } point;
    float values[2];
  } GTY((tag("0"))) coord;
  
  /* Nested struct definition */
  struct {
    char* name;
    int length;
  } info;
};

/* Test case 4: Complex type combining all three bracket types */
struct GTY(()) ComplexType {
  /* Function pointer returning pointer to array - multiple parentheses and brackets */
  int (*(*get_array_func)(void))[10];
  
  /* Array of function pointers */
  void (*handlers[5])(int);
  
  /* Struct with bitfield and nested union */
  struct {
    unsigned int type: 4;
    union {
      int int_val;
      float float_val;
    } value;
  } variant;
  
  /* Multi-dimensional array with parenthesized size */
  char buffer[(sizeof(void*) == 8 ? 64 : 32)][256];
};

/* Test case 5: Pointer chain with all bracket types */
typedef struct GTY(()) Node {
  /* Self-referential pointer */
  struct Node* GTY((skip)) next;
  
  /* Function pointer with complex signature */
  int (*compare)(const struct Node*, const struct Node*);
  
  /* Flexible array member at end */
  int data[];
} Node;

/* Test case 6: Union with nested anonymous struct */
union GTY(()) Container {
  /* Anonymous struct */
  struct {
    Node* GTY((tag("1"))) first;
    Node* GTY((tag("1"))) last;
    size_t count;
  } list;
  
  /* Array of pointers */
  void* items[16];
};

/* Forward declaration for mutual recursion */
struct GTY(()) ForwardDecl;

/* Test case 7: Struct with mutual recursion and complex types */
struct GTY(()) Graph {
  struct ForwardDecl* GTY((tag("2"))) forward;
  
  /* Array of function pointers returning pointers */
  struct Graph* (*(*traversals[3])(struct Graph*))(void);
  
  /* Nested union with anonymous struct */
  union {
    struct {
      int visited: 1;
      int processed: 1;
    } flags;
    unsigned char state;
  } status;
};

/* Complete the forward declaration */
struct GTY(()) ForwardDecl {
  struct Graph* GTY((tag("2"))) back_ref;
  char* GTY((length("strlen($)"))) name;
};

#endif /* TEST_GTY_COVERAGE_H */
