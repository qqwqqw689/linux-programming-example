#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
    int          fd;
    void         *addr;
    off_t        offset, pa_offset;
    size_t       length;
    ssize_t      s;
    struct stat  sb;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "%s file offset [length]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    // O_RDONLY: Open for reading only.
    if (fd == -1)
        handle_error("open");

    if (fstat(fd, &sb) == -1)           /* To obtain file size */
        handle_error("fstat");

    offset = atoi(argv[2]);
    // int atoi (const char * str);
    // Convert string to integer
    pa_offset = offset & ~(sysconf(_SC_PAGE_SIZE) - 1);
    /* offset for mmap() must be page aligned */
    // long sysconf(int name);
    // Determines the value of a configurable system option.
    // _SC_PAGE_SIZE: Returns the current page size in bytes.

    if (offset >= sb.st_size) {
        fprintf(stderr, "offset is past end of file\n");
        exit(EXIT_FAILURE);
    }

    if (argc == 4) {
        length = atoi(argv[3]);
        if (offset + length > sb.st_size)
            length = sb.st_size - offset;
                /* Can't display bytes past end of file */

    } else {    /* No length arg ==> display to end of file */
        length = sb.st_size - offset;
    }

    addr = mmap(NULL, length + offset - pa_offset, PROT_READ,
                MAP_PRIVATE, fd, pa_offset);
    // NULL: the kernel chooses the (page-aligned),address at which to create the mapping
    // PROT_READ: Pages may be read.
    // MAP_PRIVATE: Create a private copy-on-write mapping.
    if (addr == MAP_FAILED)
        handle_error("mmap");

    s = write(STDOUT_FILENO, addr + offset - pa_offset, length);
    // ssize_t write(int fd, const void buf[.count], size_t count);
    // STDOUT_FILENO:  the file descriptor for standard output.
    // On success, the number of bytes written is returned.
    if (s != length) {
        if (s == -1)
            handle_error("write");

        fprintf(stderr, "partial write");
        exit(EXIT_FAILURE);
    }

    munmap(addr, length + offset - pa_offset);
    // int munmap(void *addr, size_t len); 
    /*
    The munmap() function shall remove any mappings for those 
    entire pages containing any part of the address space of the 
    process starting at addr and continuing for len bytes.
    */
    close(fd);

    exit(EXIT_SUCCESS);
}