print('multi-level pointer derefence test')
print()

count = 412
p1 = &count
p2 = &p1
p3 = &p2

a = *p3
print(a)

b = *p2
print(b)

c = *p1
print(c)

print()
print('done')
