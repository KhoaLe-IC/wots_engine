# RTL Module Hierarchy & Guidelines

Thư mục này chứa mã nguồn phần cứng (RTL) được viết bằng **SystemVerilog (IEEE 1800-2017)** cho khối **WOTS+ Accelerator**.

---

## 1. Phân rã Module (RTL Decomposition)

Theo mục 5.1 của [Kế hoạch chính thức](../docs/KE_HOACH_CHINH_THUC_SOC_FIPS205.md) và [Đặc tả kỹ thuật v0.1](../docs/WOTS_ACCELERATOR_SPEC.md):

```text
wots_accel_top.sv                   # Top-level IP Core tích hợp SoC
├── wots_csr.sv                     # Giao diện thanh ghi điều khiển & trạng thái (CSR)
├── dma_descriptor_frontend.sv      # Đọc descriptor, quản lý queue lệnh và điều phối DMA
├── input_fifo.sv / output_fifo.sv  # Bộ đệm dữ liệu đầu vào / đầu ra
├── base_w_checksum.sv              # Chuyển đổi message sang base-16 và tính checksum 35 digits
├── adrs_manager.sv                 # Quản lý và tự động tăng địa chỉ ADRS 32-byte
├── chain_job_generator.sv          # Sinh 35 công việc chain độc lập cho từng lần WOTS
├── dynamic_chain_scheduler.sv      # Bộ lập lịch động phân phối chain cho các lane rảnh
├── chain_lane.sv [NUM_LANES = 2]   # Khối thực thi chuỗi Winternitz (PRF và F chain steps)
├── hash_backend_adapter.sv         # Adapter định dạng dữ liệu FIPS 205 nối với IP SHAKE256
├── reorder_buffer.sv               # Gom và sắp xếp lại 35 kết quả chain theo đúng thứ tự
├── tl_stream_controller.sv         # Điều khiển nén T_l (hấp thụ 560 byte để sinh 16-byte PK)
├── output_packer.sv                # Đóng gói vector kết quả để DMA ghi về RAM ngoài
└── perf_counters.sv                # Các bộ đếm chu kỳ hiệu năng (tổng, bận lane, stall DMA/Hash)
```

---

## 2. Quy ước Thiết kế (Design Conventions)

1. **Ngôn ngữ:** SystemVerilog (`.sv`), tuân thủ chuẩn tổng hợp được (Synthesizable RTL).
2. **Clock & Reset:**
   * Một miền xung nhịp chính duy nhất (`clk`).
   * Reset đồng bộ hoặc bất đồng bộ tích cực mức thấp (`rst_n`), thống nhất theo nền tảng FPGA đích.
3. **Giao tiếp Dòng dữ liệu (Streaming Interface):**
   * Sử dụng chuẩn bắt tay **`valid` / `ready`** hai chiều tiêu chuẩn (chuẩn AXI4-Stream / Ready-Valid).
   * Không được thay đổi dữ liệu trên bus khi `valid == 1` mà `ready == 0`.
4. **Tham số hóa:**
   * Tất cả các hằng số thuật toán ($n=16, w=16, len=35$) và số lượng lane (`NUM_LANES`) được định nghĩa trong package hoặc parameter ở top-level, không hardcode giá trị số trong logic nội bộ.
