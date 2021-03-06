#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "utils.h"


/* ----------- Misc ------------- */

/* Increase capacity by a factor of 2, unless it's already really big,
 * then increase by a constant value
 */
size_t __inc_cap(size_t current) {
    if (current > 8192) {
        return current + 8192;
    } else {
        return current * 2;
    }
}

uint64_t fnv_64(void* ptr, size_t num_bytes) {
    size_t FNV_PRIME = 1099511628211U;
    size_t FNV_OFFSET = 14695981039346656037U;
    size_t hash = FNV_OFFSET;
    for (size_t i = 0; i < num_bytes; i++) {
        hash = hash ^ *((char*)ptr + i);
        hash = hash * FNV_PRIME;
    }
    return hash;
}

void utils_noop() { return; }


/* ----------- String ------------- */


String string_new() {
    String s = { .__data=NULL, .__len=0, .__cap=0 };
    return s;
}

String string_with_capactiy(size_t cap) {
    char* data = malloc((cap + 1) * sizeof(char));
    if (data == NULL) {
        fprintf(stderr, "String alloc failure\n");
        abort();
    }
    memset(data, '\0', cap + 1);
    String s = { .__data=data, .__len=0, .__cap=cap };
    return s;
}

String string_copy(String* s) {
    Str str = string_as_str(s);
    return str_to_owned_string(&str);
}

String string_copy_from_str(Str* str) {
    return str_to_owned_string(str);
}

String string_copy_from_cstr(const char* cstr) {
    size_t len = strlen(cstr);
    char* data = malloc((len + 1) * sizeof(char));
    if (data == NULL) {
        fprintf(stderr, "String alloc failure\n");
        abort();
    }
    memset(data, '\0', len + 1);
    strncpy(data, cstr, len);
    String s = { .__data=data, .__len=len, .__cap=len };
    return s;
}

size_t string_len(String* s) {
    return s->__len;
}

size_t string_cap(String* s) {
    return s->__cap;
}

void string_resize(String* s, size_t new_cap) {
    if (new_cap == 0)
        new_cap = 16;

    char* data = realloc(s->__data, (new_cap + 1) * sizeof(char));
    if (data == NULL) {
        fprintf(stderr, "String resize failure\n");
        abort();
    }
    data[s->__len] = '\0';
    s->__data = data;
    s->__cap = new_cap;
}

void string_push_char(String* s, char c) {
    size_t avail = s->__cap - s->__len;
    if (avail == 0) {
        size_t new_cap = __inc_cap(s->__cap);
        string_resize(s, new_cap);
    }
    s->__data[s->__len] = c;
    s->__len += 1;
}

void string_push_str(String* s, Str* str) {
    string_push_cstr_bound(s, str->__data, str->__len);
}

void string_push_cstr(String* s, const char* cstr) {
    size_t len = strlen(cstr);
    string_push_cstr_bound(s, cstr, len);
}

void string_push_cstr_bound(String* s, const char* cstr, size_t str_len) {
    size_t avail = s->__cap - s->__len;
    if (str_len > avail) {
        size_t new_cap = __inc_cap(s->__cap);
        if (str_len > (new_cap - s->__len)) {
            new_cap += str_len - (new_cap - s->__len);
        }
        string_resize(s, new_cap);
    }
    size_t count = 0;
    while (*cstr && count < str_len) {
        s->__data[s->__len] = *cstr;
        s->__len++;
        count++;
        cstr++;
    }
}

char string_index(String* s, size_t index) {
    if (index >= s->__len) {
        fprintf(stderr, "Out of bounds: strlen: %lu, index: %lu", s->__len, index);
        abort();
    }
    return s->__data[index];
}

char* string_index_ref(String* s, size_t index) {
    if (index >= s->__len) {
        fprintf(stderr, "Out of bounds: strlen: %lu, index: %lu", s->__len, index);
        abort();
    }
    return s->__data + index;
}

uint8_t string_eq(void* string1, void* string2) {
    String* s1 = (String*)string1;
    String* s2 = (String*)string2;
    if (s1 == s2)
        return 0;

    size_t len = string_len(s1);
    if (len != string_len(s2))
        return 1;

    for (size_t i = 0; i < len; i++) {
        if (string_index(s1, i) != string_index(s2, i))
            return 1;
    }
    return 0;
}

