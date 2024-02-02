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


#ifndef MAX_WORDS
#define MAX_WORDS 512
#endif

// redirection operators
bool tokens(char *word) {
    switch (word[0]) {
        case '&':
            if (strcmp(word, "&") == 0) {
                return true;
            } else {
                return false;
            }
        case '>':
            if (strcmp(word, ">") == 0 || strcmp(word, ">>") == 0) {
                return true;
            } else {
                return false;
            }
        case '<':
            if (strcmp(word, "<") == 0) {
                return true;
            } else {
                return false;
            }
        default:
            return false;
    }
}
int foreground_default_status = 0;
//array of ptrs to strings with size 512, used to store each word parsed from input
char *words[MAX_WORDS];
// returns size_t
size_t wordsplit(char const *line);
// returns a char pointer
char * expand(char const *word);

char *PS1 = "$";

// argc - count of args; argv - arr of argument strings
int main(int argc, char *argv[])
{
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
//prompt:;
        /* TODO: Manage background processes */

        /* TODO: prompt */
        if (input == stdin) {
            fprintf(stderr, "%s", PS1);
        }
        int childStatus;
        // forking
        pid_t pid = fork();

        switch (pid) {
            case -1:
                // error, wrong pid
                fprintf(stderr, "Fork error!!\n");
                exit(EXIT_FAILURE);
                break;
            case 0:
                //redirection
                // for (size_t i = 0; i < nwords; ++i){
            // execute the cmd in child process
                execvp(words[0], words);
                // handle the failure of execvp
                fprintf(stderr, "Execvp error!!\n");
                exit(EXIT_FAILURE);
                break;
            default:
                // parent process
                pid = waitpid(pid, &childStatus, 0);
                exit(0);
                break;
        }



        }



        // read line from the input (from file or stdin)
        ssize_t line_len = getline(&line, &n, input);
        // if error, print err msg and exit
        if (line_len < 0) err(1, "%s", input_fn);



        // call wordsplit() fcn to split the line into words and store the num of
        // words in nwords
        size_t nwords = wordsplit(line);
        // iterate over each word taken from th einput line
        for (size_t i = 0; i < nwords; ++i) {
            // print original word into error stream
            fprintf(stderr, "Word %zu: %s\n", i, words[i]);
            // call on expand() to expand special params in the word and store the result
            char *exp_word = expand(words[i]);
            // free the memory allocated for the original word
            free(words[i]);
            // update words[] arr with the expanded word
            words[i] = exp_word;
            // print expanded word to error stream
            fprintf(stderr, "Expanded Word %zu: %s\n", i, words[i]);
        }
        // arguments passed to the shell
        if (nwords > 0) {
            // check if exit was typed
            if (strcmp(words[0], "exit") == 0) {
                // check if there was more than one arg passed w exit
                if (nwords > 2) {
                    fprintf(stderr, "exit: too many args passed w exit, only one allowed!");
                    return 1;
                } else {
                    if (nwords == 1) {
                        return foreground_default_status;
                    } else {
                        // handle the case if argument is not an integer
                        size_t argumentLen = strlen(words[1]);
                        for (size_t i = 0; i< argumentLen; ++i){
                            if (!isdigit(words[1][i])) {
                                fprintf(stderr, "exit: arg is not an int and it should be!!!");
                                return 1;
                            }
                        }
                        // got to convert the arg to integer
                        int exit_status =  atoi(words[1]);
                        return exit_status;
                    }
                }
            // handle the cd command
            } else if (strcmp(words[0], "cd") == 0) {
                // if there was no argument provided
                if (nwords == 1) {
                   if (chdir(getenv("HOME")) != 0) {
                       perror("cd");
                       return 2;
                   }
                }
                // more than one arg provided, throw error
                if (nwords > 2) {
                    fprintf(stderr, "cd: only one argument allowed!");
                    return 1;
                } else if (nwords == 2) {
//                    char before_dir[PATH_MAX];
//                    if (getcwd(before_dir, sizeof(before_dir)) != NULL) {
//                        fprintf(stderr, "Before cd: %s\n", before_dir);
//                    }else {
//                        perror("getcwd");
//                    }
                    if (chdir(words[1]) != 0) {
                        perror("cd");
                        return 2;
                    }
//                    char after_dir[PATH_MAX];
//                    if (getcwd(after_dir, sizeof(after_dir)) != NULL) {
//                        fprintf(stderr, "After cd: %s\n", after_dir);
//                    }else {
//                        perror("getcwd");
//                    }
                }
            }
        }
    }
}
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
            build_str("", NULL);
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
            build_str("0", NULL);
        }
        else if (c == '{') {
            char *envVar = malloc(end-start);
            // ignore ${
            start += 2;
            size_t paramLen = end - start - 1;
            strncpy(envVar, start, paramLen);
            // add null terminator
            envVar[paramLen] = '\0';

            // get environment var value
            char *envVal = getenv(envVar);
            free(envVar);

            if (envVal != NULL) {
                build_str(envVal, NULL);
            } else {
                build_str("", NULL);
            }
      //      build_str("<Parameter: ", NULL);
            build_str(envVar, NULL);
            build_str(">", NULL);
//            build_str("<Parameter: ", NULL);
//            build_str(start + 2, end - 1);
//            build_str(">", NULL);
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

