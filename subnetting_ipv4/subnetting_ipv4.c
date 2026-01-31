#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Address
{
    unsigned short octet[4];
};

struct Address Init()
{
    struct Address adr;
    for (int i = 0; i < 4; i++)
        adr.octet[i] = 0;
    return adr;
}


void print_adr(struct Address* adr)
{
    for (int i = 0; i < 4; i++)
    {
        if (i == 3)
        {
            printf("%i", adr->octet[i]);
            break;
        }
        printf("%i.", adr->octet[i]);
    }
}

int main() 
{
    char input[] = "10.190.27.123/10";
    const char *delim = "./";
    char *token = strtok(input, delim);
    
    struct Address octets = Init();
    int cidr;
    int i = 0;
    while (token != NULL) 
    {    
        if (i < 4)
            octets.octet[i++] = atoi(token);
        else
            cidr = atoi(token);
        token = strtok(NULL, delim);
    }

    for (int i = 0; i < 4; i++)
    {
        if (octets.octet[i] > 255)
        {
            printf("Error: octets musk be atleast 255\n");
            return 1;
        }
    }

    if (cidr < 1 || cidr > 32)
    {
        printf("Error: mask must be atleast 32\n");
        return 1;
    }

    int host_bits = 32 - cidr;
    int io;
    
    //  compute subnet mask and interesting octet
    struct Address subnet = Init();
    int bits[] = {128, 64, 32, 16, 8, 4, 2 };
    int x = 0;
    int y = cidr % 8;
    if (y == 0)
        x = 255;
    else
    {
        for (int i = 0; i < y; i++)
            x += bits[i];
    }

    if (cidr < 8)
    {
        io = 1;
        subnet.octet[0] = x;
        for (int i = 1; i < 4; i++)
            subnet.octet[i] = 0;
    }
    else if (cidr < 16)
    {
        io = 2;
        subnet.octet[0] = 255;
        subnet.octet[1] = x;
        for (int i = 2; i < 4; i++)
            subnet.octet[i] = 0;
    }
    else if (cidr < 24)
    {
        io = 3;
        subnet.octet[0] = 255;
        subnet.octet[1] = 255;
        subnet.octet[2] = x;
        subnet.octet[3] = 0;
    }
    else
    {
        io = 4;
        for (int i = 0; i < 3; i++)
            subnet.octet[i] = 255;
        subnet.octet[3] = x;
    }

    // compute all networks
    int multiples = 256 - x;
    int capacity = 2;
    int* networks = malloc(capacity * sizeof(int));
    int count = 0;
    int* tmp;
    for (int i = 0; i < 256; i += multiples)
    {
        if (count >= capacity)
        {
            capacity *= 2;
            int* tmp = realloc(networks, capacity * sizeof(int));
            networks = tmp;
        }
        networks[count++] = i;
    }
    
    printf("count: %i\n", count);

    // compute all broadcasts
    int* broadcasts = malloc((count - 1) * sizeof(int));
    for (int i = 0; i < count - 1; i++)
        broadcasts[i] = (networks[i + 1] - 1);
    
    // compute the network addresss
    struct Address network_adr = Init();
    for (int i = 0; i < 4; i++)
    {
        network_adr.octet[i] = octets.octet[i];
        if (i == io - 1)
        {
            int j = 0;
            while (j + 1 < count && octets.octet[i] >= networks[j + 1])
                j++;
            network_adr.octet[i] = networks[j++];
        }
        else if (i > io - 1)
            network_adr.octet[i] = 0;
    }

    // compute the broadcast addresss
    struct Address broadcast_adr = Init();
    for (int i = 0; i < 4; i++)
    {
        broadcast_adr.octet[i] = octets.octet[i];
        if (i == io - 1)
        {
            int j = 0;
            while (j < count - 1 && octets.octet[i] > broadcasts[j])
                j++;
            broadcast_adr.octet[i] = broadcasts[j];
        }
        else if (i > io - 1)
            broadcast_adr.octet[i] = 255;
    }

    //compute host range
    struct Address last_host = Init();
    struct Address first_host = Init();
    
    int usable_hosts = 1;

    if (io == 4)
    {
        first_host.octet[0] = network_adr.octet[0];
        first_host.octet[1] = network_adr.octet[1];
        first_host.octet[2] = network_adr.octet[2];
        first_host.octet[3] = network_adr.octet[3] + 1; 
    }
    else
    {
        for (int i = 3; i != -1; i--)
        {
            printf("%i\n", i);
            if (i <= io - 1)
                first_host.octet[i] = network_adr.octet[i];
            else
                first_host.octet[i] = network_adr.octet[i] + 1;
        }
    }
    


    
    printf("\naddress: ");
    print_adr(&octets);

    printf("/%i", cidr);

    printf("\ninteresting octet: %i\n", io);

    printf("subnet mask: ");
    print_adr(&subnet);
    

    printf("\nnetworks: ");
    for (int i = 0; i < count; i++)
        printf("%i, ", networks[i]);

    printf("\nbroadcasts: ");
    for (int i = 0; i < count - 2; i++)
        printf("%i, ", broadcasts[i]);
    
    printf("\nnetwork address: ");
    print_adr(&network_adr);
    
    printf("\nbroadcast address: ");
    print_adr(&broadcast_adr);

    printf("\nhost range: ");
    print_adr(&first_host);


    free(networks);
    free(broadcasts);

    return 0;
}
