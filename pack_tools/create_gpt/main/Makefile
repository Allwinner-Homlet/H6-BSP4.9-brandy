
##
## Makefile for Sunxi Secure Boot
##



include $(TOPDIR)/config.mk

LIB	:= $(obj)libcreate_gpt.o

COBJS-y += create_gpt.o
COBJS-y += crc.o
COBJS-y += file.o
COBJS-y += main.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
