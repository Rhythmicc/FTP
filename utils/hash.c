#include "hash.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//建立一个hash表，然后找回指针返回
hash_t hash_alloc(unsigned int buckets, hashfunc_t hash_func) {
    hash_t res = (hash_t)malloc(sizeof(hash));
    res->buckets = buckets;                     //桶的个数
    res->hash_func = hash_func;                 //hash函数
    size_t len = sizeof(hash_node *) * buckets; //数组占用字节数
    res->nodes = (hash_node **)malloc(len);
    memset(res->nodes, 0, len);
    return res;
}

//根据key获取bucket
static hash_node_t*hash_get_bucket(hash_t hash, void *key) {
    unsigned int pos = hash->hash_func(hash->buckets, key); //根据key获取key所在buckets的位置
    assert(pos < hash->buckets);

    return &(hash->nodes[pos]);                             //返回key所在链表地址
}

//根据key获取node结点
static hash_node *hash_get_node_by_key(hash_t hash, void *key, unsigned int key_size) {
    //获取bucket
    hash_node **buck = hash_get_bucket(hash, key);
    assert(buck != NULL);

    //查找元素
    hash_node *node = *buck; //指向(key所在链表的）第一个元素
    while(node != NULL && memcmp(node->key, key, key_size) != 0) node = node->next;

    return node; //包含三种情况 NULL、???
}

//清空bucket
static void hash_clear_bucket(hash_t hash, unsigned int index) {
    assert(index < hash->buckets);        //防止越界
    hash_node *node = hash->nodes[index]; //获得key所在桶第一个元素
    hash_node *tmp = node;
    while(node) {
        tmp = node->next;
        free(node->key);
        free(node->value);
        free(node);
        node = tmp;
    }
    hash->nodes[index] = NULL;
}

/// 根据key查找value
void *hash_lookup_value_by_key(hash_t hash, void *key, unsigned int key_size) {
    //先查找结点
    hash_node *node = hash_get_node_by_key(hash, key, key_size);
    if(node == NULL)return NULL;
    else return node->value;
}

/// 向hash中添加结点
void hash_add_entry(hash_t hash, void *key, unsigned int key_size, void *value, unsigned int value_size){
    hash_node **buck = hash_get_bucket(hash, key);
    assert(buck != NULL);

    hash_node *node = (hash_node *)malloc(sizeof(hash_node));
    memset(node, 0, sizeof(hash_node));
    node->key = malloc(key_size);
    node->value = malloc(value_size);
    memcpy(node->key, key, key_size);
    memcpy(node->value, value, value_size); 

    if(*buck != NULL) {
        hash_node *head = *buck;
        node->next = head;
        head->pre = node;
        *buck = node;
    }
    else *buck = node;
}

/// hash中释放结点
void hash_free_entry(hash_t hash, void *key, unsigned int key_size) {
    //查找结点
    hash_node *node = hash_get_node_by_key(hash, key, key_size);
    assert(node != NULL);

    free(node->key);
    free(node->value);

    //删除结点
    if(node->pre != NULL) node->pre->next = node->next;
    else {
        hash_node **buck = hash_get_bucket(hash, key);
        *buck = node->next;
    }
    if(node->next != NULL) node->next->pre = node->pre;
    free(node);
}

//清空hash表
void hash_clear_entry(hash_t hash) {
    for(unsigned int i = 0; i < hash->buckets; ++i) {
        hash_clear_bucket(hash, i);
        hash->nodes[i] = NULL;
    }
}

//销毁hash表
void hash_destroy(hash_t hash) {
    hash_clear_entry(hash);
    free(hash->nodes);
    free(hash);
}
