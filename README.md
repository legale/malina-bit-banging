# malina-bit-banging
Raspberry (malina) 3b gpio bit-banging programmer

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


