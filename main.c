#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
#include <string.h>

struct Arena
{
    uint8_t *memory;
    size_t offset;
    size_t size;
};

struct Arena ArenaAlloc(size_t size)
{
    struct Arena arena = {0};
    arena.memory = calloc(size, sizeof(uint8_t));
    arena.size = size;
    return arena;
}

void ArenaFree(struct Arena *arena)
{
    assert(arena != NULL);
    free(arena->memory);
    *arena = (struct Arena){0};
}

void *ArenaAllocInto(struct Arena *arena, size_t size)
{
    assert(arena != NULL);
    // Keep the offset aligned to increments of sizeof(size_t), ie: 64 bits on a 64-bit CPU.
    size_t alignedSize = (size + sizeof(size_t) - 1) / sizeof(size_t) * sizeof(size_t);
    assert(arena->offset + alignedSize < arena->size);
    uint8_t *destination = arena->memory + arena->offset;
    memset(destination, 0, size);
    arena->offset += alignedSize;
    return (void *)destination;
}

void ArenaClear(struct Arena *arena)
{
    assert(arena != NULL);
    memset(arena->memory, 0, arena->size);
    arena->offset = 0;
}

void ArenaPrint(struct Arena *arena)
{
    printf("Arena:\nmemory:%p\noffset:%zd\nsize:%zd\n",
           arena->memory, arena->offset, arena->size);
}

struct TestStruct1
{
    int32_t value;
};

struct TestStruct2
{
    int64_t value1;
    int64_t value2;
};

struct TestStruct3
{
    float x;
    float y;
    double z;
};

int main(void)
{
    printf("Hello, from ArenaAgain!\n");

    struct Arena arena = ArenaAlloc(256);
    ArenaPrint(&arena);

    struct TestStruct1 *struct1 = ArenaAllocInto(&arena, sizeof(struct TestStruct1));
    printf("struct1 allocated at offset: %lld\n", (uint8_t*)struct1 - arena.memory);
    struct TestStruct2 *struct2 = ArenaAllocInto(&arena, sizeof(struct TestStruct2));
    printf("struct2 allocated at offset: %lld\n", (uint8_t*)struct2 - arena.memory);
    struct TestStruct2 *struct3 = ArenaAllocInto(&arena, sizeof(struct TestStruct3));
    printf("struct3 allocated at offset: %lld\n", (uint8_t*)struct3 - arena.memory);

    *struct2 = (struct TestStruct2){
        .value1 = 6,
        .value2 = 7,
    };

    printf("struct2 after allocation: value 1 = %lld, value 2 = %lld\n", struct2->value1, struct2->value2);
    ArenaClear(&arena);
    printf("struct2 after clear     : value 1 = %lld, value 2 = %lld\n", struct2->value1, struct2->value2);
    ArenaPrint(&arena);

    ArenaFree(&arena);

    printf("Were memory leaks found? CRTDBG: %d\n", _CrtDumpMemoryLeaks());
}
