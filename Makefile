# PS2 SMPTE colour bars — EE Makefile (PS2SDK + gsKit)

EE_BIN  = smpte-bars.elf
EE_OBJS = src/main.o src/video_modes.o src/smpte.o

EE_INCS = -Isrc -I$(GSKIT)/include -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include

# gsKit (+ toolkit/fontm) and the IOP pad RPC stubs.
EE_LIBS = -L$(GSKIT)/lib -lgskit_toolkit -lgskit -ldmakit -lpad -lpacket -ldma -lc

EE_CFLAGS = -O2 -Wall

all: $(EE_BIN)

clean:
	rm -f $(EE_BIN) $(EE_OBJS)

# Pull in the PS2SDK EE build rules.
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal
