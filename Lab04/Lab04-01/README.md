## 📊 การสังเกตและบันทึกผล

### ตารางบันทึกผล
| ทดลอง | Events Sent | Events Received | Timer Events | Button Presses | Efficiency |
|-------|-------------|-----------------|--------------|----------------|------------|
| 1 (Normal) | | | | | |
| 2 (Multiple Give) | | | | | |
| 3 (Short Timeout) | | | | | |

### คำถามสำหรับการทดลоง
1. เมื่อ give semaphore หลายครั้งติดต่อกัน จะเกิดอะไรขึ้น?
 - ถ้าเป็น Binary Semaphore → ค่าจะเก็บได้แค่ 0 หรือ 1
     การ give หลายครั้งติดกันโดยไม่มีการ take จะไม่สะสมค่า
     (ผลลัพธ์เท่ากับให้เพียงครั้งเดียว)
   - ถ้าเป็น Counting Semaphore → สามารถสะสมจำนวนได้จนถึงค่ามากสุดที่กำหนดไว้
2. ISR สามารถใช้ `xSemaphoreGive` หรือต้องใช้ `xSemaphoreGiveFromISR`?
 - ISR ต้องใช้ฟังก์ชัน xSemaphoreGiveFromISR() เท่านั้น
     เพราะ xSemaphoreGive() ใช้ได้เฉพาะใน task context
     และไม่ปลอดภัยถ้าเรียกใน interrupt

   ตัวอย่าง:
   void MyISR(void)
   {
       BaseType_t xHigherPriorityTaskWoken = pdFALSE;
       xSemaphoreGiveFromISR(xMySemaphore, &xHigherPriorityTaskWoken);
       portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
   }
3. Binary Semaphore แตกต่างจาก Queue อย่างไร?
- Binary Semaphore: ใช้สำหรับการซิงโครไนซ์ (signal ระหว่าง task หรือ ISR ↔ task)
     เก็บสถานะได้เพียง 0 หรือ 1 ไม่เก็บข้อมูล
   - Queue: ใช้สำหรับส่งข้อมูลระหว่าง tasks
     สามารถเก็บข้อมูลหลายช่อง (ตามที่กำหนดขนาดตอนสร้าง)

## 📋 สรุปผลการทดลอง