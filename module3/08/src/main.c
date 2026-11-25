#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#define BUF_SIZE 65536

static volatile sig_atomic_t running = 1;
static void on_sigint(int s) { (void)s; running = 0; } // Обработка сигнала

typedef enum { F_ALL, F_UDP, F_DNS } filter_t; // Режимы фильтрации

// Перевод MAC-адреса в строку
static void mac_str(const unsigned char *m, char *out) {
    sprintf(out, "%02x:%02x:%02x:%02x:%02x:%02x",
            m[0], m[1], m[2], m[3], m[4], m[5]);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr,
            "Использование: %s iface <all|udp|dns>\n"
            "  all — все пакеты с интерфейса\n"
            "  udp — только UDP (любые порты)\n"
            "  dns — только UDP с портом 53\n", argv[0]);
        return 1;
    }
    const char *ifname = argv[1]; // Имя интерфейса

    // Определение типа фильтрации
    filter_t filt;
    if (!strcmp(argv[2], "all")) filt = F_ALL;
    else if (!strcmp(argv[2], "udp")) filt = F_UDP;
    else if (!strcmp(argv[2], "dns")) filt = F_DNS;
    else { fprintf(stderr, "Неизвестный фильтр: %s\n", argv[2]); return 1; }

    /* Создание сырого сокета 
        AF_PACKET - канальный уровень
        SOCK_RAW - сырой сокет. ЯДро отдаёт программе пакет со всеми заголовками
        ETH_P_ALL - константа, означающая "передавать все протоколы"
    */
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) { perror("socket"); return 1; }

    // Преобразование текстового имени интерфейса в его уникальный числовой идентификатор в ядре
    int ifindex = (int)if_nametoindex(ifname);
    if (ifindex == 0) { perror("if_nametoindex"); return 1; }

    // Привязка сокета к интерфейсу
    struct sockaddr_ll sll = {0};
    sll.sll_family = AF_PACKET;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_ifindex = ifindex;
    if (bind(fd, (struct sockaddr*)&sll, sizeof(sll)) < 0) {
        perror("bind"); return 1;
    }

    // Перевод сетевой карты в "неразборчивый" режим, чтобы программа получала весь трафик, а не только для этого компьютера
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    setsockopt(fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));

    // Действие по сигналу
    struct sigaction sa = {0};
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    // Начало захвата
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("Захват на %s, фильтр: %s. Ctrl+C — остановка.\n\n", ifname, argv[2]);

    unsigned char buf[BUF_SIZE];
    long long pktno = 0;

    while (running) {
        // Чтение пакета из сокета
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n < 0) { if (running) perror("recv"); continue; }
        if (n < (ssize_t)sizeof(struct ethhdr)) continue;

        // Распознание заголовка Ethernet в начале буфера
        struct ethhdr *eth = (struct ethhdr*)buf;
        if (ntohs(eth->h_proto) != ETH_P_IP) continue;
        if (n < (ssize_t)(sizeof(struct ethhdr) + sizeof(struct iphdr))) continue;

        // Поиск начала IPv4 заголовка в памяти
        struct iphdr *ip = (struct iphdr*)(buf + sizeof(struct ethhdr));
        unsigned int ihl = ip->ihl * 4u;
        if (ihl < sizeof(struct iphdr)) continue;

        unsigned short sport = 0, dport = 0;
        int is_udp = (ip->protocol == IPPROTO_UDP);
        
        // Определение UDP-датаграммы
        if (is_udp) {
            if (n < (ssize_t)(sizeof(struct ethhdr) + ihl + sizeof(struct udphdr))) continue;
            struct udphdr *udp = (struct udphdr*)(buf + sizeof(struct ethhdr) + ihl);
            sport = ntohs(udp->source);
            dport = ntohs(udp->dest);
        }
        
        // Отсеивание ненужный пакетов в зависимости от режима
        if (filt == F_UDP && !is_udp) continue;
        if (filt == F_DNS) {
            if (!is_udp) continue;
            if (sport != 53 && dport != 53) continue;
        }

        // Определение времени от запуска до прихода пакета
        clock_gettime(CLOCK_MONOTONIC, &now);
        double t = (now.tv_sec  - start.tv_sec)
                 + (now.tv_nsec - start.tv_nsec) / 1e9;

        // Заполенение текстовыми MAC отправителя и получателя
        char smac[18], dmac[18], sip[INET_ADDRSTRLEN], dip[INET_ADDRSTRLEN];
        mac_str(eth->h_source, smac);
        mac_str(eth->h_dest, dmac);

        // Запись IPv4 в структуру из сырых данных
        struct in_addr sa4 = { .s_addr = ip->saddr };
        struct in_addr da4 = { .s_addr = ip->daddr };

        // Конфертация IPv4 в строковый вид
        inet_ntop(AF_INET, &sa4, sip, sizeof(sip));
        inet_ntop(AF_INET, &da4, dip, sizeof(dip));

        printf("#%lld  t=%.6f с  protocol=%s\n",
               ++pktno, t,
               is_udp ? "UDP" : (ip->protocol == IPPROTO_TCP ? "TCP" :
                                 ip->protocol == IPPROTO_ICMP ? "ICMP" : "?"));
        printf("  MAC  %s -> %s\n", smac, dmac);
        if (is_udp)
            printf("  IP   %s:%u -> %s:%u\n\n", sip, sport, dip, dport);
        else
            printf("  IP   %s -> %s\n\n", sip, dip);
        fflush(stdout);
    }

    close(fd);
    printf("\nЗахвачено пакетов: %lld\n", pktno);
    return 0;
}