uint64_t string_hash(void* string) {
    String* s = (String*)string;
    size_t len = string_len(s);
    return fnv_64(s->__data, len);
}

Str string_as_str(String* s) {
    Str str = { .__data=s->__data, .__len=s->__len };
    return str;
}

Str string_trim_whitespace(String* s) {
    Str str = string_as_str(s);
    return str_trim_whitespace(&str);
}

Vec string_split_whitespace(String* s) {
    Str str = string_as_str(s);
    return str_split_whitespace(&str);
}

Vec string_split_lines(String* s) {
    Str str = string_as_str(s);
    return str_split_lines(&str);
}

Vec string_split_by_cstr(String* s, const char* cstr_pattern) {
    Str str = string_as_str(s);
    Str pattern = str_from_cstr(cstr_pattern);
    return str_split_by_str(&str, &pattern);
}

Vec string_split_by_str(String* s, Str* pattern) {
    Str str = string_as_str(s);
    return str_split_by_str(&str, pattern);
}

char* string_as_cstr(String* s) {
    return s->__data;
}

void string_clear(String* s) {
    if (s->__data == NULL)
        return;
    memset(s->__data, '\0', s->__len);
    s->__len = 0;
}

void string_drop(void* string_ptr) {
    String* s = (String*)string_ptr;
    if (s->__data == NULL)
        return;
    free(s->__data);
    s->__len = 0;
    s->__cap = 0;
}


/* ----------- Str -------------- */


Str str_from_cstr(const char* cstr) {
    size_t len = strlen(cstr);
    Str s = { .__data=cstr, .__len=len };
    return s;
}


Str str_from_ptr_len(const char* ptr, size_t len) {
    Str s = { .__data=ptr, .__len=len };
    return s;
}

Str str_trim_whitespace(Str* s) {
    size_t start = 0;
    size_t end = s->__len;
    while (start < end && isspace(str_index(s, start)))
        start++;

    size_t len;
    if (start >= end) {
        len = 0;
    } else {
        if (end > 0)
            end--;
        while (isspace(str_index(s, end)))
            end--;
        len = end - start + 1;
    }

    Str str = { .__data=(s->__data + start), .__len=len };
    return str;
}

size_t str_len(Str* str) {
    return str->__len;
}

Vec str_split_lines(Str* str) {
    const char* ptr = str->__data;
    size_t len = str->__len;
    size_t start = 0;
    size_t end = 0;
    Vec v = vec_new(sizeof(Str));
    while (start <= len) {
        end = start;
        while (end <= len && *(ptr + end) != '\n') {
            end++;
        }
        if (end >= len) {
            end = len;
        }
        Str str = str_from_ptr_len((ptr + start), (end - start));
        vec_push(&v, &str);
        start = end + 1;
    }
    return v;
}

Vec str_split_whitespace(Str* str) {
    const char* ptr = str->__data;
    size_t len = str->__len;
    size_t start = 0;
    size_t end = 0;
    Vec v = vec_new(sizeof(Str));
    while (start < len) {
        while (start < len && isspace(*(ptr + start))) {
            start++;
        }
        if (start >= len) {
            break;
        }
        end = start;
        while (end < len && !isspace(*(ptr + end))) {
            end++;
        }
        Str str = str_from_ptr_len((ptr + start), (end - start));
        vec_push(&v, &str);
        start = end + 1;
    }
    return v;
}

Vec str_split_by_cstr(Str* s, const char* cstr_pattern) {
    Str pattern = str_from_cstr(cstr_pattern);
    return str_split_by_str(s, &pattern);
}

