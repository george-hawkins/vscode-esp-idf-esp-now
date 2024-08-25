#include <cstdio>
#include <cinttypes>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "heap_memory_layout.h"
#include "esp_heap_task_info.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "memory.hpp"

soc_memory_region_t regions[16];

void printBits(size_t const size, void const * const ptr)
{
    unsigned char* b = (unsigned char*)ptr;
    unsigned char byte;
    int i, j;
    
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
}

// From esp-idf:examples/get-started/hello_world/main/hello_world_main.c:heap_caps_init
void foo() {
    size_t num_regions = soc_get_available_memory_region_max_count();
    // TODO: assert num_regions <= sizeof(regions)
    num_regions = soc_get_available_memory_regions(regions);

    for (size_t i = 0; i < num_regions; i++) {
        soc_memory_region_t& region = regions[i];
        const soc_memory_type_desc_t& type = soc_memory_types[region.type];
        if (region.type == -1) {
            continue;
        }

        // This is EXACTLY what you already see in the boot output.
        // BUT the first region seems two get split into a 16KiB and 128KiB in between.
        printf("At %08X len %08X (%d KiB): %s\n", region.start, region.size, region.size / 1024, type.name);

        for (int prio = 0; prio < SOC_MEMORY_TYPE_NO_PRIOS; prio++) {
            auto cap = type.caps[prio];
            if (cap == 0) {
                continue;
            }
            printf("caps: ");
            printBits(sizeof(cap), &cap);
            printf("\n");
        }
    }
}

// From: esp-idf:examples/get-started/hello_world/main/hello_world_main.c
void bar() {
    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // --

    // Often during boot up you see:
    // W (526) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
    // The esp_flash_get_size call returns the smaller of the two.
    // If you want the real physical size...

    if(esp_flash_get_physical_size(NULL, &flash_size) != ESP_OK) {
        printf("Get physical flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s physical flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
}

// ---------------------------------------------------------------------

// Copied from esp-idf:examples/system/heap_task_tracking/main/heap_task_tracking_main.c

#define MAX_TASK_NUM 20                         // Max number of per tasks info that it can store
#define MAX_BLOCK_NUM 20                        // Max number of per block info that it can store

static size_t s_prepopulated_num = 0;
static heap_task_totals_t s_totals_arr[MAX_TASK_NUM];
static heap_task_block_t s_block_arr[MAX_BLOCK_NUM];

static void dump_per_task_heap_info(void)
{
    // TODO: you could use C++ style initialization, i.e. {.caps[0] = }, hmm... will that work, {.foo=xyz} certainly would but {.foo[0]=xyz}, I'm not sure.
    heap_task_info_params_t heap_info;
    heap_info.caps[0] = MALLOC_CAP_8BIT;        // Gets heap with CAP_8BIT capabilities
    heap_info.mask[0] = MALLOC_CAP_8BIT;
    heap_info.caps[1] = MALLOC_CAP_32BIT;       // Gets heap info with CAP_32BIT capabilities
    heap_info.mask[1] = MALLOC_CAP_32BIT;
    heap_info.tasks = NULL;                     // Passing NULL captures heap info for all tasks
    heap_info.num_tasks = 0;
    heap_info.totals = s_totals_arr;            // Gets task wise allocation details
    heap_info.num_totals = &s_prepopulated_num;
    heap_info.max_totals = MAX_TASK_NUM;        // Maximum length of "s_totals_arr"
    heap_info.blocks = s_block_arr;             // Gets block wise allocation details. For each block, gets owner task, address and size
    heap_info.max_blocks = MAX_BLOCK_NUM;       // Maximum length of "s_block_arr"

    heap_caps_get_per_task_info(&heap_info);

    for (int i = 0 ; i < *heap_info.num_totals; i++) {
        printf("Task: %s -> CAP_8BIT: %d CAP_32BIT: %d\n",
                heap_info.totals[i].task ? pcTaskGetName(heap_info.totals[i].task) : "Pre-Scheduler allocs" ,
                heap_info.totals[i].size[0],    // Heap size with CAP_8BIT capabilities
                heap_info.totals[i].size[1]);   // Heap size with CAP32_BIT capabilities
    }

    printf("\n\n");
}

// ---------------------------------------------------------------------

static char task_list_buffer[1024];

void memory_dump() {
    printf("\n----\n\n");

    bar();

    printf("\n----\n\n");

    // Uses uxTaskGetSystemState 
    // vTaskList is only available if CONFIG_FREERTOS_USE_TRACE_FACILITY=y and CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y in sdkconfig.defaults
    // The HWM shows how much smaller you could make a given stack, e.g. HWM=4728 means you could reduce that stack size by 4728 and still survice.
    printf("Task Name\tStatus\tPrio\tHWM\tTask#\n");
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);

    // Way too much info (per block details).
    // heap_caps_dump_all();

    printf("\n----\n\n");

    // I'm not quite sure what this function (defined above) is showing - the tasks don't have their own individual heaps do they? Just their own stacks (covered by vTaskList).
    // Features needed by this fn only available if CONFIG_HEAP_TASK_TRACKING=y in sdkconfig.defaults
    // Is a task associated with each malloc and you can determine who alloc'd how much.
    dump_per_task_heap_info();

    printf("\n----\n\n");

    // The following heap_caps_print_heap_info prints similar info but also with more interesting _free_ info.
    foo();

    printf("\n----\n\n");

    // All heaps support MALLOC_CAP_DEFAULT, MALLOC_CAP_32BIT and MALLOC_CAP_8BIT (grep for MEM_COMMON_CAPS under components/heap/port in ESP-IDF repo).
    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    printf("\n----\n\n");

    // This corresponds to the "Totals" values seen above.
    std::printf("free heap size is %" PRIu32 ", minimum %" PRIu32 "\n", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());

    printf("\n----\n\n");

    // TODO: heap_caps_print_heap_info doesn't print the nice heap name that `foo` doesn, e.g. RTCRAM.
    //  Nice would be "name: RAM, allocated: 128KiB, used: XKiB, free: YKiB, min free: ZKiB, largest free block: QKiB" // Very small largest free block implies Swiss cheese.
    // TODO: work out why `foo` prints out five enties (mysterious 3FCA0000 one) while boot-up sequence and `heap_caps_print_heap_info` only print 4.
    // TODO: create task with big _stack_ - does this fn, i.e. `memory_dump` show who lost the KiBs involved (as well as who gained them, i.e. the new task).
    //  Stacks aren't alloc'd from heap (or are they?!?) so, try and find who loses space to new task stacks and print this total space and how much is used and how much free for further stacks.
    // TODO: call this function from a function that has a huge stack frame, what values reflect the difference between this and calling from a fn with little or no frame?
    // TODO: create a task that mallocs nothing and one that mallocs something, is it just the second one that shows up in dump_per_task_heap_info?
    // TODO: once done, see if you can remove stuff from sdkconfig.defaults
}