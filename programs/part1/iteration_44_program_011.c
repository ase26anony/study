+----------------+
| field1 (int)   |
+----------------+
| u (union)      |  // Either ptr OR arr[10]
|                |  // Size = max(sizeof(char*), sizeof(int[10]))
+----------------+
| callback       |  // Function pointer
+----------------+