Vec str_split_by_str(Str* s, Str* pattern) {
    const char* ptr = s->__data;
    size_t len = s->__len;
    size_t start = 0;
    size_t end = 0;

    size_t pattern_len = str_len(pattern);
    if (pattern_len == 0) {  /* split each character */
        Vec v = vec_with_capacity(sizeof(Str), len);
        for (size_t i = 0; i < len; i++) {
            Str substr = str_from_ptr_len(ptr + i, 1);
            vec_push(&v, &substr);
        }
        return v;
    }

    Vec v = vec_new(sizeof(Str));
    char pattern_start = str_index(pattern, 0);
    while (start < len) {
        end = start;
        while (end < len && *(ptr + end) != pattern_start) {
            end++;
        }
        if (end >= len) {
            break;
        }

        uint8_t match = 1;  /* match found */
        for (size_t i = 0; i < pattern_len; i++) {
            size_t offset = end + i;
            if (offset >= len) {
                match = 0;
                break;
            }
            if (*(ptr + offset) != str_index(pattern, i)) {
                match = 0;
                break;
            }
        }
        if (match) {
            Str substr = str_from_ptr_len((ptr + start), (end - start));
            vec_push(&v, &substr);
        }
        start = end + 1;
    }
    if (start < len) {
        Str substr = str_from_ptr_len((ptr + start), (end - start));
        vec_push(&v, &substr);
    }
    return v;
}

const char* str_as_ptr(Str* str) {
    return str->__data;
}

String str_to_owned_string(Str* str) {
    String s = string_with_capactiy(str->__len);
    string_push_str(&s, str);
    return s;
}

char str_index(Str* str, size_t ind) {
    if (ind >= str->__len) {
        fprintf(stderr, "Out of bounds: strlen: %lu, index: %lu", str->__len, ind);
        abort();
    }
    return str->__data[ind];
}

const char* str_index_ref(Str* str, size_t ind) {
    if (ind >= str->__len) {
        fprintf(stderr, "Out of bounds: strlen: %lu, index: %lu", str->__len, ind);
        abort();
    }
    return str->__data + ind;
}

uint8_t str_eq(void* str1_, void* str2_) {
    Str* str1 = (Str*)str1_;
    Str* str2 = (Str*)str2_;
    if (str1 == str2)
        return 0;

    size_t len = str_len(str1);
    if (len != str_len(str2))
        return 1;

    for (size_t i = 0; i < len; i++) {
        if (str_index(str1, i) != str_index(str2, i))
            return 1;
    }
    return 0;
}

uint64_t str_hash(void* str_) {
    Str* str = (Str*)str_;
    size_t len = str_len(str);
    return fnv_64((void*)str->__data, len);
}

String read_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        perror("Error opening file");
        abort();
    }
    fseek(f, 0, SEEK_END);
    size_t len = ftell(f);
    rewind(f);
    char* content = malloc((len + 1) * sizeof(char));
    fread(content, sizeof(char), len, f);
    content[len] = '\0';
    fclose(f);
    String s = { .__data=content, .__len=len, .__cap=len };
    return s;
}


/* ----------- Vec ------------- */


Vec vec_new(size_t item_size) {
    Vec v = { .__data=NULL, .__item_size=item_size, .__len=0, .__cap=0 };
    return v;
}

Vec vec_with_capacity(size_t item_size, size_t cap) {
    void* data = malloc(cap * item_size);
    if (data == NULL) {
        fprintf(stderr, "Vec alloc failure\n");
        abort();
    }
    Vec v = { .__data=data, .__item_size=item_size, .__len=0, .__cap=cap };
    return v;
}

Vec vec_copy(Vec* src) {
    size_t len = vec_len(src);
    size_t item_size = src->__item_size;
    Vec v = vec_with_capacity(item_size, len);
    v.__len = len;
    memcpy(v.__data, src->__data, len * item_size);
    return v;
}

size_t vec_len(Vec* v) {
    return v->__len;
}

size_t vec_cap(Vec* v) {
    return v->__cap;
}

void vec_resize(Vec* v, size_t new_cap) {
    if (new_cap == 0)
        new_cap = 16;

    void* data = realloc(v->__data, new_cap * v->__item_size);
    if (data == NULL) {
        fprintf(stderr, "Vec resize failure\n");
        abort();
    }
    v->__data = data;
    v->__cap = new_cap;
}

void __vec_check_resize(Vec* v) {
    size_t avail = v->__cap - v->__len;
    if (avail == 0) {
        size_t new_cap = __inc_cap(v->__cap);
        vec_resize(v, new_cap);
    }
}

void vec_push(Vec* v, void* obj) {
    __vec_check_resize(v);
    char* offset = (char*)v->__data + (v->__len * v->__item_size);
    memcpy((void*)offset, obj, v->__item_size);
    v->__len++;
}

