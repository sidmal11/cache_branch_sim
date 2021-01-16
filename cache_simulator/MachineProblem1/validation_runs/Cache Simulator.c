//Cache Simulator
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

int L_one_r_h = 0, L_two_r_h = 0;
int L1reads = 0, L2reads = 0;
int L1writes = 0, L2writes = 0;
int L1write_hits = 0, L2write_hits = 0;
int L1read_misses = 0, L2read_misses = 0;
int L1write_misses = 0, L2write_misses = 0;
int L1writebacks = 0, L2writebacks = 0;
char r_w = '0';
int L1policy_counter = 0, L2policy_counter = 0;
int L1index_width = 0, L2index_width = 0;
int L1offset_width = 0, L2offset_width = 0;
int L1tag_width = 0, L2tag_width = 0;
int L1assoc, L2assoc;
int L1block_size, L2block_size;
int repl_policy, inclusion_policy;
float L1missrate, L2missrate;
int memtraffic;
unsigned long address, L1tag, L2tag;
unsigned long L1dec_index = 0, L2dec_index = 0;
unsigned long L1dec_offset = 0, L2dec_offset = 0;
char trace_file[20];

int l1cachesize, l1assoc, l1blocksize;
int l2cachesize, l2assoc, l2blocksize;

struct node
{
    unsigned long key; //key
    long value[3];     //value
    struct node *next;
};

struct table
{
    int size;
    struct node **list;
};

typedef struct Cache
{
    unsigned long tag;
    int policycounter;
} Cache;

bool l1_v[200][32];
bool l2_v[500][32];
bool l1_d[200][32];
bool l2_d[200][32];

Cache L1cache[200][32];
Cache L2cache[500][32];
long L1Optimal[500][10000];

void create_opt_ref();
void L1readcache_LRU(Cache L1cache[][32], Cache L2cache[][32]);
void L1readcache_FIFO(Cache L1cache[][32], Cache L2cache[][32]);
void L1readcache_OPT(Cache L1cache[][32], Cache L2cache[][32]);
int optimal(Cache L1cache[][32]);
void L2write_switch(Cache L1cache[][32], Cache L2cache[][32], int, int);
void L2read_switch(Cache L2cache[][32], int);
void L1writecache_LRU(Cache L1cache[][32], Cache L2cache[][32]);
void L1writecache_FIFO(Cache L1cache[][32], Cache L2cache[][32]);
void L1writecache_OPT(Cache L1cache[][32], Cache L2cache[][32]);

struct table *createTable(int size)
{
    struct table *t = (struct table *)malloc(sizeof(struct table));
    t->size = size;
    t->list = (struct node **)malloc(sizeof(struct node *) * size);
    int i;
    for (i = 0; i < size; i++)
        t->list[i] = NULL;
    return t;
}
int LRU(Cache L1cache[][32]);
void L2writecache_LRU(Cache L1cache[][32], Cache L2cache[][32], int);
void L2writecache_OPT(Cache L1cache[][32], Cache L2cache[][32], int);
void L2writecache_FIFO(Cache L1cache[][32], Cache L2cache[][32], int);
void insert(struct table *, unsigned long, long, long, long);
void L2readcache_LRU(Cache L2cache[][32]);
void L2readcache_OPT(Cache L2cache[][32]);
void L2readcache_FIFO(Cache L2cache[][32]);
int hashCode(struct table *t, unsigned long key)
{
    if (key < 0)
        return -(key % t->size);
    return key % t->size;
}
int L2LRU(Cache L2cache[][32]);
void create_L1_L2_caches();
void calculate_summary();

void print_summary(int, int);
void read_file(char, struct table *);
long *lookup(struct table *, unsigned long);

int main(int argc, char *argv[])
{

    setbuf(stdout, NULL);

    struct table *t = createTable(10000);

    repl_policy = atoi(argv[6]);
    inclusion_policy = atoi(argv[7]);
    strcpy(trace_file, argv[8]);

    l1cachesize = atoi(argv[2]), l1assoc = atoi(argv[3]), l1blocksize = atoi(argv[1]);
    l2cachesize = atoi(argv[4]), l2assoc = atoi(argv[5]), l2blocksize = atoi(argv[1]);

    create_L1_L2_caches();
    create_opt_ref();
    read_file(r_w, t);
    calculate_summary();
    print_summary(repl_policy, inclusion_policy);

    return 0;
}

