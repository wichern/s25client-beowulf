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

.PHONY: build devcontainer run clean

build:
	$(DOCKER_RUN) bash -c 'cmake -B ${BUILD_DIR} -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) && cmake --build ${BUILD_DIR} --target ${BINARY} -- -j$(nproc)'

devcontainer:
	$(DOCKER_BUILD) ${DEVCONTAINER_NAME} .

run:
	$(DOCKER_RUN) $(BUILD_DIR)/bin/${BINARY}

clean:
	rm -rf ${S25BASE_DIR}/build_*
