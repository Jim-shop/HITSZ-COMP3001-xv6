#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *filename(char *path) {
  char *p;
  for (p = path; *p != '\0'; p++); // goto end
  for (; p >= path && *p != '/'; p--); // goto last slash
  return ++p;
}

void find(char *path, const char* name) {
    int fd;
    char buf[512], *p;
    struct stat st;
    struct dirent de;

    // printf("%s\n", path);
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    case T_FILE:
        if (strcmp(filename(path), name) == 0) {
            printf("%s\n", path);
        }
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0) continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0) {
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0) {
                find(buf, name);
            }
        }
        break;
  }
  close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Command format error.\n");
        exit(0);
    }
    char *path = argv[1];
    char *name = argv[2];
    find(path, name);
    exit(0);
}