//Function Definition
void calculate_summary()
{

    L1missrate = (float)(((float)L1read_misses + (float)L1write_misses) / ((float)L1reads + (float)L1writes));

    if (l2cachesize == 0)
        L2missrate = 0;
    else
        L2missrate = (float)(((float)L2read_misses) / ((float)L1write_misses + (float)L1read_misses));

    if (l2cachesize == 0)
        memtraffic = L1read_misses + L1write_misses + L1writebacks;
    else
        memtraffic = L2read_misses + L2write_misses + L2writebacks;
}

void print_summary(repl_policy, inclusion_policy)
{

    printf("===== L1 contents =====\n");

    int i = 0;
    while (i < l1cachesize / (l1assoc * l1blocksize))
    {
        printf("Set %d: ", i);
        int j = 0;
        while (j < l1assoc)
        {

            printf("%lx", L1cache[i][j].tag);
            if (l1_d[i][j] == 1)
            {
                printf(" D ");
            }
            else
            {
                printf("   ");
            }
            j++;
        }

        printf("\n");
        i++;
    }

    if (l2cachesize > 0)
    {

        printf("===== L2 contents =====\n");

        int i = 0;
        while (i < l2cachesize / (l2assoc * l2blocksize))
        {
            printf("Set %d: ", i);
            int j = 0;
            while (j < l2assoc)
            {

                printf("%lx", L2cache[i][j].tag);
                if (l2_d[i][j] == 1)
                {
                    printf(" D ");
                }
                else
                {
                    printf("   ");
                }
                j++;
            }

            printf("\n");
            i++;
        }
    }

    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:             %d\n", l1blocksize);
    printf("L1_SIZE:               %d\n", l1cachesize);
    printf("L1_ASSOC:              %d\n", l1assoc);
    printf("L2_SIZE:               %d\n", l2cachesize);
    printf("L2_ASSOC:              %d\n", l2assoc);
    switch (repl_policy)
    {
    case 0:
        printf("REPLACEMENT POLICY:    LRU\n");
        break;
    case 1:
        printf("REPLACEMENT POLICY:    FIFO\n");
        break;
    case 2:
        printf("REPLACEMENT POLICY:    Optimal\n");
        break;
    default:
        printf("REPLACEMENT POLICY:    ---\n");
        break;
    }

    switch (inclusion_policy)
    {
    case 0:
        printf("INCLUSION PROPERTY:    non-inclusive\n");
        break;
    case 1:
        printf("INCLUSION PROPERTY:    inclusive\n");
        break;
    case 2:
        printf("INCLUSION PROPERTY:    exclusive\n");
        break;
    default:
        printf("INCLUSION PROPERTY:    ---\n");
        break;
    }

    printf("trace_file:            %s\n", trace_file);
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads:        %d\n", L1reads);
    printf("b. number of L1 read misses:  %d\n", L1read_misses);
    printf("c. number of L1 writes:       %d\n", L1writes);
    printf("d. number of L1 write misses: %d\n", L1write_misses);
    printf("e. L1 miss rate:              %f\n", L1missrate);
    printf("f. number of L1 writebacks:   %d\n", L1writebacks);
    printf("g. number of L2 reads:        %d\n", L2reads);
    printf("h. number of L2 read misses:  %d\n", L2read_misses);
    printf("i. number of L2 writes:       %d\n", L2writes);
    printf("j. number of L2 write misses: %d\n", L2write_misses);
    printf("k. L2 miss rate:              %f\n", L2missrate);
    printf("l. number of L2 writebacks:   %d\n", L2writebacks);
    printf("m. total memory traffic:      %d\n", memtraffic);
}

