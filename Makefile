# =====================
# Opcions de compilacio
# =====================
COMPILER=gcc
OPT=-O3 -Wall
OPTg=-g -Wall
LIBS=-lpvm3 -lm

all : PBala task
debug : PBalag taskg

# ======
# Master
# ======
PBala : PBala.c antz_lib.o
	$(COMPILER) -o PBala $(OPT) PBala.c antz_lib.o $(LIBS)

PBalaG: PBala.c antz_lib.o
	$(COMPILER) -o PBala $(OPTg) PBala.c antz_lib.o $(LIBS)

# =======
# Esclaus
# =======
task: task.c antz_lib.o
	$(COMPILER) -o task $(OPT) task.c antz_lib.o $(LIBS)

taskg: task.c antz_lib.o
	$(COMPILER) -o task $(OPTg) task.c antz_lib.o $(LIBS)

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
