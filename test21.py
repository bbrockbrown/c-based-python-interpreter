print('loop with pointer-based condition')

x = 25
p = &x

while *p:
{
  *p = *p - 5
  print(x)
}

print('done')
