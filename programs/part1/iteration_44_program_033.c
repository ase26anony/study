struct my_struct {
  int field1;                    // Regular integer field
  union {                        // Anonymous union
    char *ptr;                   // Option 1: character pointer
    int arr[10];                 // Option 2: integer array of size 10
  } u;                           // Union variable named 'u'
  void (*callback)(int, char);   // Function pointer
};
