IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
BDIR=../bin
LDIR =../lib

#  ../include/*
_LIB_RDSNSR = libReadSensor.a
LIB_RDSNSR = $(patsubst %,$(LDIR)/%,$(_LIB_RDSNSR))

# obj/*
_LIBOBJ = ReadSensor.o
LIBOBJ = $(patsubst %,$(ODIR)/%,$(_LIBOBJ))

_APPOBJ_MAIN = smartFarmLandWaterSupplySystem.o
APPOBJ_MAIN = $(patsubst %,$(ODIR)/%,$(_APPOBJ_MAIN))
APP_MAIN_DEPENDENCY=-lReadSensor -pthread

_APPOBJ_CLNT = Client.o
APPOBJ_CLNT = $(patsubst %,$(ODIR)/%,$(_APPOBJ_CLNT))

_APPOBJ_SNSR = WaterLevelSensor.o
APPOBJ_SNSR = $(patsubst %,$(ODIR)/%,$(_APPOBJ_SNSR))

# bin/*
_BINF_MAIN = smartFarmLandWaterSupplySystem
BINF_MAIN =$(patsubst %,$(BDIR)/%,$(_BINF_MAIN))

_BINF_CLNT = Client
BINF_CLNT =$(patsubst %,$(BDIR)/%,$(_BINF_CLNT))

_BINF_SNSR = WaterLevelSensor
BINF_SNSR =$(patsubst %,$(BDIR)/%,$(_BINF_SNSR))

.PHONY: start
start:$(LIB_RDSNSR) $(BINF_SNSR) $(BINF_MAIN) $(BINF_CLNT)

$(BINF_SNSR): $(APPOBJ_SNSR)
	$(CC) -o $@ $^

$(BINF_MAIN): $(APPOBJ_MAIN)
	$(CC) -o $@ $< $(CFLAGS) -L $(LDIR) $(APP_MAIN_DEPENDENCY)

$(BINF_CLNT): $(APPOBJ_CLNT)
	$(CC) -o $@ $^

$(APPOBJ_SNSR):WaterLevelSensor.c
	$(CC) -c -o $@ $^

$(APPOBJ_MAIN):smartFarmLandWaterSupplySystem.c
	$(CC) -c -o $@ $^

$(APPOBJ_CLNT):Client.c
	$(CC) -c -o $@ $^

$(LIB_RDSNSR):$(LIBOBJ)
	ar rs -o $@ $^

$(LIBOBJ): ReadSensor.c
	$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: clean

clean: 
	rm -f $(ODIR)/*.o
	rm -f $(BDIR)/*
	rm -f $(LDIR)/*.a