[comment]: # (Job parallelizer in PVM for SPMD executions in computing cluster)
[comment]: # (URL: https://github.com/oscarsaleta/PVMantz)
[comment]: # ( )
[comment]: # (Copyright (C) 2016  Oscar Saleta Reig) 
[comment]: # ( )
[comment]: # (PBala is free software: you can redistribute it and/or modify)
[comment]: # (it under the terms of the GNU General Public License as published by)
[comment]: # (the Free Software Foundation, either version 3 of the License, or)
[comment]: # ((at your option) any later version.)
[comment]: # (PBala is distributed in the hope that it will be useful,)
[comment]: # (but WITHOUT ANY WARRANTY; without even the implied warranty of)
[comment]: # (MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the)
[comment]: # (GNU General Public License for more details.)
[comment]: # ( )
[comment]: # (You should have received a copy of the GNU General Public License)
[comment]: # (along with PBala.  If not, see <http://www.gnu.org/licenses/>.)

# Examples
In this folder you will find a simple example for each kind of execution that PBala presently supports.

## Data file
The data file that PBala receives for its execution is a CSV (comma-separated-values) file of the following form:
```
0,x01,x02,x03,...,x0n
1,x11,x12,x13,...,x1n
...
m,xm1,xm2,xm3,...,xmn
```
where **each row is a single execution** and _xij_ are **numbers** (and not symbolic expressions).

The first column is the _id_ of the execution (in the example, _ids_ go from 0 to _m_, but you can use the numbers that you prefer. The _ids_ are used for labeling the output files, so you can know which output corresponds to which data).

The following columns are arguments to your program, and will be accessed from within your program in different ways depending on which kind of execution you are performing.

## Maple
For a Maple execution, the job is really simple. PBala automatically defines two variables upon execution: `taskId` and `taskArgs`. `taskId` is the first integer of the datafile, which identifies the execution, whereas `taskArgs` is a vector that contains the rest of arguments: `taskArgs:=[x1,x2,...,xn]`.

In order to adapt a Maple script to be used by PBala, you just need to perform your desired computations using the `taskArgs` variable.

In the <a href="maple_example.mpl">example file</a> we print the arguments received by Maple. In a similar fashion, any more complex set of operations could be applied to these variables.

## C
For a C execution, we need a C program that can read variables from the command line, because each execution will be considered as `./program taskId x1 x2 ... xn`. Therefore, our C program will receive the variables as if they were passed to it from command line, and we will need to use them as we please.

Notice that, in order to be able to read arguments from command line, our `main` function must follow the standard prototype:

```C
int main(int argc, char *argv[])
```

In the <a href="c_example.c">C example file</a> we show how to read some arguments from the command line while checking for some possible errors, and then print them to stdout. The same procedure can be generalised for any number and type of arguments and to perform the desired computations afterwards.

## Python
The Python executions work similarly to C executions, because arguments are passed to the script from the command line execution, so our program needs to be able to read and use them.

In order to read arguments from command line, the simplest option is to import the `sys` library and then access them like this:

```Python
import sys

print("taskId is ", sys.argv[1])
print("taskArgs are ", sys.argv[2:])
```

The <a href="python_example.py">Python example</a> is a bit more elaborated, because it shows how to define a _main_ function in Python, so the behaviour of the program is even more similar to C.

## 