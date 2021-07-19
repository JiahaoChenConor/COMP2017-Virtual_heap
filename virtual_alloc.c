#include "virtual_alloc.h"

BYTE* find_state_of_ptr(BYTE* ptr_allocator,void* ptr_block, void* ptr){
    int is_find = 0;
    while (!(*ptr_allocator == 0 && *(ptr_allocator + 1) == 0))
    {
        if (ptr_block == ptr){
            is_find = 1;
            break;
        }

        int size_of_block = *(ptr_allocator);
        ptr_block += 1<<(size_of_block);
        ptr_allocator += 2;
    }

    if (is_find){
        return ptr_allocator;
    }else{
        return NULL;
    }

}

BYTE get_fit_size(uint32_t size) {
    BYTE count = 0;
    uint32_t upperbound = 1;
    // until upper bound is greater or equal to size
    while (upperbound < size)
    {
        count++;
        upperbound <<= 1;
    }
    return count;
}


BYTE* merge_two_block(BYTE* prevoius_buddy, BYTE* next_buddy){
    // ____________________________________________
    // | 13 | 1 | 13 | 1 | 13 | 1 | 13 | 2 | 0 | 0 |
    //    ^        ^
    //  prev      next

    // ____________________________________________
    // | 14 | 1 |    |   | 13 | 1 | 13 | 2 | 0 | 0 |
    //    ^        ^
    //  prev      next
    *prevoius_buddy += 1;
    *(prevoius_buddy + 1) = 1;

    while(1){
        *next_buddy = *(next_buddy + 2);
        // after move 0, the end of block section
        if (*next_buddy == 0 && *(next_buddy + 1) == 0){
            break;
        }

        next_buddy ++;
    }

    return prevoius_buddy;

    // _______________________________________
    // | 14 | 1 |  13 |  1 | 13 | 2 | 0  | 0 |
    //    ^                           ^
    //  prev                        next
}


void merge(BYTE* allocator_start, BYTE* free_ptr_allocator) {
    
    // Merge
    // ____________________________________________
    // | 13 | 1 | 13 | 1 | 13 | 2 | 13 | 2 | 0 | 0 |
    //    ^                          
    //  free_ptr_address   

    // How to judge this free part should merge with its previous block or next block ？
    // Let's say its has size 2^13 and initial size is 2^15
    // From the beginning, Let's count the total size until current block
    // For the first block, count = 2^13, count / 2^13 = 1 is odd, so his buddy is next one
    // For the second block, count = 2^13 + 2^13,  count / 2^13 = 2 is even, so his buddy is previous one


    // ________________________________________________
    // |   14     |     2   | 13 | 1 | 13 | 1 | 0 | 0 |
    //                         ^                          
    //                   free_ptr_address    
    // for the |13|1|, count = 2^14 + 2^13. count / 2^13 = 2 + 1 = 3. odd, buddy is next block
    
    // _________________________________________
    // | 13 | 2 | 13 | 1 | 13 | 1 | 13 | 2 | 0 |
    //             ^     
    //          free_ptr_address    
    // count = 2^13 + 2^13. count/2^13 = 2.even, buddy is the previous block


    BYTE* ptr_allocator = allocator_start;
    int count = 0;
    while (1)
    {
        count += 1<<(int)(*ptr_allocator);

        if (*ptr_allocator == 0 && *(ptr_allocator + 1) == 0){
            return;
        }

        if (ptr_allocator == free_ptr_allocator){
            break;
        }
        ptr_allocator += 2;
    }

    if (count / (1<<*(free_ptr_allocator)) % 2 == 0){
        BYTE* pre_buddy = free_ptr_allocator - 2;
        if (*pre_buddy == *free_ptr_allocator && *(pre_buddy + 1) == 1){
            BYTE* new_free_ptr = merge_two_block(pre_buddy, free_ptr_allocator);
            // Merge recursively
            merge(allocator_start, new_free_ptr);         
        }
        
    }else{
        BYTE* next_buddy = free_ptr_allocator + 2;
        if (*next_buddy == *free_ptr_allocator && *(next_buddy + 1) == 1){
            BYTE* new_free_ptr = merge_two_block(free_ptr_allocator, next_buddy);
            merge(allocator_start, new_free_ptr);
        } 
    }
    
    return;

}


