const char * test_1_info = "15:41:36.512258 98:9e:63:25:99:1c > 8c:0d:76:66:7c:1d, ethertype IPv4 (0x0800), length 54: (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 40)\n"
  "192.168.1.7.51632 > 192.0.73.2.443: Flags [.], cksum 0xf37c (correct), seq 4117, ack 104479, win 8148, length 0\n";

u8_t test_1[] = { 
  0x45,0x00,0x00,0x28, 0x00,0x00,0x40,0x00, 0x40,0x06,0x70,0x1e, 0xc0,0xa8,0x01,0x07,
  0xc0,0x00,0x49,0x02, 0xc9,0xb0,0x01,0xbb, 0x49,0xcb,0xb0,0xfc, 0x52,0xf7,0xb8,0xa6,
  0x50,0x10,0x1f,0xd4, 0xf3,0x7c,0x00,0x00 };

const char * test_2_info = "15:41:36.513653 8c:0d:76:66:7c:1d > 98:9e:63:25:99:1c, ethertype IPv4 (0x0800), length 205: (tos 0x0, ttl 60, id 45655, offset 0, flags [DF], proto TCP (6), length 191)\n"
  "192.0.73.2.443 > 192.168.1.7.51632: Flags [P.], cksum 0x373f (correct), seq 105931:106082, ack 3950, win 78, length 151\n";

u8_t test_2[] = {
  0x45,0x00,0x00,0xbf, 0xb2,0x57,0x40,0x00, 0x3c,0x06,0xc1,0x2f, 0xc0,0x00,0x49,0x02,
  0xc0,0xa8,0x01,0x07, 0x01,0xbb,0xc9,0xb0, 0x52,0xf7,0xbe,0x52, 0x49,0xcb,0xb0,0x55,
  0x50,0x18,0x00,0x4e, 0x37,0x3f,0x00,0x00, 0x1f,0x9f,0xb4,0x4e, 0xc8,0x75,0xb8,0x78,
  0xed,0xac,0x74,0xb2, 0x50,0x78,0x6b,0x82, 0x64,0x5e,0xcd,0xca, 0xb4,0xea,0x0b,0x7e,
  0xba,0xae,0x6e,0xd0, 0x15,0x64,0x67,0x62, 0x65,0xfc,0xfc,0x7f, 0x5a,0xa2,0x8b,0x69,
  0xb7,0x1b,0xb6,0x34, 0x94,0x14,0xcd,0x46, 0x54,0xff,0x4a,0x2b, 0xb5,0x95,0xda,0x40,
  0x80,0xc4,0x23,0xd2, 0xae,0x40,0xc7,0xbe, 0xff,0x6d,0x29,0x12, 0x5b,0x70,0xa6,0x1c,
  0x4b,0x27,0xa5,0x66, 0xe4,0x09,0x3c,0x0b, 0x50,0x27,0x29,0xe3, 0x4c,0xad,0x66,0x70,
  0x23,0x58,0x92,0x5a, 0xfc,0x40,0x85,0x54, 0xb0,0x65,0x39,0x6c, 0x21,0xd4,0x09,0x7e,
  0xac,0x06,0x66,0xd4, 0x8b,0xd1,0xf1,0x8c, 0x2b,0x95,0x8c,0x66, 0x8f,0x96,0x4a,0x46,
  0xe1,0x8b,0x28,0xdf, 0x5a,0x80,0x3a,0x92, 0x62,0xa9,0xe5,0x5f, 0xfd,0xfe,0xca,0xdc,
  0x0c,0x0f,0x85,0xe6, 0x3f,0x3a,0x0b,0x4e, 0xb7,0x80,0x2b,0x7e, 0x7d,0x0f,0xa6 };

const char * test_3_info = "15:41:36.513677 98:9e:63:25:99:1c > 8c:0d:76:66:7c:1d, ethertype IPv4 (0x0800), length 54: (tos 0x0, ttl 64, id 0, offset 0, flags [DF], proto TCP (6), length 40)\n"
  "192.168.1.7.51632 > 192.0.73.2.443: Flags [.], cksum 0xed12 (correct), seq 4117, ack 106082, win 8187, length 0\n";

