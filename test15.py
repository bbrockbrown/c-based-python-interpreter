print('short test of all features')

x = 123 
p = &x 
print(p) 

y = 456 
p2 = &y 
print(p2) 

z = 0 
rp = &z 
print(rp) 

*rp = *p + *p2 
print(z) 

print('done')
