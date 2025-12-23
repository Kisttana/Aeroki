#include "aegis_vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// --- Mock Utilities for independent compilation ---
// Assuming aegis_utils.h defines these
typedef unsigned char byte; // Assuming byte is defined
// ------------------------------------------------

int test_count = 0;
int fail_count = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        test_count++; \
        if (!(condition)) { \
            fail_count++; \
            fprintf(stderr, "\n[FAIL] %s:%d: %s\n       Condition: %s\n", __FILE__, __LINE__, message, #condition); \
        } else { \
            printf("[PASS] %s\n", message); \
        } \
    } while (0)

// Helper to safely clean up a vector after use
void safe_vec_clean(vector **vec_ptr) {
    if (*vec_ptr) {
        __vec_clean(*vec_ptr);
        *vec_ptr = NULL;
    }
}

// =========================================================================
// TEST 1: Initialization and Cleanup
// =========================================================================
void test_init_and_cleanup() {
    printf("\n--- Running Test 1: Initialization and Cleanup ---\n");
    vector *v_int_zero = NULL;
    vector *v_char_non_zero = NULL;

    // 1. vec_init(10, int, true) - Initialize, set_zero
    v_int_zero = vec_init(10, int, true);
    TEST_ASSERT(v_int_zero != NULL, "T1.1: vec_init non-null check");
    TEST_ASSERT(v_int_zero->size == 10, "T1.2: vec_init initial size check (10)");
    TEST_ASSERT(v_int_zero->capacity >= 10, "T1.3: vec_init capacity check (>=10)");
    TEST_ASSERT(v_int_zero->element_size == sizeof(int), "T1.4: vec_init element size check (int)");
    TEST_ASSERT(vec_at(v_int_zero, int, 5) == 0, "T1.5: vec_init set_zero=true check (element 5 is 0)");

    // 2. vec_init(0, char, false) - Zero size, default capacity
    v_char_non_zero = vec_init(0, char, false);
    TEST_ASSERT(v_char_non_zero->size == 0, "T1.6: vec_init zero size check (0)");
    // Default capacity is 8
    TEST_ASSERT(v_char_non_zero->capacity == 8, "T1.7: vec_init default capacity check (8)"); 
    TEST_ASSERT(v_char_non_zero->element_size == sizeof(char), "T1.8: vec_init element size check (char)");

    // 3. Cleanup
    safe_vec_clean(&v_int_zero);
    TEST_ASSERT(v_int_zero == NULL, "T1.9: vec_clean pointer check (v_int_zero)");
    safe_vec_clean(&v_char_non_zero);
    TEST_ASSERT(v_char_non_zero == NULL, "T1.10: vec_clean pointer check (v_char_non_zero)");
}

// =========================================================================
// TEST 2: Push/Pop and Capacity Increase
// =========================================================================
void test_push_pop_and_capacity() {
    printf("\n--- Running Test 2: Push/Pop and Capacity Increase ---\n");
    vector *v = vec_init(0, int, false); // Start with size 0, capacity 8

    // 1. Push up to default capacity (8 elements)
    size_t initial_capacity = v->capacity;
    for (int i = 0; i < (int)initial_capacity; i++) {
        vec_push_back(v, int, i + 10);
    }
    TEST_ASSERT(v->size == initial_capacity, "T2.1: Size equals initial capacity (8)");
    TEST_ASSERT(v->capacity == initial_capacity, "T2.2: Capacity unchanged after filling");
    TEST_ASSERT(vec_at(v, int, initial_capacity - 1) == 10 + (int)initial_capacity - 1, "T2.3: Last element value check");

    // 2. Push one more to trigger reallocation
    vec_push_back(v, int, 999);
    TEST_ASSERT(v->size == initial_capacity + 1, "T2.4: Size incremented past capacity (9)");
    TEST_ASSERT(v->capacity > initial_capacity, "T2.5: Capacity increased (reallocation check)");
    TEST_ASSERT(vec_at(v, int, initial_capacity) == 999, "T2.6: New element value check");
    
    // 3. Check data integrity after realloc
    TEST_ASSERT(vec_at(v, int, 0) == 10, "T2.7: Data integrity check after realloc (first element)");
    
    // 4. Pop back
    __vec_pop_back(v); // Using internal name for simplicity, macro calls this
    TEST_ASSERT(v->size == initial_capacity, "T2.8: Size decremented after pop (8)");
    
    // 5. Pop repeatedly to zero
    for (int i = 0; i < (int)initial_capacity + 1; i++) {
        __vec_pop_back(v);
    }
    TEST_ASSERT(v->size == 0, "T2.9: Size is 0 after excessive pop");
    
    safe_vec_clean(&v);
}

