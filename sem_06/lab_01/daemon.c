
#include <syslog.h>
#include <signal.h>
#include <sys/resource.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>


sigset_t mask;
#define LOCKFILE "/var/run/daemon.pid"
#define OUTPUT_INTERVAL 5

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;
    umask(0);
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        printf("%s: невозможно получить максимальный номер дескриптора ", cmd);
        exit(1);
    }

    if ((pid = fork()) < 0)
    {
        printf("%s: ошибка вызова функции fork", cmd);
        exit(1);
    }
    else if (pid != 0)
        exit(0);

    if (setsid() == -1)
    {
        printf("Ошибка вызова setsid\n");
        exit(1);
    }

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        printf("%s: невозможно игнорировать сигнал SIGHUP", cmd);
        exit(1);
    }

    if (chdir("/") < 0)
    {
        printf("%s: невозможно сделать текущим рабочим каталогом /", cmd);
        exit(1);
    }

    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 0; i < rl.rlim_max; i++)
        close(i);
    
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
    
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
    syslog(LOG_INFO, "Демон успешно создан!");
}

int already_running(void)
{
    int fd;
    char buf[16];
    fd = open(LOCKFILE, O_RDWR|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (fd < 0) {
        syslog(LOG_ERR, "невозможно открыть %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    struct flock f1;

    f1.l_type = F_WRLCK;
    f1.l_start = 0;
    f1.l_whence = SEEK_SET;
    f1.l_len = 0;
    if (fcntl(fd, F_SETLK, &f1) < 0) {
        if (errno == EACCES || errno == EAGAIN) {
            close(fd);
            return 1;
        }
        syslog(LOG_ERR, "невозможно установить блокировку на %s: %s", LOCKFILE, strerror(errno));
        exit(1);
    }
    ftruncate(fd, 0);
    sprintf(buf, "%ld", (long)getpid());
    write(fd, buf, strlen(buf)+1);
    return(0);
}

void print_info(void)
{
    time_t t;
    struct tm *t_info;

    time(&t);
    t_info = localtime(&t);
    syslog(LOG_INFO, "Демон работает, время: %s", asctime(t_info));
}


void * thread_work(void *arg)
{
    int err, signo;
    syslog(LOG_INFO, "Создан поток для обработки сигналов");
    for (;;) {
        err = sigwait(&mask, &signo);
        if (err != 0) {
            syslog(LOG_ERR, "ошибка вызова функции sigwait");
            exit(1);
        }
        switch (signo) {
            case SIGHUP:
                syslog(LOG_INFO, "получен сигнал SIGHUP;");
                break;
            case SIGTERM:
                syslog(LOG_INFO, "получен сигнал SIGTERM; выход");
                remove(LOCKFILE);
                exit(0);
            default:
                syslog(LOG_INFO, "получен непредвиденный сигнал %d\n", signo);
        }
    }
    return(0);
}

int main(int argc, char *argv[])
{
    int err;
    pthread_t tid;
    char *cmd;
    struct sigaction sa;
    if ((cmd = strrchr(argv[0], '/')) == NULL)
        cmd = argv[0];
    else
        cmd++;
    
    daemonize(cmd);
    
    if (already_running()) {
        syslog(LOG_ERR, "Экземпляр этого демона уже запущен");
        exit(1);
    }
    
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        syslog(LOG_ERR, "%s: невозможно восстановить действие SIG_DFL для SIGHUP", cmd);
        exit(1);
    }
    sigfillset(&mask);
    if ((err = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0)
    {
        syslog(LOG_ERR, "ошибка выполнения операции SIG_BLOCK");
        exit(1);
    }
    
    err = pthread_create(&tid, NULL, thread_work, 0);
    if (err != 0)
    {
        syslog(LOG_ERR, "невозможно создать поток");
        exit(1);
    }
    
    while (1)
    {
        sleep(OUTPUT_INTERVAL);
        print_info();
    }
    exit(0);
}