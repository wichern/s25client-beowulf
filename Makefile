BUILD_DEBUG   := build_debug
BUILD_RELEASE := build_release

TESTCASE := bin/ai-battle -m "<RTTR_RTTR>/MAPS/NEW/GreenPlains.SWD" \
      --ai aijh --ai aijh --random_init 12 --maxGF 100000

RTTR_DATA_URL := https://www.siedler25.org/uploads/nightly/s25rttr_20251111-d3618af1fd834fa9590e5097971418a38038563c-linux.x86_64.tar.bz2

all: debug

s25data.tar.bz2:
	wget $(RTTR_DATA_URL) -O s25data.tar.bz2

$(BUILD_RELEASE)/share/s25rttr/RTTR/s25client.exe: s25data.tar.bz2
	$(TAR) -xjf s25data.tar.bz2 -C $(BUILD_RELEASE)/share/s25rttr

$(BUILD_DEBUG)/share/s25rttr/RTTR/s25client.exe: s25data.tar.bz2
	$(TAR) -xjf s25data.tar.bz2 -C $(BUILD_DEBUG)/share/s25rttr

debug: $(BUILD_DEBUG)/share/s25rttr/RTTR/s25client.exe
	mkdir -p $(BUILD_DEBUG)
	cmake -S . -B $(BUILD_DEBUG) -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(BUILD_DEBUG) -j$(shell nproc) ai-battle Test_integration

.PHONY: release
release: $(BUILD_RELEASE)/share/s25rttr/RTTR/s25client.exe
	mkdir -p $(BUILD_RELEASE)
	cmake -S . -B $(BUILD_RELEASE) -DCMAKE_BUILD_TYPE=RelWithDebInfo
	$(MAKE) -C $(BUILD_RELEASE) -j$(shell nproc) ai-battle Test_integration

.PHONY: debug_run
debug_run: debug
	cd $(BUILD_DEBUG) && $(TESTCASE)

.PHONY: debug_test
debug_test: debug
	cd $(BUILD_DEBUG) && ./bin/Test_integration --run_test=PathfindingWaresSuite

.PHONY: time
time: release $(BUILD_RELEASE)/share/s25rttr/RTTR
	$(TIME) $(BUILD_RELEASE)/$(TESTCASE)

.PHONY: perf
perf: release
	$(PERF) record --call-graph dwarf $(BUILD_RELEASE)/$(TESTCASE)

.PHONY: vscode-debug-config
vscode-debug-config:
	mkdir -p .vscode
	printf '%s\n' \
	'{' \
	'  "version": "0.2.0",' \
	'  "configurations": [' \
	'    {' \
	'      "name": "Debug ai-battle",' \
	'      "type": "cppdbg",' \
	'      "request": "launch",' \
	'      "program": "$(pwd)/build-debug/bin/ai-battle",' \
	'      "args": [' \
	'        "-m", "$(RTTR_RTTR)/MAPS/NEW/GreenPlains.SWD",' \
	'        "--ai", "aijh",' \
	'        "--ai", "aijh",' \
	'        "--random_init", "12",' \
	'        "--maxGF", "100000"' \
	'      ],' \
	'      "cwd": "$(pwd)",' \
	'      "stopAtEntry": false,' \
	'      "environment": [],' \
	'      "externalConsole": false,' \
	'      "MIMode": "gdb",' \
	'      "miDebuggerPath": "/usr/bin/gdb"' \
	'    }' \
	'  ]' \
	'}' \
	> .vscode/launch.json
