print('another loop with pointer-based condition')

a = "aaaaaa 12345"
b = " 12345"

p = &a
q = &b

while *p != *q:
{
  *q = "a" + *q
  print(b)
}

print('done')