// =========================================================================
// TEST 3: Init_Fill and Fill
// =========================================================================
void test_init_fill_and_fill() {
    printf("\n--- Running Test 3: Init_Fill and Fill ---\n");
    int fill_value = 42;
    int new_value = 100;
    
    // 1. vec_init_fill
    vector *v = vec_init_fill(5, int, fill_value);
    TEST_ASSERT(v->size == 5, "T3.1: vec_init_fill size check (5)");
    TEST_ASSERT(vec_at(v, int, 0) == fill_value, "T3.2: vec_init_fill value check (start)");
    TEST_ASSERT(vec_at(v, int, 4) == fill_value, "T3.3: vec_init_fill value check (end)");

    // 2. vec_fill - partial range
    vec_fill(v, 1, 4, int, new_value); // Fills elements at indices 1, 2, 3
    TEST_ASSERT(vec_at(v, int, 0) == fill_value, "T3.4: vec_fill boundary check (start index 0)");
    TEST_ASSERT(vec_at(v, int, 1) == new_value, "T3.5: vec_fill mid-range check (index 1)");
    TEST_ASSERT(vec_at(v, int, 3) == new_value, "T3.6: vec_fill mid-range check (index 3)");
    TEST_ASSERT(vec_at(v, int, 4) == fill_value, "T3.7: vec_fill boundary check (end index 4)");

    // 3. vec_fill - range exceeding size (should stop at size)
    int boundary_value = 55;
    vec_fill(v, 3, 20, int, boundary_value); // Should fill indices 3 and 4 (since size is 5)
    TEST_ASSERT(vec_at(v, int, 2) == new_value, "T3.8: vec_fill previous value check");
    TEST_ASSERT(vec_at(v, int, 3) == boundary_value, "T3.9: vec_fill exceeding range check (index 3)");
    TEST_ASSERT(vec_at(v, int, 4) == boundary_value, "T3.10: vec_fill exceeding range check (index 4)");
    
    safe_vec_clean(&v);
}

// =========================================================================
// TEST 4: Insert and Erase (Shifting operations)
// =========================================================================
void test_insert_and_erase() {
    printf("\n--- Running Test 4: Insert and Erase (Shifting operations) ---\n");
    vector *v = vec_init(3, int, false);
    // Initial data: [10, 20, 30]
    vec_at(v, int, 0) = 10;
    vec_at(v, int, 1) = 20;
    vec_at(v, int, 2) = 30;

    // 1. Insert at middle (pos 1)
    vec_insert(v, 1,int, 15);
    // Expected: [10, 15, 20, 30]
    TEST_ASSERT(v->size == 4, "T4.1: Insert size check (4)");
    TEST_ASSERT(vec_at(v, int, 0) == 10, "T4.2: Insert integrity check (0)");
    TEST_ASSERT(vec_at(v, int, 1) == 15, "T4.3: Insert new value check (1)");
    TEST_ASSERT(vec_at(v, int, 2) == 20, "T4.4: Insert shift check (2)");
    TEST_ASSERT(vec_at(v, int, 3) == 30, "T4.5: Insert shift check (3)");

    // 2. Insert at front (pos 0)
    vec_insert(v, 0, int,5);
    // Expected: [5, 10, 15, 20, 30]
    TEST_ASSERT(v->size == 5, "T4.6: Insert size check (5)");
    TEST_ASSERT(vec_at(v, int, 0) == 5, "T4.7: Insert front check (0)");
    TEST_ASSERT(vec_at(v, int, 1) == 10, "T4.8: Insert front shift check (1)");
    
    // 3. Insert at back (pos == size-1) - This uses the last valid index.
    vec_insert(v, 4,int, 35);
    // Expected: [5, 10, 15, 20, 35, 30] - Note: inserting at index 4 shifts '30' to 5
    TEST_ASSERT(v->size == 6, "T4.9: Insert at end size check (6)");
    TEST_ASSERT(vec_at(v, int, 4) == 35, "T4.10: Insert end check (new value)");
    TEST_ASSERT(vec_at(v, int, 5) == 30, "T4.11: Insert end shift check (shifted value)");

    // 4. Erase at middle (pos 2)
    __vec_erase(v, 2); // Erase '15'
    // Expected: [5, 10, 20, 35, 30]
    TEST_ASSERT(v->size == 5, "T4.12: Erase size check (5)");
    TEST_ASSERT(vec_at(v, int, 1) == 10, "T4.13: Erase integrity check (1)");
    TEST_ASSERT(vec_at(v, int, 2) == 20, "T4.14: Erase shift check (2)");
    TEST_ASSERT(vec_at(v, int, 4) == 30, "T4.15: Erase last element check (4)");

    // 5. Erase at front (pos 0)
    __vec_erase(v, 0); // Erase '5'
    // Expected: [10, 20, 35, 30]
    TEST_ASSERT(v->size == 4, "T4.16: Erase front size check (4)");
    TEST_ASSERT(vec_at(v, int, 0) == 10, "T4.17: Erase front shift check (0)");

    // 6. Erase at back (pos size-1)
    __vec_erase(v, 3); // Erase '30'
    // Expected: [10, 20, 35]
    TEST_ASSERT(v->size == 3, "T4.18: Erase back size check (3)");
    TEST_ASSERT(vec_at(v, int, 2) == 35, "T4.19: Erase back value check (2)");

    safe_vec_clean(&v);
}

