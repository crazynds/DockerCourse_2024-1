#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 80
#define BUFFER_SIZE 1024

void get_distro(char *buffer)
{
    FILE *fp = fopen("/etc/os-release", "r");
    if (!fp)
    {
        perror("Falha ao abrir /etc/os-release");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "PRETTY_NAME", 11) == 0)
        {
            char *distro_name = strchr(line, '=');
            if (distro_name)
            {
                distro_name++;
                distro_name[strcspn(distro_name, "\n")] = '\0';
                distro_name[strcspn(distro_name, "\"")] = '\0'; // Remove quotation marks if present
                snprintf(buffer, 256, "Distribuição: %s\n", distro_name);
            }
            break;
        }
    }

    fclose(fp);
}

void get_memory_info(char *buffer)
{
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp)
    {
        perror("Falha ao abrir /proc/meminfo");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "MemTotal", 8) == 0 || strncmp(line, "MemFree", 7) == 0 || strncmp(line, "MemAvailable", 12) == 0)
        {
            strcat(buffer, line);
        }
    }

    fclose(fp);
}

void get_cpu_info(char *buffer)
{
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp)
    {
        perror("Falha ao abrir /proc/cpuinfo");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "model name", 10) == 0)
        {
            char *cpu_model = strchr(line, ':');
            if (cpu_model)
            {
                cpu_model++;
                cpu_model[strcspn(cpu_model, "\n")] = '\0';
                snprintf(buffer + strlen(buffer), 256, "Modelo de CPU: %s\n", cpu_model);
            }
            break;
        }
    }

    fclose(fp);
}

void get_uptime(char *buffer)
{
    FILE *fp = fopen("/proc/uptime", "r");
    if (!fp)
    {
        perror("Falha ao abrir /proc/uptime");
        return;
    }

    double uptime_seconds;
    if (fscanf(fp, "%lf", &uptime_seconds) == 1)
    {
        int days = (int)(uptime_seconds / (60 * 60 * 24));
        int hours = ((int)uptime_seconds % (60 * 60 * 24)) / (60 * 60);
        int minutes = ((int)uptime_seconds % (60 * 60)) / 60;
        int seconds = (int)uptime_seconds % 60;
        snprintf(buffer + strlen(buffer), 256, "Uptime: %d dias, %d horas, %d minutos, %d segundos\n", days, hours, minutes, seconds);
    }

    fclose(fp);
}

void get_system_info(char *buffer)
{
    snprintf(buffer, BUFFER_SIZE, "Informações do Sistema:\n=======================\n");

    char info[256] = {0};
    get_distro(info);
    strcat(buffer, info);
    get_memory_info(buffer);
    get_cpu_info(buffer);
    get_uptime(buffer);
}

void handle_client(int client_sock)
{
    char buffer[BUFFER_SIZE] = {0};
    read(client_sock, buffer, BUFFER_SIZE);

    char system_info[BUFFER_SIZE * 2] = {0};
    get_system_info(system_info);

    char response[BUFFER_SIZE * 3];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             strlen(system_info), system_info);

    write(client_sock, response, strlen(response));
    close(client_sock);
}

int main()
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Falha ao criar socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Falha ao fazer bind");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 10) < 0)
    {
        perror("Falha ao fazer listen");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Servidor web rodando na porta %d\n", PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0)
        {
            perror("Falha ao aceitar conexão");
            continue;
        }

        handle_client(client_sock);
    }

    close(server_sock);
    return 0;
}
