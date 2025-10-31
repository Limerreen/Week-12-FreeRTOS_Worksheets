#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define LED_HIGH_PIN GPIO_NUM_2    // สำหรับ High Priority Task
#define LED_MED_PIN GPIO_NUM_4     // สำหรับ Medium Priority Task
#define LED_LOW_PIN GPIO_NUM_5     // สำหรับ Low Priority Task
#define BUTTON_PIN GPIO_NUM_0      // Trigger button

static const char *TAG = "PRIORITY_DEMO";

// --- Global variables ---
volatile bool priority_test_running = false;

// --- Test 1: Different Priorities ---
volatile uint32_t high_task_count = 0;
volatile uint32_t med_task_count = 0;
volatile uint32_t low_task_count = 0;

void high_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "High Priority Task started (Priority 5)");
    while (1) {
        if (priority_test_running) {
            high_task_count++;
            ESP_LOGI(TAG, "HIGH PRIORITY RUNNING (%d)", high_task_count);
            gpio_set_level(LED_HIGH_PIN, 1);
            for (int i = 0; i < 100000; i++) { volatile int dummy = i * 2; }
            gpio_set_level(LED_HIGH_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(200));
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void medium_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Medium Priority Task started (Priority 3)");
    while (1) {
        if (priority_test_running) {
            med_task_count++;
            ESP_LOGI(TAG, "Medium priority running (%d)", med_task_count);
            gpio_set_level(LED_MED_PIN, 1);
            for (int i = 0; i < 200000; i++) { volatile int dummy = i + 100; }
            gpio_set_level(LED_MED_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(300));
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void low_priority_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Low Priority Task started (Priority 1)");
    while (1) {
        if (priority_test_running) {
            low_task_count++;
            ESP_LOGI(TAG, "Low priority running (%d)", low_task_count);
            gpio_set_level(LED_LOW_PIN, 1);
            for (int i = 0; i < 500000; i++) {
                volatile int dummy = i - 50;
                if (i % 100000 == 0) { vTaskDelay(1); }
            }
            gpio_set_level(LED_LOW_PIN, 0);
            vTaskDelay(pdMS_TO_TICKS(500));
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void control_task(void *pvParameters)
{
    ESP_LOGI(TAG, "Control Task started");
    while (1) {
        if (gpio_get_level(BUTTON_PIN) == 0) {
            if (!priority_test_running) {
                ESP_LOGW(TAG, "=== STARTING PRIORITY TEST ===");
                high_task_count = 0;
                med_task_count = 0;
                low_task_count = 0;
                priority_test_running = true;
                
                vTaskDelay(pdMS_TO_TICKS(10000)); // Run test for 10 seconds
                
                priority_test_running = false;
                
                ESP_LOGW(TAG, "=== PRIORITY TEST RESULTS ===");
                ESP_LOGI(TAG, "High Priority Task runs: %d", high_task_count);
                ESP_LOGI(TAG, "Medium Priority Task runs: %d", med_task_count);
                ESP_LOGI(TAG, "Low Priority Task runs: %d", low_task_count);
                
                uint32_t total_runs = high_task_count + med_task_count + low_task_count;
                if (total_runs > 0) {
                    ESP_LOGI(TAG, "High priority percentage: %.1f%%", (float)high_task_count / total_runs * 100);
                    ESP_LOGI(TAG, "Medium priority percentage: %.1f%%", (float)med_task_count / total_runs * 100);
                    ESP_LOGI(TAG, "Low priority percentage: %.1f%%", (float)low_task_count / total_runs * 100);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// --- Test 2: Equal Priorities (Round-Robin) ---
void equal_priority_task(void *pvParameters)
{
    int task_id = (int)pvParameters;
    while (1) {
        ESP_LOGI(TAG, "Equal Priority Task %d running", task_id);
        // Simulate work without yielding
        for (int i = 0; i < 300000; i++) {
            volatile int dummy = i;
        }
        // Since tasks have the same priority and a blocking delay,
        // FreeRTOS will switch to the next ready task of the same priority (Round-Robin).
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// --- Test 3: Priority Inversion ---
volatile bool shared_resource_busy = false;

void priority_inversion_high(void *pvParameters)
{
    while (1) {
        ESP_LOGW(TAG, "High-Prio (5) wants the resource.");
        // Wait for the resource
        while (shared_resource_busy) {
            ESP_LOGE(TAG, "High-Prio (5) BLOCKED by Low-Prio!");
            vTaskDelay(pdMS_TO_TICKS(10)); // Block and wait
        }
        ESP_LOGI(TAG, "High-Prio (5) GOT the resource!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void priority_inversion_medium(void *pvParameters)
{
    while(1) {
        ESP_LOGI(TAG, "Medium-Prio (3) is running, preventing High-Prio from running.");
        for (int i = 0; i < 400000; i++) { volatile int dummy = i; }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void priority_inversion_low(void *pvParameters)
{
    while (1) {
        ESP_LOGI(TAG, "Low-Prio (1) takes the resource.");
        shared_resource_busy = true;
        
        // Simulate using the resource for a long time
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        shared_resource_busy = false;
        ESP_LOGI(TAG, "Low-Prio (1) released the resource.");
        
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}


void app_main(void)
{
    // --- GPIO Configuration ---
    gpio_config_t led_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_HIGH_PIN) | (1ULL << LED_MED_PIN) | (1ULL << LED_LOW_PIN),
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&led_conf);

    gpio_config_t button_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << BUTTON_PIN,
        .pull_up_en = 1,
    };
    gpio_config(&button_conf);

    ESP_LOGI(TAG, "=== FreeRTOS Scheduling Demo ===");
    ESP_LOGI(TAG, "Choose ONE test scenario in app_main and re-flash.");

    // --- SCENARIO 1: DIFFERENT PRIORITIES ---
    // Press button to start a 10-second test.
    ESP_LOGI(TAG, "Creating tasks for DIFFERENT priority test...");
    xTaskCreate(high_priority_task, "HighPrio", 3072, NULL, 5, NULL);
    xTaskCreate(medium_priority_task, "MedPrio", 3072, NULL, 3, NULL);
    xTaskCreate(low_priority_task, "LowPrio", 3072, NULL, 1, NULL);
    xTaskCreate(control_task, "Control", 3072, NULL, 4, NULL);


    // --- SCENARIO 2: EQUAL PRIORITIES (ROUND-ROBIN) ---
    // Uncomment this block and comment out SCENARIO 1 to test.
    /*
    ESP_LOGI(TAG, "Creating tasks for EQUAL priority test...");
    xTaskCreate(equal_priority_task, "Equal1", 2048, (void*)1, 2, NULL);
    xTaskCreate(equal_priority_task, "Equal2", 2048, (void*)2, 2, NULL);
    xTaskCreate(equal_priority_task, "Equal3", 2048, (void*)3, 2, NULL);
    */

    // --- SCENARIO 3: PRIORITY INVERSION ---
    // Uncomment this block and comment out others to test.
    /*
    ESP_LOGI(TAG, "Creating tasks for PRIORITY INVERSION test...");
    xTaskCreate(priority_inversion_low, "InvLow", 2048, NULL, 1, NULL);
    xTaskCreate(priority_inversion_medium, "InvMed", 2048, NULL, 3, NULL);
    xTaskCreate(priority_inversion_high, "InvHigh", 2048, NULL, 5, NULL);
    */
}
