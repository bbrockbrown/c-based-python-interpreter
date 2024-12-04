print('address of test')
print()

x = 987
y = 654.321
z = True

p1 = &x
p2 = &y
p3 = &z

s = "this is a string"
p4 = &s
p5 = &p44     ## semantic error

print(x)
print(y)
print(z)
print(s)
print(p1)
print(p2)
print(p3)
print(p4)
print(p5)

print()
print('done')
