#!/bin/bash
clang-format -style=file -verbose -i src/*.cpp include/*.hpp test/*.cpp python/arcticpy/src/*.cpp
black python/arcticpy
