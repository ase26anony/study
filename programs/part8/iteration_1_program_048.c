/* gengtype-coverage-test.c - ISO C99 compliant test program for gengtype parser coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* Simplified parser stub for testing the specific switch cases */
typedef struct {
    const char *input;
    size_t pos;
    int line;
    int depth[3]; /* Track depth for each delimiter type */
} parser_state;

static void advance(parser_state *ps) {
    if (ps->input[ps->pos]) ps->pos++;
}

static void consume_balanced(parser_state *ps, char open, char close) {
    int *depth_ptr = NULL;
    if (open == '(') depth_ptr = &ps->depth[0];
    else if (open == '[') depth_ptr = &ps->depth[1];
    else if (open == '{') depth_ptr = &ps->depth[2];
    
    if (depth_ptr) (*depth_ptr)++;
    
    /* Simulate consumption of balanced content */
    while (ps->input[ps->pos]) {
        char c = ps->input[ps->pos];
        if (c == open) {
            consume_balanced(ps, open, close);
        } else if (c == close) {
            if (depth_ptr) (*depth_ptr)--;
            advance(ps);
            return;
        } else if (c == '\n') {
            ps->line++;
            advance(ps);
        } else {
            advance(ps);
        }
    }
}

static void parse_test_content(const char *content) {
    parser_state ps = {content, 0, 1, {0, 0, 0}};
    
    while (ps.input[ps.pos]) {
        char c = ps.input[ps.pos];
        
        /* This simulates the uncovered switch logic */
        switch (c) {
            default:
                advance(&ps);
                break;
            case '(':
                consume_balanced(&ps, '(', ')');
                break;
            case '[':
                consume_balanced(&ps, '[', ']');
                break;
            case '{':
                consume_balanced(&ps, '{', '}');
                break;
        }
    }
    
    printf("Parser completed. Depth states: (%d, %d, %d)\n", 
           ps.depth[0], ps.depth[1], ps.depth[2]);
}

/* Complex .gt file content targeting all requirements */
static const char *complex_gt_content = 
    "/* Test file for gengtype parser coverage */\n"
    "/* Requirement 1: Balanced construct nesting */\n"
    "\n"
    "%typedef struct ComplexType {\n"
    "  int (*compare)(const void *, const void *);  /* Function pointer with parens */\n"
    "  void (*handlers[5])(int, char *);           /* Array of function pointers */\n"
    "  union {\n"
    "    struct { int x; double y; } nested;\n"
    "    int arr[3][2];\n"
    "  } data;\n"
    "  struct Node *next;\n"
    "} ComplexType;\n"
    "\n"
    "/* Nested parentheses in macros */\n"
    "#define ARRAY_TYPE(T) struct { T data[10]; }\n"
    "#define FUNC_PTR(RET, ARGS) RET (*)(ARGS)\n"
    "\n"
    "%union TaggedUnion {\n"
    "  /* Multiple levels of nesting */\n"
    "  struct {\n"
    "    int id;\n"
    "    char name[(MAX_LEN + 1)];  /* Brackets in macro expansion */\n"
    "  } info;\n"
    "  double matrix[2][2];\n"
    "  void (*callback)(int (*)(char), double);\n"
    "};\n"
    "\n"
    "/* GC roots with complex types */\n"
    "GC roots {\n"
    "  struct Tree {\n"
    "    struct Tree *left;\n"
    "    struct Tree *right;\n"
    "    int value;\n"
    "    int (*visitor)(struct Tree *);\n"
    "  } *root;\n"
    "  ComplexType *ct_array[20];\n"
    "  %union TaggedUnion *tu_ptr;\n"
    "}\n"
    "\n"
    "/* Enum with array dimensions in comments */\n"
    "%enum ErrorCode {\n"
    "  ERR_NONE = 0,\n"
    "  ERR_PARSE,    /* parsing error (unbalanced?) */\n"
    "  ERR_MEM,      /* memory [allocation] failed */\n"
    "  ERR_INVAL     /* invalid {argument} */\n"
    "};\n"
    "\n"
    "/* Requirement 2: Unbalanced edge cases (commented out for valid parse) */\n"
    "/*\n"
    "%struct Unbalanced {\n"
    "  int x;\n"
    "  char str[10;  /* Missing closing bracket */\n"
    "  void (*func(int);  /* Missing closing paren */\n"
    "};  /* The struct will be skipped by parser */\n"
    "*/\n"
    "\n"
    "/* Requirement 3: Comments and macros interleaving */\n"
    "// Line comment with brackets: int arr[] = {1, 2, 3};\n"
    "/* Block comment with parens: (x + y) * z */\n"
    "/* Nested /* comment */ with braces { a: 1 } */\n"
    "\n"
    "#define NESTED_PARENS(a, b) ((a) > (b) ? (a) : (b))\n"
    "#define BRACKET_MACRO int arr[((sizeof(int) > 4) ? 8 : 4)]\n"
    "\n"
    "/* Requirement 4: GT-specific constructs with delimiters */\n"
    "%struct Container {\n"
    "  %union TaggedUnion items[10];\n"
    "  int (*sorter)(%union TaggedUnion *, %union TaggedUnion *);\n"
    "  struct {\n"
    "    int count;\n"
    "    int capacity;\n"
    "  } meta;\n"
    "};\n"
    "\n"
    "/* Requirement 5: Multiple top-level declarations */\n"
    "%typedef int (*Comparator)(const void *, const void *);\n"
    "\n"
    "%struct GraphNode {\n"
    "  int id;\n"
    "  struct GraphNode **neighbors;  /* Pointer to array of pointers */\n"
    "  int neighbor_count;\n"
    "  void (*visit)(struct GraphNode *);\n"
    "};\n"
    "\n"
    "%union Either {\n"
    "  int int_val;\n"
    "  double dbl_val;\n"
    "  char *str_val;\n"
    "  void (*func_val)(void);\n"
    "};\n"
    "\n"
    "/* Complex nested example triggering all consume_balanced calls */\n"
    "%typedef struct UltimateTest {\n"
    "  int matrix[3][(2 + 1)];                    /* Brackets with parens */\n"
    "  void (*(*signal_handlers)[5])(int, ...);   /* Complex function pointer array */\n"
    "  union {\n"
    "    struct { int x; } s;\n"
    "    int a[2];\n"
    "  } u[2];\n"
    "  struct {\n"
    "    char *name;\n"
    "    int (*methods[3])(struct UltimateTest *);\n"
    "  } vtable;\n"
    "} UltimateTest;\n"
    "\n"
    "/* Final GC roots declaration */\n"
    "GC roots {\n"
    "  UltimateTest *test_instance;\n"
    "  %struct GraphNode *graph_root;\n"
    "  %union Either choices[5];\n"
    "}\n";

