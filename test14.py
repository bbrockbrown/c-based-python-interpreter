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

px = px + 1
fred = *px
print(fred)

ps = &ps
print(ps)

ps = ps + 10
fred = *ps       ## semantic error: invalid address
print(fred)

print()
print('done')
