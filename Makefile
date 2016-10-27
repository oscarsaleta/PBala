# =====================
# Opcions de compilacio
# =====================
COMPILER=gcc
CFLAGS=-O3 -Wall
CFLAGSg=-g -Wall
LFLAGS=-lpvm3 -lm

all : PBala task
debug : PBalag taskg

# ======
# Master
# ======
PBala : PBala.c antz_lib.o
	$(COMPILER) -o PBala $(CFLAGS) PBala.c antz_lib.o $(LFLAGS)

PBalaG: PBala.c antz_lib.o
	$(COMPILER) -o PBala $(CFLAGSg) PBala.c antz_lib.o $(LFLAGS)

# =======
# Esclaus
# =======
task: task.c antz_lib.o
	$(COMPILER) -o task $(CFLAGS) task.c antz_lib.o $(LFLAGS)

taskg: task.c antz_lib.o
	$(COMPILER) -o task $(CFLAGSg) task.c antz_lib.o $(LFLAGS)

# ============
# Biblioteques
# ============
antz_lib.o : antz_lib.c
	$(COMPILER) -c $(CFLAGS) antz_lib.c $(LFLAGS)

# ======
# Neteja
# ======
clean :
	rm -f PBala
	rm -f task
	rm -f antz_lib.o
