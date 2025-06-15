S25BASE_DIR := $(realpath ${CURDIR}/../..)
PROXY := http://172.17.0.1:3128
DEVCONTAINER_NAME := s25-mlpack-dev
BINARY ?= beowulf-rl
CMAKE_BUILD_TYPE ?= Release
BUILD_DIR = build_$(CMAKE_BUILD_TYPE)

DOCKER_RUN = docker run --rm \
	$(if $(USE_PROXY),-e http_proxy=${PROXY} -e ${PROXY}) \
	--network=host \
	-v ${S25BASE_DIR}:/usr/src/s25 \
	--user $$(id -u):$$(id -g) \
	-w /usr/src/s25 \
	${DEVCONTAINER_NAME}:latest
DOCKER_BUILD = docker build --no-cache \
	$(if $(USE_PROXY), --build-arg http_proxy="${PROXY}" --build-arg https_proxy="${PROXY}") \
	--network=host -t

.PHONY: build devcontainer devcontainer-vscode run clean

build:
	$(DOCKER_RUN) bash -c 'cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) && cmake --build ${BUILD_DIR} --target ${BINARY} -- -j$(nproc)'

build_devcontainer:
	$(DOCKER_BUILD) ${DEVCONTAINER_NAME} .

install_devcontainer_vscode:
	@mkdir ${S25BASE_DIR}/.devcontainer
	@cp devcontainer.json ${S25BASE_DIR}/.devcontainer/
	@echo "Reload vscode in devcontainer now."

run:
	$(DOCKER_RUN) $(BUILD_DIR)/bin/${BINARY}

# Install s25rttr assets
s25rttr_20250614-5cc1ddb3dd2a565c51b9d2e8e4cb2b310e7ebc65-linux.x86_64.tar.bz2:
	@wget https://www.siedler25.org/uploads/nightly/s25rttr_20250614-5cc1ddb3dd2a565c51b9d2e8e4cb2b310e7ebc65-linux.x86_64.tar.bz2

s25rttr_20250614: s25rttr_20250614-5cc1ddb3dd2a565c51b9d2e8e4cb2b310e7ebc65-linux.x86_64.tar.bz2
	@tar -xjf s25rttr_20250614-5cc1ddb3dd2a565c51b9d2e8e4cb2b310e7ebc65-linux.x86_64.tar.bz2

${BUILD_DIR}/share/s25rttr/RTTR: s25rttr_20250614
	@cp -r s25rttr_20250614/share/s25rttr/RTTR ${BUILD_DIR}/share/s25rttr

.PHONY: install_s25rttr
install_s25rttr: ${BUILD_DIR}/share/s25rttr/RTTR

clean:
	rm -rf ${S25BASE_DIR}/build_*