u8_t test_3[] = {
  0x45,0x00,0x00,0x28, 0x00,0x00,0x40,0x00, 0x40,0x06,0x70,0x1e, 0xc0,0xa8,0x01,0x07,
  0xc0,0x00,0x49,0x02, 0xc9,0xb0,0x01,0xbb, 0x49,0xcb,0xb0,0xfc, 0x52,0xf7,0xbe,0xe9,
  0x50,0x10,0x1f,0xfb, 0xed,0x12,0x00,0x00 };

const char * test_4_info = "15:41:36.514216 8c:0d:76:66:7c:1d > 98:9e:63:25:99:1c, ethertype IPv4 (0x0800), length 92: (tos 0x0, ttl 60, id 45656, offset 0, flags [DF], proto TCP (6), length 78)\n"
  "192.0.73.2.443 > 192.168.1.7.51632: Flags [P.], cksum 0xd37c (correct), seq 106082:106120, ack 3950, win 78, length 38\n";

u8_t test_4[] = {
  0x45,0x00,0x00,0x4e, 0xb2,0x58,0x40,0x00, 0x3c,0x06,0xc1,0x9f, 0xc0,0x00,0x49,0x02,
  0xc0,0xa8,0x01,0x07, 0x01,0xbb,0xc9,0xb0, 0x52,0xf7,0xbe,0xe9, 0x49,0xcb,0xb0,0x55,
  0x50,0x18,0x00,0x4e, 0xd3,0x7c,0x00,0x00, 0x17,0x03,0x03,0x00, 0x21,0xc4,0x78,0xf8,
  0xed,0x6a,0x29,0xa1, 0xac,0x4c,0x96,0x31, 0xd3,0x50,0x8f,0x03, 0x2f,0x47,0x1c,0xde,
  0xb0,0xa8,0x1d,0x9b, 0x6b,0x8b,0x12,0x28, 0x24,0x7e,0x21,0x3a, 0xeb,0x48 };

const char * test_5_info = "15:41:36.156106 8c:0d:76:66:7c:1d > 01:00:5e:00:00:01, ethertype IPv4 (0x0800), length 46: (tos 0xc0, ttl 1, id 0, offset 0, flags [DF], proto IGMP (2), length 32, options (RA))\n"
  "192.168.1.1 > 224.0.0.1: igmp query v2\n";

u8_t test_5[] = {
  0x46, 0xc0, 0x00, 0x20, 0x00, 0x00, 0x40, 0x00, 0x01, 0x02, 0x42, 0x6d, 0xc0, 0xa8, 0x01, 0x01,
  0xe0, 0x00, 0x00, 0x01, 0x94, 0x04, 0x00, 0x00, 0x11, 0x64, 0xee, 0x9b, 0x00, 0x00, 0x00, 0x00
};

const char * test_6_info = "15:41:40.310170 98:9e:63:25:99:1c > 01:00:5e:00:00:fb, ethertype IPv4 (0x0800), length 46: (tos 0x0, ttl 1, id 1665, offset 0, flags [none], proto IGMP (2), length 32, options (RA))\n"
  "192.168.1.7 > 224.0.0.251: igmp v2 report 224.0.0.251\n";

u8_t test_6[] = {
  0x46, 0x00, 0x00, 0x20, 0x06, 0x81, 0x00, 0x00, 0x01, 0x02, 0x7b, 0xac, 0xc0, 0xa8, 0x01, 0x07,
  0xe0, 0x00, 0x00, 0xfb, 0x94, 0x04, 0x00, 0x00, 0x16, 0x00, 0x09, 0x04, 0xe0, 0x00, 0x00, 0xfb
};

