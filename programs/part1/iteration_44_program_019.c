+------------+
| field1     |  (4 bytes typically)
+------------+
| u.ptr      |  OR | u.arr[0] |  (8 bytes on 64-bit, 4 on 32-bit)
|            |     | u.arr[1] |
|            |     |    ...   |
|            |     | u.arr[9] |
+------------+
| callback   |  (function pointer, size depends on architecture)
+------------+
