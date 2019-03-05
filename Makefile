ROOTDIR	=$(shell pwd)
WORKDIR	=$(ROOTDIR)/build

VERSION	:= 1.0.9
targets	+= DSIXXXXTest_$(VERSION).exe

.PHONY: targets

all : $(targets)
#api.c  config.h  ember.c  httpd.h  httpd_priv.h  ip_acl.c  protocol.c  version.c
srcs		+= $(ROOTDIR)/main.cpp
srcs		+= $(ROOTDIR)/src/timer.cpp
srcs		+= $(ROOTDIR)/src/socket.cpp
srcs		+= $(ROOTDIR)/src/cmd.cpp
srcs		+= $(ROOTDIR)/src/proto.cpp
srcs		+= $(ROOTDIR)/src/sheet.cpp
srcs		+= $(ROOTDIR)/src/ssh2.cpp
srcs		+= $(ROOTDIR)/src/visax.cpp
srcs	      	:= $(subst .cpp,.c,$(srcs))

objs 		:= $(subst $(ROOTDIR),$(WORKDIR), $(subst .c,.obj,$(srcs)))

objs		+= $(ROOTDIR)/build/res.res

#$(warning ==$(objs)==)

-include $(ROOTDIR)/make/arch.mk
-include $(ROOTDIR)/make/rules.mk

$(eval $(call LinkApp,DSIXXXXTest_$(VERSION).exe,$(objs)))

run:
	$(ROOTDIR)\build\DSIXXXXTest_$(VERSION).exe
