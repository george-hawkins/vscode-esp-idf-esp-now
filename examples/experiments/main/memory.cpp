#include <cstdio>
#include <cinttypes>
#include <cstring>
#include <array>
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "heap_memory_layout.h"
#include "esp_heap_task_info.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "utils.hpp"
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
void dump_soc_memory_regions() {
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

// Derived from: esp-idf:examples/get-started/hello_world/main/hello_world_main.c
void dump_mcu_details() {
    esp_chip_info_t chip_info;

    esp_chip_info(&chip_info);

    std::uint16_t major_rev = chip_info.revision / 100;
    std::uint16_t minor_rev = chip_info.revision % 100;

    std::printf("MCU: %s chip with %u core(s), silicon revision v%u.%u\n", CONFIG_IDF_TARGET, chip_info.cores, major_rev, minor_rev);
    std::printf("Features:\n");

    if (chip_info.features & CHIP_FEATURE_WIFI_BGN) {
        std::printf("- WiFi\n");
    }
    if (chip_info.features & CHIP_FEATURE_BLE) {
        std::printf("- BLE\n");
    }
    if (chip_info.features & CHIP_FEATURE_BT) {
        std::printf("- BT\n");
    }

    std::uint32_t physical_flash_size;
    ESP_ERROR_CHECK(esp_flash_get_physical_size(NULL, &physical_flash_size));

    std::uint32_t configured_flash_size;
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &configured_flash_size));

    const char* flash_type = chip_info.features & CHIP_FEATURE_EMB_FLASH ? "embedded" : "external";

    // Often during boot up you see:
    // W (526) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k).
    // Here we print the physical flash size and the flash size configured into the binary image header flashed to the board.
    std::printf("Flash: %s %s (of which %s configured)\n", human_readable_byte_count(physical_flash_size).c_str(), flash_type, human_readable_byte_count(configured_flash_size).c_str());
}

// ---------------------------------------------------------------------


static constexpr size_t MAX_TASKS = 16;

// Derived from esp-idf:examples/system/heap_task_tracking/main/heap_task_tracking_main.c
static void dump_per_task_heap_info(void)
{
    heap_task_info_params_t heap_info;

    std::memset(heap_info.caps, 0, sizeof(heap_info.caps));
    std::memset(heap_info.mask, 0, sizeof(heap_info.mask));

    heap_info.caps[0] = MALLOC_CAP_8BIT;
    heap_info.mask[0] = MALLOC_CAP_8BIT;

    heap_info.num_tasks = 0;
    heap_info.tasks = NULL;

    heap_info.max_blocks = 0;
    heap_info.blocks = NULL;

    heap_task_totals_t totals[MAX_TASKS];

    heap_info.max_totals = std::size(totals);
    heap_info.totals = totals;

    // If num_totals isn't initialized to 0 heap_caps_get_per_task_info will probably corrupt memory and won't return the actual number of tasks.
    size_t num_totals = 0;
    heap_info.num_totals = &num_totals;

    heap_caps_get_per_task_info(&heap_info);

    std::printf("%16s | Allocated\n", "Task");

    for (int i = 0 ; i < num_totals; i++) {
        const auto& total = totals[i];
        const std::string amount = human_readable_byte_count(total.size[0]);
        const char* task_name = total.task != nullptr ? pcTaskGetName(total.task) : "pre-scheduler";

        std::printf("%16s | %9s\n", task_name, amount.c_str());
    }
}

// ---------------------------------------------------------------------

// Dump how close each task has come to overflowing its stack.
// If the high water mark for a task looks stable you could reduce its stack size by some value close to this amount.
void dump_task_high_water_marks() {
    const auto task_count = uxTaskGetNumberOfTasks();

    CHECK(task_count <= MAX_TASKS);

    std::array<TaskStatus_t, MAX_TASKS> task_statuses;

    const auto retrieved_count = uxTaskGetSystemState(task_statuses.data(), task_statuses.size(), nullptr);

    for (auto i = 0; i < retrieved_count; i++) {
        const auto& status = task_statuses[i];

        // It would also be nice to print out the maximum size limit and the current size of the stack.
        // But this is less trivial than it might seem, e.g. see https://stackoverflow.com/a/77568267/245602
        printf("%s - hwm=%" PRIu32 "B\n", status.pcTaskName, status.usStackHighWaterMark);
    }
}

