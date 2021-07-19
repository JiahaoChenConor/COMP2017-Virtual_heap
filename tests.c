#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <string.h>
#include "cmocka.h"
#include <stdio.h>

#include "virtual_alloc.h"
#define MAXLEN 1000
void * virtual_heap;
int cur_size = 1;

// program break: the address of the first byte after the end of your heap
// If the call is successful, virtual_sbrk returns the previous program break of your virtual heap
// If the call is unsuccessful (for example because the virtual heap cannot increase further in size), virtual_sbrk returns (void *)(-1)
void * virtual_sbrk(int32_t increment) {
    // Your implementation here (for your testing only)
    if (increment == 0){
        return virtual_heap + cur_size;
    }
    else{
        virtual_heap = realloc(virtual_heap, increment + cur_size);
        if (virtual_heap != NULL){
            return virtual_heap + cur_size;
        }
    }

    return (void *)(-1);

}

static int setup(void **state){
    virtual_heap = malloc(cur_size);
    return 0;
}

static int teardown(void **state){
    free(virtual_heap);
    virtual_heap = NULL;
    return 0;
}
void test_info(char *test_num){
    // redirect the output to temp file
    freopen("tests_out/temp.out", "w", stdout);
    if (*test_num == '1' && *(test_num + 1) == '\0'){
        virtual_info(NULL);
    }else{
        virtual_info(virtual_heap);
    }
    
    freopen("/dev/tty", "w", stdout);

    // compare the result
    char filename[18] = "tests_out/out.";
    strcat(filename, test_num);

    char string_a[MAXLEN];
    char string_b[MAXLEN];
    FILE* F_a = fopen(filename, "r");
    FILE* F_b = fopen(filename, "r");

    while(!feof(F_a) && !feof(F_b)){
        fgets(string_a, MAXLEN, F_a);
        fgets(string_b, MAXLEN, F_b);
        assert_string_equal(string_a, string_b);
    }
}


// Test 1: Test heapstart is null pointer
static void initial_NULL_heapstart(void **state){
    init_allocator(NULL, 10, 2);
    
}

// Test 2: Initial one heap with block 2^28, mininmum size is 2^0
static void initial_success1(void **state){
    init_allocator(virtual_heap, 28, 0);
    test_info("2");
}

// Test 3: Initial one heap with block 2^15, mininmum size is 2^12
static void initial__success2(void **state){
    init_allocator(virtual_heap, 15, 12);
    test_info("3");
}

// Test 4: Test heapstart is NULL
static void malloc_error(void **state) {
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(NULL, 8000);
    assert_null(ptr);
    test_info("4");
}

// Test 5: Malloc 0 bytes
static void malloc_0bytes(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 0);
    assert_null(ptr);
    test_info("5");
}

// Test 6: Malloc 8000 bytes
static void malloc_8000bytes(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 8000);
    assert_non_null(ptr);
    test_info("6");
}

// Test 7: Malloc 8000, then 10000 bytes
static void malloc_multiply_times(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    void* ptr2 = virtual_malloc(virtual_heap, 10000);
    assert_non_null(ptr1);
    assert_non_null(ptr2);
    test_info("7");
    

}

// Test 8: Malloc 10 bytes
static void malloc_tiny_bytes(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 10);
    assert_non_null(ptr1);
    test_info("8");
}

// Test 9: Malloc fail
static void malloc_out_of_range(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 999999999);
    assert_null(ptr1);
    test_info("9");
}

// Test 10: heapstart is NUll
static void free_NULL_heapstart(void **state){
    init_allocator(virtual_heap, 15, 12);
    int ret = virtual_free(NULL, NULL);
    assert_int_equal(ret, 1);
    test_info("10");
}

// Test 11: free NULL
static void free_NULL(void **state){
    init_allocator(virtual_heap, 15, 12);
    int ret = virtual_free(virtual_heap, NULL);
    assert_int_equal(ret, 1);
    test_info("11");
}

// Test 12: free one block
static void free_one_block(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 8000);
    int ret = virtual_free(virtual_heap, ptr);
    assert_int_equal(ret, 0);
    test_info("12");
}

// Test 13: free 
static void free_with_merge(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    assert_non_null(ptr1);

    void* ptr2 = virtual_malloc(virtual_heap, 8000);
    assert_non_null(ptr2);

    void* ptr3 = virtual_malloc(virtual_heap, 10000);
    assert_non_null(ptr3);

    int ret1 = virtual_free(virtual_heap, ptr1);
    int ret2 = virtual_free(virtual_heap, ptr2);
    assert_int_equal(ret1, 0);
    assert_int_equal(ret2, 0);
    test_info("13");
}

