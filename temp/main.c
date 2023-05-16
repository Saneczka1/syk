#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024

// definicja plików
#define SYSFS_FILE_WE1 "/sys/kernel/sykt/raba1"
#define SYSFS_FILE_WE2 "/sys/kernel/sykt/raba2"
#define SYSFS_FILE_RES "/sys/kernel/sykt/rabw"
#define SYSFS_FILE_STATUS "/sys/kernel/sykt/rabb"
#define SYSFS_FILE_ONES "/sys/kernel/sykt/rabl"

// oby nie było błędów z kompilacją i z kolejnoscią, definiuje metody na początku
unsigned int read_from_file(char *);
int write_to_file(char *, unsigned long);
struct multiplication_result multiply(unsigned long, unsigned long);
int test_module();

int main(void)
{
    int test = test_module();
    /*test polega na tym że  daje testowe znaczenia
    i już  mam wynik mnożenia
    gdyż wynik oczekiwany różni się od output-u dodaje 1
    Te
    */
    if (test > 0)
    {
        printf(" ERROR at %d values\n", test);
    }
    else
    {
        printf(" OK\n");
    }
    return 0;
}

unsigned long read_from_file(char *filePath)
{
    char buffer[MAX_BUFFER];

    int file = open(filePath, O_RDONLY);
    if (file < 0)
    {
        printf("Open %s - error number %d\n", filePath, errno);
        exit(1);
    }
    int n = read(file, buffer, MAX_BUFFER);
    if (n > 0)
    {
        unsigned long *val; 
        buffer[n] = '\0';
        sscanf(buffer, "%lx", &val);
        close(file);
        return *val;// 16 znaczy HEX
    }
    else
    {
        printf("Open %s - error %d\n", filePath, errno);
        close(file);
        exit(3);
    }
}

int write_to_file(char *filePath, unsigned long input)
{
    char buffer[MAX_BUFFER];
    FILE *file = fopen(filePath, "w");
    int fd_in = open(filePath, O_RDWR);
    if (fd_in < 0)
    {
        printf("Open %s - error number %d\n", filePath, errno);
        exit(2);
    }
    snprintf(buffer, MAX_BUFFER, "%x", input);
    write(fd_in, buffer, strlen(buffer));
    int n = write(fd_in, buffer, strlen(buffer));
    if (n != strlen(buffer))
    {
        printf("Open %s - error number %d\n", filePath, errno);
        close(fd_in);
        exit(3);
    }
    close(fd_in);
    return 0;
}

/*operacja mnożenia -  to wczytywanie danych od użytkownika
i wczytywanie outputów*/

struct multiplication_result
{
    unsigned long w;
    unsigned long l;
    unsigned  long b;
};

struct multiplication_result multiply(unsigned long arg1, unsigned long arg2)
{

    write_to_file(SYSFS_FILE_WE1, arg1);
    usleep(10000);
    write_to_file(SYSFS_FILE_WE2, arg2);
    usleep(10000);

    unsigned long readw = 0;
    unsigned long readl = 0;
    unsigned long readb = 0;
    int k = 0;
    int l = 0;
    while (l == 0)
    {
        sleep(1);

        readb = read_from_file(SYSFS_FILE_STATUS);
        readw = read_from_file(SYSFS_FILE_RES);
        readl = read_from_file(SYSFS_FILE_ONES);

        if (readb == 3 && readw != 0)
        {
            l++;
        }

        if (readb == 0)
        {
            printf("result cannot be represented in 32 bits");
            break;
        }

        if (k == 20)
        {
            break;
        }
        k++;
    }
    // readw = read_from_file(SYSFS_FILE_RES);
    // readl = read_from_file(SYSFS_FILE_ONES);
    // readb = read_from_file(SYSFS_FILE_STATUS);

    struct multiplication_result result;
    result.w = readw;
    result.l = readl;
    result.b = readb;

    return result;
}

int random_in_range(unsigned long min, unsigned long  max)
{
    return min + rand() % (max - min + 1);
}

int count_ones(unsigned long n)
{
    int count = 0;
    int a[32];

    for (int i = 0; n > 0; i++)
    {
        a[i] = n % 2;
        if (a[i] == 1)
        {
            count++;
        }
        n = n / 2;
    }
    return count;
}

int test_module()
{

    typedef struct
    {
        unsigned long a1;
        unsigned long a2;
        unsigned long w;
        unsigned long num_ones;
    } MyStruct;

    MyStruct values[500];

    for (int i = 0; i < 500; i++)
    {
        values[i].a1 = random_in_range(0, 1048575);
        values[i].a2 = random_in_range(0, 1048575);
        values[i].w = values[i].a1 * values[i].a2;
        values[i].num_ones = count_ones(values[i].w);
    }

    int k = 0;
    for (int i = 0; i < 500; i++)
    {
        struct multiplication_result result = multiply(values[i].a1, values[i].a2);
        if (result.w != values[i].w && result.l != values[i].num_ones)
        {
            printf("ERROR: a1 = %x, a2 = %x, expected w = %x, expected num_ones = %x, resultw = %x,resultl = %x\n", values[i].a1, values[i].a2, values[i].w, values[i].num_ones, result.w, result.l);
            k++;
        }
        sleep(1);
    }

    return k;
}

// tu na końcu dodałem nawias po k++
