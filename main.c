#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/wait.h>


#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

int foreground_default_status = 0;
// flag to see if it is background process
int background = 0;
// global var storing bg pid
int bgPID = -1;
//array of ptrs to strings with size 512, used to store each word parsed from input
char *words[MAX_WORDS];
// returns size_t
size_t wordsplit(char const *line);
// returns a char pointer
char * expand(char const *word);


void sig_handler(int sig){
    // empty function
}

void bg_process() {
    if (bgPID != -1) {
        pid_t wait_result;
        int childStatus;

        wait_result = waitpid(bgPID, &childStatus, WNOHANG | WUNTRACED);

        if (wait_result > 0) {
            // child process termination status for bg
            if (WIFEXITED(childStatus)) {
                fprintf(stderr, "Child process %d done. Exit status %d.\n", bgPID, WEXITSTATUS(childStatus));
            } else if (WIFSIGNALED(childStatus)) {
                // child process terminated by signal
                fprintf(stderr, "Child process %d done. Signaled %d.\n", bgPID, WTERMSIG(childStatus));
            } else if (WIFSTOPPED(childStatus)) {
                // child process stopped
                fprintf(stderr, "Child process %d stopped. Continuing.\n", bgPID);
                kill(bgPID, SIGCONT);
            }

            bgPID = -1;
        } else if (wait_result == 0) {
            if (kill(bgPID, 0) != 0) {
                bgPID = -1;
            }
        }
    }
}

