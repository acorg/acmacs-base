# -*- Makefile -*-
# Eugene Skepner 2016
# ======================================================================

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES = virus-name.cc color.cc

# ----------------------------------------------------------------------

CLANG = $(shell if g++ --version 2>&1 | grep -i llvm >/dev/null; then echo Y; else echo N; fi)
ifeq ($(CLANG),Y)
  WEVERYTHING = -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
  WARNINGS = -Wno-weak-vtables # -Wno-padded
  STD = c++14
else
  WEVERYTHING = -Wall -Wextra
  WARNINGS =
  STD = c++14
endif

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(ACMACSD_ROOT)/include $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)

LIB_DIR = $(ACMACSD_ROOT)/lib
ACMACS_BASE_LIB = $(DIST)/libacmacsbase.so
ACMACS_BASE_LDLIBS = # -L$(LIB_DIR)

# PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
# PYTHON_CONFIG = python$(PYTHON_VERSION)-config
# PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)
# PYTHON_LD_LIB = $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

# PKG_INCLUDES = $$($(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

BUILD = build
DIST = $(abspath dist)

# ----------------------------------------------------------------------

all: check-python install

install: check-acmacsd-root make-dirs install-packages

make-dirs:
	if [ ! -d $(ACMACSD_ROOT) ]; then mkdir $(ACMACSD_ROOT); fi
	for d in bin lib include include/acmacs-base py data sources; do if [ ! -d $(ACMACSD_ROOT)/$$d ]; then mkdir $(ACMACSD_ROOT)/$$d; fi; done
	cd $(ACMACSD_ROOT)/include/acmacs-base && ln -sf ../../sources/acmacs-base/cc/*.hh .

# ----------------------------------------------------------------------

install-packages: install-acmacs-base install-3rd-party

install-acmacs-base: $(ACMACS_BASE_LIB)
	ln -sf $(ACMACS_BASE_LIB) $(ACMACSD_ROOT)/lib
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(ACMACSD_ROOT)/lib/$(notdir $(ACMACS_BASE_LIB)) $(ACMACSD_ROOT)/lib/$(notdir $(ACMACS_BASE_LIB)); fi
	if [ -d $(SRC_DIR)/acmacs-base ]; then (cd $(SRC_DIR)/acmacs-base; git pull); else git clone git@github.com:acorg/acmacs-base.git $(SRC_DIR)/acmacs-base; fi
	ln -sf $(SRC_DIR)/acmacs-base/bin/* $(ACMACSD_ROOT)/bin
	ln -sf $(SRC_DIR)/acmacs-base/py/acmacs_base $(ACMACSD_ROOT)/py
	if [ ! -d $(ACMACSD_ROOT)/include/acmacs-base ]; then mkdir $(ACMACSD_ROOT)/include/acmacs-base; fi
	ln -sf $(abspath cc)/*.hh $(ACMACSD_ROOT)/include/acmacs-base

install-3rd-party:
	$(ACMACSD_ROOT)/bin/update-3rd-party

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(ACMACS_BASE_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_BASE_SOURCES)) | $(DIST)
	g++ -shared $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

clean:
	rm -rf $(DIST) $(BUILD)/*.o $(BUILD)/*.d

distclean: clean
	rm -rf $(BUILD)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD)
	@echo $<
	@g++ $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

check-python:
	@printf 'import sys\nif sys.version_info < (3, 5):\n print("Python 3.5 is required")\n exit(1)' | python3

$(DIST):
	mkdir -p $(DIST)

$(BUILD):
	mkdir -p $(BUILD)

.PHONY: check-acmacsd-root check-python

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
