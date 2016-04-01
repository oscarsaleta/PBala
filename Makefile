# =====================
# Opcions de compilacio
# =====================
COMPILER=gcc
OPT=-g -Wall
LIBS=-lpvm3 -lm

all : pvm_test task_fork

# ======
# Master
# ======
pvm_test : pvm_test.c antz_lib.o
	$(COMPILER) -o pvm_test $(OPT) pvm_test.c antz_lib.o $(LIBS)

# =======
# Esclaus
# =======
task: task.c antz_lib.o
	$(COMPILER) -o task $(OPT) task.c antz_lib.o $(LIBS)
	cp task ~/pvm3/bin/LINUX64
task_fork: task_fork.c antz_lib.o
	$(COMPILER) -o task $(OPT) task_fork.c antz_lib.o $(LIBS)
	cp task ~/pvm3/bin/LINUX64
#taskMaple : taskMaple.c
#	$(COMPILER) -o taskMaple $(OPT) taskMaple.c $(LIBS)
#	cp taskMaple ~/pvm3/bin/LINUX64
#taskC : taskC.c
#	$(COMPILER) -o taskC $(OPT) taskC.c $(LIBS)
#	cp taskC ~/pvm3/bin/LINUX64
#taskPython : taskPython.c
#	$(COMPILER) -o taskPython $(OPT) taskPython.c $(LIBS)
#	cp taskPython ~/pvm3/bin/LINUX64
#task_pari : task_pari.c
#	$(COMPILER) -o task_pari $(OPT) task_pari.c $(LIBS)
#	cp task_pari ~/pvm3/bin/LINUX64

# ============
# Biblioteques
# ============
antz_lib.o : antz_lib.c
	$(COMPILER) -c $(OPT) antz_lib.c $(TAIL)

# ======
# Neteja
# ======
clean :
	rm -f pvm_test
	rm -f antz_lib.o
#	rm -f taskMaple ~/pvm3/bin/LINUX64/taskMaple
#	rm -f taskC ~/pvm3/bin/LINUX64/taskC
#	rm -f taskPython ~/pvm3/bin/LINUX64/taskPython
	rm -f task ~/pvm3/bin/LINUX64/task
