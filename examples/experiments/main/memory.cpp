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
#include <clocale>

#include "utils.hpp"
#include "memory.hpp"

using namespace merkur;

static constexpr std::size_t MAX_REGIONS = 8;

// From esp-idf:examples/get-started/hello_world/main/hello_world_main.c:heap_caps_init
//
// You see this information (without the caps) during startup:
//
// I (500) heap_init: At 3FC9BCB0 len 00024350 (144 KiB): RAM
// I (506) heap_init: At 3FCC0000 len 0001C710 (113 KiB): Retention RAM
// I (513) heap_init: At 3FCDC710 len 00002950 (10 KiB): Retention RAM
// I (520) heap_init: At 50000010 len 00001FD8 (7 KiB): RTCRAM
//
// And you can find the caps in a more easy to read form in the memory_layout.c file for the relevant chip.
// E.g. the following comes from `esp-idf:components/heap/port/esp32c3/memory_layout.c`:
//
// const soc_memory_type_desc_t soc_memory_types[SOC_MEMORY_TYPE_NUM] = {
//     /*                                   Mem Type Name   | High Priority Matching                  | Medium Priorty Matching                 | Low Priority Matching */
//     [SOC_MEMORY_TYPE_RAM]           = { "RAM",           { ESP32C3_MEM_COMMON_CAPS | MALLOC_CAP_DMA, 0 ,                                       0}},
//     [SOC_MEMORY_TYPE_RETENTION_RAM] = { "Retention RAM", { MALLOC_CAP_RETENTION,                     ESP32C3_MEM_COMMON_CAPS | MALLOC_CAP_DMA, 0}},
//     [SOC_MEMORY_TYPE_RTCRAM]        = { "RTCRAM",        { MALLOC_CAP_RTCRAM,                        0,                                        ESP32C3_MEM_COMMON_CAPS }},
// };
//
// ...
//
// const soc_memory_region_t soc_memory_regions[] = {
//     { 0x3FC80000,           0x20000,                                   SOC_MEMORY_TYPE_RAM,             0x40380000, false}, //D/IRAM level1, can be used as trace memory
//     { 0x3FCA0000,           0x20000,                                   SOC_MEMORY_TYPE_RAM,             0x403A0000, false}, //D/IRAM level2, can be used as trace memory
//     { 0x3FCC0000,           (APP_USABLE_DRAM_END-0x3FCC0000),          SOC_MEMORY_TYPE_RETENTION_RAM,   0x403C0000, false}, //D/IRAM level3, backup dma accessible, can be used as trace memory
//     { APP_USABLE_DRAM_END,  (SOC_DIRAM_DRAM_HIGH-APP_USABLE_DRAM_END), SOC_MEMORY_TYPE_RETENTION_RAM,   MAP_DRAM_TO_IRAM(APP_USABLE_DRAM_END), true}, //D/IRAM level3, backup dma accessible, can be used as trace memory (ROM reserved area)
// #ifdef CONFIG_ESP_SYSTEM_ALLOW_RTC_FAST_MEM_AS_HEAP
//     { 0x50000000,           0x2000,                                    SOC_MEMORY_TYPE_RTCRAM,          0, false}, //Fast RTC memory
// #endif
// };
//
void dump_soc_memory_regions() {
    soc_memory_region_t regions[MAX_REGIONS];
    std::size_t num_regions = soc_get_available_memory_region_max_count();
    CHECK(MAX_REGIONS <= num_regions);
    num_regions = soc_get_available_memory_regions(regions);

    for (std::size_t i = 0; i < num_regions; i++) {
        soc_memory_region_t& region = regions[i];
        const soc_memory_type_desc_t& type = soc_memory_types[region.type];
        if (region.type == -1) {
            continue;
        }

        std::printf("At %08x size: %7s - %s\n", region.start, fmt::byte_count(region.size).c_str(), type.name);

        for (int prio = 0; prio < SOC_MEMORY_TYPE_NO_PRIOS; prio++) {
            auto cap = type.caps[prio];
            if (cap == 0) {
                continue;
            }
            std::printf("caps: %s\n", fmt::bits(cap).c_str());
        }
    }
}

