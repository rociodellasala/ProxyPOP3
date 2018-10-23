#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>

/*
 * Usage: ./a.out "echo hola"
 */

char **
create_arguments(int argc, char * argv[]){
  char ** args = malloc((argc+2)*sizeof(char*));
  args[0] = "bash";
  args[1] = "-c";
  int i;
  for (i = 1; i < argc; i++){
    args[i+1] = argv[i];
  }
  args[i+1] = NULL;
  // for (int i = 0; i < argc; i++){
  //   printf("%s\n", args[i]);
  // }
  return args;
}

int
main2(int argc, char * argv[]){

  char ** args = create_arguments(argc, argv);
  int fd[2], nbytes;
  char readbuffer[80];
  pipe(fd);

  int pid = fork();

  if (pid == -1){
    perror("fork error");
  }else if(pid == 0){
    dup2(fd[1], 1);
    int value = execve("/bin/bash", args, NULL);
    perror("execve");
    if (value == -1){
      printf("Soy un error\n");
    }
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    readbuffer[nbytes-1] = '\0';
    printf("%s",readbuffer);
  } else {
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    readbuffer[nbytes-1] = '\0';
    printf("Received string: %s\n", readbuffer);
    printf("Received bytes: %d\n", nbytes);
    nbytes = write(fd[1], "Hola", sizeof("Hola"));
    nbytes = read(fd[0], readbuffer, sizeof(readbuffer));
    readbuffer[nbytes-1] = '\0';
    printf("Received string: %s\n", readbuffer);
    printf("Received bytes: %d\n", nbytes);
  }

}