// Find if there exist suitable block
void* find_suitable_block(BYTE* allocator_start, BYTE fit_size, void* block_start, int reach_min_size) {
    int offset_in_block = 0; 
    BYTE* ptr_in_allocator = allocator_start;
    while (!(*ptr_in_allocator == 0 && *(ptr_in_allocator + 1) == 0)){
        BYTE size_of_block = *ptr_in_allocator;
        BYTE state_of_block = *(ptr_in_allocator + 1);
        // If there is exact suitable block and this block is free.
        if (reach_min_size == 0 && size_of_block == fit_size && state_of_block == 1){
            // Mark this block as allocated.
            *(ptr_in_allocator + 1) = 2;
            // return the address in block memory.
            return (block_start + offset_in_block);
        }

        // If previous splitting has already reached the min size of block, we just need to find the first block that is free.
        else if (reach_min_size == 1 && state_of_block == 1){
            *(ptr_in_allocator + 1) = 2;
            return (block_start + offset_in_block);
        }

        offset_in_block += 1<<(size_of_block);
        ptr_in_allocator += 2;
    }

    return NULL;
}

// Split the large block
// Find the first block that is greater than the size and free then split it
// If successful return 0
// If can not splitted anymore, that is to say, reaching the min_size. return 1
// If all the remaining blocks either smaller than fit_size or allocated, return 2
int split_block(BYTE* allocator_start, BYTE fit_size, BYTE min_size){
    BYTE* ptr_in_allocator = allocator_start;
    BYTE *splitted_block_address = NULL;

    while (!(*ptr_in_allocator == 0 && *(ptr_in_allocator + 1) == 0)){
        BYTE size_of_block = *ptr_in_allocator;
        BYTE state_of_block = *(ptr_in_allocator + 1);
        if (size_of_block > fit_size && state_of_block == 1){
            splitted_block_address = ptr_in_allocator;
            break;
        }
        ptr_in_allocator += 2;
    }

    if (splitted_block_address == NULL){
        return 2;
    }

    if (*splitted_block_address <= min_size){
        return 1;
    }


    // _____________________________________________
    // | 13 | 2 | 13 | 1 | 13 | 1 | 13 | 1 | 0 |  0 |
    //            ^                          ^
    //            splitted address          end_of_allocator

    // Find the end of allactor
    BYTE* end_of_allocator = allocator_start;
    while(!(*end_of_allocator == 0 && *(end_of_allocator + 1) == 0)){
        end_of_allocator += 2;
    }


    // ______________________________________________________
    // | 13 | 2 | 13 | 1 |    |    | 13 | 1 | 13 | 1 | 0 | 0 |
    //            ^    ^                      
    //            s    e         
    // From back to front until the block which will be splitted, we move each byte back by two bytes
    // in order to reserve place for new splitted block
    while (end_of_allocator != splitted_block_address + 1){
        *(end_of_allocator + 2) = *end_of_allocator;
        end_of_allocator--;
    }

    // Make the size of splitted one half. since it record the exponent of 2, so abstract 1
    *splitted_block_address -= 1;
    *(splitted_block_address + 1) = 1; 

    // Add the new buddy block
    *(splitted_block_address + 2) = *splitted_block_address;
    *(splitted_block_address + 3) = 1; 

    return 0;


}


