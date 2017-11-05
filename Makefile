# -*- Makefile -*-
# Eugene Skepner 2016
# ======================================================================

MAKEFLAGS = -w

# ----------------------------------------------------------------------

TARGETS = \
    $(ACMACS_BASE_LIB) \
    $(DIST)/json-pp \
    $(DIST)/test-rjson \
    $(DIST)/test-rjson-load \
    $(DIST)/test-argc-argv

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES = virus-name.cc color.cc text-style.cc time-series.cc json-importer.cc rjson.cc rjson-parser-pop.cc argc-argv.cc quicklook.cc
JSON_PP_SOURCES = rjson.cc json-pp.cc
TEST_RJSON_SOURCES = rjson.cc test-rjson.cc
TEST_ARGV_SOURCES = argc-argv.cc test-argc-argv.cc

# ----------------------------------------------------------------------

include $(ACMACSD_ROOT)/share/makefiles/Makefile.g++
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.vars

CXXFLAGS = -MMD -g $(OPTIMIZATION) $(PROFILE) -fPIC -std=$(STD) $(WARNINGS) -Icc -I$(BUILD)/include -I$(AD_INCLUDE) $(PKG_INCLUDES)
LDFLAGS = $(OPTIMIZATION) $(PROFILE)

ACMACS_BASE_LIB = $(DIST)/libacmacsbase.so
ACMACS_BASE_LDLIBS = $$(pkg-config --libs liblzma) $(FS_LIB) $(CXX_LIB) # -lprofiler

# ----------------------------------------------------------------------

all: install

install: check-acmacsd-root install-acmacs-base

lib: $(ACMACS_BASE_LIB)

test: $(TARGETS)
	test/test

profile-rjson: $(DIST)/profile-rjson-load
	env LLVM_PROFILE_FILE=/r/default.profraw $^ ~/ac/results/ssm/2017-0925-ssm/merges/niid-b-vic-hi.ace
	/usr/local/opt/llvm/bin/llvm-profdata merge -sparse /r/default.profraw -o /r/default.profdata
	rm /r/default.profraw
	/usr/local/opt/llvm/bin/llvm-cov report $^ -instr-profile=/r/default.profdata
	/usr/local/opt/llvm/bin/llvm-cov show $^ -instr-profile=/r/default.profdata

# ----------------------------------------------------------------------

install-acmacs-base: $(TARGETS)
	$(call install_lib,$(ACMACS_BASE_LIB))
	#@ln -sf $(SRC_DIR)/acmacs-base/bin/* $(AD_BIN)
	ln -sf $(abspath py/acmacs_base) $(AD_PY)
	if [ ! -d $(AD_INCLUDE)/acmacs-base ]; then mkdir $(AD_INCLUDE)/acmacs-base; fi
	ln -sf $(abspath cc)/*.hh $(AD_INCLUDE)/acmacs-base
	if [ ! -d $(AD_SHARE) ]; then mkdir $(AD_SHARE); fi
	ln -sf $(abspath $(DIST))/json-pp $(AD_BIN)

# ----------------------------------------------------------------------

-include $(BUILD)/*.d
include $(ACMACSD_ROOT)/share/makefiles/Makefile.dist-build.rules
RTAGS_TARGET = $(ACMACS_BASE_LIB)
include $(ACMACSD_ROOT)/share/makefiles/Makefile.rtags

# ----------------------------------------------------------------------

$(ACMACS_BASE_LIB): $(patsubst %.cc,$(BUILD)/%.o,$(ACMACS_BASE_SOURCES)) | $(DIST)
	@echo "SHARED     " $@ # '<--' $^
	@$(CXX) -shared $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# $(DIST)/test-rjson: $(patsubst %.cc,$(BUILD)/%.o,$(TEST_RJSON_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# $(DIST)/json-pp: $(patsubst %.cc,$(BUILD)/%.o,$(JSON_PP_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# $(DIST)/test-argc-argv: $(patsubst %.cc,$(BUILD)/%.o,$(TEST_ARGV_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^

$(DIST)/profile-rjson-load: cc/test-rjson-load.cc cc/rjson.cc cc/rjson-parser-pop.cc | $(DIST)
	@echo "PROFILE       " $@ # '<--' $^
	@$(CXX) -fprofile-instr-generate -fcoverage-mapping $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(ACMACS_BASE_LIB)
	@echo "LINK       " $@
	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LIB) $(ACMACS_BASE_LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
