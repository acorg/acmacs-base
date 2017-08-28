# -*- Makefile -*-
# Eugene Skepner 2016
# ======================================================================

MAKEFLAGS = -w

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES = virus-name.cc color.cc time-series.cc json-importer.cc
TEST_RJSON_SOURCES = rjson.cc test-rjson.cc

# ----------------------------------------------------------------------

include Makefile.g++
include Makefile.dist-build.vars

# -fvisibility=hidden and -flto make resulting lib smaller (pybind11) but linking is much slower
OPTIMIZATION = -O3 #-fvisibility=hidden -flto
PROFILE = # -pg
CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WEVERYTHING) $(WARNINGS) -Icc -I$(BUILD)/include -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)

ACMACS_BASE_LIB = $(DIST)/libacmacsbase.so
ACMACS_BASE_LDLIBS = # -L$(AD_LIB)

# PYTHON_VERSION = $(shell python3 -c 'import sys; print("{0.major}.{0.minor}".format(sys.version_info))')
# PYTHON_CONFIG = python$(PYTHON_VERSION)-config
# PYTHON_MODULE_SUFFIX = $(shell $(PYTHON_CONFIG) --extension-suffix)
# PYTHON_LD_LIB = $$($(PYTHON_CONFIG) --ldflags | sed -E 's/-Wl,-stack_size,[0-9]+//')

# PKG_INCLUDES = $$($(PYTHON_CONFIG) --includes)

# ----------------------------------------------------------------------

all: check-python install

install: check-acmacsd-root make-dirs install-acmacs-base

make-dirs:
	bin/__setup_dirs acmacs-base

ifeq ($(GCC7),Y)
test:
else
test: $(DIST)/test-rjson
	test/test
endif

# ----------------------------------------------------------------------

install-acmacs-base: $(ACMACS_BASE_LIB)
	ln -sf $(ACMACS_BASE_LIB) $(AD_LIB)
	if [ $$(uname) = "Darwin" ]; then /usr/bin/install_name_tool -id $(AD_LIB)/$(notdir $(ACMACS_BASE_LIB)) $(AD_LIB)/$(notdir $(ACMACS_BASE_LIB)); fi
	if [ -d $(SRC_DIR)/acmacs-base ]; then (cd $(SRC_DIR)/acmacs-base; git pull); else git clone git@github.com:acorg/acmacs-base.git $(SRC_DIR)/acmacs-base; fi
	ln -sf $(SRC_DIR)/acmacs-base/bin/* $(AD_BIN)
	ln -sf $(SRC_DIR)/acmacs-base/py/acmacs_base $(AD_PY)
	if [ ! -d $(AD_INCLUDE)/acmacs-base ]; then mkdir $(AD_INCLUDE)/acmacs-base; fi
	ln -sf $(abspath cc)/*.hh $(AD_INCLUDE)/acmacs-base
	if [ ! -d $(AD_SHARE) ]; then mkdir $(AD_SHARE); fi
	ln -sf $(abspath .)/Makefile.* $(AD_SHARE)

install-3rd-party:
	$(ACMACSD_ROOT)/bin/update-3rd-party

# ----------------------------------------------------------------------

-include $(BUILD)/*.d

# ----------------------------------------------------------------------

$(ACMACS_BASE_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_BASE_SOURCES)) | $(DIST)
	$(CXX) -shared $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

$(DIST)/test-rjson: $(patsubst %.cc,$(BUILD)/%.o,$(TEST_RJSON_SOURCES)) | $(DIST)
	$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# ----------------------------------------------------------------------

$(BUILD)/%.o: cc/%.cc | $(BUILD)
	@echo $<
	@$(CXX) $(CXXFLAGS) -c -o $@ $<

# ----------------------------------------------------------------------

include Makefile.dist-build.rules

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