const char * test_7_info = "09:16:57.935478 AF IPv6 (30), length 519: (flowlabel 0x2436b, hlim 255, next-header UDP (17) payload length: 475) fe80::10a0:c436:97f4:e58a.5353 > ff02::fb.5353: [udp sum ok] 0 [3a] [18q] PTR (QM)? _airport._tcp.local. PTR (QM)? _uscan._tcp.local. PTR (QM)? _uscans._tcp.local. PTR (QM)? _ippusb._tcp.local. PTR (QM)? _scanner._tcp.local. PTR (QM)? _ipp._tcp.local. PTR (QM)? _ipps._tcp.local. PTR (QM)? _printer._tcp.local. PTR (QM)? _pdl-datastream._tcp.local. PTR (QM)? _ptp._tcp.local. PTR (QM)? _apple-mobdev._tcp.local. PTR (QM)? 916f07e8._sub._apple-mobdev2._tcp.local. PTR (QM)? _apple-pairable._tcp.local. PTR (QM)? _daap._tcp.local. PTR (QM)? _touch-remote._tcp.local. PTR (QM)? _googlecast._tcp.local. PTR (QM)? _privet._tcp.local. PTR (QM)? _sleep-proxy._udp.local. _scanner._tcp.local. [56m47s] PTR Photosmart 7510 series [B5D209]._scanner._tcp.local., _ipp._tcp.local. [56m47s] PTR Photosmart 7510 series [B5D209]._ipp._tcp.local., _pdl-datastream._tcp.local. [56m47s] PTR Photosmart 7510 series [B5D209]._pdl-datastream._tcp.local. (467)\n";

u8_t test_7[] = {
  0x60, 0x02, 0x43, 0x6b, 0x01, 0xdb, 0x11, 0xff, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x10, 0xa0, 0xc4, 0x36, 0x97, 0xf4, 0xe5, 0x8a, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfb, 0x14, 0xe9, 0x14, 0xe9, 0x01, 0xdb, 0x4c, 0xed,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x08, 0x5f, 0x61, 0x69,
  0x72, 0x70, 0x6f, 0x72, 0x74, 0x04, 0x5f, 0x74, 0x63, 0x70, 0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c,
  0x00, 0x00, 0x0c, 0x00, 0x01, 0x06, 0x5f, 0x75, 0x73, 0x63, 0x61, 0x6e, 0xc0, 0x15, 0x00, 0x0c,
  0x00, 0x01, 0x07, 0x5f, 0x75, 0x73, 0x63, 0x61, 0x6e, 0x73, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01,
  0x07, 0x5f, 0x69, 0x70, 0x70, 0x75, 0x73, 0x62, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x08, 0x5f,
  0x73, 0x63, 0x61, 0x6e, 0x6e, 0x65, 0x72, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x04, 0x5f, 0x69,
  0x70, 0x70, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x05, 0x5f, 0x69, 0x70, 0x70, 0x73, 0xc0, 0x15,
  0x00, 0x0c, 0x00, 0x01, 0x08, 0x5f, 0x70, 0x72, 0x69, 0x6e, 0x74, 0x65, 0x72, 0xc0, 0x15, 0x00,
  0x0c, 0x00, 0x01, 0x0f, 0x5f, 0x70, 0x64, 0x6c, 0x2d, 0x64, 0x61, 0x74, 0x61, 0x73, 0x74, 0x72,
  0x65, 0x61, 0x6d, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x04, 0x5f, 0x70, 0x74, 0x70, 0xc0, 0x15,
  0x00, 0x0c, 0x00, 0x01, 0x0d, 0x5f, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2d, 0x6d, 0x6f, 0x62, 0x64,
  0x65, 0x76, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x08, 0x39, 0x31, 0x36, 0x66, 0x30, 0x37, 0x65,
  0x38, 0x04, 0x5f, 0x73, 0x75, 0x62, 0x0e, 0x5f, 0x61, 0x70, 0x70, 0x6c, 0x65, 0x2d, 0x6d, 0x6f,
  0x62, 0x64, 0x65, 0x76, 0x32, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x0f, 0x5f, 0x61, 0x70, 0x70,
  0x6c, 0x65, 0x2d, 0x70, 0x61, 0x69, 0x72, 0x61, 0x62, 0x6c, 0x65, 0xc0, 0x15, 0x00, 0x0c, 0x00,
  0x01, 0x05, 0x5f, 0x64, 0x61, 0x61, 0x70, 0xc0, 0x15, 0x00, 0x0c, 0x00, 0x01, 0x0d, 0x5f, 0x74,
  0x6f, 0x75, 0x63, 0x68, 0x2d, 0x72, 0x65, 0x6d, 0x6f, 0x74, 0x65, 0xc0, 0x15, 0x00, 0x0c, 0x00,
  0x01, 0x0b, 0x5f, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x63, 0x61, 0x73, 0x74, 0xc0, 0x15, 0x00,
  0x0c, 0x00, 0x01, 0x07, 0x5f, 0x70, 0x72, 0x69, 0x76, 0x65, 0x74, 0xc0, 0x15, 0x00, 0x0c, 0x00,
  0x01, 0x0c, 0x5f, 0x73, 0x6c, 0x65, 0x65, 0x70, 0x2d, 0x70, 0x72, 0x6f, 0x78, 0x79, 0x04, 0x5f,
  0x75, 0x64, 0x70, 0xc0, 0x1a, 0x00, 0x0c, 0x00, 0x01, 0xc0, 0x4e, 0x00, 0x0c, 0x00, 0x01, 0x00,
  0x00, 0x0d, 0x4f, 0x00, 0x22, 0x1f, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x73, 0x6d, 0x61, 0x72, 0x74,
  0x20, 0x37, 0x35, 0x31, 0x30, 0x20, 0x73, 0x65, 0x72, 0x69, 0x65, 0x73, 0x20, 0x5b, 0x42, 0x35,
  0x44, 0x32, 0x30, 0x39, 0x5d, 0xc0, 0x4e, 0xc0, 0x5d, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x0d,
  0x4f, 0x00, 0x22, 0x1f, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x73, 0x6d, 0x61, 0x72, 0x74, 0x20, 0x37,
  0x35, 0x31, 0x30, 0x20, 0x73, 0x65, 0x72, 0x69, 0x65, 0x73, 0x20, 0x5b, 0x42, 0x35, 0x44, 0x32,
  0x30, 0x39, 0x5d, 0xc0, 0x5d, 0xc0, 0x83, 0x00, 0x0c, 0x00, 0x01, 0x00, 0x00, 0x0d, 0x4f, 0x00,
  0x22, 0x1f, 0x50, 0x68, 0x6f, 0x74, 0x6f, 0x73, 0x6d, 0x61, 0x72, 0x74, 0x20, 0x37, 0x35, 0x31,
  0x30, 0x20, 0x73, 0x65, 0x72, 0x69, 0x65, 0x73, 0x20, 0x5b, 0x42, 0x35, 0x44, 0x32, 0x30, 0x39,
  0x5d, 0xc0, 0x83
};

