+----------------+
| field1 (4 bytes)|
+----------------+
| u (40 bytes)   |  // union: either ptr OR arr[10]
+----------------+
| callback (4/8 bytes) |  // pointer size depends on architecture
+----------------+