// Derived from: esp-idf:examples/get-started/hello_world/main/hello_world_main.c
//
// You see this information and more during startup:
//
// I (16) boot: chip revision: v0.4
// I (19) boot.esp32c3: SPI Speed      : 80MHz
// I (24) boot.esp32c3: SPI Mode       : DIO
// I (28) boot.esp32c3: SPI Flash Size : 2MB
// ...
// I (439) cpu_start: Unicore app
// I (447) cpu_start: Pro cpu start user code
// I (447) cpu_start: cpu freq: 160000000 Hz
// I (448) cpu_start: Application information:
// I (450) cpu_start: Project name:     experiments
// I (456) cpu_start: App version:      44b5e13-dirty
// I (461) cpu_start: Compile time:     Sep  2 2024 19:59:58
// I (467) cpu_start: ELF file SHA256:  3239d3f1f...
// I (473) cpu_start: ESP-IDF:          v5.2.2-dirty
// I (478) cpu_start: Min chip rev:     v0.3
// I (483) cpu_start: Max chip rev:     v1.99 
// I (488) cpu_start: Chip rev:         v0.4
//
// W (534) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k). Using the size in the binary image header.
//
// Note in particular that you see the project name and git SHA (plus "-dirty" if there are uncommitted changes).
//
void dump_mcu_details() {
    esp_chip_info_t chip_info;

    esp_chip_info(&chip_info);

    std::uint16_t major_rev = chip_info.revision / 100;
    std::uint16_t minor_rev = chip_info.revision % 100;

    std::printf("MCU: %s with %u core(s), silicon revision v%u.%u\n", CONFIG_IDF_TARGET, chip_info.cores, major_rev, minor_rev);

    std::uint32_t physical_flash_size;
    ESP_ERROR_CHECK(esp_flash_get_physical_size(NULL, &physical_flash_size));

    std::uint32_t configured_flash_size;
    ESP_ERROR_CHECK(esp_flash_get_size(NULL, &configured_flash_size));

    const char* flash_type = chip_info.features & CHIP_FEATURE_EMB_FLASH ? "embedded" : "external";

    // Often during boot up you see:
    // W (526) spi_flash: Detected size(4096k) larger than the size in the binary image header(2048k).
    // Here we print the physical flash size and the flash size configured into the binary image header flashed to the board.
    std::printf("Flash: %s %s (of which %s configured)\n", fmt::byte_count(physical_flash_size).c_str(), flash_type, fmt::byte_count(configured_flash_size).c_str());
}

// ---------------------------------------------------------------------


static constexpr std::size_t MAX_TASKS = 16;

// Dump the amount of space each task has allocat
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

    // num_totals is both an input and an output parameter - if it's left uninitialized and ends up with a non-zero
    // value then the actual number of tasks won't be returned and if its value is larger than max_totals it'll
    // result in memory corruption (when heap_caps_get_per_task_info tries to clear the totals array).
    std::size_t num_totals = 0;
    heap_info.num_totals = &num_totals;

    heap_caps_get_per_task_info(&heap_info);

    std::printf("          Task | Allocated\n");

    for (int i = 0 ; i < num_totals; i++) {
        const auto& total = totals[i];
        const char* task_name = total.task != nullptr ? pcTaskGetName(total.task) : "pre-scheduler";

        std::printf("%14s | %7s\n", task_name, fmt::thousands(total.size[0]).c_str());
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

    std::printf("          Task | High water mark\n");

    for (auto i = 0; i < retrieved_count; i++) {
        const auto& status = task_statuses[i];

        // It would also be nice to print out the maximum size limit and the current size of the stack.
        // But this is less trivial than it might seem, e.g. see https://stackoverflow.com/a/77568267/245602
        printf("%14s | %7s\n", status.pcTaskName, fmt::thousands(status.usStackHighWaterMark).c_str());
    }
}

// ---------------------------------------------------------------------

// If the largest free contiguous block is much smaller than the total free space then the heap has become Swiss cheese.
static void print_heap_details(const char* name, std::size_t total_size, std::size_t used, std::size_t free_size, std::size_t min_free, const std::string& max_free_block) {
    std::printf(
        "%14s | %7s | %7s | %7s | %8s | %7s\n",
        name,
        fmt::thousands(total_size).c_str(),
        fmt::thousands(used).c_str(),
        fmt::thousands(free_size).c_str(),
        fmt::thousands(min_free).c_str(),
        max_free_block.c_str()
    );
}

