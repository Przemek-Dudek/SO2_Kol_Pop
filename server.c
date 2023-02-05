#include "common.h"

int terminate;
int t_compleated;
sem_t* main_sem; //semafor sluzacy do zabezpieczenie sekcji krytycznej w pamieci wspoldzielonej
pthread_mutex_t main_mutex = PTHREAD_MUTEX_INITIALIZER; //muteks ktory sluzy do zabezpieczania zmiennych terminate i t_compleated przed hazardem

void *comunicate(void *arg)
{
    struct data_t *pdata = (struct data_t*)arg;

    pthread_mutex_lock(&main_mutex);
    while(!terminate) {
        t_compleated = 1;
        pthread_mutex_unlock(&main_mutex);
        sem_init(&pdata->sem_1,1,0);
        sem_init(&pdata->sem_2,1,0);

        sem_wait(&pdata->sem_1);
        sem_wait(main_sem);

        printf("Client connected\n");
        printf("size = %d\n", pdata->size);

        int wsk = shm_open("/msg_tab", O_CREAT | O_RDWR, 0600);
        ftruncate(wsk, sizeof(int32_t)*pdata->size);
        int32_t *tab = (int32_t*) mmap(NULL, sizeof(int32_t)*pdata->size, PROT_READ | PROT_WRITE, MAP_SHARED, wsk, 0);

        printf("Memory allocated\n");

        pthread_mutex_lock(&main_mutex);
        t_compleated = 0;
        pthread_mutex_unlock(&main_mutex);

        sem_post(&pdata->sem_2);
        sem_post(main_sem);

        sem_wait(&pdata->sem_1);

        sem_wait(main_sem);
        pdata->sum = 0;

        //sleep(10);

        for(int i = 0; i < pdata->size; i++) {
            pdata->sum += *(tab + i);
        }

        printf("Data calculated = %d\n", pdata->sum);

        printf("Data sent to client\n\n");

        sem_post(&pdata->sem_1);
        sem_post(main_sem);

        sem_wait(&pdata->sem_2);

        shm_unlink("/msg_tab");
        munmap(tab, sizeof(int32_t)*pdata->size);
        close(wsk);

        sem_close(&pdata->sem_1);
        sem_close(&pdata->sem_2);

        pthread_mutex_lock(&main_mutex);
        t_compleated = 1;
    }

    pthread_mutex_unlock(&main_mutex);
}

int main()
{
    pthread_t comunicate_t;

    pthread_mutex_lock(&main_mutex);
    terminate = 0;
    pthread_mutex_unlock(&main_mutex);

    int fd = shm_open("/msg_data", O_CREAT | O_RDWR, 0600);
    ftruncate(fd, sizeof(struct data_t));
    struct data_t *pdata = (struct data_t*) mmap(NULL, sizeof(struct data_t*), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    main_sem = sem_open("main_sem", O_CREAT | O_EXCL, 0777, 1);
    if (main_sem == SEM_FAILED){
        sem_close(main_sem);
        sem_unlink("main_sem");

        munmap(pdata, sizeof(struct data_t));
        shm_unlink("/msg_data");

        sem_destroy(&pdata->sem_1);
        sem_destroy(&pdata->sem_2);

        return 4;
    }

    sem_wait(main_sem);
    pdata->ammount = 0;
    sem_post(main_sem);

    pthread_create(&comunicate_t, NULL, comunicate, pdata);

    int flag = 0;

    pthread_mutex_lock(&main_mutex);
    while(!(terminate && t_compleated)) {
        pthread_mutex_unlock(&main_mutex);
        if(flag == 0) {
            char input[5];

            scanf("%4s", input);
            while(getchar() != '\n');
            if(strcmp(input, "quit") == 0) {
                terminate = 1;
                flag = 1;
            }

            if(strcmp(input, "stat") == 0) {
                printf("Liczba zapytan = %d\n", pdata->ammount);
            }
        }
    }

    sem_close(main_sem);

    sem_destroy(&pdata->sem_1);
    sem_destroy(&pdata->sem_2);

    munmap(pdata, sizeof(struct data_t));
    close(fd);

    pthread_mutex_destroy(&main_mutex);
    sem_unlink("main_sem");

    shm_unlink("/msg_tab");
    shm_unlink("/msg_data");
    return 0;
}