// argc - count of args; argv - arr of argument strings
int main(int argc, char *argv[]) {


    // storing original signal actions
    struct sigaction oldactInt = {0};
    struct sigaction oldactStp = {0};
    struct sigaction ignore_action = {0};
    // IGNORE signals
    ignore_action.sa_handler = SIG_IGN;

    struct sigaction SIGINT_handler = {0};
    SIGINT_handler.sa_handler = sig_handler;
    sigfillset(&SIGINT_handler.sa_mask);
    SIGINT_handler.sa_flags = 0;


    bg_process();
    // file input handling
    FILE *input = stdin;
    char *input_fn = "(stdin)";
    // one cmd line arg - filename
    if (argc == 2) {
        input_fn = argv[1];
        // try to open the filename
        input = fopen(input_fn, "re");
        if (!input) err(1, "%s", input_fn);
        // if there are more than two args, throw an error
    } else if (argc > 2) {
        errx(1, "too many arguments");
    }

    // reading input lines
    char *line = NULL;


    // size of the buffer for input lines
    size_t n = 0;
    for (;;) {
        //  sigaction(SIGTSTP, &oldactStp, NULL);
        bg_process();
        // different prompt for user and root
        char *PS1 = getenv("PS1");
        // if user
        if (input == stdin) {
            sigaction(SIGTSTP, &ignore_action, &oldactStp);
            fprintf(stderr, "%s", (PS1 == NULL) ? "" : PS1);

        }

        // catch originL disposition of SIGINT
        sigaction(SIGINT, &SIGINT_handler, &oldactInt);


        // read line from the input (from file or stdin)
        ssize_t line_len = getline(&line, &n, input);

        sigaction(SIGINT, &ignore_action, NULL);


        if (feof(input)) {
            break;
        }  else if (line_len == -1 && errno == EINTR) {
            fprintf(stderr, "\n");
            errno = 0;
            clearerr(input);
            continue;
        }else if (line_len < 0) {
            err(1, "%s", input_fn);
        }

        // call wordsplit() fcn to split the line into words and store the num of
        // words in nwords
        size_t nwords = wordsplit(line);

        // iterate over each word taken from the input line
        for (size_t i = 0; i < nwords; ++i) {
            // check if the curr word is the last in the cmd and if it's '&' bg op
            // not descresing nwords
            // do background selection inside fork

            //}
            // call on expand() to expand special params in the word and store the result
            char *exp_word = expand(words[i]);
            // free the memory allocated for the original word
            free(words[i]);
            // update words[] arr with the expanded word
            words[i] = exp_word;
            // print expanded word to error stream
            //     fprintf(stderr, "Expanded Word %zu: %s\n", i, words[i]);
        }
        //if (nwords < MAX_WORDS) {
        words[nwords] = NULL;
        //  }

        // arguments passed to the shell
        if (nwords > 0) {
            // check if exit was typed
            if (strcmp(words[0], "exit") == 0) {
                // check if there was more than one arg passed w exit
                if (nwords == 1) {
                    return foreground_default_status;
                } else if (nwords > 2) {

                    // if more than two args provided
                    fprintf(stderr, "exit: too many args passed w exit, only one allowed!");
                    exit(EXIT_FAILURE);

                }
                else {
                    // handle the case if argument is not an integer
                    size_t argumentLen = strlen(words[1]);
                    for (size_t i = 0; i < argumentLen; ++i) {
                        if (!isdigit(words[1][i])) {
                            fprintf(stderr, "exit: arg is not an int and it should be!!!");
                            exit(EXIT_FAILURE);
                        }
                    }
                    // got to convert the arg to integer
                    int exit_status = atoi(words[1]);
                    return exit_status;
                }
            }  // end of exit
                // handle the cd command
            else if (strcmp(words[0], "cd") == 0) {
                // if there was no argument provided
                if (nwords == 1) {
                    if (chdir(getenv("HOME")) != 0) {
                        perror("cd");
                        return 2;
                    }
                } else if (nwords == 2) {
                    if (chdir(words[1]) != 0) {
                        fprintf(stderr, "CD error!");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // if more than two args provided
                    fprintf(stderr, "exit: too many args passed w cd, only one allowed!");
                    exit(EXIT_FAILURE);
                }
            } // end of cd
            else {
                // implement the forking

                int childStatus;
                // forking
                char *exec_args[MAX_WORDS];
                size_t args_count = 0;
                pid_t pid = fork();

                switch (pid) {
                    case -1:
                        // error, wrong pid
                        fprintf(stderr, "Fork error!!\n");
                        exit(1);
                        break;

                    case 0:
                        sigaction(SIGINT, &oldactInt, NULL);
                        sigaction(SIGTSTP, &oldactStp, NULL);

                        //redirection
                        for (size_t i = 0; i < nwords; ++i) {
                            // input
                            if (strcmp(words[i], "<") == 0){
                                // check if filename shows after <
                                if (i < nwords - 1) {
                                    const char *filename = words[i + 1];
                                    int file_dp = open(filename, O_RDONLY);
                                    if (file_dp == -1) {
                                        fprintf(stderr, "Issues with open!");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup2(file_dp, STDIN_FILENO) == -1){
                                        fprintf(stderr, "issue with dup2");
                                        close(file_dp);
                                        exit(EXIT_FAILURE);
                                    }
                                    close(file_dp);
                                    // skip next filenames
                                    i += 1;
                                } else {
                                    fprintf(stderr, "No file after '<' provided!\n");
                                    exit(EXIT_FAILURE);
                                }

                                // handle output redirection
                            } else if (strcmp(words[i], ">") == 0)  {
                                if (i < nwords - 1) {
                                    const char *filename = words[i + 1];
                                    int file_dp = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                                    if (file_dp == -1){
                                        fprintf(stderr, "open error");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup2(file_dp, STDOUT_FILENO) == -1) {
                                        fprintf(stderr, "dup2 issue");
                                        close(file_dp);
                                        exit(EXIT_FAILURE);
                                    }
                                    close(file_dp);
                                    i++;
                                } else {
                                    fprintf(stderr, "no file provided after > !!");
                                    exit(EXIT_FAILURE);
                                }
                                // make sure there is filename after >
                                // if not, failure
                                // check for bg op &
                            } else if (strcmp(words[i], ">>") == 0)  {
                                if (i < nwords - 1) {
                                    const char *filename = words[i + 1];
                                    int file_dp = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0777);
                                    if (file_dp == -1){
                                        fprintf(stderr, "open error");
                                        exit(EXIT_FAILURE);
                                    }
                                    if (dup2(file_dp, STDOUT_FILENO) == -1) {
                                        fprintf(stderr, "dup2 issue");
                                        close(file_dp);
                                        exit(EXIT_FAILURE);
                                    }
                                    close(file_dp);
                                    i++;
                                } else {
                                    fprintf(stderr, "no file provided after > !!");
                                    exit(EXIT_FAILURE);
                                }

                            } else if (strcmp(words[i], "&") == 0) {
                                background = 1;
                            } else {
                                exec_args[args_count++] = words[i];
                            }
                        }
                        //null terminate the new arr
                        exec_args[args_count] = NULL;

                        // storing child process ID
                        if (background) {
                            bgPID = pid;
                        }

                        // execute the cmd in child process
                        execvp(exec_args[0], exec_args);
                        // handle the failure of execvp
                        fprintf(stderr, "Execvp error!!\n");
                        exit(EXIT_FAILURE);

                        break;
                    default:

                        if (strcmp(words[nwords -1], "&") == 0) {


                            //  if yes then run cmd in bg
                            background = 1;
                            //  deleting '&' from the cmd

                            free(words[nwords - 1]);
                            words[nwords - 1] = NULL;
                            bgPID = pid;
                            //   fprintf(stdout, "%d\n", bgPID);

                        }

                        // new arr to store cmds for execvp
                        // parent process
                        // if not built-in command:
                        if (!(strcmp(words[0], "cd") == 0 || strcmp(words[0], "exit") == 0)) {

                            if (!background){
                                pid_t wait_result;
                                do {
                                    wait_result = waitpid(pid, &childStatus, WUNTRACED);
                                    if (wait_result == -1) {
                                        fprintf(stderr, "waitpid failed!!!!");
                                        exit(EXIT_FAILURE);
                                    } else if (wait_result > 0) {
                                        // child stopped or exited
                                        if (WIFEXITED(childStatus)) {
                                            // child ended normally
                                            foreground_default_status = WEXITSTATUS(childStatus);
                                        }
                                            // wifsignaled by man pg: returns T if child process was terminated by a signal
                                        else if (WIFSIGNALED(childStatus)) {
                                            // num of the signal that caused child to terminate
                                            int signal_num = WTERMSIG(childStatus);
                                            // calculate the exit status
                                            foreground_default_status = 128 + signal_num;
                                        } else if (WIFSTOPPED(childStatus)) {
                                            int signal_num = WSTOPSIG(childStatus);
                                            fprintf(stderr, "Child process %d stopped. Continuing.\n", (int)pid);
                                            kill(pid, SIGCONT);
                                            bgPID = pid;
                                        } else if (WIFCONTINUED(childStatus)) {
                                            fprintf(stderr, "Child process %d stopped. Continuing.\n", (int)pid);
                                            fprintf(stdout, "%d\n", (int)pid);
                                        }
                                    }
                                } while (!WIFEXITED(childStatus) && !WIFSIGNALED(childStatus) && !WIFSTOPPED(childStatus));

                            }
                            //  bgPID = pid;
                        }
                        background = 0;
                        break;
                } // switch bracket
            } // close forking
        } // end of check if args > 0 /  < 2
        // else: handle the error
        //    reset_signals();
    } // close for loop
    return 0;
} // close main

