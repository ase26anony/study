struct my_struct {
  int field1;           // 4 bytes (plus 4 bytes padding for alignment)
  union u {             // 40 bytes (size of arr[10])
    char *ptr;          // 8 bytes (but union takes max size)
    int arr[10];        // 40 bytes
  } u;
  void (*callback)(...); // 8 bytes
};                      // Total: ~56 bytes (with padding)
