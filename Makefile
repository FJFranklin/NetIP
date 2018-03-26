all:	nip

ALL_SOURCES=\
	ip_buffer.cpp \
	ip_channel.cpp \
	ip_connection.cpp \
	ip_header.cpp \
	ip_manager.cpp \
	ip_serial.cpp \
	ip_timer.cpp \
	ip_types.cpp \
	netip/unix/ip_arch.cc

ALL_OBJECTS=\
	nip.o\
	ip_buffer.o \
	ip_channel.o \
	ip_connection.o \
	ip_header.o \
	ip_manager.o \
	ip_serial.o \
	ip_timer.o \
	ip_types.o \
	netip/unix/ip_arch.o

ALL_HEADERS=\
	netip/ip_address.hh \
	netip/ip_buffer.hh \
	netip/ip_channel.hh \
	netip/ip_config.hh \
	netip/ip_connection.hh \
	netip/ip_defines.hh \
	netip/ip_header.hh \
	netip/ip_manager.hh \
	netip/ip_protocol.hh \
	netip/ip_serial.hh \
	netip/ip_timer.hh \
	netip/ip_types.hh \
	netip/unix/ip_arch.hh \
	netip/unix/ip_arch_serial.hh \
	netip/unix/ip_arch_serial.cc \
	tests.hh

clean:	
	rm $(ALL_OBJECTS)

nip:	$(ALL_OBJECTS) $(ALL_HEADERS)
	c++ -o nip $(ALL_OBJECTS)

%.o:	%.cpp $(ALL_HEADERS)
	c++ -c $< -o $@ -DIP_ARCH_UNIX -I.

%.o:	%.cc $(ALL_HEADERS)
	c++ -c $< -o $@ -DIP_ARCH_UNIX -I.