// initialize the arr words[] with NULL pointers
char *words[MAX_WORDS] = {0};


/* Splits a string into words delimited by whitespace. Recognizes
 * comments as '#' at the beginning of a word, and backslash escapes.
 *
 * Returns number of words parsed, and updates the words[] array
 * with pointers to the words, each as an allocated string.
 */
// takes const char pointer; returns size_t
size_t wordsplit(char const *line) {
    // len of word
    size_t wlen = 0;
    // word index
    size_t wind = 0;
    // char pointer c points to the beginning of the input line
    char const *c = line;
    // skip whitespace
    for (;*c && isspace(*c); ++c); /* discard leading space */
    // iterate over chars in the input line until the end is reached
    for (; *c;) {
        // is max num of words reached?
        if (wind == MAX_WORDS) break;
        // check if char is a comment? is so, exit the loop
        if (*c == '#') break;
        // iterate over chars until whitespace is encountered
        for (;*c && !isspace(*c); ++c) {
            // handle backslash escapes, advance ptr if \\ found
            if (*c == '\\') ++c;
            // dynamically allocate memory for the curr word, take into account '\\'
            void *tmp = realloc(words[wind], sizeof **words * (wlen + 2));
            if (!tmp) {
                err(1, "realloc");
            }

            words[wind] = tmp;
            // cp chars from input line to the alloc mem
            // update the word length
            words[wind][wlen++] = *c;
            // null-terminate the word
            words[wind][wlen] = '\0';
        }

        // check for special tokens

        // increment words index
        ++wind;
        // reset word len
        wlen = 0;
        for (;*c && isspace(*c); ++c);
    }

    // free memory for realloc -----------------
    return wind;
}


/* Find next instance of a parameter within a word. Sets
 * start and end pointers to the start and end of the parameter
 * token.
 */
