ifneq ("$(wildcard /usr/include/linux/input.h)","")
LINUX_INPUT=1
else
LINUX_INPUT=0
endif

PYTHON_CFLAGS=$(shell python3-config --cflags) -DHAVE_LINUX_INPUT_H=$(LINUX_INPUT)
PYTHON_LDFLAGS=$(shell python3-config --ldflags)

all:	nip pyccar

runcar:	pyccar
	PYTHONPATH=`pwd`/examples/pyccar ./pyccar --fb-touch

runX:	pyccar
	PYTHONPATH=`pwd`/examples/pyccar ./pyccar --timeout

runfb0:	pyccar
	PYTHONPATH=`pwd`/examples/pyccar ./pyccar --fb-touch --timeout

runfb1:	pyccar
	PYTHONPATH=`pwd`/examples/pyccar ./pyccar --fb-touch --fb-device=/dev/fb1 --timeout

NETIP_SOURCES=\
	ip_buffer.cpp \
	ip_channel.cpp \
	ip_connection.cpp \
	ip_manager.cpp \
	ip_serial.cpp \
	ip_timer.cpp \
	ip_types.cpp \
	netip/unix/ip_arch.cc

NIP_SOURCES=\
	examples/nip/nip.cc

PYCCAR_SOURCES=\
	examples/pyccar/pyccar.cc \
	examples/pyccar/TouchInput.cc \
	examples/pyccar/Window.cc \
	examples/pyccar/PyCCarUI.cc

ALL_SOURCES=$(NETIP_SOURCES) $(NIP_SOURCES) $(PYCCAR_SOURCES)

NETIP_OBJECTS=\
	ip_buffer.o \
	ip_channel.o \
	ip_connection.o \
	ip_manager.o \
	ip_serial.o \
	ip_timer.o \
	ip_types.o \
	netip/unix/ip_arch.o

NIP_OBJECTS=\
	examples/nip/nip.o

PYCCAR_OBJECTS=\
	examples/pyccar/pyccar.o \
	examples/pyccar/TouchInput.o \
	examples/pyccar/Window.o \
	examples/pyccar/PyCCarUI.o

ALL_OBJECTS=$(NETIP_OBJECTS) $(NIP_OBJECTS) $(PYCCAR_OBJECTS)

NETIP_HEADERS=\
	netip/ip_address.hh \
	netip/ip_buffer.hh \
	netip/ip_channel.hh \
	netip/ip_config.hh \
	netip/ip_connection.hh \
	netip/ip_defines.hh \
	netip/ip_manager.hh \
	netip/ip_protocol.hh \
	netip/ip_serial.hh \
	netip/ip_timer.hh \
	netip/ip_types.hh \
	netip/unix/ip_arch.hh \
	netip/unix/ip_arch_serial.hh \
	netip/unix/ip_arch_serial.cc

NIP_HEADERS=\
	examples/nip/tests.hh

PYCCAR_HEADERS=\
	examples/pyccar/BBox.hh \
	examples/pyccar/TouchInput.hh \
	examples/pyccar/Window.hh \
	examples/pyccar/pyccar.hh

ALL_HEADERS=$(NETIP_HEADERS) $(NIP_HEADERS) $(PYCCAR_HEADERS)

OTHER_FILES=\
	examples/pyccar/pyccarui.py

clean:	
	rm -f $(ALL_OBJECTS) *~ */*~ */*/*~

pyccar:		$(NETIP_OBJECTS) $(PYCCAR_OBJECTS)
		c++ -o pyccar $(NETIP_OBJECTS) $(PYCCAR_OBJECTS) $(PYTHON_LDFLAGS)

nip:	$(NETIP_OBJECTS) $(NIP_OBJECTS)
	c++ -o nip $(NETIP_OBJECTS) $(NIP_OBJECTS)

%.o:	%.cpp $(NETIP_HEADERS)
	c++ -c $< -o $@ -DIP_ARCH_UNIX -I.

%.o:	%.cc $(ALL_HEADERS)
	c++ -c $< -o $@ -DIP_ARCH_UNIX -I. $(PYTHON_CFLAGS)
