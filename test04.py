print('pointer arithmetic test')
print()

x = 3.14159
y = x + 2

i = 1000
j = i + 1

p = &i
pj = p + 1

print(p)
print(pj)

q = &y
qx = q - 1

print(q)
print(qx)

ptr = &j
ptr_neg = ptr - 123
ptr_pos = ptr + 456

print(ptr)
print(ptr_neg)
print(ptr_pos)

print()
print('done')
