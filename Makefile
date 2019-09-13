# To cross compile:
#
# Set up as usual for bitbake:
# $ . setup-environment build
#
# In the build directory:
# $ bitbake meta-ide-support
# $ . tmp/environment-setup-cortexa9hf-vfp-neon-poky-linux-gnueabi
#
# Now a make in this directory should work.

VPATH = ./c_library/ardupilotmega

INCS = -I./c_library/ardupilotmega -I./c_library/common

CFLAGS += -Wall $(INCS)
CXXFLAGS += -Wall $(INCS) -std=c++11

LIBS = -static-libstdc++

SRCS_CPP = main.cpp
SRCS_CPP += Smoothing.cpp
SRCS_CPP += System.cpp
SRCS_CPP += Node.cpp
SRCS_CPP += Router.cpp
SRCS_CPP += SLIP.cpp
SRCS_CPP += Target.cpp
SRCS_CPP += TargetLinkPacket.cpp
SRCS_CPP += TargetMavlink.cpp
SRCS_CPP += ./INIReader/cpp/INIReader.cpp
SRCS_CPP += TargetSlip.cpp
#SRCS_CPP += TargetVideo.cpp

SRCS_C = arp_table.c
SRCS_C += util.c
SRCS_C += ./INIReader/ini.c

OBJS = $(SRCS_CPP:.cpp=.o) $(SRCS_C:.c=.o)

MAIN = controller

all: $(MAIN)

$(MAIN): $(OBJS)
	$(LINK.cpp) -o $(MAIN) $(OBJS) $(LIBS)

clean:
	$(RM) *.o *~ $(MAIN)

BASE := ../..

fmt:
	@python $(BASE)/tools/build/clang-format-run.py --apply

fmt-diff:
	@python $(BASE)/tools/build/clang-format-run.py

.PHONY: all clean fmt fmt-diff
