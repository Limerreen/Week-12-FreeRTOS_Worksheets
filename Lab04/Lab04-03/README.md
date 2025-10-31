## 📊 การสังเกตและบันทึกผล

### ตารางผลการทดลอง
| ทดลอง | Resources | Producers | Success Rate | Avg Wait | Resource Utilization |
|-------|-----------|-----------|--------------|----------|---------------------|
| 1 (3R, 5P) | 3 | 5 | | | |
| 2 (5R, 5P) | 5 | 5 | | | |
| 3 (3R, 8P) | 3 | 8 | | | |

### คำถามสำหรับการทดลอง
1. เมื่อ Producers มากกว่า Resources จะเกิดอะไรขึ้น?
- จะเกิดการรอคิว (blocking) เพราะมี Producers ที่ยังไม่สามารถขอใช้ resource ได้
     ทำให้บาง task ต้องรอจนกว่า resource ถูกคืนกลับ (xSemaphoreGive)
2. Load Generator มีผลต่อ Success Rate อย่างไร?
 - เมื่อ Load (จำนวน Producers) สูงขึ้น
     ความหนาแน่นในการขอใช้ resource จะมากขึ้น
     ส่งผลให้ Success Rate ลดลงและเวลารอเฉลี่ย (Avg Wait) เพิ่มขึ้น
3. Counting Semaphore จัดการ Resource Pool อย่างไร?
   - Counting Semaphore จะเก็บจำนวนของ resource ที่เหลืออยู่
     ทุกครั้งที่ task ขอใช้ resource จะทำการ take (xSemaphoreTake)
     เมื่อใช้งานเสร็จจะ give (xSemaphoreGive) กลับคืน
     ทำให้ระบบสามารถบริหาร resource หลายตัวได้อย่างปลอดภัยและไม่ชนกัน

## 📋 สรุปผลการทดลอง
- เมื่อจำนวน Producers มากกว่า Resources ระบบจะเกิดการรอ (blocking)
  แต่ไม่เกิดการเสียข้อมูล เพราะ Counting Semaphore จัดการคิวให้อัตโนมัติ
- Load ที่มากขึ้นทำให้ Success Rate ลดลง และเวลารอโดยเฉลี่ยสูงขึ้น
- Counting Semaphore สามารถจัดการ resource pool ได้อย่างมีประสิทธิภาพ
  ป้องกันการเข้าถึงทรัพยากรเกินจำนวนที่มีอยู่จริง