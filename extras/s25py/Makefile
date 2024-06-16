# This build script is for development. It is not supposed to be integrated in the overall build.

BUILD_DIR = ../../build
PYVER := $(shell python3 -c "import sys; version=sys.version_info; print(f'python{version.major}.{version.minor}')")
CPU_COUNT := $(shell nproc)

$(BUILD_DIR)/Makefile:
	( \
		mkdir -p $(BUILD_DIR) && \
		cd $(BUILD_DIR) && \
		cmake .. -DCMAKE_BUILD_TYPE=Debug \
	)


.PHONY: build
build: $(BUILD_DIR)/Makefile
	( \
		cd $(BUILD_DIR) && \
		make -j$(CPU_COUNT) s25py && \
		. ../.venv/bin/activate && \
		cp lib/s25py.cpython-*.so $$VIRTUAL_ENV/lib/$(PYVER)/site-packages \
	)

.PHONY: run
run:
	( \
		. ../../.venv/bin/activate && \
		RTTR_PREFIX_DIR=$(pwd)/../.. ./battle.py \
	)

.PHONY: debug
debug:
	( \
		. ../../.venv/bin/activate && \
		RTTR_PREFIX_DIR=$(pwd)/../.. gdb --args python ./battle.py \
	)