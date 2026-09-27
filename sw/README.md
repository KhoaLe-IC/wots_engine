# Software Driver & Firmware Architecture

Thư mục này chứa mã nguồn phần mềm, driver điều khiển và các tệp định nghĩa giao tiếp giữa CPU và **WOTS+ Accelerator**.

---

## 1. Cấu trúc Thư mục

- `include/wots_regs.h`: Tệp header C chuẩn định nghĩa bản đồ thanh ghi CSR (Register Map), các bitmask điều khiển/trạng thái và cấu trúc DMA Descriptor 32-byte.
- `driver/`: Driver C cho hệ điều hành nhúng (Linux / FreeRTOS) hoặc baremetal firmware trên CPU (Nios II, ARM Cortex-A, hoặc RISC-V).

---

## 2. Quy trình Giao tiếp HW/SW qua DMA

1. **Chuẩn bị Dữ liệu:** Firmware cấp phát vùng nhớ trong RAM ngoài chứa:
   * Seed (`SK.seed`, `PK.seed`) và `ADRS` (32 bytes).
   * Message (16 bytes) nếu ở chế độ `WOTS_SIGN`.
   * Signature buffer (560 bytes) nếu ở chế độ `WOTS_PK_FROM_SIG`.
2. **Cấu hình Descriptor:** Ghi descriptor vào mảng descriptor ring trong RAM ngoài.
3. **Kích hoạt Accelerator:**
   * Ghi địa chỉ descriptor vào thanh ghi `WOTS_REG_DESC_BASE`.
   * Ghi số lượng job vào `WOTS_REG_JOB_COUNT`.
   * Ghi `WOTS_CTRL_START_MASK` vào `WOTS_REG_CONTROL`.
4. **Xử lý Hoàn tất:** Đợi ngắt `IRQ_DONE` hoặc thăm dò cờ `DONE` trong `WOTS_REG_STATUS`, đọc các bộ đếm hiệu năng để đánh giá tốc độ.
