default: bin

.PHONY: bin clean .FORCE
.FORCE: ;

CC = gcc
LD = gcc
AR = gcc-ar
MD = mkdir -p
RM = rm -rf

CSTD          = c99
CFLAGS        = -std=$(CSTD) -Wall -Wextra -pedantic -Werror -I$(INCLUDEDIR)
LDFLAGS       =
LIBS          = $(SMLISPDIR)/build/libsmlisp.a
ARFLAGS       = cr

TESTFLAGS     = -fsanitize=address -fsanitize=leak -fsanitize=undefined
DEBUGFLAGS   := -g -O0 $(TESTFLAGS)
RELEASEFLAGS  = -DNDEBUG -flto -O3
DEPFLAGS      = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

ifdef RELEASE
	CFLAGS  += $(RELEASEFLAGS)
	LDFLAGS += $(RELEASEFLAGS)
else
	CFLAGS  += $(DEBUGFLAGS)
	LDFLAGS += $(DEBUGFLAGS)
	TESTFLAGS =
endif

BUILDDIR   = build
OBJDIR     = build/objs
DEPDIR     = build/deps
DIRS       = $(BUILDDIR) $(OBJDIR) $(DEPDIR)

SMLISPDIR  = ../smlisp
INCLUDEDIR = $(SMLISPDIR)/include
SRCDIR     = .

OBJS       = $(patsubst %.c,$(OBJDIR)/%.o,$(notdir $(wildcard $(SRCDIR)/*.c)))

$(DIRS):
	$(MD) $@

$(OBJDIR)/%.o : $(SRCDIR)/%.c
$(OBJDIR)/%.o : $(SRCDIR)/%.c $(DEPDIR)/%.d | $(DIRS)
	$(CC) $(DEPFLAGS) $(CFLAGS) -c -o $@ $<
	@mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d && touch $@

$(BUILDDIR)/test : $(OBJS) $(LIBS) | $(DIRS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

bin: $(BUILDDIR)/test

$(SMLISPDIR)/build/libsmlisp.a: .FORCE
	make -C $(SMLISPDIR)

clean:
	$(RM) $(DIRS)
	make -C $(SMLISPDIR) clean

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d

include $(wildcard $(DEPDIR)/*.d)
