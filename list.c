#include "dirent.h"
#include "unistd.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "time.h"

#define MAX_PATH_LEN (256)  //存储路径的字符数组最大空间
#define MAX_PATH_COUNT (16) //存储路径的字符数组最大空间
int order[7] = {0};         // 各个数字代表含义 order[1]->r; [2]->a; [3]->l; [4]->h; [5]->m; [6]->--
//采用order数组来存储外部输入的命令，当order[i]>0时代表，需要执行相关操作，同时用order[i]来存储对应命令的参数

// 将 char[] 转换为数字，返回1，表示成功。其他没啥可说的
int Char_To_Int(char ch[], int *num)
{
    *num = 0;
    for (int i = 0; ch[i] != '\0'; i++)
        if (ch[i] <= '9' && ch[i] >= '0')
            *num = *num * 10 + (ch[i] - '0');
        else
            return 0;
    if (*num < 0)
        return 0;
    return 1;
}

//命令执行函数
int Command_Execution(char *path)
{
    //定量定义
    int i, j = 0; //用于后面做循环变量
    struct stat st;
    DIR *directory;       // 定义 Dir
    struct dirent *entry; // 定义 Dirent entry
    time_t time_now;      // 当前时间

    //命令处理
    time_now = time(NULL); // 获取当前时间;
    stat(path, &st);       //读取path路径下的数据
    if (stat(path, &st) < 0)
    {
        printf("invalid path: %s\n", path);
        return 1;
    }
    if (S_ISDIR(st.st_mode)) // 如果对象是目录
    {
        printf("%s:\n", path);
        directory = opendir(path); //打开目录
        if (directory == NULL)     //目录打开失败
        {
            printf("Open directory \"%s\": %s (ERROR %d)\n", path, strerror(errno), errno);
            return 1;
        }
        char path1[MAX_PATH_LEN] = {0};
        while ((entry = readdir(directory)) != NULL) //遍历path目录
        {
            snprintf(path1, sizeof(path1) - 1, "%s/%s", path, entry->d_name); //path1=payh+'/'+entry->d_name
            stat(path1, &st);                                                 //读取path1路径下的数据

            if (order[2] != 1 && entry->d_name[0] == '.') //-a
                continue;
            if (order[3] > 0 && st.st_size < order[3]) //-l
                continue;
            if (order[4] > 0 && st.st_size > order[4]) //-h
                continue;
            if (order[5] > 0 && ((time_now - st.st_mtime) / 86400 + 1) > order[5]) //-m
                continue;
            printf("%s, ", entry->d_name);
        } //while
        closedir(directory);
    }    //is DIR
    else //对象为文件
    {
        for (i = 0; path[i] != '\0'; i++)
            if (path[i] == '/')
                j++;

        if (order[2] != 1 && path[j] == '.') //-a
            return 1;
        if (order[3] > 0 && st.st_size < order[3]) //-l
            return 1;
        if (order[4] > 0 && st.st_size > order[4]) //-h
            return 1;
        if (order[5] > 0 && ((time_now - st.st_mtime) / 86400 + 1) > order[5]) //-m
            return 1;
        printf("%s, ", path);
    }
    return 1;
}

//递归遍历函数
void trave_dir(char *path)
{
    DIR *d = NULL;
    struct dirent *dp = NULL;
    struct stat st;
    char p[MAX_PATH_LEN] = {0};

    if (stat(path, &st) < 0)
    {
        printf("invalid path: %s\n", path);
        return;
    }

    if (!S_ISDIR(st.st_mode))
    {
        Command_Execution(path); //每次进入一个新目录时都执行一次Command_Execution函数
        printf("\n\n");
        return;
    }

    if (!(d = opendir(path)))
    {
        printf("Open directory \"%s\": %s (ERROR %d)\n", path, strerror(errno), errno);
        return;
    }

    Command_Execution(path); //每次进入一个新目录时都执行一次Command_Execution函数
    printf("\n\n");

    while ((dp = readdir(d)) != NULL)
    {
        // 把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录
        if ((!strncmp(dp->d_name, ".", 1)) || (!strncmp(dp->d_name, "..", 2)))
            continue;

        snprintf(p, sizeof(p) - 1, "%s/%s", path, dp->d_name);
        stat(p, &st);
        if (S_ISDIR(st.st_mode))
            trave_dir(p);
    }
    closedir(d);

    return;
}

int main(int argc, char *argv[])
{
    int j = 0;                                     //定量定义
    char path[MAX_PATH_COUNT][MAX_PATH_LEN] = {0}; //路径变量
    struct stat st;

    //获取全部命令
    for (int i = 1; i < argc; i++)
    {
        if (!order[6] && argv[i][0] == '-')
        {
            if (argv[i][1] == 'r')
                order[1] = 1;
            else if (argv[i][1] == 'a')
                order[2] = 1;
            else if (argv[i][1] == 'l')
            {
                i++;
                if (i < argc && Char_To_Int(argv[i], &order[3]) == 0) //检测l命令之后的内容格式是否正确
                {
                    printf("Wrong order!\n");
                    return 1;
                }
            }
            else if (argv[i][1] == 'h')
            {
                i++;
                if (i < argc && Char_To_Int(argv[i], &order[4]) == 0)
                {
                    printf("Wrong order!\n");
                    return 1;
                }
            }
            else if (argv[i][1] == 'm')
            {
                i++;
                if (i < argc && Char_To_Int(argv[i], &order[5]) == 0)
                {
                    printf("Wrong order!\n");
                    return 1;
                }
            }
            else if (argv[i][1] == '-')
                order[6] = 1;
            else
            {
                printf("Wrong order!\n");
                return 1;
            }
        }
        else if (j < MAX_PATH_COUNT)
        {
            strcpy(path[j], argv[i]);
            j++;
        }
        else
        {
            printf("Wrong order!\n");
            return 1;
        }
    }

    if (j == 0) //后无参数，读取当前目录
    {
        getcwd(path[j], sizeof(path));
        j++;
    }

    if (order[1] == 1)
        for (int i = 0; i < j; i++)
            trave_dir(path[i]);
    else
        for (int i = 0; i < j; i++)
        {
            Command_Execution(path[i]);
            printf("\n\n");
        }

    return 1;
} //main