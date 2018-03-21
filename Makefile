all:	nip

ALL_SOURCES=\
	ip_channel.cpp \
	ip_connection.cpp \
	ip_header.cpp \
	ip_manager.cpp \
	ip_timer.cpp \
	ip_types.cpp \
	netip/unix/ip_arch.cc

ALL_HEADERS=\
	netip/ip_address.hh \
	netip/ip_buffer.hh \
	netip/ip_channel.hh \
	netip/ip_config.hh \
	netip/ip_connection.hh \
	netip/ip_defines.hh \
	netip/ip_header.hh \
	netip/ip_manager.hh \
	netip/ip_timer.hh \
	netip/ip_types.hh \
	netip/unix/ip_arch.hh \
	tests.hh

nip:	nip.cc $(ALL_SOURCES) $(ALL_HEADERS)
	c++ -o nip nip.cc $(ALL_SOURCES) -DIP_ARCH_UNIX -I.