void vec_insert(Vec* v, void* obj, size_t ind) {
    if (ind > v->__len) {
        fprintf(stderr, "Out of bounds (ind > len): veclen: %lu, insert-index: %lu\n", v->__len, ind);
        abort();
    }
    __vec_check_resize(v);
    size_t trailing_elems = v->__len - ind;
    char* insert_ptr = (char*)v->__data + (ind * v->__item_size);
    char* offset_ptr = insert_ptr + v->__item_size;
    memmove(offset_ptr, insert_ptr, (trailing_elems * v->__item_size));
    memcpy((void*)insert_ptr, obj, v->__item_size);
    v->__len++;
}

void vec_remove(Vec* v, size_t ind) {
    if (ind >= v->__len) {
        fprintf(stderr, "Out of bounds (ind >= len): veclen: %lu, insert-index: %lu\n", v->__len, ind);
        abort();
    }
    size_t trailing_elems = v->__len - 1 - ind;
    if (trailing_elems == 0) {
        v->__len--;
    } else {
        char* dest_ptr = (char*)v->__data + (ind * v->__item_size);
        char* src_ptr = dest_ptr + v->__item_size;
        memmove(dest_ptr, src_ptr, (trailing_elems * v->__item_size));
        v->__len--;
    }
}

void vec_remove_with(Vec* v, size_t ind, mapFn drop) {
    if (ind >= v->__len) {
        fprintf(stderr, "Out of bounds (ind >= len): veclen: %lu, insert-index: %lu\n", v->__len, ind);
        abort();
    }
    char* remove_ptr = (char*)v->__data + (ind * v->__item_size);
    drop((void*)remove_ptr);
    vec_remove(v, ind);
}

void* vec_index_ref(Vec* v, size_t ind) {
    if (ind >= v->__len) {
        fprintf(stderr, "Out of bounds: veclen: %lu, index: %lu\n", v->__len, ind);
        abort();
    }
    char* offset = (char*)v->__data + (ind * v->__item_size);
    return (void*)offset;
}

inline void* vec_index_ref_unchecked(Vec* v, size_t ind) {
    char* offset = (char*)v->__data + (ind * v->__item_size);
    return (void*)offset;
}

void vec_iter_ref(Vec* v, mapFn func) {
    size_t len = vec_len(v);
    for (size_t i = 0; i < len; i++) {
        void* ref = vec_index_ref(v, i);
        func(ref);
    }
}

uint8_t vec_eq(void* v1_, void* v2_, cmpEq cmp_func) {
    Vec* v1 = (Vec*)v1_;
    Vec* v2 = (Vec*)v2_;
    if (v1 == v2)
        return 0;

    size_t len = vec_len(v1);
    if (len != vec_len(v2))
        return 1;

    for (size_t i = 0; i < len; i++) {
        void* a = vec_index_ref(v1, i);
        void* b = vec_index_ref(v2, i);
        if (cmp_func(a, b))
            return 1;
    }
    return 0;
}

uint64_t vec_hash_with(void* v, hashFn hash_func) {
    size_t len = vec_len(v);
    uint64_t hash = 17;
    for (size_t i = 0; i < len; i++) {
        void* ref = vec_index_ref(v, i);
        hash = hash * 31 + hash_func(ref);
    }
    return hash;
}

void vec_clear(Vec* v, mapFn drop) {
    if (v->__data == NULL)
        return;
    vec_iter_ref(v, drop);
    v->__len = 0;
}

void vec_drop(void* vec_ptr) {
    Vec* v = (Vec*)vec_ptr;
    if (v->__data == NULL)
        return;
    free(v->__data);
    v->__len = 0;
    v->__cap = 0;
}

void vec_drop_with(Vec* v, mapFn drop) {
    if (v->__data == NULL)
        return;
    vec_iter_ref(v, drop);
    free(v->__data);
    v->__len = 0;
    v->__cap = 0;
}

Slice vec_as_slice(Vec* v) {
    Slice sl = {
        .__data=v->__data,
        .__item_size=v->__item_size,
        .__len=v->__len,
    };
    return sl;
}

