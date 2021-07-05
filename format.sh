#!/bin/bash
clang-format -style=file -verbose -i src/*.cpp include/*.hpp test/*.cpp ./*.cpp arcticpy/src/interface.[ch]pp
