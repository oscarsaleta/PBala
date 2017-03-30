<!--
    This file is part of PBala (http://github.com/oscarsaleta/PBala)

    Copyright (C) 2016  O. Saleta

    PBala is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
-->

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
where **each row is a single execution**. This example would perform _m_ executions of the same program, and each execution would take one row as its arguments.

**Notice:** the arguments _x<sub>i,j</sub>_ can be anything that your program understands. For example, in C they will most likely be numbers, but for Maple they can be symbolic expressions. However, the first element of each row **must be a number**.

The first column is the _id_ of the execution (in the example, _ids_ go from 0 to _m_, but you can use the numbers that you prefer. The _ids_ are used for labeling the output files, so you can know which output corresponds to which data).

The following columns are arguments to your program, and will be accessed from within your program in different ways depending on which kind of execution you are performing.

## Example programs to use with PBala
### Maple
For a Maple execution, the job is really simple. PBala automatically defines two variables upon execution: `taskId` and `taskArgs`. `taskId` is the first integer of the datafile, which identifies the execution, whereas `taskArgs` is a vector that contains the rest of arguments: `taskArgs:=[x1,x2,...,xn]`.

In order to adapt a Maple script to be used by PBala, you just need to perform your desired computations using the `taskArgs` variable.

In the [example file](maple_example.mpl "Maple example") we print the arguments received by Maple. In a similar fashion, any more complex set of operations could be applied to these variables.

### C
For a C execution, we need a C program that can read variables from the command line, because each execution will be considered as `./program taskId x1 x2 ... xn`. Therefore, our C program will receive the variables as if they were passed to it from command line, and we will need to use them as we please.

Notice that, in order to be able to read arguments from command line, our `main` function must follow the standard prototype:

```C
int main(int argc, char *argv[])
```

In the [C example file](c_example.c "C example") we show how to read some arguments from the command line while checking for some possible errors, and then print them to stdout. The same procedure can be generalised for any number and type of arguments and to perform the desired computations afterwards.

### Python
The Python executions work similarly to C executions, because arguments are passed to the script from the command line execution, so our program needs to be able to read and use them.

In order to read arguments from command line, the simplest option is to import the `sys` library and then access them like this:

```Python
import sys

print("taskId is ", sys.argv[1])
print("taskArgs are ", sys.argv[2:])
```

The [Python example](python_example.py "Python example") is a bit more elaborated, because it also shows how to define a _main_ function in Python, so the behaviour of the program is even more similar to C.

### PARI/GP
Given that PARI is intended to work as a Computer Algebra System (CAS), PARI/GP executions are implemented to work the same as Maple executions. This is achieved by creating an intermediate PARI scripts that defines a number `taskId` and a vector `taskArgs` and then executes the desired program. Thus, we just need to use these variables as if they were already defined.

In the [PARI/GP example](pari_example.gp "PARI example"), we simply print `taskId` and `taskArgs` to showcase that the program can use these variables as if they were defined by ourselves.

### Sage
Sage is a CAS (same as Maple and PARI), so we have implemented it to work exactly as in Maple and PARI. An auxiliary program created during execution of PBala will define the variables `taskId` and `taskArgs` from the arguments of the data file, and we can use these two variables as we please in our Sage script.

See a piece of code that shows the simplest example in [the Sage script](sage_example.sage "Sage example").

## Example of a full execution in an _antz_ node
Imagine our datafile is called `datafile.txt` and contains the following lines:
```
71,0,73,74,0
72,0,74,75,0
73,0,75,76,0
74,0,76,77,0
75,0,77,78,0
76,0,78,79,0
77,0,79,80,0
78,0,80,81,0
79,0,81,82,0
80,0,82,83,0
```

Suppose we want to run a program 10 times, each time with one row of data, and we want to use two cores of node _a01_, one of node _a03_, three of node _a05_ and one of node _a07_. Then we need the following file, which we can name `nodefile.txt`:
```
a01 2
a03 1
a05 3
a07 1
```
This will allow us to make 7 simultaneous executions of our program, so the three first slaves that finish will perform the remaining three executions.

Now we just need a directory for storing the results (so we don't clutter our main directory with files), which we will conveniently name `results`.

Imagine we want to execute `maple_exampe.mpl` using the previous datafile and nodefile. We need to have the `PBala` and `PBala_task` executables in the current directory or somewhere in the _PATH_ of our system, and we can execute the following command:
```
./PBala -eh 0 maple_example.mpl datafile.txt nodefile.txt results
```
The options `-eh` tell PBala that we want to generate error files (in case something goes wrong) and a slave file (that tells us which node has performed which execution).

If we want to keep working and leave PBala in the background, we can do this to have the output of PBala go to a text file instead of to the terminal:
```
./PBala -eh 0 maple_example.mpl datafile.txt nodefile.txt results 1> output.txt 2> error.txt &
```
This will make the standard output go to `output.txt` and the errors go to `error.txt`, and will leave the terminal ready to keep working while PBala runs in the background (`&`).

 C, Python, PARI and Sage executions work exactly the same, only changing the corresponding arguments on the command line call.