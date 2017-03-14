[//]: # ( Job parallelizer in PVM for SPMD executions in computing cluster )
[//]: # ( URL: https://github.com/oscarsaleta/PVMantz )
[//]: # ( )
[//]: # ( Copyright (C) 2016  Oscar Saleta Reig ) 
[//]: # ( )
[//]: # ( PBala is free software: you can redistribute it and/or modify )
[//]: # ( it under the terms of the GNU General Public License as published by )
[//]: # ( the Free Software Foundation, either version 3 of the License, or )
[//]: # ( (at your option) any later version. )
[//]: # ( PBala is distributed in the hope that it will be useful, )
[//]: # ( but WITHOUT ANY WARRANTY; without even the implied warranty of )
[//]: # ( MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the )
[//]: # ( GNU General Public License for more details. )
[//]: # ( )
[//]: # ( You should have received a copy of the GNU General Public License )
[//]: # ( along with PBala.  If not, see <http://www.gnu.org/licenses/>. )

In this folder you will find a simple example for each kind of execution that PBala presently supports.

# Data file
The data file that PBala receives for its execution is a CSV (comma-separated-values) file of the following form:
```
0,x01,x02,x03,...,x0n
1,x11,x12,x13,...,x1n
...
m,xm1,xm2,xm3,...,xmn
```
where **each row is a single execution**.

The first column is the _id_ of the execution (in the example, _ids_ go from 0 to _m_, but you can use the numbers that you prefer. The _ids_ are used for labeling the output files, so you can know which output corresponds to which data).

The following columns are arguments to your program, and will be accessed from within your program in different ways depending on which kind of execution you are performing.

# Maple
For a Maple execution, the job is really simple. PBala automatically defines two variables upon execution: `taskId` and `taskArgs`. `taskId` is the first integer of the datafile, which identifies the execution, whereas `taskArgs` is a vector that contains the rest of arguments: `taskArgs:=[x1,x2,...,xn]`.

In order to adapt a Maple script to be used by PBala, you just need to perform your desired computations using the `taskArgs` variable.

In the <a href="maple_example.mpl">example file</a> we print the arguments received by Maple. In a similar fashion, any more complex set of operations could be applied to these variables.

# C
For a C execution, we need a C program that can read variables from the command line, because each execution will be considered as `./program taskId x1 x2 ... xn`. Therefore, our C program will receive the variables as if they were passed to it from command line, and we will need to use them as we please.

Notice that, in order to be able to read arguments from command line, our `main` function must follow the standard prototype:

```C
int main(int argc, char *argv[])
```