#$(MKDIR) -p "$(subst \,/,$(dir $@))"
$(ROOTDIR)/build/%$(OEXT): $(ROOTDIR)/%$(CPPEXT)
	$(MKDIR) -p $(dir $@)
	$(CPP) $(COMPILE) $< $(CPPFLAGS) $(CCOUT)"$@"

#$(MKDIR) -p "$(subst \,/,$(dir $@))"
$(ROOTDIR)/build/%$(OEXT): $(ROOTDIR)/%$(CEXT)
	$(MKDIR) -p $(dir $@)
	$(CC) $(COMPILE) $< $(CFLAGS) $(CCOUT)"$@"

#$(MKDIR) -p "$(subst \,/,$(dir $@))"
$(ROOTDIR)/build/%$(RESEXT): $(ROOTDIR)/%$(RCEXT)
	$(MKDIR) -p $(dir $@)
	$(RCC) $(RCCFLAGS) $(RCOUT) $@ $<
clean:
	$(RM) $(RMFLAGS) $(subst \,/,$(ROOTDIR)\build)

define LinkApp
$1: $2
	$(LD) $(LDFLAGS) $2 $(LDOUT)"$(ROOTDIR)/build/$1"
endef