SliceIter vec_iter(Vec* v) {
    SliceIter iter = {
        .__data=v->__data,
        .__item_size=v->__item_size,
        .__len=v->__len,
        .__ind=0,
    };
    return iter;
}


/* ----------- Slice ------------- */

void* slice_index_ref(Slice* sl, size_t ind) {
    if (ind >= sl->__len) {
        fprintf(stderr, "Out of bounds: len: %lu, index: %lu\n", sl->__len, ind);
        abort();
    }
    return slice_index_ref_unchecked(sl, ind);
}

inline void* slice_index_ref_unchecked(Slice* sl, size_t ind) {
    char* offset = (char*)sl->__data + (ind * sl->__item_size);
    return (void*)offset;
}

Slice slice_from_ptr_len(size_t item_size, const void* ptr, size_t len) {
    Slice sl = { .__data=ptr, .__item_size=item_size, .__len=len };
    return sl;
}

SliceIter slice_iter(Slice* sl) {
    SliceIter iter = {
        .__data=(void*)sl->__data,
        .__item_size=sl->__item_size,
        .__len=sl->__len,
        .__ind=0,
    };
    return iter;
}

uint8_t slice_iter_done(SliceIter* iter) {
    return iter->__ind < iter->__len ? 1 : 0;
}

void* slice_iter_next(SliceIter* iter) {
    char* ref = (char*)iter->__data + iter->__item_size * iter->__ind;
    iter->__ind++;
    return (void*)ref;
}


/* ----------- HashMap ------------- */

HashMap hashmap_new(size_t key_size, size_t item_size, hashFn hash_func, cmpEq cmp_func, mapFn drop_key, mapFn drop_item) {
    HashMap map = {
        .__buckets=vec_new(sizeof(Vec)),
        .__key_size=key_size,
        .__item_size=item_size,
        .__len=0,
        .__cap=0,
        .__load_factor=0.8,
        .__hash=hash_func,
        .__cmp=cmp_func,
        .__drop_key=drop_key,
        .__drop_item=drop_item,
    };
    return map;
}

HashMap hashmap_with_capacity(size_t key_size, size_t item_size, size_t capacity,
                              hashFn hash_func, cmpEq cmp_func, mapFn drop_key, mapFn drop_item) {
    Vec buckets = vec_with_capacity(sizeof(Vec), capacity);
    for (size_t i = 0; i < capacity; i++) {
        Vec bucket = vec_with_capacity(sizeof(HashMapKV), 1);
        vec_push(&buckets, &bucket);
    }
    HashMap map = {
        .__buckets=buckets,
        .__key_size=key_size,
        .__item_size=item_size,
        .__len=0,
        .__cap=capacity,
        .__load_factor=0.8,
        .__hash=hash_func,
        .__cmp=cmp_func,
        .__drop_key=drop_key,
        .__drop_item=drop_item,
    };
    return map;
}

HashMap hashmap_with_props(size_t key_size, size_t item_size, size_t capacity, double load_factor,
                           hashFn hash_func, cmpEq cmp_func, mapFn drop_key, mapFn drop_item) {
    Vec buckets = vec_with_capacity(sizeof(Vec), capacity);
    for (size_t i = 0; i < capacity; i++) {
        Vec bucket = vec_with_capacity(sizeof(HashMapKV), 1);
        vec_push(&buckets, &bucket);
    }
    HashMap map = {
        .__buckets=vec_with_capacity(sizeof(Vec), capacity),
        .__key_size=key_size,
        .__item_size=item_size,
        .__len=0,
        .__cap=capacity,
        .__load_factor=load_factor,
        .__hash=hash_func,
        .__cmp=cmp_func,
        .__drop_key=drop_key,
        .__drop_item=drop_item,
    };
    return map;
}

size_t hashmap_len(HashMap* map) {
    return map->__len;
}

size_t hashmap_cap(HashMap* map) {
    return map->__cap;
}

void hashmap_drop(HashMap* map) {
    size_t num_buckets = vec_len(&map->__buckets);
    for (size_t bucket_ind = 0; bucket_ind < num_buckets; bucket_ind++) {
        Vec* bucket = vec_index_ref(&map->__buckets, bucket_ind);
        size_t bucket_len = vec_len(bucket);
        for (size_t kv_ind = 0; kv_ind < bucket_len; kv_ind++) {
            HashMapKV* kv_ref = vec_index_ref(bucket, kv_ind);
            map->__drop_key(kv_ref->key);
            map->__drop_item(kv_ref->value);
        }
    }
    vec_drop_with(&map->__buckets, vec_drop);
}

