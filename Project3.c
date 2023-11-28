#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <windows.h>
long long get_current_time()
{
    FILETIME ft;
    GetSystemTimePreciseAsFileTime(&ft);
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    return uli.QuadPart;
}
long long const quarter = 0xffffffff / 4;
char memory[1];
char memory2[1];
char memory3[1];
char memory4[1];
int *read_memory(int offset, int size)
{
    int *data = (int *)malloc(size * sizeof(int));
    int i;
    for (i = 0; i < size; i++)
    {
        data[i] = rand();
    }
    return data;
    //////
    for (i = 0; i < size; i++)
    {
        if (offset + i < quarter)
            data[i] = memory[offset + i];
        else
        {
            if (offset + i < 2 * quarter)
                data[i] = memory2[offset + i];
            else
            {
                if (offset + i < 3 * quarter)
                    data[i] = memory3[offset + i];
                else
                    data[i] = memory4[offset + i];
            }
        }
    }
    return data;
}
// Define cache line struct
typedef struct
{
    bool valid;
    int tag;
    bool dirty; // new
    long long access_time;
    int *data;
} cache_line;
// Define cache struct
typedef struct
{
    int size;
    int block_size;
    int associativity;
    bool unified;        // false for split
    bool write_back;     // false for write_through
    bool write_allocate; // false for no-write-allocate
    int num_blocks;
    int num_sets;
    unsigned int hit;
    unsigned int miss;
    cache_line **lines;
} cache;
void create_cache(cache *c, int cache_size, int block_size, int associativity, int unified, int write_back, int write_allocate)
{
    // here, just assign all parameters needed and counters for hit and miss
    // Write your code here
    c->size = cache_size;
    c->associativity = associativity;
    c->block_size = block_size;

    c->unified = unified;
    c->write_allocate = write_allocate;
    c->write_back = write_back;
    c->hit = 0;
    c->miss = 0;

}
// Initialize cache with given parameters
void init_cache(cache *c)
{
    int i, j;
    // Calculate number of blocks and sets
    c->num_blocks = c->size/c->block_size;
    if (c->associativity == 1)
    { // dirrect-mapping
        c->num_sets = c->num_blocks;
    }
    else
    {
        c->num_sets = c->num_blocks / c->associativity;
    }
    // Allocate cache lines
    c->lines = (cache_line **)malloc(c->num_sets * sizeof(cache_line *));
    for (i = 0; i < c->num_sets; i++)
    {
        c->lines[i] = (cache_line *)malloc(c->associativity *
                                           sizeof(cache_line));
        for (j = 0; j < c->associativity; j++)
        {
            c->lines[i][j].valid = false;
            c->lines[i][j].tag = -1;
            c->lines[i][j].dirty = false;
            c->lines[i][j].access_time = 0;
            c->lines[i][j].data = (int *)malloc(c->block_size *
                                                sizeof(int));
        }
    }
}
// Free cache memory, use the nested for loop to do so and remember that cache is divided into rows and columns 
void free_cache(cache *c)
{
    int i, j;
    for (i = 0; i < c->num_sets; i++)
    {
        for (j = 0; j < c->associativity; j++)
        {
            free(c->lines[i][j].data);
        }
        // Write your code here
        free(c->associativity);

    }
    // Write your code here
    free(c->num_sets);
}
int get_tag(cache *c, int address)
{
    return address / (c->block_size * c->num_sets);
}
int get_set_index(cache *c, int address)
{
    return (address / c->block_size) % c->num_sets;
}
int get_offset(cache *c, int address)
{
    return address % c->block_size;
}
int get_lru_index(cache *c, int set_index)
{
    // Find empty cache line or evict line using LRU policy
    int i = 0;
    int min_access_time = c->lines[set_index][0].access_time;
    int lru_index = 0;
    for (i = 0; i < c->associativity; i++)
    {
        if (!c->lines[set_index][i].valid)
        {
            // Found empty line, use it
            lru_index = i;
            break;
        }
        if (c->lines[set_index][i].access_time < min_access_time)
        {
            // Found line with minimum access_time, evict it
            lru_index = i;
            min_access_time = c->lines[set_index][i].access_time;
        }
    }
    return lru_index;
}
void write_memory(int address, int *data, int size)
{
    // remember, memory is divided into 4 quarters. Here, you need to send data to one quarter using the if condition.return;
    for (int i = 0; i < size; i++)
    {
        if (address < quarter)
            // Write your code here
            memory[i]=data[i];
            else
            {
                if (address < 2 * quarter)
                    // Write your code here
                    memory2[i]=data[i];
                    else
                    {
                        if (address < 3 * quarter)
                            // Write your code here
                            memory3[i]=data[i];
                            else
                            {
                                // Write your code here
                                memory4[i]=data[i];
                            }
                    }
            }
    }
}
int read_cache(cache *c, int address)
{
    int i, j;
    int set_index = get_set_index(c, address);
    int tag = get_tag(c, address);
    int offset = get_offset(c, address);
    bool hit = false;
    int data;
    // Search cache for requested data
    for (i = 0; i < c->associativity; i++)
    {
        if (c->lines[set_index][i].valid && c->lines[set_index][i].tag ==
                                                tag)
        {
            // Cache hit
            hit = true;
            data = c->lines[set_index][i].data[offset];
            // Update access_time of the cache line
            c->lines[set_index][i].access_time = get_current_time();
            break;
        }
    }
    if (hit)
    {
        // Update cache statistics
        // printf("Cache hit!\n");
        c->hit++;
    }
    else
    {
        // Cache miss
        // printf("Cache miss!\n");
        c->miss++;
        int lru_index = get_lru_index(c, set_index);
        // Read data from main memory and store it in the cache line
        if (c->write_allocate)
        {
            c->lines[set_index][lru_index].tag = tag;
            c->lines[set_index][lru_index].valid = 1;
            c->lines[set_index][lru_index].access_time =
                get_current_time();
            int *main_memory_data = read_memory(address-offset, c->block_size);
            memcpy(c->lines[set_index][lru_index].data, main_memory_data, c->block_size);
            free(main_memory_data);
            data = c->lines[set_index][lru_index].data[offset];
        }
        else
        {
            int *main_memory_data = read_memory(address, c->block_size);
            data = main_memory_data[0];
            free(main_memory_data);
        }
    }
    return data;
}
void write_cache(cache *c, int address, int value)
{
    int set_index = get_set_index(c, address);
    int tag = get_tag(c, address);
    int offset = get_offset(c, address);
    int i;
    // check if the tag matches for any line in the set
    for (i = 0; i < c->associativity; i++)
    {
        if (c->lines[set_index][i].valid && c->lines[set_index][i].tag ==
                                                tag)
        {
            // cache hit: update the data in the cache line
            c->lines[set_index][i].data[offset] = value;
            c->lines[set_index][i].access_time = get_current_time();
            c->hit++;
            return;
        }
    }
    // cache miss: write allocate
    c->miss++;
    int evict_index = get_lru_index(c, set_index);
    if (!c->write_back && c->lines[set_index][evict_index].valid && c->lines[set_index][evict_index].dirty)
    {
        // write back the evicted cache line to main memory
        write_memory(c->lines[set_index][evict_index].tag, c->lines[set_index][evict_index].data, c->block_size);
    }
    // fetch the block from main memory
    if (!c->write_allocate)
        return;
    int *main_memory_data = read_memory(address - offset, c->block_size);
    memcpy(c->lines[set_index][evict_index].data, main_memory_data, c->block_size);
    free(main_memory_data);
    // update the cache line with the new data and tag
    c->lines[set_index][evict_index].valid = 1;
    c->lines[set_index][evict_index].dirty = 1;
    c->lines[set_index][evict_index].tag = tag;
    c->lines[set_index][evict_index].data[offset] = value;
    c->lines[set_index][evict_index].access_time = get_current_time();
}
void flush_cache(cache *c)
{
    //Use a nested for loop and if condition to flush the cache after being used. See hit_rate_associativity_analysis for an example of how and where flush_cache function is used.
 for (int i = 0; i < c->num_sets; i++)
{
    for (int j = 0; j < c->associativity; j++)
    {
        if (c->lines[i][j].valid && c->lines[i][j].dirty)
        {
            write_memory(c->lines[i][j].tag, c->lines[i][j].data, c->block_size);
        }
        c->lines[i][j].valid = false;
        c->lines[i][j].dirty = false;
    }
}
}
void load_instruction_split(cache *c, unsigned int address)
{
    // Calculate tag and set index
    int tag = get_tag(c, address);
    int set_index = get_set_index(c, address);
    // Search for the block in the set
    int i;
    for (i = 0; i < c->associativity; i++)
    {
        cache_line *line = &c->lines[set_index][i];
        if (line->valid && line->tag == tag)
        {
            // Cache hit
            c->hit++;
            line->access_time = time(NULL); // Update access time
            return;
        }
    }
    // Find a free line or evict a line according to the cache's replacement policy 
    int free_index = -1;
    int oldest_index = -1;
    time_t oldest_time = time(NULL);
    // use a for loop and if condition to iterate through the cache.
    //Remember, there are many situations where a line can be replaced.
    for (i = 0; i < c->associativity; i++)
    {
        cache_line *line = &c->lines[set_index][i];
        if (!line->valid)
        {
            // Found a free line
            free_index = i;
            break;
        }
        else if (c->write_back && !line->dirty && line->access_time < oldest_time)
        {
            // Found a clean line to evict
            oldest_index = i;
            oldest_time = line->access_time;
        }
    }
    if (free_index == -1 && oldest_index == -1)
    {
        // All lines are dirty, evict the oldest one
        for (i = 0; i < c->associativity; i++)
        {
            cache_line *line = &c->lines[set_index][i];
            if (line->access_time < oldest_time)
            {
                oldest_index = i;
                oldest_time = line->access_time;
            }
        }
    }

    // Read the block from memory
    int *data = read_memory(address, c->block_size);
    // Write the block to the cache line
    // Write your code here
    write_cache(c, address, *data);
}
cache_line *get_lru_line(cache *c, int set_index)
{
    int lru_index = 0;
    long long oldest_access_time = c->lines[set_index][0].access_time;
    for (int i = 1; i < c->associativity; i++)
    {
        if (c->lines[set_index][i].access_time < oldest_access_time)
        {
            lru_index = i;
            oldest_access_time = c->lines[set_index][i].access_time;
        }
    }
    return c->lines[lru_index];
}
void load_instruction_unified(cache *c, int address)
{
    int tag = address / c->block_size;
    int set_index = tag % c->num_sets;
    int block_offset = address % c->block_size;
    // check if instruction is in cache
    for (int i = 0; i < c->associativity; i++)
    {
        cache_line *line = &c->lines[set_index][i];
        if (line->valid && line->tag == tag)
        { // hit
            line->access_time = get_current_time();
            c->hit++;
            return;
        }
    }
    // instruction not in cache

    int old_hit = c->hit;
    cache_line *lru_line = get_lru_line(c, set_index);
    if (lru_line->dirty)
    {
        write_cache(c, lru_line->tag * c->block_size,* lru_line->data);
    }
    read_cache(c, tag * c->block_size);
    c->hit = old_hit;
    lru_line->valid = true;
    lru_line->tag = tag;
    lru_line->access_time = get_current_time();
    lru_line->dirty = false;
    c->lines[set_index][get_lru_index(c, set_index)] = *lru_line;
}
// Function to read the trace file and call the appropriate cache function for each line
void simulate_cache(cache* c, char* filename)
{
    FILE *fp;
    fopen_s(&fp, filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }
    // Initialize cache here (e.g. using init_cache function)
    char buf[100];
    int i = -1;
    while (fgets(buf, 100, fp))
    {
        i++;
        int operation;
        long addr;
        sscanf_s(buf, "%d %x", &operation, &addr);
        switch (operation)
        {
        case 0:                  // Data load reference
            read_cache(c, addr); // Call read_cache function
            break;
        case 1:                      // Data store reference
            write_cache(c, addr, 1); // Call write_cache function
            break;
        case 2: // Instruction load reference
            if (c->unified)
                load_instruction_unified(c, addr); // Call read_cache function 
                else load_instruction_split(c, addr);
            break;
        default:
            printf("Invalid operation code\n");
            break;
        }
    }
    fclose(fp);
    // Print statistics here (e.g. using print_stats function)
}
void hit_rate_associativity_analysis(int cache_size, int block_size, int associativity, int unified, int write_back, int write_allocate)
{
#define N 5
    double rates[N];
    // create and initialize the cache
    for (int i = 0; i < N; i++)
    {
        cache my_cache;
        associativity = i + 1;
        create_cache(&my_cache, cache_size, block_size, associativity,
                     unified, write_back, write_allocate);
        init_cache(&my_cache);
        simulate_cache(&my_cache, "trace.txt");
        flush_cache(&my_cache);
        printf("Associativity = %d, Cash miss: %lu, cash hit: %lu => hit rate : % lf % %\n ", associativity, my_cache.miss, my_cache.hit,
                              (double)my_cache.hit /
                          (my_cache.hit + my_cache.miss));
        rates[i] = (double)my_cache.hit / (my_cache.hit + my_cache.miss);
    }
    int max = 0;
    for (int i = 0; i < N; i++)
    {
        if (rates[max] < rates[i])
            max = i;
    }
    printf("The best rate is for ");
    if (max == 0)
        printf("direct-mapped cache\n");
    else
        printf("%d set-associative cache\n", max);
}
void hit_rate_block_size_analysis(int cache_size, int block_size, int associativity, int unified, int write_back, int write_allocate)
{
#define N 8
    double rates[N];
    block_size = 1;
    for (int i = 0; i < N; i++)
    {
        cache my_cache;
        if (cache_size / block_size < 4)
            break;
        block_size *= 2;
        create_cache(&my_cache, cache_size, block_size, associativity,
                     unified, write_back, write_allocate);
        init_cache(&my_cache);
        simulate_cache(&my_cache, "trace.txt");
        flush_cache(&my_cache);
        printf("Block size = %d, Cash miss: %lu, cash hit: %lu => hit rate : %lf % %\n ", block_size, my_cache.miss, my_cache.hit,
                              (double)my_cache.hit /
                          (my_cache.hit + my_cache.miss));
        rates[i] = (double)my_cache.hit / (my_cache.hit + my_cache.miss);
    }
    int max = 0;
    for (int i = 0; i < N; i++)
    {
        if (rates[max] < rates[i])
            max = i;
    }
    printf("The best rate for cache size %d is ", cache_size);
    printf("block size %d\n", (int)pow(2, max) * 2);
}
int main(int argc, char *argv[])
{
    // default values
    int cache_size = 2048;  // 2 KB
    int block_size = 32;    // 32 bytes
    int associativity = 2;  // direct-mapped
    int unified = 1;        // unified cache
    int write_back = 1;     // write-through
    int write_allocate = 1; // write-allocate
    // parse command line arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-c") == 0)
        {
            cache_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            block_size = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-a") == 0)
        {
            associativity = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-u") == 0)
        {
            unified = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-wb") == 0)
        {
            write_back = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-wa") == 0)
        {
            write_allocate = atoi(argv[++i]);
        }
        else
        {
            printf("Unknown argument: %s\n", argv[i]);
            return 1;
        }
    }
    hit_rate_associativity_analysis(cache_size, block_size,
                                    associativity, unified, write_back, write_allocate);
    printf("\n#######################################\n\n");
    hit_rate_block_size_analysis(cache_size, block_size, associativity,
                                 unified, write_back, write_allocate);
    return 0;
}