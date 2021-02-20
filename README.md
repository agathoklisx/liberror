## Description.
This tool generates on the fly, a small C library that implements informative error
functions, without any dependency.

It does that by parsing <errno.h>, by using the preprocessor and sed.

# Application Interface.

It exposes the following functions.

```C
char *errno_string (int errnum);
```
This is a strerror() replacement. It gets an `errno` as an argument and returns a
null terminated string that shouldn't be never freed.
If `errnum` is out of range, less than zero or greater than the last defined `errno`,
then it returns "Unknown error".

```C
char *errno_string_s (int, char *, size_t);
```
Likewise with errno_string(), but this gets a char pointer `buf` argument.
In that case, no more than `buflen` bytes are written in the string, including the
terminating null byte.

```C
char *errno_name (int errnum);
```
This function returns a null terminated string, with the literal `errnun` name,
e.g., "EPERM".
If `errnum` is out of range, less than zero or greater than the last defined `errno`,
it returns "EUNKNOWN". This string should be never freed.
```C
char *errno_name_s (int errnum, char *buf, size_t buflen);
```
Likewise with errno_name(), but this gets a char pointer `buf` argument.
In that case, no more than `buflen` bytes are written in the string, including the
terminating null byte.

```C
int error_exists (int errnum);
```
This function returns 0 if `errnum` is out of range, or 1 otherwise.

*Note*: In case of Linux the errors 41 and 58 are not defined, so they are also
considered unknown errors.

## Usage.
```sh
  cc -o generate_lib generate_lib.c
  ./generate_lib
```
Now you can even copy "liberror.c" and "error.h" into a project, or compile it
as a library.

## Test
```sh
  cc -o test -DTEST liberror.c
```

## Platforms.

Developed on a Linux machine, but it should be easy to port to others. See a
similar project that does that.

 * https://github.com/mentalisttraceur/errnoname

If you do that, drop me an email so I can enhance the generator.

## LICENSE
This is in the public domain. You can do anything you want with it. You have no
legal obligation to do anything else other than to let your conscience to guide
you to do the right thing, by an own inner decision and not because you have to
do because of a law.

I believe in the freedom of the code, and I see it as a precious gift. I do not
believe that any kind of copyright is a wise choise for our future. There is an
unbelievable strong intuition, streaming from an unbelievable strong certainity,
that if we really really really want to serve ourselves (which is quite logical,
as ourself is the most beloved being and probably the one who care most),  then
the only thing that we just have to do, is to serve the others! and our shared
and common space and time, involved in an eternal dance, destined to travel at
the same destination and in an endless evolution.
