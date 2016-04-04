taskId;
X;
Y:=sum(X[i],i=1..nops(X));
Y;

with(LinearAlgebra):
a:=Vector(1..10000000,0);
for i from 1 to 5000000 do
    a[i]:=i:
od:
a;


