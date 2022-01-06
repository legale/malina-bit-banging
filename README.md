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
```make```

## Use example
### help
```
build/bitbanging -h
```
### read page
```
build/bitbanging read_page 1000
````

### write file to page
```
build/bitbanging write_file file.txt 1000 64
```
This will write first 64 bytes from file.txt to NAND memory chip from page 1000
```

