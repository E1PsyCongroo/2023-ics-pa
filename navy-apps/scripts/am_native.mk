AS        = gcc-12
CC        = gcc-12
CXX       = g++-12
LNK_ADDR = $(if $(VME), 0x40000000, 0x03000000)
LDFLAGS += -Ttext-segment $(LNK_ADDR)
