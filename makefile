include makefile.inc

NOW = $(shell date +"%Y-%m-%d(%H:%M:%S %z)")

# TARGETS
TARGET = $(TARGET_NAME)-$(VERSION)

# directories
# source directories
SRCDIR = ./src
INCDIR_PRIV = ./include_priv
INCDIRS = $(INCDIR_PRIV)

# build destination directories
OBJDIR = ./output/$(MACHINE)
PKGDIR = ./output/$(MACHINE)/pkg/

# files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))
		
# compilation and linking flags
CFLAGS += -Werror -Wall -Wextra \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings -Wredundant-decls -Wmissing-include-dirs \
		  -Wno-format-nonliteral  -Wno-format-nonliteral \
          -g3 -Wmissing-declarations $(addprefix -I ,$(INCDIRS)) \
		  -pthread 

ifeq ($(CC_NAME),g++)
    CFLAGS += -std=c++2a
else
	CFLAGS += -Wstrict-prototypes -Wold-style-definition -Wnested-externs -std=c11
endif

LDFLAGS += -lamxc -lamxd -lamxo

# helper functions - used in multiple targets
define install_to
	$(INSTALL) -d $(1)$(PREFIX)$(INSTALL_BIN_DIR)
	$(INSTALL) -m 0755 $(TARGET_NAME) $(1)$(PREFIX)$(INSTALL_BIN_DIR)/$(TARGET_NAME)
endef

define create_changelog
	@$(ECHO) "Update changelog"
	mv CHANGELOG.md CHANGELOG.md.bak 
	head -n 9 CHANGELOG.md.bak > CHANGELOG.md
	$(ECHO) "" >> CHANGELOG.md 
	$(ECHO) "## Release $(VERSION) - $(NOW)" >> CHANGELOG.md  
	$(ECHO) "" >> CHANGELOG.md 
	$(GIT) log --pretty=format:"- %s" $$($(GIT) describe --tags | grep -v "merge" | cut -d'-' -f1)..HEAD  >> CHANGELOG.md
	$(ECHO) "" >> CHANGELOG.md 
	tail -n +10 CHANGELOG.md.bak >> CHANGELOG.md
	rm CHANGELOG.md.bak
endef

# targets
all: include_priv/version.h $(TARGET_NAME)

$(TARGET_NAME):  include_priv/version.h $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

-include $(OBJECTS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)/
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(CC) $(CFLAGS) -MM -MP -MT '$(@) $(@:.o=.d)' -MF $(@:.o=.d) $(<)

include_priv/version.h : include_priv/version.h.m4
	@echo "Current version = $(VMAJOR).$(VMINOR).$(VBUILD)"
	m4 -DMAJOR=$(VMAJOR) -DMINOR=$(VMINOR) -DBUILD=$(VBUILD) $(<) > $(@)

$(OBJDIR)/:
	$(MKDIR) -p $@

changelog:
	$(call create_changelog) 

package: $(TARGET_NAME)
	$(call install_to,$(PKGDIR)) 
	cd $(PKGDIR) && $(TAR) -czvf ../$(TARGET_NAME)-$(VERSION).tar.gz .
	cp $(PKGDIR)../$(TARGET_NAME)-$(VERSION).tar.gz .
	make -C packages

install: $(TARGET_NAME)
	$(call install_to,$(DEST)) 

clean:
	rm -rf ./output/ $(TARGET_NAME)

.PHONY: clean changelog
