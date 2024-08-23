

## build mupdf 

mupdf version = 1.24.3

```sh
cd third_party/mupdf
../venv/bin/pip install libclang setuptools
../venv/bin/python scripts/mupdfwrap.py -b m01
```

https://mupdf.readthedocs.io/en/latest/language-bindings.html#building-the-c-bindings
