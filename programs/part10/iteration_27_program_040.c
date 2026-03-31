acc = init[j]
for i = 0..4999:
    acc = acc * mul[j] + input[i]
result[j] = acc
