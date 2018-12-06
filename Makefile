# -*- Makefile -*-
# ======================================================================

TARGETS = \
    $(ACMACS_BASE_LIB) \
    $(DIST)/json-pp \
    $(DIST)/time-series-gen \
    $(DIST)/test-rjson2 \
    $(DIST)/test-argc-argv \
    $(DIST)/test-string-split \
    $(DIST)/test-date2 \
    $(DIST)/test-double-to-string \
    $(DIST)/test-settings

all: install-acmacs-base

include $(ACMACSD_ROOT)/share/Makefile.config

# ----------------------------------------------------------------------

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

ACMACS_BASE_SOURCES = \
    virus-name.cc color.cc text-style.cc time-series.cc read-file.cc layout.cc \
    argc-argv.cc quicklook.cc timeit.cc gzip.cc \
    rjson.cc json-importer.cc

JSON_PP_SOURCES = rjson.cc json-pp.cc
TEST_RJSON_SOURCES = rjson.cc test-rjson.cc
TEST_ARGV_SOURCES = argc-argv.cc test-argc-argv.cc

# ----------------------------------------------------------------------

ACMACS_BASE_LIB_MAJOR = 1
ACMACS_BASE_LIB_MINOR = 0
ACMACS_BASE_LIB = $(DIST)/$(call shared_lib_name,libacmacsbase,$(ACMACS_BASE_LIB_MAJOR),$(ACMACS_BASE_LIB_MINOR))
ACMACS_BASE_LDLIBS = -L$(BOOST_LIB_PATH) -lboost_date_time $(XZ_LIBS) $(BZ2_LIBS) $(GZ_LIBS) $(CXX_LIBS)

# ----------------------------------------------------------------------

lib: $(ACMACS_BASE_LIB)

test: install-acmacs-base | $(TARGETS)
	test/test

# ----------------------------------------------------------------------

install-acmacs-base: $(TARGETS)
	$(call install_lib,$(ACMACS_BASE_LIB))
	#@ln -sf $(SRC_DIR)/acmacs-base/bin/* $(AD_BIN)
	ln -sf $(abspath py/acmacs_base) $(AD_PY)
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
	printf "%-16s %s\n" "SHARED" $@
	$(call make_shared,libacmacsbase,$(ACMACS_BASE_LIB_MAJOR),$(ACMACS_BASE_LIB_MINOR)) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

$(DIST)/%: $(BUILD)/%.o | $(ACMACS_BASE_LIB)
	printf "%-16s %s\n" "LINK" $@
	$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LIB) $(ACMACS_BASE_LDLIBS) $(AD_RPATH)

# ----------------------------------------------------------------------
# ----------------------------------------------------------------------

# $(DIST)/test-rjson: $(patsubst %.cc,$(BUILD)/%.o,$(TEST_RJSON_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# $(DIST)/json-pp: $(patsubst %.cc,$(BUILD)/%.o,$(JSON_PP_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# $(DIST)/test-argc-argv: $(patsubst %.cc,$(BUILD)/%.o,$(TEST_ARGV_SOURCES)) | $(DIST)
#	@echo "LINK       " $@ # '<--' $^
#	@$(CXX) $(LDFLAGS) -o $@ $^

# profile-rjson: $(DIST)/profile-rjson-load
#	env LLVM_PROFILE_FILE=/r/default.profraw $^ ~/ac/results/ssm/2017-0925-ssm/merges/niid-b-vic-hi.ace
#	/usr/local/opt/llvm/bin/llvm-profdata merge -sparse /r/default.profraw -o /r/default.profdata
#	rm /r/default.profraw
#	/usr/local/opt/llvm/bin/llvm-cov report $^ -instr-profile=/r/default.profdata
#	/usr/local/opt/llvm/bin/llvm-cov show $^ -instr-profile=/r/default.profdata

# $(DIST)/profile-rjson-load: cc/test-rjson-load.cc cc/rjson.cc cc/rjson-parser-pop.cc | $(DIST)
#	@printf "%-16s %s\n" "PROFILE" $@
#	@$(CXX) -fprofile-instr-generate -fcoverage-mapping $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(ACMACS_BASE_LDLIBS)

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