// =========================================================================
// TEST 5: Different Data Types (Struct/String)
// =========================================================================
typedef struct {
    int id;
    float value;
    char name[10];
} TestData;

void test_complex_types() {
    printf("\n--- Running Test 5: Different Data Types ---\n");
    
    TestData d1 = {1, 1.1f, "One"};
    TestData d2 = {2, 2.2f, "Two"};
    TestData d3 = {3, 3.3f, "Three"};

    vector *v = vec_init(0, TestData, false);

    // 1. Push complex data
    vec_push_back(v, TestData, d1);
    vec_push_back(v, TestData, d2);
    
    TEST_ASSERT(v->size == 2, "T5.1: Struct Push size check");
    
    // 2. Access and verify
    TestData read_d1 = vec_at(v, TestData, 0);
    TEST_ASSERT(read_d1.id == 1 && read_d1.value == 1.1f && strcmp(read_d1.name, "One") == 0, "T5.2: Struct Read/Access check (d1)");

    // 3. Insert complex data
    vec_insert(v, 1,TestData,d3); // [d1, d3, d2]
    TestData read_d3 = vec_at(v, TestData, 1);
    TEST_ASSERT(read_d3.id == 3 && read_d3.value == 3.3f && strcmp(read_d3.name, "Three") == 0, "T5.3: Struct Insert check (d3)");
    TestData read_shifted_d2 = vec_at(v, TestData, 2);
    TEST_ASSERT(read_shifted_d2.id == 2, "T5.4: Struct Shift integrity check (d2)");

    // 4. Test string vector
    vector *v_str = vec_init(0, char*, false);
    char *s1 = strdup("Hello");
    char *s2 = strdup("World");
    
    vec_push_back(v_str, char*, s1);
    vec_push_back(v_str, char*, s2);
    
    TEST_ASSERT(v_str->size == 2, "T5.5: char* Push size check");
    TEST_ASSERT(v_str->element_size == sizeof(char*), "T5.6: char* Element size check");
    
    char *read_s1 = vec_at(v_str, char*, 0);
    TEST_ASSERT(strcmp(read_s1, "Hello") == 0, "T5.7: char* Read check (Hello)");

    // Clean up allocated strings before cleaning vector
    free(s1);
    free(s2);

    safe_vec_clean(&v);
    safe_vec_clean(&v_str);
}


// =========================================================================
// TEST 6: Edge Cases and Error Checking (via assert)
// =========================================================================
void test_edge_cases() {
    printf("\n--- Running Test 6: Edge Cases ---\n");
    vector *v = vec_init(1, int, false);
    vec_at(v, int, 0) = 1;
    
    // 1. Pop when size is 1
    __vec_pop_back(v);
    TEST_ASSERT(v->size == 0, "T6.1: Pop from size 1 check");

    // 2. Pop when size is 0 (should be safe due to internal check)
    __vec_pop_back(v);
    TEST_ASSERT(v->size == 0, "T6.2: Pop from size 0 check (safe)");

    // 3. vec_init_fill with size 0
    vector *v_empty_fill = vec_init_fill(0, long, 999L);
    TEST_ASSERT(v_empty_fill->size == 0, "T6.3: Init_fill with size 0");
    safe_vec_clean(&v_empty_fill);
    
    // 4. Shrink to fit (already tested implicitly, but verify explicit call)
    // Current v: size 0, capacity 8. Push 3 elements.
    vec_push_back(v, int, 1);
    vec_push_back(v, int, 2);
    vec_push_back(v, int, 3);
    
    size_t old_capacity = v->capacity;
    __vec_shrink_to_fit(v);
    TEST_ASSERT(v->capacity == 3, "T6.4: Shrink to fit capacity check (3)");
    TEST_ASSERT(v->size == 3, "T6.5: Shrink to fit size check (3)");
    TEST_ASSERT(vec_at(v, int, 0) == 1, "T6.6: Shrink to fit data integrity check");
    
    safe_vec_clean(&v);
}


// =========================================================================
// MAIN TEST RUNNER
// =========================================================================
int main() {
    test_init_and_cleanup();
    //test_push_pop_and_capacity();
    //test_init_fill_and_fill();
    test_insert_and_erase();
    test_complex_types();
    test_edge_cases();

    printf("\n============================================\n");
    printf("TEST SUITE SUMMARY:\n");
    printf("Total Tests Run: %d\n", test_count);
    printf("Tests Passed:    %d\n", test_count - fail_count);
    printf("Tests Failed:    %d\n", fail_count);
    printf("============================================\n");

    if (fail_count > 0) {
        printf("!!! WARNING: %d test(s) failed. Review the FAIL messages above. !!!\n", fail_count);
        return EXIT_FAILURE;
    } else {
        printf("SUCCESS! All tests passed.\n");
        return EXIT_SUCCESS;
    }
}
