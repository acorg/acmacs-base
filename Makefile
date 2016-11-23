# -*- Makefile -*-
# Eugene Skepner 2016
# ======================================================================

SRC_DIR = $(ACMACSD_ROOT)/sources

# ----------------------------------------------------------------------

all: check-python install

install: check-acmacsd-root make-dirs install-packages

make-dirs:
	if [ ! -d $(ACMACSD_ROOT) ]; then mkdir $(ACMACSD_ROOT); fi
	for d in bin lib include py data sources; do if [ ! -d $(ACMACSD_ROOT)/$$d ]; then mkdir $(ACMACSD_ROOT)/$$d; fi; done

# ----------------------------------------------------------------------

install-packages: install-acmacs-base install-pybind11 install-rapidjson

install-acmacs-base:
	if [ -d $(SRC_DIR)/acmacs-base ]; then (cd $(SRC_DIR)/acmacs-base; git pull); else git clone git@github.com:acorg/acmacs-base.git $(SRC_DIR)/acmacs-base; fi

install-pybind11:
	if [ -d $(SRC_DIR)/pybind11 ]; then (cd $(SRC_DIR)/pybind11; git pull); else git clone https://github.com/pybind/pybind11.git $(SRC_DIR)/pybind11; fi
	rsync -av $(SRC_DIR)/pybind11/include/ $(ACMACSD_ROOT)/include

install-rapidjson:
	if [ -d $(SRC_DIR)/rapidjson ]; then (cd $(SRC_DIR)/rapidjson; git pull); else git clone https://github.com/miloyip/rapidjson/ $(SRC_DIR)/rapidjson; fi
	rsync -av $(SRC_DIR)/rapidjson/include/ $(ACMACSD_ROOT)/include

# ----------------------------------------------------------------------

check-acmacsd-root:
ifndef ACMACSD_ROOT
	$(error ACMACSD_ROOT is not set)
endif

check-python:
	@printf 'import sys\nif sys.version_info < (3, 5):\n print("Python 3.5 is required")\n exit(1)' | python3

.PHONY: check-acmacsd-root check-python

# ======================================================================
### Local Variables:
### eval: (if (fboundp 'eu-rename-buffer) (eu-rename-buffer))
### End:
