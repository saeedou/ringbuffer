# cring


## Build
```bash
mkdir build
cd build
cmake ..
make all
```

## Test
Install https://github.com/pylover/cutest.

```bash
cd build
make test
```

## Install

```bash
cd build
make install
```

### Lint
```bash
pip install prettyc
cd build
make lint
```

### Debian package
```bash
cd build
cpack
sudo dpkg -i libcring-*.deb
```
