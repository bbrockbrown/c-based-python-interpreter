print('invalid address test')
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

px = px - 1
fred = *px        ## semantic error: invalid address

print()
print('done')
