#!/usr/bin/env bash

rsync -ru --delete assets build

if [[ ! -d build/shaders ]]
then
    mkdir -p build/shaders
fi
glslangValidator -V graphics/shaders/shader.vert -o build/shaders/vertex.spv
glslangValidator -V graphics/shaders/shader.frag -o build/shaders/fragment.spv