// scan a word for special params and set start/end pointers to the s/e param token
char
param_scan(char const *word, char const **start, char const **end)
{
    static char const *prev;
    if (!word) word = prev;

    // no param found yet
    char ret = 0;
    // no start yet
    *start = 0;
    // no end yet
    *end = 0;
    // iterate over each character in the word until end is reached or param is found
    for (char const *s = word; *s && !ret; ++s) {
        // look for occurrence of $
        s = strchr(s, '$');
        // if not found, break out the loop
        if (!s) break;
        // check char following $ to see its type
        switch (s[1]) {
            case '$':
            case '!':
            case '?':
                // set ret to the char $,!, or ?
                ret = s[1];
                // set start and end to corresponding positions
                *start = s;
                *end = s + 2;
                break;
            case '{':;
                // look for closing bracket
                char *e = strchr(s + 2, '}');
                if (e) {
                    ret = s[1];
                    *start = s;
                    *end = e + 1;
                }
                break;
        }
    }
    // set prev to end position
    prev = *end;
    // return the type of param found
    return ret;
}

/* Simple string-builder function. Builds up a base
 * string by appending supplied strings/character ranges
 * to it.
 */
// str builder that appends strings or chars to the base string
char *
build_str(char const *start, char const *end)
{
    // len of base str
    static size_t base_len = 0;
    // set base to store base str
    static char *base = 0;

    // if start is null
    if (!start) {
        /* Reset; new base string, return old one */
        char *ret = base;
        base = NULL;
        base_len = 0;
        return ret;
    }
    /* Append [start, end) to base string
     * If end is NULL, append whole start string to base string.
     * Returns a newly allocated string that the caller must free.
     */
    size_t n = end ? end - start : strlen(start);
    size_t newsize = sizeof *base *(base_len + n + 1);
    void *tmp = realloc(base, newsize);
    if (!tmp) err(1, "realloc");
    base = tmp;
    memcpy(base + base_len, start, n);
    base_len += n;
    base[base_len] = '\0';

    return base;
}

/* Expands all instances of $! $$ $? and ${param} in a string
 * Returns a newly allocated string that the caller must free
 */

char *
expand(char const *word)
{
    // convert pid into string in here, use malloc 20bytes
    // use sprintf
    char const *pos = word;
    char const *start, *end;
    // start is mutable ; ptr to address of where we find our first dollar sign
    char c = param_scan(pos, &start, &end);
    // clear build string
    build_str(NULL, NULL);
    // append everything up until start ??????
    build_str(pos, start);
//    pid_t originalPid = getpid();
//    printf("Original pid: %d\n", originalPid);
    char *pidStr = NULL;
    while (c) {
        // default to null for now, later take background's pid
        if (c == '!') {
            if (bgPID != -1) {
                // bg pid as string
                //   fprintf(stderr, "%d", bgPID);
                char *bgPidStr;
                asprintf(&bgPidStr, "%d", bgPID);
                build_str(bgPidStr, NULL);
                free(bgPidStr);
            } else {
                build_str("", NULL);
            }

        }

        else if (c == '$'){
            // handle the prompt
//            if (strcmp(start, "PS1") == 0) {
//                build_str(PS1, NULL);
//            } else {
            pid_t currPid = getpid();
            asprintf(&pidStr,"%d", currPid);
            build_str(pidStr, NULL);
        }

        else if (c == '?') {
            // default to 0 for now, later it will be foreground pid
            char *status = NULL;
            asprintf(&status, "%d", foreground_default_status);
            build_str(status, NULL);
            free(status);
        }
        else if (c == '{') {
            // ignore ${
            start += 2;
            size_t paramLen = end - start - 1;
            // add one to take null terminator into account
            char *envVar = malloc(paramLen + 1);


            strncpy(envVar, start, paramLen);
            // add null terminator
            envVar[paramLen] = '\0';

            // get environment var value
            char *envVal = getenv(envVar);
            free(envVar);

            // append to output whatever is in { }
            // build_str("", NULL);

            //if environment var exist, append its val
            if (envVal != NULL) {
                build_str(envVal, NULL);
            }
            // build_str("", NULL);


        }
        pos = end;
        c = param_scan(pos, &start, &end);
        build_str(pos, start);
    }

//    pid_t expandedPid = getpid();
//    printf("Expanded pid: %d\n", expandedPid);
    free(pidStr);
    return build_str(start, NULL);
}