void hashmap_resize(HashMap* map, size_t new_cap) {
    if (new_cap == 0)
        new_cap = 16;

    if (new_cap < map->__cap) {
        fprintf(stderr, "Cannot resize hashmap to be smaller than current capacity. current: %lu, want: %lu\n",
                map->__cap, new_cap);
        abort();
    }

    HashMap new_map = hashmap_with_capacity(map->__key_size, map->__item_size, new_cap,
                                            map->__hash, map->__cmp, map->__drop_key, map->__drop_item);
    HashMapIter iter = hashmap_iter(map);
    while (!hashmap_iter_done(&iter)) {
        HashMapKV* kv_ref = hashmap_iter_next(&iter);
        hashmap_insert_with_hash(&new_map, kv_ref->key, kv_ref->value, kv_ref->hash_key);
    }
    hashmap_drop(map);
    *map = new_map;
    return;
}

void hashmap_insert(HashMap* map, void* key, void* value) {
    size_t hash = map->__hash(key);
    hashmap_insert_with_hash(map, key, value, hash);
}

void hashmap_insert_with_hash(HashMap* map, void* key, void* value, size_t hash) {
    if ((double)map->__len >= map->__load_factor * (double)map->__cap) {
        size_t new_cap = __inc_cap(map->__cap);
        hashmap_resize(map, new_cap);
    }
    Vec* bucket = vec_index_ref(&map->__buckets, hash % vec_len(&map->__buckets));
    size_t bucket_len = vec_len(bucket);
    for (size_t i = 0; i < bucket_len; i++) {
        HashMapKV* kv_ref = vec_index_ref(bucket, i);
        if (map->__cmp(key, kv_ref->key) == 0) {
            map->__drop_key(kv_ref->key);
            map->__drop_item(kv_ref->value);
            memcpy(kv_ref->key, key, map->__key_size);
            memcpy(kv_ref->value, value, map->__item_size);
            return;
        }
    }
    HashMapKV kv = {
        .hash_key=hash,
        .key=key,
        .value=value,
    };
    vec_push(bucket, &kv);
    map->__len++;
}

void* hashmap_get_ref(HashMap* map, void* key) {
    size_t hash = map->__hash(key);
    Vec* bucket = vec_index_ref(&map->__buckets, hash % vec_len(&map->__buckets));
    size_t bucket_len = vec_len(bucket);
    for (size_t i = 0; i < bucket_len; i++) {
        HashMapKV* kv_ref = vec_index_ref(bucket, i);
        if (map->__cmp(key, kv_ref->key) == 0) {
            return kv_ref->value;
        }
    }
    return NULL;
}

HashMapIter hashmap_iter(HashMap* map) {
    size_t bucket_ind = 0;
    if (vec_len(&map->__buckets) > 0) {
        while (1) {
            Vec* bucket = vec_index_ref(&map->__buckets, bucket_ind);
            if (vec_len(bucket) > 0) {
                break;
            }
            bucket_ind++;
        }
    }
    HashMapIter iter = {
        .__map=map,
        .__count=0,
        .__bucket_ind=bucket_ind,
        .__bucket_inner_ind=0,
    };
    return iter;
}

uint8_t hashmap_iter_done(HashMapIter* iter) {
    if (iter->__count >= hashmap_len(iter->__map)) {
        return 1;
    } else {
        return 0;
    }
}

HashMapKV* hashmap_iter_next(HashMapIter* iter) {
    Vec* bucket = vec_index_ref_unchecked(&iter->__map->__buckets, iter->__bucket_ind);
    HashMapKV* kv_ref = vec_index_ref_unchecked(bucket, iter->__bucket_inner_ind);
    iter->__count++;
    iter->__bucket_inner_ind++;
    size_t bucket_len = vec_len(bucket);
    while (iter->__bucket_inner_ind >= bucket_len) {
        iter->__bucket_ind++;
        iter->__bucket_inner_ind = 0;
    }
    return kv_ref;
}

