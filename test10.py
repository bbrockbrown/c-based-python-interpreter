print('pointer derefence test')
print()

x = 3.14159
y = x + 299

px = &x
py = &y

i = 12
j = i ** 2

pi = &i
pj = &j

s = "this is a string, "
ps = &s

a = *px + 99.25
print(a)

b = i - *py
print(b)

t = *ps + " with more to follow"
print(t)

c = *pj * *pi
print(c)

fred = *xyz        ## semantic error

d = *px < *py
e = *pxy >= *py      
f = *pj / *pi
print(d)
print(e)
print(f)

print()
print('done')
