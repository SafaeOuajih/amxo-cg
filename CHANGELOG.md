# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]


## Release v1.1.6 - 2021-07-06(12:49:49 +0000)

### Fixes

- Not passing any input files results in exit code 0

## Release v1.1.5 - 2021-06-17(07:43:20 +0000)

### Fixes

- Generation of version.h files should not be .PHONY

### Other

- Issue: ambiorix/applications/amxo-cg#14 Parameters defined using `counted with` are not present in XML output

## Release v1.1.4 - 2021-06-10(14:34:55 +0000)

- Issue: ambiorix/applications/amxo-cg#13 Missing reference to libxml2-dev in README
- Issue: ambiorix/applications/amxo-cg#14 Parameters defined using `counted with` are not present in XML output

## Release v1.1.3 - 2021-06-08(17:48:12 +0000)

## Release v1.1.2 - 2021-06-08(11:29:50 +0000)

### Fixes

- [tr181 plugins][makefile] Dangerous clean target for all tr181 components

## Release v1.1.1 - 2021-05-21(12:41:17 +0000)

### Fixes

- Include tree is not build correctly

### Changes

- add command line reset option
- Update README

## Release v1.1.0 - 2021-05-09(20:58:00 +0000)

### New

- Generate xml/html documentation from odl files

### Fixes

- Fixes test makefile
- amxo-cg does not exit with status code different from 0 when odl contains errors
-  It must be possible to add saved files

### Other

- Enable auto opensourcing

## Release v1.0.4 - 2021-04-21(10:35:14 +0000)

### Fixes

- Disable function resolving

## Release 1.0.3 - 2021-04-16(11:14:46 +0000)

## Release 1.0.2 - 2021-04-15(20:18:29 +0000)

### Changes

-  remove fakeroot dependency on host to build WRT image 

## Release 1.0.1 - 2021-04-15(11:08:39 +0000)

### Fixes

- VERSION_PREFIX is needed in buildfiles 

## Release 1.0.0 - 2021-04-08(22:08:56 +0000)

### Changes

- Move copybara to baf

## Release 0.2.5 - 2021-02-26(18:06:18 +0000)

### Changes

- Migrate to new licenses format (baf)

## Release 0.2.4 - 2021-01-19(08:26:39 +0000)

### New

- Auto generate make files using build agnostic file (baf)

## Release 0.2.3 - 2021-01-05(09:46:08 +0000)

### New 

- Add contributing.md

## Release 0.2.2 - 2020-11-29(18:54:14 +0000)

### Changes

- Update readme

### Fixes

- Fix debian package dependencies

## Release 0.2.1 - 2020-11-16(14:29:52 +0000)

### Changes

- Update gitlab CI/CD yml file

## Release 0.2.0 - 2020-10-02(17:14:55 +0000)

### Changes

- Update code style

## Release 0.1.2 - 2020-09-22(06:11:31 +0000)

### Fixes

- fix clean target

## Release v0.1.1 - 2020-09-04(16:52:04 +0000)

### Fixes

- Fixes g++ warnings and errors

## Release 0.1.0 - 2020-08-30(09:19:44 +0000)

### Changes

- Apply API changes of libamxd
- Needs libamxd version 1.0.0 or higher
- Common macros moved to libamxc

## Release 0.0.5 - 2020-07-29(10:40:36 +0000)

### Changes

- Adds RAW_VERSION to makefile.inc, VERSION must be X.Y.Z or X.Y.Z-HASH

### Fixes

- Compilation issue with frotified musl

## Release 0.0.4 - 2020-07-06(08:46:01 +0000)

### Changes

- Uses std=c11 instead of std=c18, to support older toolchains and compilers

## Release 0.0.3 - 2020-06-30(08:33:39 +0000)

### New

- Add copybara file

### Changes

- update license to BSD+patent

## Release 0.0.2 - 2020-06-08(09:14:50 +0000)

### Fixes

- Install target, removes strip from install (yocto)

## Release 0.0.1 - 2020-05-24(17:14:43 +0000)

### New

- Verbose parse logging
- Data model methods C function template generator


## Release 0.0.0 - 2020-05-21(10:14:30 +0000)

### Changes

- Initial version
