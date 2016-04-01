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
task_fork: task_fork.c antz_lib.o
	$(COMPILER) -o task $(OPT) task_fork.c antz_lib.o $(LIBS)
	cp task ~/pvm3/bin/LINUX64

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
	rm -f task ~/pvm3/bin/LINUX64/task
