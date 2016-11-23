# -*- Makefile -*-
# Eugene Skepner 2016
# ======================================================================

SRC_DIR = $(abspath $(ACMACSD_ROOT)/sources)

# ----------------------------------------------------------------------

all: check-python install

install: check-acmacsd-root make-dirs install-packages

make-dirs:
	if [ ! -d $(ACMACSD_ROOT) ]; then mkdir $(ACMACSD_ROOT); fi
	for d in bin lib include py data sources; do if [ ! -d $(ACMACSD_ROOT)/$$d ]; then mkdir $(ACMACSD_ROOT)/$$d; fi; done

# ----------------------------------------------------------------------

install-packages: install-acmacs-base install-3rd-party

install-acmacs-base:
	if [ -d $(SRC_DIR)/acmacs-base ]; then (cd $(SRC_DIR)/acmacs-base; git pull); else git clone git@github.com:acorg/acmacs-base.git $(SRC_DIR)/acmacs-base; fi
	ln -sf $(SRC_DIR)/acmacs-base/bin/* $(ACMACSD_ROOT)/bin
	ln -sf $(SRC_DIR)/acmacs-base/py/acmacs_base $(ACMACSD_ROOT)/py

install-3rd-party:
	$(ACMACSD_ROOT)/bin/update-3rd-party

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