void create_L1_L2_caches()
{
    /*Creating L1 cache*/

    L1assoc = l1assoc;
    L1block_size = l1blocksize;
    L1index_width = (int)(log(l1cachesize / (l1assoc * l1blocksize)) / log(2));

    L1offset_width = (int)(log(l1blocksize) / log(2));

    L1tag_width = 64 - L1index_width - L1offset_width;
    // printf("%d %d %d",L1index_width, L1offset_width, L1tag_width);
    int i = 0;
    while (i < l1cachesize / (l1assoc * l1blocksize))
    {
        int j = 0;
        while (j < l1assoc)
        {
            l1_d[i][j] = false;
            L1cache[i][j].tag = 0;
            L1cache[i][j].policycounter = 0;
            l1_v[i][j] = false;
            j++;
        }
        i++;
    }

    /*Creating L2 cache*/

    if (l2assoc == 0 || l2blocksize == 0)
    {
        l2cachesize = 0;
    }

    else
    {

        L2assoc = l2assoc;
        L2block_size = l2blocksize;
        L2index_width = (int)(log(l2cachesize / (l2assoc * l2blocksize)) / log(2));
        L2offset_width = (int)(log(l2blocksize) / log(2));

        i = 0;
        while (i < l2cachesize / (l2assoc * l2blocksize))
        {
            int j = 0;
            while (j < l2assoc)
            {
                l2_d[i][j] = false;
                L2cache[i][j].tag = 0;
                L2cache[i][j].policycounter = 0;
                l2_v[i][j] = false;
                j++;
            }
            i++;
        }
    }
}

