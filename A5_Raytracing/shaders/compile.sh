#!/bin/sh

glslc --target-env=vulkan1.3 --target-spv=spv1.6 raygen.rgen -o raygen.spv
glslc --target-env=vulkan1.3 --target-spv=spv1.6 miss.rmiss -o raymiss.spv
glslc --target-env=vulkan1.3 --target-spv=spv1.6 closesthit.rchit -o closesthit.spv
glslc --target-env=vulkan1.3 --target-spv=spv1.6 lightmiss.rmiss -o lightmiss.spv

