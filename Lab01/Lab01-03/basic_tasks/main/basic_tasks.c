#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>


#define LED1_PIN GPIO_NUM_2
#define LED2_PIN GPIO_NUM_4

static const char *TAG = "BASIC_TASKS";

// --- Exercise 2: Shared resource and Mutex ---
volatile int shared_counter = 0;
SemaphoreHandle_t shared_counter_mutex;

// Helper function to convert task state enum to string
const char* task_state_to_string(eTaskState state) {
    switch (state) {
        case eRunning: return "Running";
        case eReady: return "Ready";
        case eBlocked: return "Blocked";
        case eSuspended: return "Suspended";
        case eDeleted: return "Deleted";
        default: return "Invalid";
    }
}

// Task function for LED1 (Step 1)
void led1_task(void *pvParameters)
{
    int *task_id = (int *)pvParameters;
    ESP_LOGI(TAG, "LED1 Task started with ID: %d", *task_id);
    
    while (1) {
        gpio_set_level(LED1_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_set_level(LED1_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// Task function for LED2 (Step 1)
void led2_task(void *pvParameters)
{
    char *task_name = (char *)pvParameters;
    ESP_LOGI(TAG, "LED2 Task started: %s", task_name);
    
    while (1) {
        for (int i = 0; i < 5; i++) {
            gpio_set_level(LED2_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(LED2_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task for displaying system statistics (Step 1)
void system_info_task(void *pvParameters)
{
    ESP_LOGI(TAG, "System Info Task started");
    
    while (1) {
        ESP_LOGI(TAG, "=== System Information ===");
        ESP_LOGI(TAG, "Free heap: %u bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Min free heap: %u bytes", esp_get_minimum_free_heap_size());
        ESP_LOGI(TAG, "Number of tasks: %u", uxTaskGetNumberOfTasks());
        
        TickType_t uptime = xTaskGetTickCount();
        uint32_t uptime_sec = uptime * portTICK_PERIOD_MS / 1000;
        ESP_LOGI(TAG, "Uptime: %u seconds", uptime_sec);
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Task for controlling other tasks (Step 2)
void task_manager(void *pvParameters)
{
    ESP_LOGI(TAG, "Task Manager started");
    
    TaskHandle_t *handles = (TaskHandle_t *)pvParameters;
    TaskHandle_t led1_handle = handles[0];
    TaskHandle_t led2_handle = handles[1];
    
    int command_counter = 0;
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(8000)); // Wait before starting the cycle
        command_counter++;
        
        switch (command_counter % 6) {
            case 1:
                ESP_LOGW(TAG, "Manager: Suspending LED1");
                vTaskSuspend(led1_handle);
                break;
            case 2:
                ESP_LOGW(TAG, "Manager: Resuming LED1");
                vTaskResume(led1_handle);
                break;
            case 3:
                ESP_LOGW(TAG, "Manager: Suspending LED2");
                vTaskSuspend(led2_handle);
                break;
            case 4:
                ESP_LOGW(TAG, "Manager: Resuming LED2");
                vTaskResume(led2_handle);
                break;
            case 5:
                ESP_LOGW(TAG, "Manager: Getting task info");
                eTaskState led1_state = eTaskGetState(led1_handle);
                eTaskState led2_state = eTaskGetState(led2_handle);
                ESP_LOGI(TAG, "LED1 State: %s", task_state_to_string(led1_state));
                ESP_LOGI(TAG, "LED2 State: %s", task_state_to_string(led2_state));
                break;
            case 0:
                ESP_LOGW(TAG, "Manager: Reset cycle");
                break;
        }
    }
}

// Task for testing priorities (Step 3)
void high_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "High Priority Task started");
    while (1) {
        ESP_LOGE(TAG, "HIGH PRIORITY TASK RUNNING!");
        for (volatile int i = 0; i < 1000000; i++) {}
        ESP_LOGE(TAG, "High priority task yielding");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void low_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Low Priority Task started");
    while (1) {
        ESP_LOGI(TAG, "Low priority task running...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task for displaying runtime statistics (Step 3)
void runtime_stats_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Runtime Stats Task started");
    char *buffer = malloc(2048);
    if (buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer for runtime stats");
        vTaskDelete(NULL); // Delete self if malloc fails
    }
    
    while (1) {
        printf("\n--- Runtime Stats ---\n");
        vTaskGetRunTimeStats(buffer);
        printf("%s\n", buffer);
        
        printf("--- Task List ---\n");
        printf("Name\t\tState\tPrio\tStack\tNum\n");
        vTaskList(buffer);
        printf("%s\n", buffer);
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    // The free() call was here, but it's unreachable in an infinite loop.
    // The memory is reclaimed when the task is deleted or the device reboots.
}

// Task for self-deletion (Exercise 1)
void temporary_task(void *pvParameters)
{
    int *duration = (int *)pvParameters;
    ESP_LOGI(TAG, "Temporary task will run for %d seconds", *duration);
    
    for (int i = *duration; i > 0; i--) {
        ESP_LOGI(TAG, "Temporary task countdown: %d", i);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    ESP_LOGW(TAG, "Temporary task self-deleting");
    vTaskDelete(NULL);
}

// Producer task (Exercise 2) - Now with Mutex
void producer_task(void *pvParameters)
{
    while (1) {
        if (xSemaphoreTake(shared_counter_mutex, portMAX_DELAY) == pdTRUE) {
            shared_counter++;
            ESP_LOGI(TAG, "Producer: counter = %d", shared_counter);
            xSemaphoreGive(shared_counter_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Consumer task (Exercise 2) - Now with Mutex
void consumer_task(void *pvParameters)
{
    int last_value = -1;
    while (1) {
        int current_value = -1;
        if (xSemaphoreTake(shared_counter_mutex, portMAX_DELAY) == pdTRUE) {
            current_value = shared_counter;
            xSemaphoreGive(shared_counter_mutex);
        }

        if (current_value != last_value) {
            ESP_LOGI(TAG, "Consumer: received %d", current_value);
            last_value = current_value;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "=== FreeRTOS All-in-One Demo ===");
    
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED1_PIN) | (1ULL << LED2_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&io_conf);
    
    // Create Mutex for shared counter
    shared_counter_mutex = xSemaphoreCreateMutex();
    if (shared_counter_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex!");
    }

    static int led1_id = 1;
    static char led2_name[] = "FastBlinker";
    static int temp_duration = 10;
    
    TaskHandle_t led1_handle = NULL, led2_handle = NULL;
    
    // --- Create all tasks ---

    // Step 1
    xTaskCreate(led1_task, "LED1_Task", 2048, &led1_id, 2, &led1_handle);
    xTaskCreate(led2_task, "LED2_Task", 2048, led2_name, 2, &led2_handle);
    xTaskCreate(system_info_task, "SysInfo_Task", 3072, NULL, 1, NULL);

    // Step 2
    static TaskHandle_t task_handles[2];
    task_handles[0] = led1_handle;
    task_handles[1] = led2_handle;
    xTaskCreate(task_manager, "TaskManager", 3072, task_handles, 3, NULL);

    // Step 3
    xTaskCreate(high_priority_task, "HighPrioTask", 2048, NULL, 4, NULL);
    xTaskCreate(low_priority_task, "LowPrioTask", 2048, NULL, 1, NULL);
    // Note: Runtime stats requires specific sdkconfig settings (e.g. CONFIG_FREERTOS_GENERATE_RUN_TIME_STATS)
    xTaskCreate(runtime_stats_task, "StatsTask", 3072, NULL, 1, NULL);

    // Exercise 1
    xTaskCreate(temporary_task, "TempTask", 2048, &temp_duration, 1, NULL);

    // Exercise 2
    xTaskCreate(producer_task, "Producer", 2048, NULL, 2, NULL);
    xTaskCreate(consumer_task, "Consumer", 2048, NULL, 2, NULL);

    ESP_LOGI(TAG, "All tasks created. Main task will be deleted.");
    
    // The main task is no longer needed.
    vTaskDelete(NULL);
}