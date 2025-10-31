## 📊 การสังเกตและบันทึกผล

### ตารางผลการทดลอง
| ทดลอง | Successful | Failed | Corrupted | Success Rate | สังเกต |
|-------|------------|--------|-----------|-------------|---------|
| 1 (With Mutex) | | | | | |
| 2 (No Mutex) | | | | | |
| 3 (Changed Priority) | | | | | |

### คำถามสำหรับการทดลอง
1. เมื่อไม่ใช้ Mutex จะเกิด data corruption หรือไม่?
- ใช่ เกิด data corruption ได้
     เพราะหลาย task เข้าถึงและแก้ไข shared resource พร้อมกัน
     โดยไม่มีการป้องกัน (critical section ไม่ถูกล็อก)
2. Priority Inheritance ทำงานอย่างไร?
 - เมื่อ task ที่มี priority ต่ำถือ Mutex อยู่
     แล้วมี task ที่ priority สูงมารอ Mutex เดียวกัน
     ระบบจะ “ยก priority” ของ task ที่ถือ Mutex
     ให้เท่ากับของ task ที่สูงกว่า
     เพื่อป้องกัน priority inversion
     และคืนค่า priority เดิมเมื่อปล่อย Mutex
3. Task priority มีผลต่อการเข้าถึง shared resource อย่างไร?
- Task ที่มี priority สูงจะได้สิทธิ์เข้าทำงานก่อน
     แต่หากใช้ Mutex จะต้องรอจน task ที่ถือ Mutex อยู่ปล่อยก่อน
     เพื่อให้การเข้าถึง shared resource ปลอดภัยและเป็นลำดับ

## 📋 สรุปผลการทดลอง
- การใช้ Mutex ช่วยป้องกันการเกิด data corruption ได้อย่างมีประสิทธิภาพ
- Priority Inheritance ช่วยแก้ปัญหา priority inversion
  ทำให้ระบบยังคงตอบสนองได้ดีแม้มี task หลายระดับ priority
- การไม่ใช้ Mutex มีโอกาสเกิดข้อมูลเสียหาย (corrupted data)
  จากการเข้าถึงพร้อมกันของหลาย task