// ---------------------------------------------------------------------

static void dump_heap_details(const char* name, std::uint32_t caps) {
    std::size_t total_size = heap_caps_get_total_size(caps);

    if (total_size == 0) {
        return;
    }

    multi_heap_info_t info;
    bool print_largest_free_block = true;

    heap_caps_get_info(&info, caps);

    // On the C3, both the standard RAM and the retension RAM support DMA. So some special case logic is needed...
    if (caps == MALLOC_CAP_DMA) {
        std::size_t ret_total_size = heap_caps_get_total_size(MALLOC_CAP_RETENTION);

        if (ret_total_size > 0) {
            total_size -= ret_total_size;

            multi_heap_info_t ret_info;

            heap_caps_get_info(&ret_info, MALLOC_CAP_RETENTION);

            info.total_free_bytes -= ret_info.total_free_bytes;
            info.total_allocated_bytes -= ret_info.total_allocated_bytes;
            info.minimum_free_bytes -= ret_info.minimum_free_bytes;
            info.allocated_blocks -= ret_info.allocated_blocks;
            info.free_blocks -= ret_info.free_blocks;
            info.total_blocks -= ret_info.total_blocks;

            if (info.largest_free_block == ret_info.largest_free_block) {
                // Unfortunately, the largest block was in the retention RAM so, we don't have a value for standard RAM.
                print_largest_free_block = false;
            }
        }
    }

    // If the largest free contiguous block is much smaller than the total free space then the heap has become Swiss cheese.
    std::printf("%s - size %u, used: %u, free: %u, min free: %u", name, total_size, info.total_allocated_bytes, info.total_free_bytes, info.minimum_free_bytes);

    if (print_largest_free_block) {
        std::printf(", largest free contiguous block: %u\n", info.largest_free_block);
    } else {
        std::printf("\n");
    }
}

static size_t get_total_allocated_size() {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

    return info.total_allocated_bytes;

}

