#!/usr/bin/python
import sys

def main(argv):
	#with open(argv[1],'w') as fout:
	#	fout.write(str(argv)+"\n")
	print(str(argv)+"\n")
        i=0;
        for j in range(0,100000000):
            i+=j;
        print(str(i)+"\n")

if __name__ == "__main__":
	main(sys.argv[1:])
