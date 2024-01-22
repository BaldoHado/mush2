#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <mush.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

void handler(int num) {
    errno = EINTR;
    printf("\n");
}

int arrangePipe(int length, struct clstage stages[]) {
    int cur_pipe[2] = {0, 0};
    int prev_pipe[2] = {0, 0};
    int idx = 0;
    int status;
    sigset_t set;
    int retVal = 1;
    if (-1 == sigemptyset(&set)) {
        perror("sigemptyset");
        exit(EXIT_FAILURE);
    }
    if (-1 == sigaddset(&set, SIGINT)) {
        perror("sigaddset");
        exit(EXIT_FAILURE);
    }
    if (-1 == sigprocmask(SIG_BLOCK, &set, NULL)) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    for (idx = 0; idx < length; idx++) {
        pid_t child;
        int inFd;
        int outFd;
        if (strstr(stages[idx].argv[0], "cd") != NULL) {
            if (stages[idx].argc != 2) {
                printf("usage: cd [ destdir ]\n");
            } else {
                chdir(stages[idx].argv[1]);
            }
            return 1;
        }
        if (idx < length - 1) {
            if (-1 == pipe(cur_pipe)) {                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        if (!(child = fork())) {
            if (-1 == child) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            
            if (stages[idx].inname) {
                if (-1 == (inFd = open(stages[idx].inname, O_RDONLY))) {
                    if (errno != EINTR) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                if (-1 == dup2(inFd, STDIN_FILENO)) {
                    if (errno != EINTR) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                close(inFd);
            }
            if (stages[idx].outname) {
                outFd = open(stages[idx].outname,O_RDWR|O_CREAT|O_TRUNC, 0666);
                if (-1 == outFd) {
                    if (errno != EINTR) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                }
                if (-1 == dup2(outFd, STDOUT_FILENO)) {
                    if (errno != EINTR) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                }
                close(outFd);
            }
            if (length != 1) {
                if (idx == 0) {
                    if (-1 == dup2(cur_pipe[1], STDOUT_FILENO)) {
                        if (errno != EINTR) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    close(cur_pipe[0]);
                    close(cur_pipe[1]);    
                } else if (idx == length - 1){
                    if (-1 == dup2(prev_pipe[0], STDIN_FILENO)) {
                        if (errno != EINTR) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    close(prev_pipe[0]);
                    close(prev_pipe[1]);
                } else {
                    if (-1 == dup2(cur_pipe[1], STDOUT_FILENO)) {
                        if (errno != EINTR) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    if (-1 == dup2(prev_pipe[0], STDIN_FILENO)) {
                        if (errno != EINTR) {
                            perror("dup2");
                            exit(EXIT_FAILURE);
                        }
                    }
                    close(prev_pipe[0]);
                    close(prev_pipe[1]);
                    close(cur_pipe[0]);
                    close(cur_pipe[1]);
                }       
            }
            if (-1 == sigprocmask(SIG_UNBLOCK, &set, NULL)) {
                perror("sigprocmask");
                exit(EXIT_FAILURE);
            }
            execvp(stages[idx].argv[0], stages[idx].argv);
            perror(stages[idx].argv[0]);
            _exit(EXIT_FAILURE);
        } else {
            if (idx > 0) {
                close(prev_pipe[0]);
                close(prev_pipe[1]);
            }
        }
        prev_pipe[0] = cur_pipe[0];
        prev_pipe[1] = cur_pipe[1];
    } 
    if (-1 == sigprocmask(SIG_UNBLOCK, &set, NULL)) {
        perror("sigprocmask");
        exit(EXIT_FAILURE);
    }
    for (idx = 0; idx < length; idx++) {
        if (-1 == wait(&status)) {
            if (errno != EINTR) {
                perror("wait");
            } 
        }
        if (WIFEXITED(status)){
            if (WEXITSTATUS(status)) {
                // Child exited with err
                retVal = 0;
            }  
        } else {
            // Child err
            retVal = 0;
        }
    }


    return retVal;
}


int main(int argc, char *argv[]) {
    FILE *in = stdin;

    struct sigaction sa; 
    sa.sa_handler = handler;
    if (-1 == sigemptyset(&sa.sa_mask)) {
        perror("sigemptyset");
        return 1;
    }
    sa.sa_flags = 0;
    if (-1 == sigaction(SIGINT, &sa, NULL)) {
        perror("sigaction");
        return 1;
    }
    while (1) {
        char *line;
        pipeline pline;
        if (argc == 2) {
            in = fopen(argv[1], "r");
        } else {
            in = stdin;
        }
        if (isatty(fileno(in))) {
            printf("8-P ");
        }
        errno = 0;
        if (NULL == (line = readLongString(in))) {
            if (errno != EINTR) {
                if (feof(in)) {
                 exit(0);
                }
                perror("readlongstring");
                clearerr(stdin);
                clearerr(stdout);
            }
            continue; 
        }
        if (NULL == (pline = crack_pipeline(line))) {
            if (errno != EINTR) {
                perror("crack pipeline");
                clearerr(stdin);
                clearerr(stdout);      
            }
            continue;
   
        }
        if (line && pline) {

            arrangePipe(pline->length, pline->stage);        
            free_pipeline(pline);
            free(line);
            clearerr(stdin);
            clearerr(stdout);
        }
   }
    return 0;

}