void init_allocator(void * heapstart, uint8_t initial_size, uint8_t min_size) {
    // Find the size of heap and reinitialize it arrording to initial_size and my data structure
    if (heapstart == NULL){
        return;
    }

    void* end_of_heap = virtual_sbrk(0);
    int existing_size = end_of_heap - heapstart;
    int extra_size = (1<<initial_size) - existing_size; // extra size for data/block part
    int size_data_structure = ((1<<(initial_size - min_size)) * 2) + 4;

    // This step is aimed for my test.    
    // Since I use realloc() in virtual_sbrk, heapstart will be free, so if I want use this pointer, I need to reassign.
    // Since virtual_sbrk will return break point of last heap, so we update the heapstart with "bp_last_heap - existing_size".
    // virtual_sbrk(extra_size + size_data_structure) Just use this line to replace next line is ok for test in ed.
    heapstart = virtual_sbrk(extra_size + size_data_structure) - existing_size; 


    // Record the initial size and min size in the beginning of heap
    BYTE* initial_size_ptr = (BYTE*) heapstart;
    BYTE* min_size_ptr = (BYTE*) (heapstart + 1);

    *initial_size_ptr = initial_size;
    *min_size_ptr = min_size;

    // The starting of block is 2 offset after heapstart
    void* block_start = heapstart + 2;
    void* allocator_start = block_start+ (int)(1<<initial_size);

    // After the end of block section, there are one section record the state of blocks. (allocator)
    // For one block, I use two bytes to record its state
    //                        ____________________________________
    //                       | size of block  |  state of block  |
    //                        ------------------------------------
    //  size: 2^(size of block)
    //  state of block: 1 means this part of block is free, 2 means allocated

    //                       _____________________________________
    //  Initially, it is     |initial_size|    1   |   0   |   0  |
    //                       -------------------------------------
    //  * Why I use two continuously 0 to represent the end of block ?
    //     -- Since the size of one block could be 2^0, 0 is size of block, but for the state only 1 and 2, so we can't judge the end of allocator
    //         by judging whether there are two continuously 0.
 
    BYTE* allocator_start_byte = (BYTE*) allocator_start;
    *allocator_start_byte = initial_size;
    *(allocator_start_byte + 1) = 1; 
    *(allocator_start_byte + 2) = 0;
    *(allocator_start_byte + 3) = 0;

}


void * virtual_malloc(void * heapstart, uint32_t size) {
    // Return NULL if you cannot fulfil the allocation or if size is 0
    if (heapstart == NULL || size == 0){
        return NULL;
    }

    BYTE* initial_size_ptr = (BYTE *)heapstart;
    BYTE* min_size_ptr = (BYTE *)(heapstart + 1);

    BYTE fit_size = get_fit_size(size);

    // If the size is too large
    if (fit_size > *initial_size_ptr){
        return NULL;
        
    }

    BYTE* block_start = (BYTE*)(heapstart + 2);
    BYTE* allocator_start = (BYTE*) (block_start + (int)(1<<*initial_size_ptr));
    
    int ret = 0;
    while (1){
        void* address = find_suitable_block(allocator_start, fit_size, block_start, ret);
        // If finding a suitable one
        if (address != NULL){
            return address;
        }
        // Else, there is no suitable one, means there should exist one bigger block or not exist.
        ret = split_block(allocator_start, fit_size, *min_size_ptr); 

        // Split successfully, continue find one suitable block
        if (ret == 0){
            continue;
        // The block is free but can not be splitted anymore, 
        // we let the function find_suitable_block to find the first one that is bigger than fit_size and fill it
        }else if (ret == 1){
            continue;
        // Can not find one block which is free and bigger
        }else if (ret == 2){
            // fprintf(stderr, "No more space\n");
            return NULL;
        }
    }

    return NULL;
   
}

