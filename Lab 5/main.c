#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int printRecursiveStats(char name[]) {
	struct stat    fileStat;
	struct group * grp;
	struct passwd *pwd;

	stat(name, &fileStat);

	pwd = getpwuid(fileStat.st_uid);
	grp = getgrgid(fileStat.st_gid);

	printf("%c%c%c%c%c%c%c%c%c%c Links: %lu (%ldB, %lubl) inode: %lu, %c, %s:%s \"%s\" %s",
	       S_ISDIR(fileStat.st_mode) ? 'd' : '-',
	       fileStat.st_mode & S_IRUSR ? 'r' : '-',
	       fileStat.st_mode & S_IWUSR ? 'w' : '-',
	       fileStat.st_mode & S_IXUSR ? 'x' : '-',
	       fileStat.st_mode & S_IRGRP ? 'r' : '-',
	       fileStat.st_mode & S_IWGRP ? 'w' : '-',
	       fileStat.st_mode & S_IXGRP ? 'x' : '-',
	       fileStat.st_mode & S_IRUSR ? 'r' : '-',
	       fileStat.st_mode & S_IWOTH ? 'w' : '-',
	       fileStat.st_mode & S_IXOTH ? 'x' : '-',
	       fileStat.st_nlink,
	       fileStat.st_size, fileStat.st_blocks,
	       fileStat.st_ino,
	       S_ISLNK(fileStat.st_mode) ? 'S' : '-',
	       pwd != NULL ? pwd->pw_name : "NULL", grp != NULL ? grp->gr_name : "NULL",
	       name,
	       ctime(&fileStat.st_mtime));

	if (S_ISDIR(fileStat.st_mode)) {
		DIR *          pDir;
		struct dirent *pDirent;

		printf("Entering directory %s\n", name);

		pDir = opendir(name);
		if (pDir == NULL) {
			printf("Cannot open directory %s\n", name);
			return 1;
		}

		char path[256];

		while ((pDirent = readdir(pDir)) != NULL) {
			strcpy(path, name);
			if (strcmp(pDirent->d_name, ".") != 0 && strcmp(pDirent->d_name, "..") != 0) {
				strcat(path, "/");
				strcat(path, pDirent->d_name);
				printRecursiveStats(path);
			}
		}
		printf("Exiting directory %s\n", name);
		closedir(pDir);
	}

	return 0;
}

int main(int argc, char *argv[]) {
	if (argc == 0) {
		printf("Need either a directory or a file\n");
		return 1;
	}

	return printRecursiveStats(argv[1]);
}
