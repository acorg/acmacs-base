# -*- Makefile -*-
# ======================================================================

TARGETS = \
  $(ACMACS_BASE_LIB) \
  $(DIST)/json-pp \
  $(DIST)/time-series-gen \
  $(DIST)/test-rjson2 \
  $(DIST)/test-argv \
  $(DIST)/test-string-split \
  $(DIST)/test-date2 \
  $(DIST)/test-double-to-string \
  $(DIST)/test-settings-v1 \
  $(DIST)/test-settings \
  $(DIST)/test-injson \
  $(DIST)/test-to-json \
  $(DIST)/test-read-file

all: install-acmacs-base

CONFIGURE_BOOST = 1
include $(ACMACSD_ROOT)/share/Makefile.config

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES = \
  color.cc text-style.cc read-file.cc layout.cc \
  argv.cc argc-argv.cc \
  normalize.cc \
  quicklook.cc timeit.cc gzip.cc \
  rjson.cc json-importer.cc settings.cc

JSON_PP_SOURCES = rjson.cc json-pp.cc
TEST_RJSON_SOURCES = rjson.cc test-rjson.cc
TEST_ARGV_SOURCES = argc-argv.cc test-argc-argv.cc

# ----------------------------------------------------------------------

ACMACS_BASE_LIB_MAJOR = 1
ACMACS_BASE_LIB_MINOR = 0
ACMACS_BASE_LIB = $(DIST)/$(call shared_lib_name,libacmacsbase,$(ACMACS_BASE_LIB_MAJOR),$(ACMACS_BASE_LIB_MINOR))
ACMACS_BASE_LDLIBS = $(L_BOOST) -lboost_date_time $(XZ_LIBS) $(BZ2_LIBS) $(GZ_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

lib: $(ACMACS_BASE_LIB)

test: install-acmacs-base | $(TARGETS)
	test/test
.PHONY: test

# ----------------------------------------------------------------------

install-acmacs-base: $(TARGETS)
	$(call install_lib,$(ACMACS_BASE_LIB))
	#@ln -sf $(SRC_DIR)/acmacs-base/bin/* $(AD_BIN)
	$(call symbolic_link,$(abspath py)/acmacs_base,$(AD_PY)/acmacs_base)
	if [ ! -d $(AD_INCLUDE)/acmacs-base ]; then mkdir $(AD_INCLUDE)/acmacs-base; fi
	ln -sf $(abspath cc)/*.hh $(AD_INCLUDE)/acmacs-base
	if [ ! -d $(AD_SHARE) ]; then mkdir $(AD_SHARE); fi
	ln -sf $(abspath $(DIST))/json-pp $(AD_BIN)
	ln -sf $(abspath dist)/time-series-gen $(AD_BIN)

.PHONY: install-acmacs-base test

# ----------------------------------------------------------------------

RTAGS_TARGET = $(ACMACS_BASE_LIB)

# ----------------------------------------------------------------------

$(ACMACS_BASE_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_BASE_SOURCES)) | $(DIST)
	$(call echo_shared_lib,$@)
	$(call make_shared_lib,libacmacsbase,$(ACMACS_BASE_LIB_MAJOR),$(ACMACS_BASE_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(DIST) $(ACMACS_BASE_LIB)
	$(call echo_link_exe,$@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LIB) $(ACMACS_BASE_LDLIBS) $(AD_RPATH)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