int virtual_free(void * heapstart, void * ptr) {
    if (heapstart == NULL){
        return 1;
    }
    BYTE* initial_size_ptr = (BYTE *)heapstart;
    BYTE* block_start = (BYTE*)(heapstart + 2);
    BYTE* allocator_start = (BYTE*) (block_start + (int)(1<<*initial_size_ptr));

    BYTE* ptr_allocator = allocator_start;
    void* ptr_block = (void*) block_start;


    // Find the state in allactor part (ptr_allocator) according to ptr
    ptr_allocator = find_state_of_ptr(ptr_allocator, ptr_block, ptr);
    if (ptr_allocator == NULL){
        return 1;
    }

    // Free that part and merge if possible
    int state = *(ptr_allocator + 1);
    if (state == 2){
        // If that part is allocated
        *(ptr_allocator + 1) = 1; 
        merge(allocator_start, ptr_allocator);
    }else{
        return 1;
    }

    
    return 0;
}

void * virtual_realloc(void * heapstart, void * ptr, uint32_t size) {
    
    if (heapstart == NULL){
        return NULL;
    }
   
    // If size is 0 (including if ptr is NULL in this case), you should behave as if virtual_free(ptr) was called.
    if (size == 0){
        int ret = virtual_free(heapstart, ptr);
        if (ret == 1){
            return NULL;
        }
        
        return NULL;
    }

    // If ptr is NULL, you should behave as if virtual_malloc(size) was called.
    if (ptr == NULL){
        return virtual_malloc(heapstart, size);
    }
    //if size != 0 and ptr != null
    
    // After free the pointer successfully, we need to find the target block with new size
    // 1. If we can not find such block, we just allocate the original size and the states of blocks will become the original one
    //      Also, we don't change the data in original place, so everything will be fine
    // 2. If we find such block, we just move the data in original place to new block, we don't need to care about the data in 
    //      original place since the pointer has been free, these data will be regarded as garbage data.
    

    BYTE* original_ptr = (BYTE*) ptr;
    BYTE* initial_size_ptr = (BYTE *)heapstart;
    BYTE* block_start = (BYTE*)(heapstart + 2);
    BYTE* allocator_start = (BYTE*) (block_start + (int)(1<<*initial_size_ptr));

    // Find the information about ptr
    BYTE* ptr_block = (BYTE*) block_start ;
    BYTE* ptr_allocator = allocator_start;
    ptr_allocator = find_state_of_ptr(ptr_allocator, ptr_block, ptr);

    
    uint32_t original_size = 1<<(*ptr_allocator);
    
    
    // Free ptr
    int ret = virtual_free(heapstart, ptr);
    // If unsuccessful
    if (ret == 1){
        return NULL;
    }

    void* malloc_res = virtual_malloc(heapstart, size);
    // malloc unsuccessfully
    // reset data -> malloc with its origin size and (store the data into that ptr again) no need
    if (malloc_res == NULL){
       
        void* malloc_res = virtual_malloc(heapstart, original_size);
        if (malloc_res == NULL){
            return NULL;
        }
    }
    // malloc successfully.
    // move the data into new place
    
    else{
        BYTE* new_ptr = (BYTE*) malloc_res;
        // Get the smaller one
        int data_size = size < original_size ? size : original_size;
        for (int i = 0; i < data_size; i++){
            *(new_ptr + i) = *(original_ptr + i);
        }
    }

    return malloc_res;

}


void virtual_info(void * heapstart) {
    if (heapstart == NULL){
        return;
    }

    BYTE* initial_size_ptr = (BYTE *)heapstart;

    BYTE* block_start = (BYTE*)(heapstart + 2);
    BYTE* allocator_start = (BYTE*) (block_start + (int)(1<<*initial_size_ptr));

    BYTE* allocator_ptr = allocator_start;
    while (!(*allocator_ptr == 0 && *(allocator_ptr + 1) == 0)){
        BYTE block_size = *(allocator_ptr);
        BYTE block_state = *(allocator_ptr + 1);
        if (block_state == 1){
            printf("free %d\n", (int)(1<<block_size));
        }else if (block_state == 2){
            printf("allocated %d\n", (int)(1<<block_size));
        }
        allocator_ptr += 2;
    }
    
}
