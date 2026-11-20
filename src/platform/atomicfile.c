#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#define platform_pid() ((long)_getpid())
#else
#include <dirent.h>
#include <unistd.h>
#define platform_pid() ((long)getpid())
#endif

#include "atomicfile.h"

int file_write_atomic(const char *path, const void *data, size_t len)
{
    char tmp[512];
    FILE *f;
    size_t written;
    int rc;

    if (path == NULL || data == NULL) return 0;
    if (strlen(path) + 24 >= sizeof(tmp)) return 0;
    snprintf(tmp, sizeof(tmp), "%s.tmp.%ld", path, platform_pid());

    f = fopen(tmp, "wb");
    if (f == NULL) return 0;
    written = fwrite(data, 1, len, f);
    if (written != len || fflush(f) != 0) {
        fclose(f);
        remove(tmp);
        return 0;
    }
    fclose(f);

    rc = rename(tmp, path);
    if (rc != 0) {
        remove(path);
        rc = rename(tmp, path);
    }
    if (rc != 0) {
        remove(tmp);
        return 0;
    }
    return 1;
}

void file_cleanup_tmp(const char *dir)
{
    if (dir == NULL) return;
#ifdef _WIN32
    {
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(".\\*.tmp.*", &fd);
        char name[1024];
        if (h == INVALID_HANDLE_VALUE) return;
        do {
            snprintf(name, sizeof(name), "%s\\%s", dir, fd.cFileName);
            remove(name);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#else
    {
        DIR *d = opendir(dir);
        struct dirent *e;
        char name[512];
        const char *p;
        if (d == NULL) return;
        while ((e = readdir(d)) != NULL) {
            for (p = e->d_name; *p != '\0'; ++p) {
                if (strncmp(p, ".tmp.", 5) == 0) {
                    snprintf(name, sizeof(name), "%s/%s", dir, e->d_name);
                    remove(name);
                    break;
                }
            }
        }
        closedir(d);
    }
#endif
}
