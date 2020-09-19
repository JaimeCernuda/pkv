//
// Created by hariharan on 8/21/19.
//

#ifndef PKV_COMMON_DAEMONIZE_H
#define PKV_COMMON_DAEMONIZE_H

#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/stat.h>
#include <cstring>
#include <boost/stacktrace.hpp>

#define RUNNING_DIR	"/tmp"

void finalize();

void log_message(char *filename,char *message){
    FILE *logfile;
    logfile=fopen(filename,"a");
    if(!logfile) return;
    fprintf(logfile,"%s\n",message);
    fclose(logfile);
}
char main_log_file[256]="example.log";

template <class Allocator>
std::string to_string(const boost::stacktrace::basic_stacktrace<Allocator>& bt) {
    if (bt) {
        return boost::stacktrace::detail::to_string(&bt.as_vector()[0], bt.size());
    } else {
        return std::string();
    }
}
void signal_handler(int sig){
    switch(sig) {
    case SIGHUP:{
        log_message(main_log_file,"hangup signal caught");
        break;
    }
    case SIGTERM:{
        log_message(main_log_file,"terminate signal caught");
        finalize();
        exit(0);
        break;
    }
    default:{
        void *array[20];
        size_t size;
        // get void*'s for all entries on the stack
        size = backtrace(array, 20);
        // print out all the frames to stderr
        fprintf(stderr, "Error: signal %d:\n", sig);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
        ::raise(SIGTERM);
    }

    }
}

void daemonize(const char* lock_file, const char* log_file){
    strcpy(main_log_file,log_file);
    int i,lfp;
    char str[10];
    if(getppid()==1) return; /* already a daemon */
    i=fork();
    if (i<0) exit(1); /* fork error */
    if (i>0) exit(0); /* parent exits */
    /* child (daemon) continues */
    setsid(); /* obtain a new process group */
    for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
    i=open(log_file,O_RDWR); dup(i); dup(i); /* handle standart I/O */
    umask(027); /* set newly created file permissions */
    chdir(RUNNING_DIR); /* change running directory */
    lfp=open(lock_file,O_RDWR|O_CREAT,0640);
    if (lfp<0) exit(1); /* can not open */
    if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
    /* first instance continues */
    sprintf(str,"%d\n",getpid());
    write(lfp,str,strlen(str)); /* record pid to lockfile */
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGHUP,signal_handler); /* catch hangup signal */
    signal(SIGTERM,signal_handler); /* catch kill signal */
}
void catch_all_signals(){
    signal(SIGTERM,signal_handler); /* catch kill signal */
    signal(SIGHUP,signal_handler); /* catch hangup signal */
    signal(SIGABRT,signal_handler); /* catch hangup signal */
    signal(SIGSEGV,signal_handler); /* catch hangup signal */
    signal(SIGBUS,signal_handler); /* catch hangup signal */
    signal(SIGCHLD,SIG_IGN); /* ignore child */
    signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
}

#endif //PKV_COMMON_DAEMONIZE_H