void dump_heap_details_by_type() {
    dump_heap_details("RAM", MALLOC_CAP_DMA);
    dump_heap_details("Retention RAM", MALLOC_CAP_RETENTION);
    dump_heap_details("RTCRAM", MALLOC_CAP_RTCRAM);
    dump_heap_details("SPIRAM", MALLOC_CAP_SPIRAM);
    dump_heap_details("TCM", MALLOC_CAP_TCM);

    auto total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    auto used = get_total_allocated_size();
    std::printf("Totals - size: %u, used: %u, free: %lu, min free: %lu\n", total, used, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
}

// ---------------------------------------------------------------------

static char task_list_buffer[1024];

void memory_dump() {
    printf("\n----\n\n");

    dump_mcu_details();

    printf("\n----\n\n");

    // SOC_MAX_CONTIGUOUS_RAM_SIZE determines the max stack size - see esp-idf:components/heap/heap_private.h
    printf("Max stack size: %s\n", human_readable_byte_count(SOC_MAX_CONTIGUOUS_RAM_SIZE).c_str());

    printf("\n----\n\n");

    dump_heap_details_by_type();

    printf("\n----\n\n");

    // I'm not quite sure what this function (defined above) is showing - the tasks don't have their own individual heaps do they? Just their own stacks (covered by vTaskList).
    // Features needed by this fn only available if CONFIG_HEAP_TASK_TRACKING=y in sdkconfig.defaults
    // Is a task associated with each malloc and you can determine who alloc'd how much.
    dump_per_task_heap_info();

    printf("\n----\n\n");

    dump_task_high_water_marks();

    printf("\n----\n\n");
    printf("\n----\n\n");

    // All heaps support MALLOC_CAP_DEFAULT, MALLOC_CAP_32BIT and MALLOC_CAP_8BIT (grep for MEM_COMMON_CAPS under components/heap/port in ESP-IDF repo).
    heap_caps_print_heap_info(MALLOC_CAP_DEFAULT);

    printf("\n----\n\n");

    // TODO: heap_caps_print_heap_info doesn't print the nice heap name that `dump_soc_memory_regions` doesn, e.g. RTCRAM.
    //  Nice would be "name: RAM, allocated: 128KiB, used: XKiB, free: YKiB, min free: ZKiB, largest free contiguous block: QKiB" // Very small largest free block implies Swiss cheese.
    // TODO: work out why `dump_soc_memory_regions` prints out five enties (mysterious 3FCA0000 one) while boot-up sequence and `heap_caps_print_heap_info` only print 4.
    //  As the code is the same for `dump_soc_memory_regions` and boot sequence, see how it can possibly come out with a different result (especiall when heap_caps_print_heap_info comes out with the same result).
    // TODO: create task with big _stack_ - does this fn, i.e. `memory_dump` show who lost the KiBs involved (as well as who gained them, i.e. the new task).
    //  Stacks aren't alloc'd from heap (or are they?!?) so, try and find who loses space to new task stacks and print this total space and how much is used and how much free for further stacks.
    // TODO: call this function from a function that has a huge stack frame, what values reflect the difference between this and calling from a fn with little or no frame?
    // TODO: create a task that mallocs nothing and one that mallocs something, is it just the second one that shows up in dump_per_task_heap_info?
    // TODO: once done, see if you can remove stuff from sdkconfig.defaults
    // TODO: the flash RAM info printed out by `bar` isn't that interesting (it's essentially fixed at compile time) but is it also possible to print out free flash
    //  And how much e.g. used for NVS. Some of these details surely already appear in boot sequence.

    printf("\n----\n\n");

    // The following heap_caps_print_heap_info prints similar info but also with more interesting _free_ info.
    dump_soc_memory_regions();

    printf("\n----\n\n");

    // Uses uxTaskGetSystemState 
    // vTaskList is only available if CONFIG_FREERTOS_USE_TRACE_FACILITY=y and CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y in sdkconfig.defaults
    // The HWM shows how much smaller you could make a given stack, e.g. HWM=4728 means you could reduce that stack size by 4728 and still survice.
    printf("Task Name\tStatus\tPrio\tHWM\tTask#\n");
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);

    // Way too much info (per block details).
    // heap_caps_dump_all();

    // -----------------------------------------------------------------

    // When you do a build, you see something like this in the _Terminal_ tab:
    // 
    //  Total sizes:
    //  Used stat D/IRAM:  114712 bytes ( 206584 remain, 35.7% used)
    //        .data size:   11608 bytes
    //        .bss  size:   16784 bytes
    //        .text size:   86320 bytes
    //  Used Flash size :  645964 bytes
    //             .text:  519320 bytes
    //           .rodata:  126388 bytes
    //  Total image size:  743892 bytes (.bin may be padded larger)
    //
    // During boot you see the .text values:
    //
    // I (79)  esp_image: segment 0: paddr=00010020 vaddr=3c080020 size=1eeb4h (126644) map
    // I (114) esp_image: segment 1: paddr=0002eedc vaddr=3fc95200 size=0113ch (  4412) load
    // I (115) esp_image: segment 2: paddr=00030020 vaddr=42000020 size=7ec98h (519320) map <-- Flash .text size
    // I (201) esp_image: segment 3: paddr=000aecc0 vaddr=3fc9633c size=01c1ch (  7196) load
    // I (203) esp_image: segment 4: paddr=000b08e4 vaddr=40380000 size=15130h ( 86320) load <- D/IRAM .text size
    //
    // I don't know why above it only give a remain/used figure for D/IRAM.
    // It would be nice to see similar values or Flash. You actually see this when building - once the .bin file is produced, checkSize 
    // Oddly, the largest binary size seems to be 1MiB (even if you set _Flash size_ to 4MiB).
    // At the end of building the .bin file, the build process runs `check_size.py`.
    // You can do the same:
    //
    // $ cd .../build
    // $ ls *.bin
    // experiments.bin
    // $ printf '0x%x\n' $(stat --printf='%s' *.bin)
    // 0x10fd90
    // $ check_sizes.py --offset 0x8000 partition --type app partition_table/partition-table.bin experiments.bin
    // Error: app partition is too small for binary experiments.bin size 0x10fd90:
    //   - Part 'factory' 0/0 @ 0x10000 size 0x100000 (overflow 0xfd90)
    //
    // Above the .bin was 0xfd90 bytes over the 1MiB limit. If all was well you'd get something like this:
    //
    // $ printf '0x%x\n' $(stat --printf='%s' *.bin)
    // 0xb5a40
    // $ check_sizes.py --offset 0x8000 partition --type app partition_table/partition-table.bin experiments.bin
    // experiments.bin binary size 0xb5a40 bytes. Smallest app partition is 0x100000 bytes. 0x4a5c0 bytes (29%) free.
}
