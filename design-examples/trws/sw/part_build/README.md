## Software only mockup
Currently does not compute the correct result, only passes through.

## Input is:

````
MEM gi --- gamma tbl initialize ( n ushort)

MEM wi --- initial weights ( n weights of size n )

MEM mi --- stream of ( n(n-1)/2 messages of size n)

STORAGE res --- stream of ( n(n-1)/2 messages of size n)

Output is:

STORAGE off --- stream of ( n(n-1)/2 messages of size n)

MEM mo --- stream of ( n(n-1)/2 messages of size n)
````
