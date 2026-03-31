p = &arr[0]
for i = 0 to 99:
    load val from memory at address p
    p = p + 4
    sum = sum + val
