#ifndef COMMANDS_H
#define COMMANDS_H

void my_ls(const char *);
void my_pwd();
void my_cd(const char *);
void my_cat(const char *);
void my_mkdir(const char *);
void my_rmdir(const char *);
void my_touch(const char *);
void my_rm(const char *);
void my_cp(const char *, const char *);
void my_mv(const char *, const char *);
void help();

/* GUI Integration Functions */
void execute_command(const char *input, char *output);
void get_current_directory(char *buffer, int size);

#endif
