#include "common.h"

int main(int num, char** arg) {
    if(num != 2) {
        printf("Wrong amount of arguments\n");
        return 1;
    }

    int fd = shm_open("/msg_data", O_RDWR, 0600);
    err(fd == -1, "shm_open");

    struct data_t* pdata = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");

    sem_t* main_sem = sem_open("main_sem", 0);
    if (main_sem == SEM_FAILED){
        printf("Serwer off\n");
        munmap(pdata, sizeof(struct data_t));
        close(fd);
        return 4;
    }

    int ch = sem_trywait(main_sem);
    if(ch == -1 && errno == EAGAIN){
        printf("Serwer pelny\n");
        munmap(pdata, sizeof(struct data_t));
        close(fd);

        return 5;
    }

    printf("Connected\n");

    FILE *f = fopen(arg[1], "r");
    if(f == NULL) {
        printf("Odczyt nieudany\n");
        return 2;
    }

    int size = 0;

    for(int tmp; fscanf(f, "%d", &tmp) == 1; size++);

    fseek(f, 0, SEEK_SET);

    pdata->size = size;

    sem_post(&pdata->sem_1);
    sem_post(main_sem);

    sem_wait(main_sem);
    sem_wait(&pdata->sem_2);

    int pay = shm_open("/msg_tab", O_CREAT | O_RDWR, 0600);
    int32_t *tab = (int32_t*) mmap(NULL, sizeof(int32_t)*pdata->size, PROT_READ | PROT_WRITE, MAP_SHARED, pay, 0);

    for(int i = 0; fscanf(f, "%d", tab+i) == 1; i++)

    fclose(f);

    sem_post(&pdata->sem_1);
    sem_post(main_sem);

    sem_wait(&pdata->sem_1);

    sem_wait(main_sem);
    printf("Wynik sumowania = %d\n", pdata->sum);
    pdata->ammount += 1;

    sem_post(&pdata->sem_2);
    sem_post(main_sem);

    shm_unlink("/msg_tab");
    munmap(tab, sizeof(int32_t)*pdata->size);
    munmap(pdata, sizeof(struct data_t));
    close(fd);
    close(pay);

    return 0;
}