static void dump_heap_details(const char* name, std::uint32_t caps) {
    std::size_t total_size = heap_caps_get_total_size(caps);

    if (total_size == 0) {
        return;
    }

    multi_heap_info_t info;
    bool has_max_free = true;

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
                has_max_free = false;
            }
        }
    }

    const std::string max_free_block = has_max_free ? fmt::thousands(info.largest_free_block) : "unknown";

    print_heap_details(name, total_size, info.total_allocated_bytes, info.total_free_bytes, info.minimum_free_bytes, max_free_block);
}

static std::size_t get_total_allocated_size() {
    multi_heap_info_t info;
    heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

    return info.total_allocated_bytes;

}

void dump_heap_details_by_type() {
    std::printf("          Type |    Size |    Used |    Free | Min free | Max free block\n");

    dump_heap_details("RAM", MALLOC_CAP_DMA);
    dump_heap_details("Retention RAM", MALLOC_CAP_RETENTION);
    dump_heap_details("RTCRAM", MALLOC_CAP_RTCRAM);
    dump_heap_details("SPIRAM", MALLOC_CAP_SPIRAM);
    dump_heap_details("TCM", MALLOC_CAP_TCM);

    auto total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    print_heap_details("Totals", total, get_total_allocated_size(), esp_get_free_heap_size(), esp_get_minimum_free_heap_size(), "");
}

// ---------------------------------------------------------------------

void dump_max_supported_stack_size() {
    // SOC_MAX_CONTIGUOUS_RAM_SIZE determines the stack size upper bound - see esp-idf:components/heap/heap_private.h
    printf("Max supported stack size: %s\n", fmt::byte_count(SOC_MAX_CONTIGUOUS_RAM_SIZE).c_str());
}

// ---------------------------------------------------------------------

void memory_dump() {
    printf("\n----\n\n");

    dump_mcu_details();
    dump_max_supported_stack_size();

    printf("\n----\n\n");

    dump_task_high_water_marks();

    printf("\n----\n\n");

    // I'm not quite sure what this function (defined above) is showing - the tasks don't have their own individual heaps do they? Just their own stacks (covered by vTaskList).
    // Features needed by this fn only available if CONFIG_HEAP_TASK_TRACKING=y in sdkconfig.defaults
    // Is a task associated with each malloc and you can determine who alloc'd how much.
    dump_per_task_heap_info();

    printf("\n----\n\n");

    dump_heap_details_by_type();

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

    // The following heap_caps_print_heap_info prints similar info but also with more interesting _free_ info.
    dump_soc_memory_regions();

    printf("\n----\n\n");

    // Uses uxTaskGetSystemState 
    // vTaskList is only available if CONFIG_FREERTOS_USE_TRACE_FACILITY=y and CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y in sdkconfig.defaults
    // The HWM shows how much smaller you could make a given stack, e.g. HWM=4728 means you could reduce that stack size by 4728 and still survice.
    printf("Task Name\tStatus\tPrio\tHWM\tTask#\n");
    char task_list_buffer[1024];
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
    // This is produced with `idf.py size` and, along with `idf.py size-components` is discussed here in the _Minimizing binary size_ page:
    // <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/performance/size.html>
    //
    // During boot you see the .text values:
    //
    // I (79)  esp_image: segment 0: paddr=00010020 vaddr=3c080020 size=1eeb4h (126644) map
    // I (114) esp_image: segment 1: paddr=0002eedc vaddr=3fc95200 size=0113ch (  4412) load
    // I (115) esp_image: segment 2: paddr=00030020 vaddr=42000020 size=7ec98h (519320) map <-- Flash .text size
    // I (201) esp_image: segment 3: paddr=000aecc0 vaddr=3fc9633c size=01c1ch (  7196) load
    // I (203) esp_image: segment 4: paddr=000b08e4 vaddr=40380000 size=15130h ( 86320) load <- D/IRAM .text size
    //
    // You can see the above output discribed in the _Application image structures_ page:
    // <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/app_image_format.html>
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