const char * test_8_info = "09:17:07.334192 AF IPv6 (30), length 60: (flowlabel 0xc52c6, hlim 64, next-header ICMPv6 (58) payload length: 16) fe80::1 > fe80::1: [icmp6 sum ok] ICMP6, echo request, seq 0\n";

u8_t test_8[] = {
  0x60, 0x0c, 0x52, 0xc6, 0x00, 0x10, 0x3a, 0x40, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x51, 0x2a, 0xeb, 0xee, 0x00, 0x00,
  0x5a, 0xb0, 0xd1, 0x93, 0x00, 0x05, 0x19, 0x4f
};

const char * test_9_info = "09:17:07.334213 AF IPv6 (30), length 60: (hlim 64, next-header ICMPv6 (58) payload length: 16) fe80::1 > fe80::1: [icmp6 sum ok] ICMP6, echo reply, seq 0\n";

u8_t test_9[] = {
  0x60, 0x00, 0x00, 0x00, 0x00, 0x10, 0x3a, 0x40, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00, 0x50, 0x2a, 0xeb, 0xee, 0x00, 0x00,
  0x5a, 0xb0, 0xd1, 0x93, 0x00, 0x05, 0x19, 0x4f
};

struct ip_packet {
  const char * info;
  const u8_t * data;

  u16_t size;
};

struct ip_packet tests[] = {
  { test_1_info, test_1, sizeof test_1 },
  { test_2_info, test_2, sizeof test_2 },
  { test_3_info, test_3, sizeof test_3 },
  { test_4_info, test_4, sizeof test_4 },
  { test_5_info, test_5, sizeof test_5 },
  { test_6_info, test_6, sizeof test_6 },
  { test_7_info, test_7, sizeof test_7 },
  { test_8_info, test_8, sizeof test_8 },
  { test_9_info, test_9, sizeof test_9 }
};

u8_t test_count = sizeof (tests) / sizeof (tests[0]);
