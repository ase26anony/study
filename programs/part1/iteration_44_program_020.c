+------------+
| field1     | (4 bytes typically)
+------------+
| union u    | (40 bytes typically - size of int[10])
|   ptr      |
|   OR       |
|   arr[10]  |
+------------+
| callback   | (4 or 8 bytes depending on architecture)
+------------+
