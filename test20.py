print('multi-level pointer derefence test')
print()

count = 412
p1 = &count
p2 = &p1
p3 = &p2

x = 10
y = 20
c = 30

a = *p3
print(a)

b = *p2
print(b)

c = *p1
print(c)

p4 = &p3

pass
*p4 = &p1

pass
*p3 = 99

print(p1)
print(p2)
print(p3)
print(p4)

print()
print('done')
