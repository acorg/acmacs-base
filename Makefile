# -*- Makefile -*-
# ======================================================================

TARGETS = \
  $(ACMACS_BASE_LIB) \
  $(DIST)/json-pp-v2 \
  $(DIST)/json-pp \
  $(DIST)/css-amino-acid-nucleotide-colors \
  $(DIST)/cxx-regex-search \
  $(DIST)/time-series-gen \
  $(DIST)/test-rjson-v2 \
  $(DIST)/test-rjson-v3 \
  $(DIST)/test-argv \
  $(DIST)/test-string-split \
  $(DIST)/test-date2 \
  $(DIST)/test-double-to-string \
  $(DIST)/test-settings-v1 \
  $(DIST)/test-settings-v2 \
  $(DIST)/test-settings-v3 \
  $(DIST)/test-injson \
  $(DIST)/test-to-json \
  $(DIST)/test-time-series \
  $(DIST)/test-read-file \
  $(DIST)/test-find-color \
  $(DIST)/test-bezier-gradient \
  $(DIST)/test-string-join \
  $(DIST)/test-string-substitute \
  $(DIST)/test-color-modifier \
  $(DIST)/test-brotli

all: install-acmacs-base

CONFIGURE_BOOST = 1
# ECHO_COMPILATION_COMMAND = 1
include $(ACMACSD_ROOT)/share/Makefile.config

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES =  \
  settings-v2.cc       \
  settings-v3.cc       \
  settings-v3-env.cc   \
  rjson-v2.cc          \
  rjson-v3.cc          \
  time-series.cc       \
  read-file.cc         \
  color.cc             \
  layout.cc            \
  color-modifier.cc    \
  argc-argv.cc         \
  messages.cc          \
  color-gradient.cc    \
  argv.cc              \
  color-continent.cc   \
  timeit.cc            \
  quicklook.cc         \
  color-distinct.cc    \
  color-amino-acid.cc  \
  color-hsv.cc         \
  rjson-v3-helper.cc   \
  fmt.cc               \
  html.cc              \
  gzip.cc              \
  xz.cc                \
  coredump.cc          \
  log.cc

# JSON_PP_SOURCES = rjson.cc json-pp.cc
# TEST_RJSON_SOURCES = rjson.cc test-rjson.cc
# TEST_ARGV_SOURCES = argc-argv.cc test-argc-argv.cc

# ----------------------------------------------------------------------

ACMACS_BASE_LIB_MAJOR = 1
ACMACS_BASE_LIB_MINOR = 0
ACMACS_BASE_LIB = $(DIST)/$(call shared_lib_name,libacmacsbase,$(ACMACS_BASE_LIB_MAJOR),$(ACMACS_BASE_LIB_MINOR))
ACMACS_BASE_LDLIBS = $(XZ_LIBS) $(BZ2_LIBS) $(GZ_LIBS) $(BROTLI_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

lib: $(ACMACS_BASE_LIB)

test: install-acmacs-base | $(TARGETS)
	test/test
.PHONY: test

# ----------------------------------------------------------------------

install-acmacs-base: make-installation-dirs $(TARGETS)
	$(call install_lib,$(ACMACS_BASE_LIB))
	$(call install_all,$(AD_PACKAGE_NAME))

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
