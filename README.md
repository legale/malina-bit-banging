# malina-bit-banging
Raspberry (malina) 3b gpio bit-banging programmer

## Download
```
git clone https://github.com/legale/malina-bit-banging.git 
cd malina-bit-banging
git submodule init
git submodule update
```

## Compile
```gcc yarpio/yarpio.c direct5.c -o bitbanging```

## Use example
### help
```
bitbanging -h
```
### read page
```
bitbanging read_page 1000
````

### write file to page
```
bitbanging write_page file.txt 1000 64
```
This will write first 64 bytes from file.txt to page 1000


