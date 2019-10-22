#!/usr/bin/env bash

rsync -ru --delete assets build

if [[ ! -d build/assets/shaders ]]
then
    mkdir -p build/assets/shaders
fi
glslangValidator -V graphics/shaders/shader.vert -o build/assets/shaders/vertex.spv
glslangValidator -V graphics/shaders/shader.frag -o build/assets/shaders/fragment.spv