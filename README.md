# Serdec

> [!WARNING]
> This is not a ready to use library. Just a proof of concept.

This is a proof of concept of an idea to automatically generate Serializer/Deserializer (similar to [serde](https://serde.rs/)) in C based on on a struct definition.

The structure is defined in [./src/person.h](./src/person.h) then processed by [./src/generator.c](./src/generator.c) to produce Serializers and Deserializers that are linked with [./src/parser.c](./src/parser.c). The whole process is tied together by [./nob.c](./nob.c) build program.

## Quick Start

```console
$ cc -o nob nob.c
$ ./nob
$ ./build/parser ./person.json
```

The automatically generated Serializer/Deserializer can be found in `./build/serdec_person.h`.

## Screencast

It was developed on a livestream. Watch it here:

[![screencast](./screencast.png)](https://www.youtube.com/watch?v=hnM6aSpWJ8c)
