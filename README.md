# Steganography Using LSB Technique

## Overview

This project hides a secret text file inside a BMP image using the Least Significant Bit (LSB) technique and later retrieves the hidden data through decoding.

## Files

- main.c
- encode.c
- encode.h
- decode.c
- decode.h
- common.h
- types.h

## Compilation

```bash
gcc *.c -o a.out
```

## Encoding

```bash
./a.out -e beautiful.bmp secret.txt stego.bmp
```

## Decoding

```bash
./a.out -d stego.bmp output.txt
```

## Encoding Process

1. Open source image and secret file
2. Check image capacity
3. Copy BMP header
4. Encode magic string
5. Encode extension size
6. Encode extension
7. Encode secret file size
8. Encode secret file data
9. Copy remaining image data

## Decoding Process

1. Open stego image
2. Skip BMP header
3. Decode magic string
4. Decode extension size
5. Decode extension
6. Decode secret file size
7. Decode secret file data
8. Write data to output file

## Sample

Input:

```text
My password is SECRET ;)
```

Output:

```text
My password is SECRET ;)
```

## Author

Omkar Bhagat
