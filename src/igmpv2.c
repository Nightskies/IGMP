#include "../include/igmpv2.h"
#include "../include/host.h"
#include "../include/msg.h"

char * build_packet(const uint32_t src, int type, const uint32_t group)
{
    ip * ip_hdr;
    igmp * igmp_hdr;

    char * packet = (char *)malloc(BUF_SIZE);
    if (packet == NULL)
        ERROR("malloc returned Null");

    memset(packet, 0, BUF_SIZE);

    // build ip packet
    ip_hdr = (ip *)packet;

    ip_hdr->version = 4; // Ipv4
    ip_hdr->ihl = (sizeof(ip) + 4) >> 2; // +4 for Router Alert option
    ip_hdr->tos = 0xc0; // Internet Control
    ip_hdr->ttl = 1;
    ip_hdr->tot_len = htons(MIN_IP_LEN + RAOPT_LEN + MIN_IGMPV2_LEN);
    ip_hdr->protocol = IPPROTO_IGMP;
    ip_hdr->saddr = src;
    if(type == IGMP_HOST_LEAVE_MESSAGE)
        ip_hdr->daddr = parse_to_ip(ALLRTRS_GROUP);
    else
        ip_hdr->daddr = group;

    // Router Alert option
    ((unsigned char*)packet + MIN_IP_LEN)[0] = IPOPT_RA;
    ((unsigned char*)packet + MIN_IP_LEN)[1] = 0x04;
    ((unsigned char*)packet + MIN_IP_LEN)[2] = 0x00;
    ((unsigned char*)packet + MIN_IP_LEN)[3] = 0x00;

    // build igmp packet
    igmp_hdr = (igmp *)(packet + MIN_IP_LEN + RAOPT_LEN);
    igmp_hdr->type = type;
    igmp_hdr->code = 0;
    igmp_hdr->group = group;
    igmp_hdr->csum = 0;
    igmp_hdr->csum = build_csum_igmp((uint16_t *)igmp_hdr, MIN_IP_LEN + RAOPT_LEN);

    printf(STYLE_GREEN_BOLD "built igmp packet [type = 0x%x, code = %d, group = %s, csum = 0x%x]\n" STYLE_RESET, 
        igmp_hdr->type, igmp_hdr->code, parse_to_str(group), igmp_hdr->csum);

    return packet;
}

uint16_t build_csum_igmp(uint16_t * addr, int len) 
{
    int nleft = len;
    uint16_t * w = addr;
    uint16_t answer = 0;
    int32_t sum = 0;

    while (nleft > 1) 
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) 
    {
        *(uint8_t *) (&answer) = *(uint8_t *)w ;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

uint32_t timer(const unsigned char max_res_time)
{
    return rand() % (max_res_time + 1);
}