# =====================
# Opcions de compilacio
# =====================
COMPILER=gcc
OPT=-g -Wall
LIBS=-lpvm3 -lm

all : PBala task

# ======
# Master
# ======
PBala : PBala.c antz_lib.o
	$(COMPILER) -o PBala $(OPT) PBala.c antz_lib.o $(LIBS)

# =======
# Esclaus
# =======
task: task.c antz_lib.o
	$(COMPILER) -o task $(OPT) task.c antz_lib.o $(LIBS)

# ============
# Biblioteques
# ============
antz_lib.o : antz_lib.c
	$(COMPILER) -c $(OPT) antz_lib.c $(TAIL)

# ======
# Neteja
# ======
clean :
	rm -f PBala
	rm -f task
	rm -f antz_lib.o