/* Additional test with unbalanced delimiters for error handling */
static const char *unbalanced_gt_content =
    "%struct BadType1 {\n"
    "  int x;\n"
    "  char str[10;  /* Missing ] */\n"
    "};\n"
    "\n"
    "%struct BadType2 {\n"
    "  void (*func(int, char);  /* Missing ) */\n"
    "};\n"
    "\n"
    "%struct BadType3 {\n"
    "  struct { int a;  /* Missing } */\n"
    "};\n";

int main(void) {
    FILE *fp;
    const char *filename = "test_coverage.gt";
    
    /* Write complex .gt file */
    fp = fopen(filename, "w");
    if (!fp) {
        perror("Failed to create .gt file");
        return 1;
    }
    
    fputs(complex_gt_content, fp);
    fclose(fp);
    
    printf("Generated %s with complex type definitions\n", filename);
    
    /* Parse the content using our stub parser */
    printf("\n=== Parsing complex content ===\n");
    parse_test_content(complex_gt_content);
    
    /* Also test with unbalanced content */
    printf("\n=== Parsing unbalanced content ===\n");
    parse_test_content(unbalanced_gt_content);
    
    /* Create a second test file with different patterns */
    const char *filename2 = "test_nested.gt";
    fp = fopen(filename2, "w");
    if (fp) {
        /* Focus on deeply nested constructs */
        fputs("%typedef struct DeepNest {\n", fp);
        fputs("  int (*(*deep[2])[3])(char (*(*)(int))[5]);\n", fp);
        fputs("  struct {\n", fp);
        fputs("    union {\n", fp);
        fputs("      int x;\n", fp);
        fputs("      double y;\n", fp);
        fputs("    } u[2][2];\n", fp);
        fputs("  } data;\n", fp);
        fputs("} DeepNest;\n", fp);
        fclose(fp);
        printf("\nGenerated %s with deep nesting\n", filename2);
    }
    
    /* Instructions for using with actual gengtype */
    printf("\n=== To run with actual gengtype ===\n");
    printf("1. Build gengtype with coverage: g++ -fprofile-arcs -ftest-coverage \\\n");
    printf("   -I. -I../../include -o gengtype-cov gengtype-parse.cc gengtype.cc gengtype-state.cc\n");
    printf("2. Run: ./gengtype-cov -p %s\n", filename);
    printf("3. Generate coverage: gcov gengtype-parse.cc\n");
    
    /* Cleanup */
    remove(filename);
    remove(filename2);
    
    return 0;
}
