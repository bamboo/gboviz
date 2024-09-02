# Efficient visualization for Gradle build operations

This is an experiment on using [simdjson](https://github.com/simdjson/simdjson?tab=readme-ov-file#simdjson--parsing-gigabytes-of-json-per-second) to efficiently process large [Gradle](https://github.com/gradle/gradle) build operation traces (gigabytes of json).

This prototype produces a data file that can be used with many of the [D3 based visualizations provided by Observable, Inc](https://observablehq.com/@d3/zoomable-sunburst?intent=fork). This is still far from ideal from a performance perspective as the visualization scripts are not so efficient and there's a lot of visualization data to process. Different strategies will be explored in a future opportunity.

## Building

    mkdir build && cd build
    cmake ..
    cmake --build . -j4

## Using

### Produce a build operation trace from a Gradle build

In a Gradle build directory, run Gradle with the `-Dorg.gradle.internal.operations.trace` option, for example:

    ./gradlew -Dorg.gradle.internal.operations.trace=`pwd`/operations -Dorg.gradle.internal.operations.trace.tree=false --configuration-cache --dry-run assemble

The command above will cause Gradle to capture a trace of all build operations in the `operations-log.txt` file.

### Process the build operation trace

    build/gboviz <path/to/operations-log.txt>

The command above will (very quickly) produce a `data.js` file that can be used with one of the provided [visualization templates](./templates).
