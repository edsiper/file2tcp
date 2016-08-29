# File 2 TCP

This program allows you to send a static file to a local/remote TCP server, it aims to provide the following features:

- Allow concurrency of writers through POSIX threads.
- Use zero-copy strategy to write data as fast as it can.

Only supported for Linux.

## Use case

While developing [Fluent Bit](http://fluentbit.io) I found that's very critical to do benchmarking and meassure how the service is behaving. This tool is being created to benchmark the [in_forward](http://fluentbit.io/documentation/0.8/input/forward.html) plugin initially.

## Getting Started

Build the tool with the following commands:

```bash
$ cd build/
$ cmake ../

```
