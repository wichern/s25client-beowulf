BUILD_DEBUG   := build_debug
BUILD_RELEASE := build_release

TESTCASE := bin/ai-battle -m "<RTTR_RTTR>/MAPS/NEW/GreenPlains.SWD" \
      --ai aijh --ai aijh --random_init 12 --maxGF 100000

RTTR_DATA_URL := https://www.siedler25.org/uploads/nightly/s25rttr_20251111-d3618af1fd834fa9590e5097971418a38038563c-linux.x86_64.tar.bz2
SHELL := /bin/bash

all: debug_test

s25data.tar.bz2:
	wget $(RTTR_DATA_URL) -O s25data.tar.bz2

$(BUILD_RELEASE)/share/s25rttr/RTTR: s25data.tar.bz2
	mkdir -p $(BUILD_RELEASE)/share/s25rttr
	tar -xjf s25data.tar.bz2 -C $(BUILD_RELEASE)/share/s25rttr
	mv $(BUILD_RELEASE)/share/s25rttr/s25rttr_*/share/s25rttr/RTTR $(BUILD_RELEASE)/share/s25rttr/RTTR
	touch $@

$(BUILD_DEBUG)/share/s25rttr/RTTR: s25data.tar.bz2
	mkdir -p $(BUILD_DEBUG)/share/s25rttr
	tar -xjf s25data.tar.bz2 -C $(BUILD_DEBUG)/share/s25rttr
	mv $(BUILD_DEBUG)/share/s25rttr/s25rttr_*/share/s25rttr/RTTR $(BUILD_DEBUG)/share/s25rttr/RTTR
	touch $@

.PHONY: debug_ai_battle
debug_ai_battle:
	mkdir -p $(BUILD_DEBUG)
	cmake -S . -B $(BUILD_DEBUG) -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(BUILD_DEBUG) -j$(shell nproc) ai-battle

.PHONY: debug_Test_integration
debug_Test_integration:
	mkdir -p $(BUILD_DEBUG)
	cmake -S . -B $(BUILD_DEBUG) -DCMAKE_BUILD_TYPE=Debug
	$(MAKE) -C $(BUILD_DEBUG) -j$(shell nproc) Test_integration

. PHONY: release_ai_battle
release_ai_battle:
	mkdir -p $(BUILD_RELEASE)
	cmake -S . -B $(BUILD_RELEASE) -DCMAKE_BUILD_TYPE=RelWithDebInfo
	$(MAKE) -C $(BUILD_RELEASE) -j$(shell nproc) ai-battle 

.PHONY: debug_run
debug_run: debug_ai_battle $(BUILD_DEBUG)/share/s25rttr/RTTR
	cd $(BUILD_DEBUG) && $(TESTCASE)

.PHONY: debug_test
debug_test: debug_Test_integration $(BUILD_DEBUG)/share/s25rttr/RTTR
	cd $(BUILD_DEBUG) && ./bin/Test_integration --run_test=PathfindingWaresSuite

.PHONY: time
time: release_ai_battle $(BUILD_RELEASE)/share/s25rttr/RTTR
	cd $(BUILD_RELEASE) && time $(TESTCASE)

.PHONY: perf
perf: release_ai_battle $(BUILD_RELEASE)/share/s25rttr/RTTR
	cd $(BUILD_RELEASE) && perf record --call-graph dwarf $(TESTCASE)

.PHONY: vscode-debug-config
vscode-debug-config:
	@mkdir -p .vscode
	@printf '%s\n' \
	'{' \
	'  "version": "0.2.0",' \
	'  "configurations": [' \
	'    {' \
	'      "name": "Debug ai-battle",' \
	'      "type": "cppdbg",' \
	'      "request": "launch",' \
	'      "program": "$${workspaceFolder}/build_debug/bin/ai-battle",' \
	'      "args": [' \
	'        "-m", "\"<RTTR_RTTR>/MAPS/NEW/GreenPlains.SWD\"",' \
	'        "--ai", "aijh",' \
	'        "--ai", "aijh",' \
	'        "--random_init", "12",' \
	'        "--maxGF", "100000"' \
	'      ],' \
	'      "cwd": "$${fileDirname}",' \
	'      "stopAtEntry": false,' \
	'      "environment": [],' \
	'      "externalConsole": false,' \
	'      "MIMode": "gdb",' \
    '      "setupCommands": [' \
    '          {' \
    '              "description": "Enable pretty-printing for gdb",' \
    '              "text": "-enable-pretty-printing",' \
    '              "ignoreFailures": true' \
    '          },' \
    '          {' \
    '              "description": "Set Disassembly Flavor to Intel",' \
    '              "text": "-gdb-set disassembly-flavor intel",' \
    '              "ignoreFailures": true' \
    '          }' \
    '      ]' \
	'      "miDebuggerPath": "/usr/bin/gdb"' \
	'    },' \
	'    {' \
	'      "name": "Debug Test_integration",' \
	'      "type": "cppdbg",' \
	'      "request": "launch",' \
	'      "program": "$${workspaceFolder}/build_debug/bin/Test_integration",' \
	'      "args": ["--run_test=PathfindingWaresSuite"],' \
	'      "cwd": "$${fileDirname}",' \
	'      "stopAtEntry": false,' \
	'      "environment": [],' \
	'      "externalConsole": false,' \
	'      "MIMode": "gdb",' \
    '      "setupCommands": [' \
    '          {' \
    '              "description": "Enable pretty-printing for gdb",' \
    '              "text": "-enable-pretty-printing",' \
    '              "ignoreFailures": true' \
    '          },' \
    '          {' \
    '              "description": "Set Disassembly Flavor to Intel",' \
    '              "text": "-gdb-set disassembly-flavor intel",' \
    '              "ignoreFailures": true' \
    '          }' \
    '      ]' \
	'      "miDebuggerPath": "/usr/bin/gdb"' \
	'    }' \
	'  ]' \
	'}' \
	> .vscode/launch.json

# Statistics (real)
#
# dstarlite3
# * 18.652s
# * 19.549s
# * 19.601s
#
# U in noBaseBuilding
# * 18.960s
# * 18.363s
# * 18.365
#
# Directly update all vertices in batches
# [ ] update UT to make sure that goal is always a building
# [ ] debug vertex updates
#
#
# Implement Update() or lazy remove in U