void read_file(char r_w, struct table *t)
{

    FILE *fp;
    fp = fopen(trace_file, "r"); //reading trace file
    fseek(fp, 0, SEEK_SET);
    if (fp == NULL)
    {
        printf("\nFile does not contain any data: Exiting\n");
        exit(1);
    }

    while (!feof(fp))
    {

        fscanf(fp, "%c %lx\n", &r_w, &address);

        long *ptr;
        if (lookup(t, address) != NULL)
        {
            ptr = lookup(t, address);
            L1tag = ptr[0];
            L1dec_index = ptr[1];
            L1dec_offset = ptr[2];
        }
        else
        {
            /*decoding trace for L1*/
            L1tag = address >> (L1index_width + L1offset_width);
            L1dec_index = (address << L1tag_width) >> (L1tag_width + L1offset_width);
            L1dec_offset = (address << (L1tag_width + L1index_width)) >> (L1tag_width + L1index_width);

            insert(t, address, L1tag, L1dec_index, L1dec_offset);
        }

        /*decoding trace for L2*/
        if (l2cachesize != 0)
        {
            L2tag_width = 64 - L2index_width - L2offset_width;
            L2tag = address >> (L2index_width + L2offset_width);
            L2dec_index = (address << L2tag_width) >> (L2tag_width + L2offset_width);
            L2dec_offset = (address << (L2tag_width + L2index_width)) >> (L2tag_width + L2index_width);
        }

        switch (r_w)
        {
        case 'r':
            switch (repl_policy)
            {
            case 0:
                L1readcache_LRU(L1cache, L2cache);
                break;
            case 1:
                L1readcache_FIFO(L1cache, L2cache);
                break;
            case 2:
                L1readcache_OPT(L1cache, L2cache);
                break;
            default:
                printf("Sorry wont work\n");
                break;
            }
            break;

        case 'w':
            switch (repl_policy)
            {
            case 0:
                L1writecache_LRU(L1cache, L2cache);
                break;
            case 1:
                L1writecache_FIFO(L1cache, L2cache);
                break;
            case 2:
                L1writecache_OPT(L1cache, L2cache);
                break;
            default:
                printf("Sorry wont work\n");
                break;
            }
        }
    }
    fclose(fp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////// HELPER FUNCTTONS
void L1readcache_LRU(Cache L1cache[][32], Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;
    bool flag = false;
    L1reads += 1;
    L1policy_counter += 1;

    int counter2 = 0, i = 0;
    while (i < L1assoc)
    {
        if (l1_v[L1dec_index][i] != false && (L1cache[L1dec_index][i].tag == L1tag)) //handling read hits
        {
            L_one_r_h += 1;
            counter = 1;
            L1cache[L1dec_index][i].policycounter = L1policy_counter;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        L1read_misses += 1;
        int i = 0;
        while (i < L1assoc)
        {

            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            if (l1_v[L1dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }
        if (counter2 == L1assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = LRU(L1cache);
        }

        if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
        {
            L1writebacks += 1;
            if (l2cachesize != 0)
                L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
        }

        if (l2cachesize != 0)
            L2read_switch(L2cache, repl_policy);

        L1cache[L1dec_index][rep_candidate].tag = L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
        l1_d[L1dec_index][rep_candidate] = false;
        l1_v[L1dec_index][rep_candidate] = true;
    }
}

void L1readcache_FIFO(Cache L1cache[][32], Cache L2cache[][32])
{
    //FIFO implementation
    int counter = 0, rep_candidate = 0;
    bool flag = false;

    L1reads += 1;
    L1policy_counter += 1;

    int counter2 = 0, i = 0;
    while (i < L1assoc)
    {
        if (l1_v[L1dec_index][i] != false && (L1cache[L1dec_index][i].tag == L1tag)) //handling read hits
        {
            L_one_r_h += 1;
            counter = 1;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        L1read_misses += 1;
        int i = 0;
        while (i < L1assoc)
        {

            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            if (l1_v[L1dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }

        if (counter2 == L1assoc)
        {
            flag = true;
        }

        switch (flag)
        {
        case true:
            rep_candidate = LRU(L1cache);
        }

        if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
        {
            L1writebacks += 1;
            if (l2cachesize != 0)
                L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
        }
        if (l2cachesize != 0)
            L2read_switch(L2cache, repl_policy);

        L1cache[L1dec_index][rep_candidate].tag = L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
        l1_d[L1dec_index][rep_candidate] = false;
        l1_v[L1dec_index][rep_candidate] = true;
    }
}

void L1readcache_OPT(Cache L1cache[][32], Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;
    bool flag = false;
    L1reads += 1;
    L1policy_counter += 1;
    int i = 0, counter2 = 0;
    while (i < L1assoc)
    {
        if (l1_v[L1dec_index][i] != false && (L1cache[L1dec_index][i].tag == L1tag)) //handling read hits
        {
            L_one_r_h += 1;
            counter = 1;
            //changed
            int replace = 0;
            while (L1Optimal[L1dec_index][replace] != L1tag)
            {
                replace++;
            }
            L1Optimal[L1dec_index][replace] = -1;
            L1cache[L1dec_index][i].policycounter = L1policy_counter;
            //
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        L1read_misses += 1;
        int i = 0;
        while (i < L1assoc)
        {

            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                // changed
                int replace = 0;
                while (L1Optimal[L1dec_index][replace] != L1tag)
                {
                    replace++;
                }
                L1Optimal[L1dec_index][replace] = -1;
                //
                break;
            }
            if (l1_v[L1dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }

        if (counter2 == L1assoc)
        {
            flag = true;
        }

        switch (flag)
        {
        case true:
            rep_candidate = optimal(L1cache);
        default:
            if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
            {
                L1writebacks += 1;
                if (l2cachesize != 0)
                    L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
            }
            if (l2cachesize != 0)
                L2read_switch(L2cache, repl_policy);

            L1cache[L1dec_index][rep_candidate].tag = L1tag;
            L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
            l1_d[L1dec_index][rep_candidate] = false;
            l1_v[L1dec_index][rep_candidate] = true;
        }
    }
}

//change input type
void insert(struct table *t, unsigned long key, long val1, long val2, long val3)
{
    int pos = hashCode(t, key);
    struct node *list = t->list[pos];
    struct node *newNode = (struct node *)malloc(sizeof(struct node));
    struct node *temp = list;
    while (temp)
    {
        if (temp->key == key)
        {
            temp->value[0] = val1;
            temp->value[1] = val2;
            temp->value[2] = val3;

            return;
        }
        temp = temp->next;
    }
    newNode->key = key;
    newNode->value[0] = val1;
    newNode->value[1] = val2;
    newNode->value[2] = val3;
    newNode->next = list;
    t->list[pos] = newNode;
}

void L1writecache_LRU(Cache L1cache[][32], Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;
    L1writes += 1;
    bool flag = false;
    L1policy_counter += 1;

    int i = 0, counter2 = 0;
    while (i < L1assoc)
    {

        if (l1_v[L1dec_index][i] != false && (L1tag == L1cache[L1dec_index][i].tag)) //handling write hits
        {
            L1write_hits += 1;
            counter = 1;
            L1cache[L1dec_index][i].policycounter = L1policy_counter;
            l1_d[L1dec_index][i] = true;
            break;
        }
        i++;
    }
    if (counter == 0) //handling write misses
    {
        int i = 0;
        L1write_misses += 1;
        while (i < L1assoc)
        {
            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            else
                counter2 += 1;
            i++;
        }

        if (counter2 == L1assoc)
        {
            flag = true;
        }

        switch (flag)
        {
        case true:
            rep_candidate = LRU(L1cache);
        }

        if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
        {
            L1writebacks += 1;
            if (l2cachesize != 0)
                L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
        }
        if (l2cachesize != 0)
            L2read_switch(L2cache, repl_policy);

        L1cache[L1dec_index][rep_candidate].tag = L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
        l1_d[L1dec_index][rep_candidate] = true;
        l1_v[L1dec_index][rep_candidate] = true;
    }
}

void L1writecache_FIFO(Cache L1cache[][32], Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;
    L1writes += 1;
    bool flag = false;
    L1policy_counter += 1;

    int i = 0, counter2 = 0;
    while (i < L1assoc)
    {

        if (l1_v[L1dec_index][i] != false && (L1tag == L1cache[L1dec_index][i].tag)) //handling write hits
        {
            L1write_hits += 1;
            counter = 1;
            l1_d[L1dec_index][i] = true;
            break;
        }
        i++;
    }
    if (counter == 0) //handling write misses
    {
        int i = 0;
        L1write_misses += 1;
        while (i < L1assoc)
        {
            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            else
                counter2 += 1;
            i++;
        }
        if (counter2 == L1assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = LRU(L1cache);
        }

        if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
        {
            L1writebacks += 1;
            if (l2cachesize != 0)
                L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
        }
        if (l2cachesize != 0)
            L2read_switch(L2cache, repl_policy);

        L1cache[L1dec_index][rep_candidate].tag = L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
        l1_d[L1dec_index][rep_candidate] = true;
        l1_v[L1dec_index][rep_candidate] = true;
    }
}

void L1writecache_OPT(Cache L1cache[][32], Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;
    L1writes += 1;
    bool flag = false;
    L1policy_counter += 1;
    int counter2 = 0, i = 0;
    while (i < L1assoc)
    {

        if (l1_v[L1dec_index][i] != false && (L1tag == L1cache[L1dec_index][i].tag)) //handling write hits
        {
            L1write_hits += 1;
            counter = 1;
            int replace = 0;
            while (L1Optimal[L1dec_index][replace] != L1tag)
            {
                replace++;
            }
            L1Optimal[L1dec_index][replace] = -1;
            L1cache[L1dec_index][i].policycounter = L1policy_counter;
            l1_d[L1dec_index][i] = true;
            break;
        }
        i++;
    }
    if (counter == 0) //handling write misses
    {
        int i = 0;
        L1write_misses += 1;
        while (i < L1assoc)
        {
            if (l1_v[L1dec_index][i] != true)
            {
                rep_candidate = i;
                int replace = 0;
                while (L1Optimal[L1dec_index][replace] != L1tag)
                {
                    replace++;
                }
                L1Optimal[L1dec_index][replace] = -1;
                break;
            }
            else
                counter2 += 1;
            i++;
        }
        if (counter2 == L1assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = optimal(L1cache);
        }

        if (l1_d[L1dec_index][rep_candidate] != false && l1_v[L1dec_index][rep_candidate] != false)
        {
            L1writebacks += 1;
            if (l2cachesize != 0)
                L2write_switch(L1cache, L2cache, rep_candidate, repl_policy);
        }
        if (l2cachesize != 0)
            L2read_switch(L2cache, repl_policy);

        L1cache[L1dec_index][rep_candidate].tag = L1tag;
        L1cache[L1dec_index][rep_candidate].policycounter = L1policy_counter;
        l1_d[L1dec_index][rep_candidate] = true;
        l1_v[L1dec_index][rep_candidate] = true;
    }
}

long *lookup(struct table *t, unsigned long key)
{
    int pos = hashCode(t, key);
    struct node *list = t->list[pos];
    struct node *temp = list;
    while (temp)
    {
        if (temp->key == key)
        {
            return temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

int LRU(Cache L1cache[][32])
{
    int lru = 0, lruway = 0;
    int i = 1;
    lru = L1cache[L1dec_index][0].policycounter;
    while (i < L1assoc)
    {

        if (L1cache[L1dec_index][i].policycounter < lru)
        {
            lru = L1cache[L1dec_index][i].policycounter;
            lruway = i;
        }
        i++;
    }
    return lruway;
}

void L2writecache_LRU(Cache L1cache[][32], Cache L2cache[][32], int rep_candidate)
{
    unsigned long decode_add = 0, tag = 0, l2tag = 0;
    unsigned long l2dec_index = 0;

    tag = L1cache[L1dec_index][rep_candidate].tag;
    bool flag = false;
    decode_add = ((((tag << L1index_width) | (L1dec_index)) << (L1offset_width)) | (L1dec_offset)); //decoding address from tag of L1 eviction block

    l2tag = decode_add >> (L2index_width + L2offset_width);
    l2dec_index = (decode_add << L2tag_width) >> (L2tag_width + L2offset_width);

    int counter = 0, counter2 = 0, rep2_candidate = 0;
    L2writes += 1;
    L2policy_counter += 1;

    int i = 0;
    while (i < L2assoc)
    {

        if (l2_v[l2dec_index][i] != false && (l2tag == L2cache[l2dec_index][i].tag)) //handling write hits
        {
            L2write_hits += 1;
            counter = 1;
            L2cache[l2dec_index][i].policycounter = L2policy_counter;
            l2_d[l2dec_index][i] = true;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {

        L2write_misses += 1;
        int i = 0;
        while (i < L2assoc)
        {
            if (l2_v[l2dec_index][i] != true)
            {
                rep2_candidate = i;
                break;
            }
            else
                counter2 += 1;
            i++;
        }
        if (counter2 == L2assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep2_candidate = LRU(L2cache);
        }

        if (l2_d[l2dec_index][rep2_candidate] != false && l2_v[l2dec_index][rep2_candidate] != false)
        {
            L2writebacks += 1;
        }

        L2cache[l2dec_index][rep2_candidate].tag = l2tag;
        L2cache[l2dec_index][rep2_candidate].policycounter = L2policy_counter;
        l2_d[l2dec_index][rep2_candidate] = true;
        l2_v[l2dec_index][rep2_candidate] = true;
    }
}

void L2writecache_FIFO(Cache L1cache[][32], Cache L2cache[][32], int rep_candidate)
{
    unsigned long decode_add = 0, tag = 0, l2tag = 0;
    unsigned long l2dec_index = 0;

    tag = L1cache[L1dec_index][rep_candidate].tag;
    decode_add = ((((tag << L1index_width) | (L1dec_index)) << (L1offset_width)) | (L1dec_offset)); //decoding address from tag of L1 eviction block

    l2tag = decode_add >> (L2index_width + L2offset_width);
    l2dec_index = (decode_add << L2tag_width) >> (L2tag_width + L2offset_width);

    int counter = 0, rep2_candidate = 0;
    L2writes += 1;
    bool flag = false;
    L2policy_counter += 1;

    int i = 0, counter2 = 0;
    while (i < L2assoc)
    {

        if (l2_v[l2dec_index][i] != false && (l2tag == L2cache[l2dec_index][i].tag)) //handling write hits
        {
            L2write_hits += 1;
            counter = 1;
            l2_d[l2dec_index][i] = true;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        int i = 0;
        L2write_misses += 1;
        while (i < L2assoc)
        {
            if (l2_v[l2dec_index][i] != true)
            {
                rep2_candidate = i;
                break;
            }
            else
                counter2 += 1;
            i++;
        }
        if (counter2 == L2assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep2_candidate = LRU(L2cache);
        }

        if (l2_d[l2dec_index][rep2_candidate] != false && l2_v[l2dec_index][rep2_candidate] != false)
        {
            L2writebacks += 1;
        }

        L2cache[l2dec_index][rep2_candidate].tag = l2tag;
        L2cache[l2dec_index][rep2_candidate].policycounter = L2policy_counter;
        l2_d[l2dec_index][rep2_candidate] = true;
        l2_v[l2dec_index][rep2_candidate] = true;
    }
}

void L2writecache_OPT(Cache L1cache[][32], Cache L2cache[][32], int rep_candidate)
{
    //not implemented yet
}

void L2readcache_LRU(Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;

    L2reads += 1;
    bool flag = false;
    L2policy_counter += 1;

    int counter2 = 0, i = 0;
    while (i < L2assoc)
    {
        if (l2_v[L2dec_index][i] != false && (L2cache[L2dec_index][i].tag == L2tag)) //handling read hits
        {
            L_two_r_h += 1;
            counter = 1;
            L2cache[L2dec_index][i].policycounter = L2policy_counter;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        int i = 0;
        L2read_misses += 1;
        while (i < L2assoc)
        {

            if (l2_v[L2dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            if (l2_v[L2dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }
        if (counter2 == L2assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = L2LRU(L2cache);
        }

        if (l2_d[L2dec_index][rep_candidate] != false && l2_v[L2dec_index][rep_candidate] != false)
        {
            L2writebacks += 1;
        }

        L2cache[L2dec_index][rep_candidate].tag = L2tag;
        L2cache[L2dec_index][rep_candidate].policycounter = L2policy_counter;
        l2_d[L2dec_index][rep_candidate] = false;
        l2_v[L2dec_index][rep_candidate] = true;
    }
}

void L2readcache_FIFO(Cache L2cache[][32])
{
    int counter = 0, rep_candidate = 0;

    L2reads += 1;
    bool flag = false;
    L2policy_counter += 1;

    int counter2 = 0, i = 0;
    while (i < L2assoc)
    {
        if (l2_v[L2dec_index][i] != false && (L2cache[L2dec_index][i].tag == L2tag)) //handling read hits
        {
            L_two_r_h += 1;
            counter = 1;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        int i = 0;
        L2read_misses += 1;
        while (i < L2assoc)
        {

            if (l2_v[L2dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            if (l2_v[L2dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }
        if (counter2 == L2assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = L2LRU(L2cache);
        }

        if (l2_d[L2dec_index][rep_candidate] != false && l2_v[L2dec_index][rep_candidate] != false)
        {
            L2writebacks += 1;
        }

        L2cache[L2dec_index][rep_candidate].tag = L2tag;
        L2cache[L2dec_index][rep_candidate].policycounter = L2policy_counter;
        l2_d[L2dec_index][rep_candidate] = false;
        l2_v[L2dec_index][rep_candidate] = true;
    }
}

void L2readcache_OPT(Cache L2cache[][32])
{

    int counter = 0, rep_candidate = 0;

    L2reads += 1;
    bool flag = false;
    L2policy_counter += 1;

    int counter2 = 0, i = 0;
    while (i < L2assoc)
    {
        if (l2_v[L2dec_index][i] != false && (L2cache[L2dec_index][i].tag == L2tag)) //handling read hits
        {
            L_two_r_h += 1;
            counter = 1;
            break;
        }
        i++;
    }
    if (counter == 0) //handling read misses
    {
        int i = 0;
        L2read_misses += 1;
        while (i < L2assoc)
        {

            if (l2_v[L2dec_index][i] != true)
            {
                rep_candidate = i;
                break;
            }
            if (l2_v[L2dec_index][i] != false)
            {
                counter2 += 1;
            }
            i++;
        }
        if (counter2 == L2assoc)
            flag = true;

        switch (flag)
        {
        case true:
            rep_candidate = L2LRU(L2cache);
        }
        if (l2_d[L2dec_index][rep_candidate] != false && l2_v[L2dec_index][rep_candidate] != false)
        {
            L2writebacks += 1;
        }

        L2cache[L2dec_index][rep_candidate].tag = L2tag;
        L2cache[L2dec_index][rep_candidate].policycounter = L2policy_counter;
        l2_d[L2dec_index][rep_candidate] = false;
        l2_v[L2dec_index][rep_candidate] = true;
    }
}

int L2LRU(Cache L2cache[][32])
{
    int i = 1, lru = 0, lruway = 0;

    lru = L2cache[L2dec_index][0].policycounter;
    while (i < L2assoc)
    {

        if (L2cache[L2dec_index][i].policycounter < lru)
        {
            lru = L2cache[L2dec_index][i].policycounter;
            lruway = i;
        }
        i++;
    }
    return lruway;
}

void create_opt_ref()
{

    for (int itr = 0; itr < 500; itr++)
    {
        for (int jtr = 0; jtr < 10000; jtr++)
        {
            L1Optimal[itr][jtr] = -2;
        }
    }

    unsigned long L1dec_index_opt = 0, L1tag_opt = 0, L1dec_offset_opt = 0;
    FILE *fp1;
    fp1 = fopen(trace_file, "r"); //reading trace file
    fseek(fp1, 0, SEEK_SET);
    if (fp1 == NULL)
    {
        printf("\nFile does not contain any data: Exiting\n");
        exit(1);
    }
    unsigned long add = 0;

    while (!feof(fp1))
    {

        fscanf(fp1, "%c %lx\n", &r_w, &add);

        /*decoding trace for L1*/
        L1tag_opt = add >> (L1index_width + L1offset_width);
        L1dec_index_opt = (add << L1tag_width) >> (L1tag_width + L1offset_width);
        L1dec_offset_opt = (add << (L1tag_width + L1index_width)) >> (L1tag_width + L1index_width);

        int col = 0;
        while (1)
        {
            if (L1Optimal[L1dec_index_opt][col] == -2)
            {
                L1Optimal[L1dec_index_opt][col] = L1tag_opt;
                break;
            }
            col++;
        }
    }

    fclose(fp1);
}

int optimal(Cache L1cache[][32])
{

    int candidate = 0, candidate_dist = 0;
    int max_dist[l1assoc];
    printf("%d ", l1assoc);
    int delete_iter = 0, delete_iter_iter = 0;
    //to find which is the latest ele which is not -1 to later delete it
    while (L1Optimal[L1dec_index][delete_iter] != L1tag)
    {
        // if
        delete_iter++;
    }
    delete_iter_iter = delete_iter;

    for (int b = 0; b < l1assoc; b++)
    {
        max_dist[b] = 0;
    }

    // for(int a=0;a<l1assoc;a++){
    //   long tag = L1cache[L1dec_index][a].tag;
    //
    //   // while((L1Optimal[L1dec_index][delete_iter_iter+1]!=tag || L1Optimal[L1dec_index][delete_iter_iter+1]!=-2) && delete_iter_iter<=10000){
    //   //   delete_iter_iter++;
    //   // }
    //
    //   max_dist[a]=delete_iter_iter;
    //   delete_iter_iter=delete_iter;
    // }

    for (int a = 0; a < l1assoc; a++)
    {
        if (max_dist[a] > candidate_dist)
        {
            candidate = a;
            candidate_dist = max_dist[a];
        }
    }

    L1Optimal[L1dec_index][delete_iter] == -1;
    printf("%d", candidate);

    // return candidate;
}

void L2read_switch(Cache L2cache[][32], int repl)
{

    switch (repl)
    {
    case 0:
        L2readcache_LRU(L2cache);
        break;
    case 1:
        L2readcache_FIFO(L2cache);
        break;
    case 2:
        L2readcache_OPT(L2cache);
        break;
    default:
        printf("Somethings not right");
        break;
    }
}

void L2write_switch(Cache L1cache[][32], Cache L2cache[][32], int rep_candidate, int repl)
{
    {
        switch (repl)
        {
        case 0:
            L2writecache_LRU(L1cache, L2cache, rep_candidate);
            break;
        case 1:
            L2writecache_FIFO(L1cache, L2cache, rep_candidate);
            break;
        case 2:
            L2writecache_OPT(L1cache, L2cache, rep_candidate);
            break;
        default:
            printf("Somethings not right");
            break;
        }
    }
}