// Test 14: free tiny block
static void free_tiny_block(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 10);
    int ret = virtual_free(virtual_heap, ptr);
    assert_int_equal(ret, 0);
    test_info("14");
}

// Test 15: free unallocated pointer
static void free_unallocated_pointer(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 10);
    int ret = virtual_free(virtual_heap, ptr);
    assert_int_equal(ret, 0);

    int ret2 = virtual_free(virtual_heap, ptr);
    assert_int_equal(ret2, 1);
    test_info("15");

}

// Test 16: heapstart is NUll
static void realloc_NULL_heapstart(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 10);
    assert_non_null(ptr);

    void* ptr2 = virtual_realloc(NULL, NULL, 1000);
    assert_null(ptr2);
    test_info("16");
}

// Test 17: New size is 0, work as free, including ptr is NUll
static void realloc_0_bytes(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_malloc(virtual_heap, 10);
    assert_non_null(ptr);

    void* ptr2 = virtual_realloc(NULL, NULL, 0);
    assert_null(ptr2);
    test_info("17");
}

 // Test 18: Ptr is null,(size is not 0) work as malloc
static void realloc_NULL_pointer(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr = virtual_realloc(virtual_heap, NULL, 8000);
    assert_non_null(ptr);
    test_info("18");
}

// Test 19: realloc with a bigger size
static void realloc_bigger_size(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    void* ptr2 = virtual_realloc(virtual_heap, ptr1, 10000);
    assert_non_null(ptr2);
    test_info("19");
    assert_memory_equal(ptr1, ptr2, 8000);
}

// Test 20: realloc with a smaller size
static void realloc_small_size(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 10000);
    void* ptr2 = virtual_realloc(virtual_heap, ptr1, 8000);
    assert_non_null(ptr2);
    test_info("20");
    assert_memory_equal(ptr1, ptr2, 8000);
}

 // Test 21: realloc with a smaller size, smaller than the minimum size
static void realloc_tiny_size(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    void* ptr2 = virtual_realloc(virtual_heap, ptr1, 10);
    assert_non_null(ptr2);
    test_info("21");
    assert_memory_equal(ptr1, ptr2, 10);
}

 // Test 22: realloc with the same size
static void realloc_same_size(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    void* ptr2 = virtual_realloc(virtual_heap, ptr1, 8000);
    assert_non_null(ptr2);
    test_info("22");
    assert_memory_equal(ptr1, ptr2, 8000);
}

// Test 23: realloc with huge size
static void realloc_size_out_of_range(void **state){
    init_allocator(virtual_heap, 15, 12);
    void* ptr1 = virtual_malloc(virtual_heap, 8000);
    void* ptr2 = virtual_realloc(virtual_heap, ptr1, 9999999);
    assert_null(ptr2);
    test_info("23");
}


int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(initial_NULL_heapstart, setup, teardown),
        cmocka_unit_test_setup_teardown(initial_success1, setup, teardown),
        cmocka_unit_test_setup_teardown(initial__success2, setup, teardown),
       cmocka_unit_test_setup_teardown(malloc_error, setup, teardown),
       cmocka_unit_test_setup_teardown(malloc_0bytes, setup, teardown),
        cmocka_unit_test_setup_teardown(malloc_8000bytes, setup, teardown),
        cmocka_unit_test_setup_teardown(malloc_multiply_times, setup, teardown),
        cmocka_unit_test_setup_teardown(malloc_tiny_bytes, setup, teardown),
        cmocka_unit_test_setup_teardown(malloc_out_of_range, setup, teardown),
        cmocka_unit_test_setup_teardown(free_NULL_heapstart, setup, teardown),
        cmocka_unit_test_setup_teardown(free_NULL, setup, teardown),
        cmocka_unit_test_setup_teardown(free_one_block, setup, teardown),
        cmocka_unit_test_setup_teardown(free_with_merge, setup, teardown),
        cmocka_unit_test_setup_teardown(free_tiny_block, setup, teardown),
        cmocka_unit_test_setup_teardown(free_unallocated_pointer, setup, teardown),
        cmocka_unit_test_setup_teardown(realloc_NULL_heapstart, setup, teardown),
        cmocka_unit_test_setup_teardown(realloc_0_bytes, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_NULL_pointer, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_bigger_size, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_small_size, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_tiny_size, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_same_size, setup, teardown),
         cmocka_unit_test_setup_teardown(realloc_size_out_of_range, setup, teardown),

        
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
