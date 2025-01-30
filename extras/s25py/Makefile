# This build script is for development. It is not supposed to be integrated in the overall build.

BUILD_DIR = ../../build
VENV_DIR = ../../.venv
PYVER := $(shell python3 -c "import sys; version=sys.version_info; print(f'python{version.major}.{version.minor}')")
CPU_COUNT := $(shell nproc)
S25PY_FILENAME := s25py$(shell python3-config --extension-suffix)

$(BUILD_DIR)/Makefile:
	( \
		mkdir -p $(BUILD_DIR) && \
		cd $(BUILD_DIR) && \
		cmake .. -DCMAKE_BUILD_TYPE=Debug \
	)

# Install s25py in venv
$(VENV_DIR)/lib/$(PYVER)/site-packages/$(S25PY_FILENAME): $(BUILD_DIR)/lib/$(S25PY_FILENAME) build
	cp $(BUILD_DIR)/lib/$(S25PY_FILENAME) $(VENV_DIR)/lib/$(PYVER)/site-packages/$(S25PY_FILENAME)

# Since we have not set up all dependency correctly, this has to be phony
.PHONY: $(BUILD_DIR)/lib/$(S25PY_FILENAME)
$(BUILD_DIR)/lib/$(S25PY_FILENAME): $(BUILD_DIR)/Makefile
	( \
		cd $(BUILD_DIR) && \
		make -j$(CPU_COUNT) s25py \
	)

.PHONY: build
build: $(BUILD_DIR)/lib/$(S25PY_FILENAME)

.PHONY: run
run: $(VENV_DIR)/lib/$(PYVER)/site-packages/$(S25PY_FILENAME)
	( \
		. $(VENV_DIR)/bin/activate && \
		RTTR_PREFIX_DIR=$(shell pwd)/../.. ./my_game.py \
	)

.PHONY: debug
debug: $(VENV_DIR)/lib/$(PYVER)/site-packages/$(S25PY_FILENAME)
	( \
		. $(VENV_DIR)/bin/activate && \
		RTTR_PREFIX_DIR=$(shell pwd)/../.. gdb --args python ./my_game.py \
	)
