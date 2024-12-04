print('loop based on pointer arithmetic')

a = 10
b = 20
c = 30
d = 40
e = 50
f = 60
g = 70
h = 80
i = 90
j = 100

p = &j

while 10 != *p:
{
  print(p)
  x = *p
  print(x)
  
  p = p - 1
}

print(p)
x = *p
print(x)

print('done')
